/* Copyright 2012-2020 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


/* #include <stdio.h> */
#include <string.h>
#include "debug_tool/debug_tool.h"
#include "midi/midifile.h"
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void free_midifile_track(midifile_track_t *mtrack)
{
  if (mtrack)
    {
      free_list_node(&(mtrack->track.tickev_list), free_tickev);
      if (mtrack->track.name != NULL)
        free(mtrack->track.name);
      free(mtrack);
    }
}

void _free_midifile_track(void *addr)
{
  free_midifile_track((midifile_track_t *) addr);
}

void free_mfinfo_port(void *addr)
{
  midifile_portinfo_t *portinfo = (midifile_portinfo_t *) addr;
  free(portinfo->name);
  free(portinfo);
}

void free_midifile(midifile_t *midifile)
{
  if (midifile)
    {
      free_list_node(&(midifile->track_list), _free_midifile_track);
      free_list_node(&(midifile->info.portinfo_list), free_mfinfo_port);
      free(midifile);
    }
}

size_t get_midifile_track_size(byte_t *buffer)
{
  size_t track_size = 0;

  if (strncmp((char *) buffer, "MTrk", 4))
    return 0;
  buffer += 4;

  track_size = buffer[0];
  track_size = (track_size << 8) + buffer[1];
  track_size = (track_size << 8) + buffer[2];
  track_size = (track_size << 8) + buffer[3];

  //  eventlist = get_midi_events(buffer);
  return track_size;
}

void get_sysex_portname(list_t *port_list, byte_t *buf)
{
  size_t len;
  midifile_portinfo_t *portinfo = myalloc(sizeof (midifile_portinfo_t));

  portinfo->id = ((buf[0] << 24 & 0xFF)
                  + (buf[1] << 16 & 0xFF)
                  + (buf[2] << 8 & 0xFF)
                  + (buf[3] & 0xFF));
  len = (buf[4] << 8) + buf[5];
  portinfo->name = myalloc(len + 1);

  strncpy(portinfo->name, (char *) &(buf[6]), len);
  portinfo->name[len] = '\0';
  push_to_list_tail(port_list, portinfo);
}

msq_bool_t get_mlp_sysex_file_version(byte_t *buffer,
                                      size_t size,
                                      u_int32_t *version)
{
  if (buffer[0] == 0
      && buffer[1] == 'M'
      && buffer[2] == 'L'
      && buffer[3] == 'P')
    if (buffer[4] == MLP_SYSEX_FILE_VERSION)
      {
        *version =  buffer[5] << 24;
        *version += buffer[6] << 16;
        *version += buffer[7] << 8;
        *version += buffer[8];
        return MSQ_TRUE;
      }
  return MSQ_FALSE;
}

