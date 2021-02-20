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


#include <string.h>

#include "debug_tool/debug_tool.h"
#include "midi/midi_tool.h"
#include "midi/midifile.h"

size_t write_midifile_header(int fd, uint_t track_list_len, uint_t ppq)
{
  byte_t midi_header_chunk[14];

  /* midifile chunk id and size */
  memcpy(midi_header_chunk, "MThd\0\0\0\6", 8);

  /* midiformat type */
  midi_header_chunk[8] = 0;
  midi_header_chunk[9] = 1;

  /* number of track */
  midi_header_chunk[10] = (track_list_len >> 8) & 0xFF;
  midi_header_chunk[11] = track_list_len & 0xFF;

  /* time division */
  if (ppq <= 0x8FF)
    {
      midi_header_chunk[12] = (ppq >> 8) & 0x7F;
      midi_header_chunk[13] = ppq & 0xFF;
    }
  else
    output_error("Error ppq is too high (ppq=%d)", ppq);
  return write(fd, midi_header_chunk, 14);
}

buf_node_t *sysex_buf_node_end(byte_t *buffer, size_t len)
{
  buf_node_t *new_node = myalloc(sizeof (buf_node_t));

  new_node->buffer = myalloc(len + 1);
  bcopy(buffer, new_node->buffer, len);
  new_node->buffer[len] = 0xF7;
  new_node->len = len + 1;
  new_node->next = NULL;
  return new_node;
}

buf_node_t *add_buf_node(byte_t *buffer, size_t len)
{
  buf_node_t *new_node = myalloc(sizeof (buf_node_t));

  new_node->buffer = myalloc(len);
  bcopy(buffer, new_node->buffer, len);
  new_node->len = len;
  new_node->next = NULL;
  return new_node;
}

void free_buf_list(buf_node_t *buff)
{
  buf_node_t *tmp = NULL;

  while (buff)
    {
      free(buff->buffer);
      tmp = buff->next;
      free(buff);
      buff = tmp;
    }
}

buf_node_t *create_midifile_trackhdr(size_t track_size)
{
  byte_t track_buffer[8];

  track_buffer[0] = 'M';
  track_buffer[1] = 'T';
  track_buffer[2] = 'r';
  track_buffer[3] = 'k';

  track_buffer[4] = (track_size >> 24) & 0xFF;
  track_buffer[5] = (track_size >> 16) & 0xFF;
  track_buffer[6] = (track_size >> 8) & 0xFF;
  track_buffer[7] = track_size & 0xFF;

  return add_buf_node(track_buffer, 8);
}

#define set_first_bit(_byte) ((_byte) | 0x80)

buf_node_t *create_var_len_val(uint_t value)
{
  byte_t buffer[4];
  size_t size = GETVLVSIZE(value);

  switch (size)
    {
    case 1:
      *buffer = value & 0x7F;
     break;
    case 2:
      *buffer = (value >> 7) & 0x7F;
      *buffer = set_first_bit(*buffer);
      buffer[1] = value & 0x7F;
      break;
    case 3:
      *buffer = (value >> 14) & 0x7F;
      *buffer = set_first_bit(*buffer);
      buffer[1] = (value >> 7) & 0x7F;
      buffer[1] = set_first_bit(buffer[1]);
      buffer[2] = value & 0x7F;
      break;
    case 4:
      buffer[0] = (value >> 21) & 0x7F;
      buffer[0] = set_first_bit(buffer[0]);
      buffer[1] = (value >> 14) & 0x7F;
      buffer[1] = set_first_bit(buffer[1]);
      buffer[2] = (value >> 7) & 0x7F;
      buffer[2] = set_first_bit(buffer[2]);
      buffer[3] = value & 0x7F;
      break;
    default:
      output_error("unexpected variable len");
      return NULL;
    }
  return add_buf_node(buffer, size);
}

buf_node_t *create_midicev_buf(midicev_t *midicev)
{
  byte_t buffer[3];

  convert_midicev_to_mididata(midicev, buffer);
  return add_buf_node(buffer, 3);
}

midicev_t *get_next_midicev(list_iterator_t *iter)
{
  seqev_t   *seqev = NULL;

  while (iter_node(iter))
    {
      seqev = iter_node_ptr(iter);
      if (seqev->deleted == MSQ_FALSE && seqev->type == MIDICEV)
        return (midicev_t *) seqev->addr;
      iter_next(iter);
    }
  return NULL;
}

