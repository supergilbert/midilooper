/* Copyright 2012-2016 Gilbert Romer */

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


#ifndef __MIDIFILE_H
#define __MIDIFILE_H


#include "seqtool/seqtool.h"
#include "midi/midiev.h"

/* #define copy_to_2B(dst, src)	(dst) = *(src);			\ */
/*                                       (dst) = *(src + 1) */

/* #define copy_to_4B(dst, src)	(dst) = *(src);		\ */
/*                                       (dst) = *(src + 1);	\ */
/*                                       (dst) = *(src + 2);	\ */
/*                                       (dst) = *(src + 3)) */

#define get_midifile_type_str(type) ((type) == 0 ?	"Single track"	\
				     : (type) == 1 ?	"Multiple tracks, synchronous" \
				     : (type) == 2 ? "Multiple tracks, asynchronous" \
				     : "Unknown midifile format type")

#define TICK_PER_BEAT 0
#define FRAME_PER_SEC 1
typedef struct
{
  bool_t     flag;              /* True=FPS FALE=TPB (tick per beat) */

  union {
    uint16_t tick_per_beat;
    struct {
      byte_t smpte_frames;
      byte_t ticks_per_frame;
    }        frame_per_sec;
  }          value;

}            midifile_time_division_t;

typedef enum {
  ONETRACK_MIDIFILE = 0,
  MULTITRACK_MIDIFILE_SYNC,
  MULTITRACK_MIDIFILE_USYNC
}			seqtype_t;

typedef struct midifile_hdr_chunk_s
{
  seqtype_t                format_type;
  uint16_t                 number_of_track;
  midifile_time_division_t time_division;
}                          midifile_hdr_chunk_t;

typedef struct
{
  int    id;
  char   *name;
}           midifile_portinfo_t;

typedef struct
{
  seqtype_t type;
  uint_t    tempo; /* microseconds */
  uint_t    ppq;
  list_t    portinfo_list;
  /* char      *name; */
}           midifile_info_t;

typedef struct
{
  byte_t notes[256];
  byte_t keys[256];
  size_t notes_sz;
  size_t keys_sz;
} midif_trackb_t;

typedef struct
{
  uint_t         sysex_loop_start;
  uint_t         sysex_loop_len;
  int            sysex_portid;
  midif_trackb_t bindings;
  track_t        track;
} midifile_track_t;

typedef struct
{
  /* midifile_hdr_chunk_t  *hdr_info; */
  midifile_info_t       info;
  /* integrer les fonctions de clock au prochaine sequence, compatible au track node*/
  uint_t                number_of_track;
  list_t                track_list;
}                       midifile_t;

typedef struct buf_node_s
{
  byte_t *buffer;
  size_t len;
  struct buf_node_s *next;
} buf_node_t;

void set_be16b_uint(byte_t *buf, uint_t val);
void set_be32b_uint(byte_t *buf, uint_t val);
buf_node_t *sysex_buf_node_end(byte_t *buffer, size_t len);
buf_node_t *add_buf_node(byte_t *buffer, size_t len);
size_t get_buf_list_size(buf_node_t *buff);
buf_node_t *get_var_len_buf(uint_t tick);
buf_node_t *_append_sysex_header(buf_node_t *tail, size_t len, byte_t type);
void free_buf_list(buf_node_t *buff);
size_t write_buf_list(int fd, buf_node_t *buff);
buf_node_t *create_midifile_trackhdr(size_t track_size);

#define MSQ_SYSEX_TRACK_LOOPSTART 0   /* 4 byte track sequence length */
#define MSQ_SYSEX_TRACK_LOOPLEN   1   /* 4 byte track sequence length */
#define MSQ_SYSEX_TRACK_KEYPRESS  2   /* bytes list of key bindings */
#define MSQ_SYSEX_TRACK_NOTEPRESS 3   /* bytes list of note bindings */

#define MSQ_SYSEX_PORTNAME        128 /* 4 byte portid
                                         2 byte namelen
                                         etc. name data */
#define MSQ_TRACK_PORTID          129 /* followed by 4 byte */

#define GETVLVSIZE(_tick) (_tick < 128) ? 1 :            \
  (((_tick >> 7) < 128) ? 2 :                            \
   (((_tick >> 14) < 128) ? 3 :                          \
    4))

#include <unistd.h>
/* #include "seqtool/seqtool.h" */

/* int get_midifile_hdr_chunk(midifile_hdr_chunk_t *mdhdr, void *ptr); */
/* void read_midifile(char *filepath); */
size_t get_midifile_track_size(byte_t *buffer);

bool_t get_midifile_hdr(midifile_hdr_chunk_t *mdhdr, void *ptr);
midifile_t *read_midifile_fd(int fd);
void free_midifile_track(midifile_track_t *mtrack);
void free_midifile(midifile_t *sequence);
size_t midifile_trackev_size(track_t *track);

track_t  *midifile_to_onetrack(char *filename);
buf_node_t *_append_metaev_set_tempo(buf_node_t *tail, uint_t tempo);
buf_node_t *_append_metaev_eot(buf_node_t *tail);

void write_midifile_trackhdr(int fd, size_t track_size);
size_t write_midifile_header(int fd, uint_t track_list_len, uint_t ppq);
void write_midifile_track(int fd, midifile_track_t *mtrack, bool_t template);

#endif