void get_mlp_sysex(midifile_info_t *info,
                   midifile_track_t *track,
                   byte_t *buffer,
                   size_t size)
{
  if (buffer[0] == 0
      && buffer[1] == 'M'
      && buffer[2] == 'L'
      && buffer[3] == 'P')
    {
      info->is_msq = MSQ_TRUE;
      switch (buffer[4])
        {
        case MLP_SYSEX_ENGINE_TYPE:
          info->engine_type =  buffer[5] << 24;
          info->engine_type += buffer[6] << 16;
          info->engine_type += buffer[7] << 8;
          info->engine_type += buffer[8];
          break;
        case MLP_SYSEX_TRACK_LOOPSTART:
          track->sysex_loop_start =  buffer[5] << 24;
          track->sysex_loop_start += buffer[6] << 16;
          track->sysex_loop_start += buffer[7] << 8;
          track->sysex_loop_start += buffer[8];
          break;
        case MLP_SYSEX_TRACK_LOOPLEN:
          track->sysex_loop_len =  buffer[5] << 24;
          track->sysex_loop_len += buffer[6] << 16;
          track->sysex_loop_len += buffer[7] << 8;
          track->sysex_loop_len += buffer[8];
          break;
        case MLP_SYSEX_PORTNAME:
          get_sysex_portname(&(info->portinfo_list), &(buffer[5]));
          break;
        case MLP_SYSEX_TRACK_PORTID:
          track->sysex_portid =  buffer[5] << 24;
          track->sysex_portid += buffer[6] << 16;
          track->sysex_portid += buffer[7] << 8;
          track->sysex_portid += buffer[8];
          break;
        case MLP_SYSEX_MUTE_TRACK_BINDING_KEYPRESS:
          track->mute_bindings.keys_sz = (size_t) buffer[5];
          memcpy(track->mute_bindings.keys,
                 &(buffer[6]),
                 track->mute_bindings.keys_sz);
          break;
        case MLP_SYSEX_MUTE_TRACK_BINDING_NOTEPRESS:
          track->mute_bindings.notes_sz = (size_t) buffer[5];
          memcpy(track->mute_bindings.notes,
                 &(buffer[6]),
                 track->mute_bindings.notes_sz);
          break;
        case MLP_SYSEX_MUTE_TRACK_BINDING_PROGPRESS:
          track->mute_bindings.programs_sz = (size_t) buffer[5];
          memcpy(track->mute_bindings.programs,
                 &(buffer[6]),
                 track->mute_bindings.programs_sz);
          break;
        case MLP_SYSEX_MUTE_TRACK_BINDING_CTRLCHG:
          track->mute_bindings.controls_sz = (size_t) buffer[5];
          memcpy(track->mute_bindings.controls,
                 &(buffer[6]),
                 track->mute_bindings.controls_sz);
          break;
        case MLP_SYSEX_REC_TRACK_BINDING_KEYPRESS:
          track->rec_bindings.keys_sz = (size_t) buffer[5];
          memcpy(track->rec_bindings.keys,
                 &(buffer[6]),
                 track->rec_bindings.keys_sz);
          break;
        case MLP_SYSEX_REC_TRACK_BINDING_NOTEPRESS:
          track->rec_bindings.notes_sz = (size_t) buffer[5];
          memcpy(track->rec_bindings.notes,
                 &(buffer[6]),
                 track->rec_bindings.notes_sz);
          break;
        case MLP_SYSEX_REC_TRACK_BINDING_PROGPRESS:
          track->rec_bindings.programs_sz = (size_t) buffer[5];
          memcpy(track->rec_bindings.programs,
                 &(buffer[6]),
                 track->rec_bindings.programs_sz);
          break;
        case MLP_SYSEX_REC_TRACK_BINDING_CTRLCHG:
          track->rec_bindings.controls_sz = (size_t) buffer[5];
          memcpy(track->rec_bindings.controls,
                 &(buffer[6]),
                 track->rec_bindings.controls_sz);
          break;
        case MLP_SYSEX_TRACK_MUTED:
          track->sysex_muted = MSQ_TRUE;
          break;
        default:
          output_error("Unexpected SYSEX 0x%X", buffer[4]);
        }
    }
}

msq_bool_t filter_add_midicev_to_track(track_t *track,
                                       uint_t tick,
                                       midicev_t *mcev,
                                       list_t *noteon_list)
{
  msq_mcev_tick_t *noteon_ret;

  switch (mcev->type)
    {
    case MSQ_MIDI_NOTEON:
      if (mcev->event.note.val != 0)
        {
          msq_mcev_tick_list_add(noteon_list, tick, mcev);
          return MSQ_TRUE;
        }
      mcev->type = MSQ_MIDI_NOTEOFF;
    case MSQ_MIDI_NOTEOFF:
      noteon_ret = _msq_mcev_tick_list_pop_prev_noteon(noteon_list,
                                                       tick,
                                                       mcev->chan,
                                                       mcev->event.note.num);
      if (noteon_ret != NULL)
        {
          copy_midicev_to_track(track, noteon_ret->tick, &(noteon_ret->mcev));
          copy_midicev_to_track(track, tick, mcev);
          free(noteon_ret);
          return MSQ_TRUE;
        }
      return MSQ_FALSE;
    case MSQ_MIDI_CONTROLCHANGE:
    case MSQ_MIDI_PITCHWHEELCHANGE:
      copy_midicev_to_track(track, tick, mcev);
      return MSQ_TRUE;
      break;
    }
  return MSQ_FALSE;
}

/* Get midifile track information, and add
   midi channel event to track to the track structure */
