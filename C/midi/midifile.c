/* #include <stdio.h> */
#include <string.h>
#include "debug_tool/debug_tool.h"
#include "midi/midifile.h"
#include <stdlib.h>
#include <unistd.h>

void free_midifile(midifile_t *midifile)
{
  if (midifile)
    {
      free_list_node(&(midifile->track_list), free_track);
      free(midifile);
    }
}

size_t		get_midifile_track_size(byte_t *buffer)
{
  size_t	track_size = 0;

  if (strncmp((char *) buffer, "MTrk", 4))
    return 0;
  buffer += 4;

  track_size = (track_size << 8) + buffer[0];
  track_size = (track_size << 8) + buffer[1];
  track_size = (track_size << 8) + buffer[2];
  track_size = (track_size << 8) + buffer[3];

  //  eventlist = get_midi_events(buffer);
  return track_size;
}

/* Get midifile track information, and add
   midi channel event to track to the track structure */
bool_t          get_midifile_track(midifile_info_t *info,
                                   track_t **addr,
                                   byte_t *buffer,
                                   size_t size)
{
  track_t *track = myalloc(sizeof (track_t));
  byte_t        *end;
  /* byte_t        cmd; */
  uint_t        tick;//, smallest = (uint_t) -1, biggest = 0;
  uint_t        offset;
  /* miditickev_node_t     *cur_miditickev = NULL; */
  midicev_t             chan_ev;
  midimev_t             meta_ev;

  bzero(track, sizeof (track_t));
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
        debug_midi("\033[32m> buffer addr: %1$p=%1$u\033[0m\n", buffer);
        /* debug_midi("> deltatime: %d\n", tick); */
	/* if (tick < smallest) */
	/*   smallest = tick; */
	/* if (tick > biggest) */
	/*   biggest = tick; */
        switch (*buffer)
          {
          case 0xFF:
            offset = get_midi_meta_event(&meta_ev, buffer); /* retour donne le type et loffset */
            if (0 == offset)
              return FALSE;
            debug_midi("Meta event %s detected %u\n", midime_to_str(meta_ev.type), meta_ev.type);
            switch (meta_ev.type)
              {
              case ME_NAME:
                if (track->name)
                  output_warning("Found two meta event name for this track (keeping the first one \"%s\")\n",
                                track->name);
                else
                  track->name = strdup((const char *) meta_ev.data);
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
                break;
              }
            break;

          case 0xF0:
          case 0xF7:
            debug_midi("SysEx event detected\n");
            output_error("\nUnsuported System Exclusive Event type: 0x%02X\n", *buffer);
            buffer++;
            offset = get_varlen_from_ptr(&buffer); /* /!\ */
            //return FALSE;
            break;

          default:
            bzero(&chan_ev, sizeof (midicev_t));
            offset = get_midi_channel_event(&chan_ev, buffer); /* cmd = *buffer >> 4; */
            if (0 == offset)
              {
                output_error("\nProblem with event type: 0x%02X\n", *buffer);
                free_track(track);
                return FALSE;
              }
            /* debug_midi("got midi channel event cmd %s number %i, on channel \"%s\" and tick %i\n", */
            /*       midicmd_to_str(chan_ev.type), */
            /*       track->number_of_ev, */
            /*       track->name, */
            /*       tick); */
            if (chan_ev.type >= 0x8 && chan_ev.type <= 0xE)
              {
                debug_midi("Inserting midi channel event\n");
                copy_midicev_to_track(track, tick, &chan_ev);
              }
          }
        buffer += offset;
        debug_midi("buffer addr: %p ---------------------------\n", buffer);
      }

  if (buffer != end)
    output_error("Unexpected size of track buffer=%p end=%p\n", buffer, end);
  if (LIST_HEAD(&(track->tickev_list)) != NULL)
    {
      debug_midi(">> channel name \"%s\" number of event = %i\n", track->name, track->tickev_list.len);
      /* debug_midi(">> first tick event at %d, and last tick event at %d\n", smallest, biggest); */
      /* track->dbg_first_ev = smallest; */
      /* track->dbg_last_ev = biggest; */
      *addr = track;
    }
  else
    {
      output_warning("No tick event found\n");
      free_track(track);
    }
  return TRUE;
}