buf_node_t *_create_tickev_buf(buf_node_t *tail, tickev_t *tickev, uint_t last_tick)
{
  uint_t          tickdiff;
  midicev_t       *midicev;
  list_iterator_t iter;

  iter_init(&iter, &(tickev->seqev_list));
  if (NULL != (midicev = get_next_midicev(&iter)))
    {
      tickdiff = tickev->tick - last_tick;
      tail->next = create_var_len_val(tickdiff);
      tail = tail->next;
      tail->next = create_midicev_buf(midicev);
      tail = tail->next;
      iter_next(&iter);
      while (NULL != (midicev = get_next_midicev(&iter)))
        {
          tail->next = create_var_len_val(0);
          tail = tail->next;
          tail->next = create_midicev_buf(midicev);
          tail = tail->next;
          iter_next(&iter);
        }
      return tail;
    }
  return NULL;
}

buf_node_t *_append_tickev_list(buf_node_t *tail, list_t *tickevlist)
{
  list_iterator_t iter;
  buf_node_t      *tmp = NULL;
  tickev_t        *tickev = NULL;
  uint_t          last_tick = 0;

  for (iter_init(&iter, tickevlist);
       iter_node(&iter);
       iter_next(&iter))
    {
      tickev = iter_node_ptr(&iter);
      if (tickev->deleted == MSQ_FALSE)
        {
          tmp = _create_tickev_buf(tail, tickev, last_tick);
          if (tmp != NULL)
            {
              tail = tmp;
              last_tick = tickev->tick;
            }
        }
    }
  return tail;
}

buf_node_t *_append_metaev_track_name(buf_node_t *tail, char *name)
{
  byte_t me_name_type[3] = {0, 0xFF, 0x03};
  uint_t name_len = 0;
  if (name == NULL)
    return tail;

  name_len = strlen(name);
  tail->next = add_buf_node(me_name_type, 3);
  tail = tail->next;
  tail->next = create_var_len_val(name_len);
  tail = tail->next;
  tail->next = add_buf_node((byte_t *) name, name_len);
  return tail->next;
}

buf_node_t *_append_metaev_set_tempo(buf_node_t *tail, uint_t tempo)
{
  byte_t me_name_type[3] = {0, 0xFF, 0x51};
  byte_t bp_buf[4];

  tail->next = add_buf_node(me_name_type, 3);
  tail = tail->next;

  bp_buf[0] = 3;
  bp_buf[1] = (tempo >> 16) & 0xFF;
  bp_buf[2] = (tempo >> 8) & 0xFF;
  bp_buf[3] = tempo & 0xFF;

  tail->next = add_buf_node(bp_buf, 4);
  return tail->next;
}

buf_node_t *_append_sysex_header(buf_node_t *tail, size_t buflen, byte_t type)
{
  byte_t hdr[2] = {0, 0xF0};
  byte_t msq_hdr[5] = {0, 'M', 'L', 'P', type};

  tail->next = add_buf_node(hdr, 2);
  tail = tail->next;
  tail->next = create_var_len_val(buflen + 5);
  tail = tail->next;
  tail->next = add_buf_node(msq_hdr, 5);
  return tail->next;
}

void set_be32b_uint(byte_t *buf, uint_t val)
{
  buf[0] = (val >> 24) & 0xFF;
  buf[1] = (val >> 16) & 0xFF;
  buf[2] = (val >> 8) & 0xFF;
  buf[3] = val & 0xFF;
}

void set_be16b_uint(byte_t *buf, uint_t val)
{
  buf[0] = (val >> 8) & 0xFF;
  buf[1] = val & 0xFF;
}

#define MSQ_FILE_VERSION 1

buf_node_t *_append_sysex_file_version(buf_node_t *tail)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MLP_SYSEX_FILE_VERSION);
  set_be32b_uint(buf, MSQ_FILE_VERSION);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_engine_type(buf_node_t *tail, uint_t engine_type)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MLP_SYSEX_ENGINE_TYPE);
  set_be32b_uint(buf, engine_type);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_loopstart(buf_node_t *tail, uint_t start)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MLP_SYSEX_TRACK_LOOPSTART);
  set_be32b_uint(buf, start);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_looplen(buf_node_t *tail, uint_t len)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MLP_SYSEX_TRACK_LOOPLEN);
  set_be32b_uint(buf, len);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_type_bindings(buf_node_t *tail,
                                        uint_t type,
                                        byte_t *array,
                                        byte_t array_sz)
{
  byte_t tmp;
  tail = _append_sysex_header(tail,
                              array_sz + 2, /* sz (8b) + array + 0xF7 */
                              type);
  /* Add size */
  tail->next = add_buf_node(&array_sz, 1);
  tail = tail->next;
  /* Add array */
  tail->next = add_buf_node(array, array_sz);
  tail = tail->next;
  /* Add sysex end */
  tmp = 0xF7;
  tail->next = add_buf_node(&tmp, 1);
  return tail->next;
}