msq_bool_t get_midifile_track(midifile_info_t *info,
                              midifile_track_t **addr,
                              byte_t *buffer,
                              size_t size)
{
  midifile_track_t *midifile_track = myalloc(sizeof (midifile_track_t));
  byte_t           *end = NULL;
  uint_t           tick;//, smallest = (uint_t) -1, biggest = 0;
  uint_t           offset;
  u_int32_t        version;
  midicev_t        chan_ev;
  midimev_t        meta_ev;
  byte_t           status_byte = 0;
  msq_bool_t       tempo_set = MSQ_FALSE;
  static list_t noteon_list = {};

  midifile_track->sysex_portid = -1;
  debug_midi("!!! start=%p end=%p\n", buffer, &(buffer[size]));

  debug_midi("---------------------------\n");
#ifdef DEBUG_MIDI_MODE
  print_bin(stdout, buffer, 6);
#endif

  /* Gros travail dark a faire.
     Peut etre une API de callback sur les evenements. */

  if (size > 0)
    for (end = &(buffer[size]),
           tick = get_varlen_from_ptr(&buffer),
           meta_ev.type = 1;
         buffer < end && meta_ev.type != ME_ENDOFTRACK;
         tick += get_varlen_from_ptr(&buffer))
      {
#ifdef DEBUG_MIDI_MODE
	print_bin(stdout, buffer, 16);
#endif
        debug_midi("\033[32m> buffer addr: %p=%u\033[0m\n", buffer, buffer);
        if (*buffer & 0x80)
          {
            switch (*buffer)
              {
              case 0xFF:
                buffer++;
                offset = get_midi_meta_event(&meta_ev, buffer); /* retour donne le type et loffset */
                if (0 == offset)
                  {
                    msq_mcev_tick_list_clear(&noteon_list);
                    return MSQ_FALSE;
                  }
                debug_midi("Meta event %s detected %u\n", midime_to_str(meta_ev.type), meta_ev.type);
                switch (meta_ev.type)
                  {
                  case ME_NAME:
                    if (midifile_track->track.name)
                      output_warning("Found two meta event name for this track (keeping the first one \"%s\")\n",
                                     midifile_track->track.name);
                    else
                      midifile_track->track.name = strdup((const char *) meta_ev.data);
                    debug_midi("Found name: >%s<\n", meta_ev.data);
                    break;
                  case ME_COPYRIGHTNOTICE:
                    debug_midi("Found copyright: >%s<\n", meta_ev.data);
                    break;
                  case ME_CUEPOINT:
                    debug_midi("Found cuepoint: >%s<\n", meta_ev.data);
                    break;
                  case ME_SETTEMPO:
                    info->tempo = meta_ev.val;
                    tempo_set = MSQ_TRUE;
                    break;
                  }
                status_byte = 0;
                break;

              case 0xF0:
                buffer++;
                offset = get_varlen_from_ptr(&buffer); /* /!\ */
                if (info->is_msq == MSQ_TRUE)
                  get_mlp_sysex(info, midifile_track, buffer, offset);
                else
                  if (get_mlp_sysex_file_version(buffer,
                                                 offset,
                                                 &version) == MSQ_TRUE
                      && version == 1)
                    info->is_msq = MSQ_TRUE;
                status_byte = 0;
                break;

              case 0xF7:
                debug_midi("SysEx event detected\n");
                debug_midi("\nUnsuported System Exclusive Event type: 0x%02X\n", *buffer);
                buffer++;
                offset = get_varlen_from_ptr(&buffer); /* /!\ */
                status_byte = 0;
                break;

              default:
                bzero(&chan_ev, sizeof (midicev_t));
                offset = get_midi_channel_event(&chan_ev, buffer); /* cmd = *buffer >> 4; */
                if (0 == offset)
                  {
                    output_error("Problem while searching channel event (type=0x%02X)", *buffer);
                    free_midifile_track(midifile_track);
                    msq_mcev_tick_list_clear(&noteon_list);
                    return MSQ_FALSE;
                  }
                if (chan_ev.type >= 0x8 && chan_ev.type <= 0xE)
                  {
                    status_byte = (chan_ev.type << 4) + chan_ev.chan;
                    debug_midi("Add midi channel event to track\n");
                    filter_add_midicev_to_track(&(midifile_track->track),
                                                tick,
                                                &chan_ev,
                                                &noteon_list);
                  }
              }
          }
        else if (status_byte != 0)
          {
            debug_midi("Using midi running status (0x%X).\n", status_byte);
            buffer--;
            *buffer = status_byte;
            bzero(&chan_ev, sizeof (midicev_t));
            offset = get_midi_channel_event(&chan_ev, buffer); /* cmd = *buffer >> 4; */
            if (0 == offset)
              {
                output_error("Problem while searching channel event (type=0x%02X)", *buffer);
                free_midifile_track(midifile_track);
                msq_mcev_tick_list_clear(&noteon_list);
                return MSQ_FALSE;
              }
            if (chan_ev.type >= 0x8 && chan_ev.type <= 0xE)
              {
                debug_midi("Add midi channel event to track\n");
                filter_add_midicev_to_track(&(midifile_track->track),
                                            tick,
                                            &chan_ev,
                                            &noteon_list);
              }
          }
        else
          {
            output_error("Problem with event type: 0x%02X", *buffer);
            msq_mcev_tick_list_clear(&noteon_list);
            return MSQ_FALSE;
            /* output_warning("Skipping event(s) and searching for next command"); */
            /* for (offset = 0; */
            /*      (buffer + offset) < end; */
            /*      offset++) */
            /*   if (buffer[offset] & 0x80) */
            /*     break; */
            /* if ((buffer + offset) < end) */
            /*   { */
            /*     output_warning("New command found (continue midifile read)"); */
            /*   } */
            /* else */
            /*   { */
            /*     output_error("No command found in track"); */
            /*     return MSQ_FALSE; */
            /*   } */
          }
        buffer += offset;
        debug_midi("buffer addr: %p ---------------------------\n", buffer);
      }

  if (buffer != end)
    output_error("Unexpected size of track buffer=%p end=%p\n", buffer, end);
  if (LIST_HEAD(&(midifile_track->track.tickev_list)) != NULL || tempo_set == MSQ_FALSE)
    {
      debug_midi(">> channel name \"%s\" number of event = %i\n", midifile_track->track.name, midifile_track->track.tickev_list.len);
      *addr = midifile_track;
    }
  else
    {
      debug_midi("No tick event found\n");
      free_midifile_track(midifile_track);
    }
  msq_mcev_tick_list_clear(&noteon_list);
  return MSQ_TRUE;
}