#define BUFFER_DEFAULT_SIZE 524288

midifile_t *get_midifile_tracks(int fd,
                                midifile_hdr_chunk_t *midifile_hdr)
{
  size_t                size = 0;
  size_t                bsize = BUFFER_DEFAULT_SIZE;
  byte_t                *buffer = NULL;
  unsigned int          idx;
  midifile_t            *midifile = NULL;
  list_t                track_list;
  track_t               *track = NULL;
  midifile_info_t       info;


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
      read(fd, buffer, 8);
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
          output_error("track_%i Error while getting track size\n", idx);
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
      if (FALSE == get_midifile_track(&info, &(track), buffer, size))
        {
          output_error(ERROR_FMT"Problem with track %i\n", ERROR_ARG, idx);
          free(buffer);
          return NULL;
        }
      else
        debug_midi("track_%i ok\n", idx);

      if (track != NULL)
        {
          push_to_list(&track_list, (void *) track);
          track = NULL;
        }
    }
  free(buffer);

  if (LIST_HEAD(&track_list) != NULL)
    {
      midifile = myalloc(sizeof (midifile_t));
      /* midifile->type = midifile_hdr->format_type; */
      COPY_LIST_NODE(&track_list, &(midifile->track_list));
      midifile->number_of_track = midifile_hdr->number_of_track;
      bcopy(&info, &midifile->info, sizeof (midifile_info_t));
    }
  return midifile;
}

bool_t		get_midifile_hdr(midifile_hdr_chunk_t *mdhdr, void *ptr)
{
  char		*str = ptr;
  unsigned char	*buffer = ptr;
  size_t	size = 0;

  if (strncmp(str, "MThd", 4))
    return 0;
  buffer += 4;
  copy_to_2B(size, buffer);
  if (buffer[3] != 6)
    return 0;
  buffer += 4;
  mdhdr->format_type = buffer[2];
  buffer += 2;
  mdhdr->number_of_track = buffer[0];
  mdhdr->number_of_track = (mdhdr->number_of_track << 8) + buffer[1];
  buffer += 2;

  if (0x8000 & buffer[0])
    {
      mdhdr->time_division.flag = TRUE;
      mdhdr->time_division.value.frame_per_sec.smpte_frames = buffer[0] & 127;
      mdhdr->time_division.value.frame_per_sec.ticks_per_frame = buffer[1];
    }
  else
    {
      mdhdr->time_division.flag = FALSE;
      mdhdr->time_division.value.tick_per_beat = ((buffer[0] & 127) << 7) + buffer[1];
    }
  return 1;
}

void    output_midifile_hdr(midifile_hdr_chunk_t *midifile_hdr)
{
  debug_midi("Format type     : %s (type=%d)\n"
        "Number of track : %d\n",
        midifile_hdr->format_type == 0 ? "Single track"
        : midifile_hdr->format_type == 1 ? "Multiple tracks, synchronous"
        : midifile_hdr->format_type == 2 ? "Multiple tracks, asynchronous"
        : "Unknown midifile format type",
        midifile_hdr->format_type,
        midifile_hdr->number_of_track);
  if (midifile_hdr->time_division.flag)
    debug_midi("Time division: %d frames per second, %d ticks\n",
          midifile_hdr->time_division.value.frame_per_sec.smpte_frames,
          midifile_hdr->time_division.value.frame_per_sec.ticks_per_frame);
  else
    debug_midi("Time division: %d ticks per beat\n",
          midifile_hdr->time_division.value.tick_per_beat);
}

midifile_t  *read_midifile_fd(int fd)
{
  byte_t                buffer[14];
  midifile_hdr_chunk_t  midifile_hdr;
  size_t                size;



  //  bzero(buffer, 14);
  size = read(fd, buffer, 14);
  if (size != 14)
    {
      output_error("Problem while reading file header\n");
      close(fd);
      return NULL;
    }

#ifdef DEBUG_MIDI_MODE
  print_bin(stdout, buffer, 14);
#endif

  if (!get_midifile_hdr(&midifile_hdr, buffer))
    {
      output_error("Wrong midi file header\n");
      close(fd);
      return NULL;
    }

  output_midifile_hdr(&midifile_hdr);

  return get_midifile_tracks(fd, &midifile_hdr);
}