buf_node_t *_append_sysex_bindings(buf_node_t *tail, midifile_track_t *mtrack)
{
  if (mtrack->mute_bindings.keys_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MLP_SYSEX_MUTE_TRACK_BINDING_KEYPRESS,
                                       mtrack->mute_bindings.keys,
                                       mtrack->mute_bindings.keys_sz);
  if (mtrack->mute_bindings.notes_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MLP_SYSEX_MUTE_TRACK_BINDING_NOTEPRESS,
                                       mtrack->mute_bindings.notes,
                                       mtrack->mute_bindings.notes_sz);
  if (mtrack->mute_bindings.programs_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MLP_SYSEX_MUTE_TRACK_BINDING_PROGPRESS,
                                       mtrack->mute_bindings.programs,
                                       mtrack->mute_bindings.programs_sz);


  if (mtrack->rec_bindings.keys_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MLP_SYSEX_REC_TRACK_BINDING_KEYPRESS,
                                       mtrack->rec_bindings.keys,
                                       mtrack->rec_bindings.keys_sz);
  if (mtrack->rec_bindings.notes_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MLP_SYSEX_REC_TRACK_BINDING_NOTEPRESS,
                                       mtrack->rec_bindings.notes,
                                       mtrack->rec_bindings.notes_sz);
  if (mtrack->rec_bindings.programs_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MLP_SYSEX_REC_TRACK_BINDING_PROGPRESS,
                                       mtrack->rec_bindings.programs,
                                       mtrack->rec_bindings.programs_sz);
  return tail;
}

buf_node_t *_append_sysex_portid(buf_node_t *tail, int sysex_portid)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MLP_SYSEX_TRACK_PORTID);
  set_be32b_uint(buf, sysex_portid);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_muted(buf_node_t *tail)
{
  byte_t sysex_end = 0xF7;

  tail = _append_sysex_header(tail, 1, MLP_SYSEX_TRACK_MUTED);
  tail->next = add_buf_node(&sysex_end, 1);
  return tail->next;
}

/* end of track */
buf_node_t *_append_metaev_eot(buf_node_t *tail)
{
  byte_t buffer[4];

  buffer[0] = 0;
  buffer[1] = 0xFF;
  buffer[2] = 0x2F;
  buffer[3] = 0;
  tail->next = add_buf_node(buffer, 4);
  return tail->next;
}

size_t write_buf_list(int fd, buf_node_t *buff)
{
  size_t size = 0;

  while (buff)
    {
      size += write(fd, buff->buffer, buff->len);
      buff = buff->next;
    }
  return size;
}

size_t get_buf_list_size(buf_node_t *buff)
{
  size_t size = 0;
  while (buff)
    {
      size += buff->len;
      buff = buff->next;
    }
  return size;
}

void write_midifile_track(int fd, midifile_track_t *mtrack, msq_bool_t template)
{
  buf_node_t *tail = NULL, *header = NULL, head = {NULL, 0, NULL};
  char       *name = NULL;

  if (mtrack->track.name)
    name = mtrack->track.name;
  else
    name = "No name";

  tail = _append_metaev_track_name(&head, name);

  tail = _append_sysex_loopstart(tail, mtrack->sysex_loop_start);
  tail = _append_sysex_looplen(tail, mtrack->sysex_loop_len);
  tail = _append_sysex_bindings(tail, mtrack);
  if (mtrack->sysex_portid != -1)
    tail = _append_sysex_portid(tail, mtrack->sysex_portid);
  if (mtrack->sysex_muted == MSQ_TRUE)
    tail = _append_sysex_muted(tail);

  if (template == MSQ_FALSE)
    tail = _append_tickev_list(tail, &(mtrack->track.tickev_list));
  /* tail = _append_metaev_eot(tail); */
  _append_metaev_eot(tail);

  /* Adding midifile track size information */
  header = create_midifile_trackhdr(get_buf_list_size(head.next));

  header->next = head.next;

  write_buf_list(fd, header);
  free_buf_list(header);
}