#define BUFFER_DEFAULT_SIZE 524288

midifile_t *get_midifile_tracks(int fd,
                                midifile_hdr_chunk_t *midifile_hdr)
{
  size_t           size = 0;
  size_t           bsize = BUFFER_DEFAULT_SIZE;
  byte_t           *buffer = NULL;
  unsigned int     idx;
  midifile_t       *midifile = NULL;
  list_t           track_list;
  midifile_track_t *midifile_track = NULL;
  midifile_info_t  info = {.type = MULTITRACK_MIDIFILE_USYNC,
                           .tempo = 500000,
                           .ppq = 192,
                           .portinfo_list = {},
                           .is_msq = MSQ_FALSE};

  bzero(&track_list, sizeof (list_t));
  info.type = midifile_hdr->format_type;
  if (midifile_hdr->time_division.flag == TICK_PER_BEAT)
    info.ppq = midifile_hdr->time_division.value.tick_per_beat;
  else
    {
      output_error("Unsupported timedivision type FRAME_PER_SEC (smpte_frames=%d ticks_per_frame=%d)\n",
                  midifile_hdr->time_division.value.frame_per_sec.smpte_frames,
                  midifile_hdr->time_division.value.frame_per_sec.ticks_per_frame);
      return NULL;
    }

  for (buffer = myalloc(bsize),
         bzero(buffer, bsize),
         idx = 0;
       idx < midifile_hdr->number_of_track;
       idx++)
    {
      debug_midi("\033[33m\n### Reading track %i ###\033[0m\n", idx);
      size = read(fd, buffer, 8);
#ifdef DEBUG_MIDI_MODE
      print_bin(stdout, buffer, 8);
#endif
      size = get_midifile_track_size(buffer);
      if (0 != strncmp("MTrk", (const char *) buffer, 4))
        {
          output_error("!!! At track index %i encounter an unknown midi chunk %c%c%c%c\n", idx,
                      buffer[0], buffer[1], buffer[2], buffer[3]);
          free(buffer);
          return NULL;
        }
      if (size == 0)
        {
          output_error("track_%i Error while getting track size (size == 0)\n", idx);
          free(buffer);
          return NULL;
        }
      debug_midi("size of track %i = %d\n", idx, size);
      if (size > bsize)
        {
          bsize = size * 2;
          free(buffer);
          buffer = myalloc(bsize);
          //buffer = realloc(buffer, bsize);
        }

      bzero(buffer, bsize);
      size = read(fd, buffer, size);
      debug_midi("size read = %d\n", size);
      if (MSQ_FALSE == get_midifile_track(&info, &(midifile_track), buffer, size))
        {
          output_error("Problem with track %i\n", idx);
          free(buffer);
          return NULL;
        }
      else
        debug_midi("track_%i ok\n", idx);

      if (midifile_track != NULL)
        {
          push_to_list_tail(&track_list, (void *) midifile_track);
          midifile_track = NULL;
        }
    }
  free(buffer);

  midifile = myalloc(sizeof (midifile_t));
  /* midifile->type = midifile_hdr->format_type; */
  if (LIST_HEAD(&track_list) != NULL)
    {
      midifile->track_list.head = track_list.head;
      midifile->track_list.tail = track_list.tail;
      midifile->track_list.len = track_list.len;
    }
  /* midifile->number_of_track = midifile_hdr->number_of_track; */
  bcopy(&info, &midifile->info, sizeof (midifile_info_t));
  return midifile;
}

