/* Copyright 2012-2014 Gilbert Romer */

/* This file is part of gmidilooper. */

/* gmidilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* gmidilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include <string.h>

#include "debug_tool/debug_tool.h"
#include "midi/midi_tool.h"
#include "midi/midifile.h"

void write_midifile_header(int fd, uint_t track_list_len, uint_t ppq)
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
  write(fd, midi_header_chunk, 14);
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

buf_node_t *get_midifile_trackhdr(size_t track_size)
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

buf_node_t *get_var_len_buf(uint_t tick)
{
  byte_t buffer[4];
  size_t size = GETVLVSIZE(tick);

  switch (size)
    {
    case 1:
      *buffer = tick & 0x7F;
     break;
    case 2:
      *buffer = (tick >> 7) & 0x7F;
      *buffer = set_first_bit(*buffer);
      buffer[1] = tick & 0x7F;
      break;
    case 3:
      *buffer = (tick >> 14) & 0x7F;
      *buffer = set_first_bit(*buffer);
      buffer[1] = (tick >> 7) & 0x7F;
      buffer[1] = set_first_bit(buffer[1]);
      buffer[2] = tick & 0x7F;
      break;
    case 4:
      buffer[0] = (tick >> 21) & 0x7F;
      buffer[0] = set_first_bit(buffer[0]);
      buffer[1] = (tick >> 14) & 0x7F;
      buffer[1] = set_first_bit(buffer[1]);
      buffer[2] = (tick >> 7) & 0x7F;
      buffer[2] = set_first_bit(buffer[2]);
      buffer[3] = tick & 0x7F;
      break;
    default:
      output_error("unexpected variable len");
      return NULL;
    }
  return add_buf_node(buffer, size);
}

buf_node_t *get_midicev_buf(midicev_t *midicev)
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
      if (seqev->deleted == FALSE && seqev->type == MIDICEV)
        return (midicev_t *) seqev->addr;
      iter_next(iter);
    }
  return NULL;
}

buf_node_t *_get_tickev_buf(buf_node_t *tail, tickev_t *tickev, uint_t last_tick)
{
  uint_t          tickdiff;
  midicev_t       *midicev;
  list_iterator_t iter;

  iter_init(&iter, &(tickev->seqev_list));
  if (NULL != (midicev = get_next_midicev(&iter)))
    {
      tickdiff = tickev->tick - last_tick;
      tail->next = get_var_len_buf(tickdiff);
      tail = tail->next;
      tail->next = get_midicev_buf(midicev);
      tail = tail->next;
      iter_next(&iter);
      while (NULL != (midicev = get_next_midicev(&iter)))
        {
          tail->next = get_var_len_buf(0);
          tail = tail->next;
          tail->next = get_midicev_buf(midicev);
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
      if (tickev->deleted == FALSE)
        {
          tmp = _get_tickev_buf(tail, tickev, last_tick);
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
  tail->next = get_var_len_buf(name_len);
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
  byte_t msq_hdr[5] = {0, 'M', 'S', 'Q', type};

  tail->next = add_buf_node(hdr, 2);
  tail = tail->next;
  tail->next = get_var_len_buf(buflen + 5);
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

buf_node_t *_append_sysex_loopstart(buf_node_t *tail, uint_t start)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MSQ_SYSEX_TRACK_LOOPSTART);
  set_be32b_uint(buf, start);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_looplen(buf_node_t *tail, uint_t len)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MSQ_SYSEX_TRACK_LOOPLEN);
  set_be32b_uint(buf, len);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
  return tail->next;
}

buf_node_t *_append_sysex_type_bindings(buf_node_t *tail, uint_t type, byte_t *array, size_t sz)
{
  byte_t array_sz = sz;
  tail = _append_sysex_header(tail,
                              sz + 1,
                              type);
  /* Add size */
  tail->next = add_buf_node(&array_sz, 1);
  tail = tail->next;
  /* Marking end of sysex array size < 256 */
  array[sz] = 0xF7;
  /* Add array with sysex end */
  tail->next = add_buf_node(array, sz + 1);
  return tail->next;
}

buf_node_t *_append_sysex_bindings(buf_node_t *tail, midif_trackb_t *bindings)
{
  if (bindings->keys_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MSQ_SYSEX_TRACK_KEYPRESS,
                                       bindings->keys,
                                       bindings->keys_sz);
  if (bindings->notes_sz > 0)
    tail = _append_sysex_type_bindings(tail,
                                       MSQ_SYSEX_TRACK_NOTEPRESS,
                                       bindings->notes,
                                       bindings->notes_sz);
  return tail;
}

buf_node_t *_append_sysex_portid(buf_node_t *tail, int sysex_portid)
{
  byte_t buf[5];

  tail = _append_sysex_header(tail, 5, MSQ_TRACK_PORTID);
  set_be32b_uint(buf, sysex_portid);
  buf[4] = 0xF7;
  tail->next = add_buf_node(buf, 5);
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

void write_buf_list(int fd, buf_node_t *buff)
{
  while (buff)
    {
      write(fd, buff->buffer, buff->len);
      buff = buff->next;
    }
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

void write_midifile_track(int fd, midifile_track_t *mtrack)
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
  tail = _append_sysex_bindings(tail, &(mtrack->bindings));
  if (mtrack->sysex_portid != -1)
    tail = _append_sysex_portid(tail, mtrack->sysex_portid);

  tail = _append_tickev_list(tail, &(mtrack->track.tickev_list));
  /* tail = _append_metaev_eot(tail); */
  _append_metaev_eot(tail);


  header = get_midifile_trackhdr(get_buf_list_size(head.next));

  header->next = head.next;

  write_buf_list(fd, header);
  free_buf_list(header);
}
