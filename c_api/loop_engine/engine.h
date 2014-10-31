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


#ifndef __ENGINE_H
#define __ENGINE_H

#include <pthread.h>

/* #include "seqtool/seqtool.h" */
#include "midi/midifile.h"
#include "clock/clock.h"
#include "asound/aseq_tool.h"

typedef enum
  {
    engine_cont = 0,
    engine_stop
  }     engine_rq;

typedef struct
{
  pthread_t        thread_id;
  bool_t           thread_ret;
  pthread_rwlock_t lock;
  engine_rq        rq;
  bool_t           isrunning;
  list_t           track_list;
  list_t           aseqport_list;
  snd_seq_t        *aseqh;
  clockloop_t      looph;
  uint_t           ppq;
  uint_t           tempo;       /* beat in micro seconde */
}       engine_ctx_t;

typedef enum
  {
    req_pending_notes = 0,
    req_play_midicev,
  } trackreq;

typedef struct
{
  bool_t    used;
  trackreq  req;
  midicev_t midicev;
} trackreq_t;

typedef struct
{
  aseqport_ctx_t   *aseqport_ctx;
  track_t          *track;
  uint_t           loop_start;
  uint_t           loop_len;
  bool_t           mute;
  list_iterator_t  current_tickev;
  pthread_rwlock_t lock;
  list_t           trash;
  engine_ctx_t     *engine;
  bool_t           deleted;
  bool_t           need_sync;
  list_t           req_list;
  pthread_mutex_t  req_lock;
  byte_t           pending_notes[256];
} track_ctx_t;

void       trackreq_play_midicev(track_ctx_t *trackctx, midicev_t *midicev);
void       trackreq_play_pendings(track_ctx_t *trackctx);
trackreq_t *trackreq_getnext_req(list_t *req_list);
#define    free_trackreq(reqlist) (free_list_node((reqlist), free))

engine_ctx_t    *init_engine_ctx(char *aname);
void            free_engine_ctx(engine_ctx_t *ctx);
bool_t          start_engine(engine_ctx_t *ctx);
void            stop_engine(engine_ctx_t *ctx);
void            wait_engine(engine_ctx_t *ctx);
clock_req_t     engine_cb(void *arg);
bool_t          engine_isrunning(engine_ctx_t *ctx);
aseqport_ctx_t  *engine_create_aport(engine_ctx_t *ctx, char *name);
bool_t          engine_del_port(engine_ctx_t *ctx,
                                aseqport_ctx_t *aseqportctx);
bool_t          engine_del_track(engine_ctx_t *ctx, track_ctx_t *trackctx);
track_ctx_t     *engine_new_track(engine_ctx_t *ctx, char *name);
track_ctx_t     *engine_copy_trackctx(engine_ctx_t *ctx, track_ctx_t *trackctx);
void            engine_read_midifile(engine_ctx_t *ctx, midifile_t *midifile);
void            engine_set_bpm(engine_ctx_t *ctx, uint_t bpm);
void            engine_set_tempo(engine_ctx_t *ctx, uint_t tempo);
void            engine_reset_pulse(engine_ctx_t *ctx);

void            engine_save_project(engine_ctx_t *ctx, char *file_path);

void            play_trackctx(uint_t tick, track_ctx_t *track_ctx, bool_t *ev_to_drain);
uint_t          trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick);

#define play_track_pending_notes(track_ctx)                           \
  alsa_output_pending_notes((track_ctx)->aseqport_ctx,                \
                            (track_ctx)->pending_notes)

# include "ev_iterator.h"

typedef struct
{
  list_iterator_t evit;
  list_iterator_t tickit;
} trash_ctn_t;

void            trackctx_event2trash(track_ctx_t *traxkctx,
                                     ev_iterator_t *ev_iterator);

#endif