msq_bool_t get_midifile_hdr(midifile_hdr_chunk_t *mdhdr, void *ptr)
{
  char		*str = ptr;
  unsigned char	*buffer = ptr;
  /* size_t	size = 0; */

  if (strncmp(str, "MThd", 4))
    return MSQ_FALSE;
  buffer += 4;
  /* size = buffer[0]; size += (size << 8) + buffer[1]; */
  /* copy_to_2B(size, buffer); */
  if (buffer[3] != 6)
    return MSQ_FALSE;
  buffer += 4;
  mdhdr->format_type = buffer[2];
  buffer += 2;
  mdhdr->number_of_track = buffer[0];
  mdhdr->number_of_track = (mdhdr->number_of_track << 8) + buffer[1];
  buffer += 2;

  if (0x8000 & buffer[0])
    {
      mdhdr->time_division.flag = MSQ_TRUE;
      mdhdr->time_division.value.frame_per_sec.smpte_frames = buffer[0];
      mdhdr->time_division.value.frame_per_sec.ticks_per_frame = buffer[1];
    }
  else
    {
      mdhdr->time_division.flag = MSQ_FALSE;
      mdhdr->time_division.value.tick_per_beat = (buffer[0] << 8) + buffer[1];
    }
  return MSQ_TRUE;
}

void output_midifile_hdr(midifile_hdr_chunk_t *midifile_hdr)
{
  output("Format type     : %s (type=%d)\n"
         "Number of track : %d\n",
         midifile_hdr->format_type == 0 ? "Single track"
         : midifile_hdr->format_type == 1 ? "Multiple tracks, synchronous"
         : midifile_hdr->format_type == 2 ? "Multiple tracks, asynchronous"
         : "Unknown midifile format type",
         midifile_hdr->format_type,
         midifile_hdr->number_of_track);
  if (midifile_hdr->time_division.flag)
    output("Time division: %d frames per second, %d ticks\n",
           midifile_hdr->time_division.value.frame_per_sec.smpte_frames,
           midifile_hdr->time_division.value.frame_per_sec.ticks_per_frame);
  else
    output("Time division: %d ticks per beat\n",
           midifile_hdr->time_division.value.tick_per_beat);
}

midifile_t *read_midifile(const char *path)
{
  byte_t               buffer[14];
  midifile_hdr_chunk_t midifile_hdr;
  size_t               size;
  int                  midifile_fd;
  midifile_t           *midifile;

  midifile_fd = open(path, O_RDONLY);
  if (midifile_fd == -1)
    {
      output_error("Unable to open filename: %s", path);
      return NULL;
    }
  //  bzero(buffer, 14);
  size = read(midifile_fd, buffer, 14);
  if (size != 14)
    {
      output_error("Problem while reading file header\n");
      close(midifile_fd);
      return NULL;
    }

#ifdef DEBUG_MIDI_MODE
  print_bin(stdout, buffer, 14);
#endif

  if (get_midifile_hdr(&midifile_hdr, buffer) == MSQ_FALSE)
    {
      output_error("Wrong midi file header\n");
      close(midifile_fd);
      return NULL;
    }

  /* output_midifile_hdr(&midifile_hdr); */

  midifile = get_midifile_tracks(midifile_fd, &midifile_hdr);

  close(midifile_fd);

  return midifile;
}
