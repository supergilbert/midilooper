#include <string.h>

#include "debug_tool/debug_tool.h"
#include "midi/midifile.h"

void write_midifile_header(int fd, uint_t track_list_len, uint_t ppq)
{
  byte_t midi_header_chunk[14];

  /* midi file chunk id and size */
  memcpy(midi_header_chunk, "MThd\0\0\0\6", 8);

  /* midi format type */
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

typedef struct
{
  byte_t *buffer;
  uint_t idx;
  uint_t tick;
} buf_hdl_t;

void fill_midifile_buffer_trackhdr(buf_hdl_t *hdl, size_t track_size)
{
  byte_t *track_buffer = &(hdl->buffer[hdl->idx]);

  track_buffer[0] = 'M';
  track_buffer[1] = 'T';
  track_buffer[2] = 'r';
  track_buffer[3] = 'k';

  track_buffer[4] = (track_size >> 24) & 0xFF;
  track_buffer[5] = (track_size >> 16) & 0xFF;
  track_buffer[6] = (track_size >> 8) & 0xFF;
  track_buffer[7] = track_size & 0xFF;

  hdl->idx += 8;
}

#define set_first_bit(_byte) ((_byte) | 0x80)

void fill_var_len_buf(buf_hdl_t *buf_hdl, uint_t tick)
{
  byte_t *buf = &(buf_hdl->buffer[buf_hdl->idx]);
  size_t size = GETVLVSIZE(tick);

  switch (size)
    {
    case 0:
      *buf = tick & 0x7F;
      buf_hdl->idx += 1;
     break;
    case 1:
      *buf = (tick >> 7) & 0x7F;
      *buf = set_first_bit(*buf);
      buf[1] = tick & 0x7F;
      buf_hdl->idx += 2;
      break;
    case 2:
      *buf = (tick >> 14) & 0x7F;
      *buf = set_first_bit(*buf);
      buf[1] = (tick >> 7) & 0x7F;
      buf[1] = set_first_bit(buf[1]);
      buf[2] = tick & 0x7F;
      buf_hdl->idx += 3;
      break;
    case 3:
      buf[0] = (tick >> 21) & 0x7F;
      buf[0] = set_first_bit(buf[0]);
      buf[1] = (tick >> 14) & 0x7F;
      buf[1] = set_first_bit(buf[1]);
      buf[2] = (tick >> 7) & 0x7F;
      buf[2] = set_first_bit(buf[2]);
      buf[3] = tick & 0x7F;
      buf_hdl->idx += 4;
      break;
    default:
      output_error("unexpected variable len");
    }
}

void write_midicev(byte_t *buf, midicev_t *midicev)
{
  switch (midicev->type)
    {
    case NOTEOFF:
    case NOTEON:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.note.num & 0xFF;
      buf[2] = midicev->event.note.val & 0xFF;
      break;
    case KEYAFTERTOUCH:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.aftertouch.num & 0xFF;
      buf[2] = midicev->event.aftertouch.val & 0xFF;
      break;
    case CONTROLCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.ctrl.num & 0xFF;
      buf[2] = midicev->event.ctrl.val & 0xFF;
      break;
    case PROGRAMCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.prg_chg;
      buf[2] = 0;
      break;
    case CHANNELAFTERTOUCH:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.chan_aftertouch;
      buf[2] = 0;
      break;
    case PITCHWHEELCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.ctrl.num & 0xFF;
      buf[2] = midicev->event.ctrl.val & 0xFF;
      break;
    default:
      output_error("Unexpected channel event type\n");
      break;
    }
}

void fill_midicev_buf(buf_hdl_t *buf_hdl, midicev_t *midicev)
{
  write_midicev(&(buf_hdl->buffer[buf_hdl->idx]), midicev);
  buf_hdl->idx += 3;
}

midicev_t *get_next_midicev(list_iterator_t *iter)
{
  seqev_t   *seqev = NULL;

  while (iter_node(iter))
    {
      seqev = iter_node_ptr(iter);
      if (seqev->type == MIDICEV)
        return (midicev_t *) seqev->addr;
      iter_next(iter);
    }
  return NULL;
}

void hdl_tickev_buf(void *tev_ptr, void *hdlptr)
{
  tickev_t        *tickev = (tickev_t *) tev_ptr;
  buf_hdl_t       *buf_hdl = (buf_hdl_t *) hdlptr;
  uint_t          tickdiff;
  midicev_t       *midicev;
  list_iterator_t iter;

  iter_init(&iter, &(tickev->seqev_list));
  if (NULL != (midicev = get_next_midicev(&iter)))
    {
      tickdiff = tickev->tick - buf_hdl->tick;
      buf_hdl->tick = tickev->tick;
      fill_var_len_buf(buf_hdl, tickdiff);
      fill_midicev_buf(buf_hdl, midicev);
      iter_next(&iter);
      while (NULL != (midicev = get_next_midicev(&iter)))
        {
          fill_var_len_buf(buf_hdl, 0);
          fill_midicev_buf(buf_hdl, midicev);
          iter_next(&iter);
        }
    }
}

void fill_metaev_track_name(buf_hdl_t *buf_hdl, char *name, uint_t name_len)
{
  buf_hdl->buffer[buf_hdl->idx] = 0;
  buf_hdl->idx++;
  buf_hdl->buffer[buf_hdl->idx] = 0xFF;
  buf_hdl->idx++;
  buf_hdl->buffer[buf_hdl->idx] = 0x03;
  buf_hdl->idx++;
  fill_var_len_buf(buf_hdl, name_len);
  memcpy(&(buf_hdl->buffer[buf_hdl->idx]), name, name_len);
  buf_hdl->idx += name_len;
}

size_t write_sysex_tracklen(byte_t *buf, uint_t len)
{
  buf[0] = 0;                   /* tick difference */
  buf[1] = 0xF0;                /* sysex */
  buf[2] = 10;                   /* variable length */
  buf[3] = 0;                   /* unknown manufucturer id */
  buf[4] = 'M';
  buf[5] = 'S';
  buf[6] = 'Q';
  buf[7] = MSQ_TRACK_LEN_SYSEX;
  buf[8] = (len >> 24) & 0xFF;
  buf[9] = (len >> 16) & 0xFF;
  buf[10] = (len >> 8) & 0xFF;
  buf[11] = len & 0xFF;
  buf[12] = 0xF7;
  return 13;
}

void write_midifile_track(int fd, track_t *track)
{
  size_t    trackev_size = midifile_trackev_size(track) + 4 + 13; /* Add end of
                                                                     track event
                                                                     + track len */
  buf_hdl_t buf_hdl = {NULL, 0, 0};
  uint_t    name_len = 0;

  debug(" XXX midiev size = %d", trackev_size - 4);

  /* Allocate buffer with track information */
  if (track->name)
    {
      debug("buf_hdl.idx %d", buf_hdl.idx);
      name_len = strlen(track->name);

      debug("name_len %d", name_len);

      /* Add name event size */
      trackev_size += name_len;
      trackev_size += 4;
      trackev_size += GETVLVSIZE(name_len);

      debug("new trackev size = %d", trackev_size);

      buf_hdl.buffer = myalloc(trackev_size + 8);
      fill_midifile_buffer_trackhdr(&buf_hdl, trackev_size);
      debug("buf_hdl.idx %d", buf_hdl.idx);

      fill_metaev_track_name(&buf_hdl, track->name, name_len);
      debug("buf_hdl.idx %d", buf_hdl.idx);

      debug(" XXX expected final idx = %d", 8 + trackev_size);
    }
  else
    {
      buf_hdl.buffer = myalloc(trackev_size + 8);
      fill_midifile_buffer_trackhdr(&buf_hdl, trackev_size);
      buf_hdl.idx += 8;
    }

  buf_hdl.idx += write_sysex_tracklen(&(buf_hdl.buffer[buf_hdl.idx]), track->len);

  /* midi channel events */
  foreach_list_node(&(track->tickev_list), hdl_tickev_buf, &buf_hdl);


  debug("buf_hdl.idx %d", buf_hdl.idx);

  /* meta event end of track */
  buf_hdl.buffer[buf_hdl.idx] = 0;
  buf_hdl.idx++;
  buf_hdl.buffer[buf_hdl.idx] = 0xFF;
  buf_hdl.idx++;
  buf_hdl.buffer[buf_hdl.idx] = 0x2F;
  buf_hdl.idx++;
  buf_hdl.buffer[buf_hdl.idx] = 0;
  buf_hdl.idx++;

  write(fd, buf_hdl.buffer, trackev_size + 8);

  free(buf_hdl.buffer);

  debug("buf_hdl.idx %d track size %d", buf_hdl.idx, trackev_size + 8);
}

