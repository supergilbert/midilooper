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

#include "midi/midifile.h"

typedef struct midioutput
{
  void        *hdl;
  const char  *(*get_name)(void *hdl);
  void        (*set_name)(void *hdl, char *name);
  bool_t      (*send_ev)(struct midioutput *output, midicev_t *midicev);
  bool_t      (*buff_ev)(struct midioutput *output, midicev_t *midicev);
} output_t;

#define output_get_name(output)       (output)->get_name((output)->hdl)
#define output_set_name(output, name) (output)->set_name((output)->hdl, name)
#define send_ev(output, midicev)      (output)->send_ev(output, midicev)
#define buff_ev(output, midicev)      (output)->buff_ev(output, midicev)

typedef enum
  {
    NANOSLEEP_ENGINE = 0        /* + Alsa */
  } engine_type;

typedef struct engine_ctx
{
  void     *hdl;
  list_t   output_list;
  list_t   track_list;
  uint_t   ppq;                   /* Pulse per quater note (beat) */
  uint_t   tempo;                 /* Quater note in micro second */
  bool_t   (*is_running)(struct engine_ctx *engine);
  void     (*destroy)(struct engine_ctx *engine);
  bool_t   (*start)(struct engine_ctx *engine);
  void     (*stop)(struct engine_ctx *engine);
  output_t *(*create_output)(struct engine_ctx *engine, char *name);
  bool_t   (*delete_output)(struct engine_ctx *engine, output_t *output);
  uint_t   (*get_tick)(struct engine_ctx *engine);
  uint_t   (*set_bpm)(struct engine_ctx *engine);
  void     (*_send_buff)(struct engine_ctx *engine);
  void     (*reset_pulse)(struct engine_ctx *engine);
} engine_ctx_t;

#define engine_is_running(eng)            (eng)->is_running(eng)
#define engine_destroy(eng)               (eng)->destroy(eng)
#define engine_start(eng)                 (eng)->start(eng)
#define engine_stop(eng)                  (eng)->stop(eng)
#define engine_create_output(eng, name)   (eng)->create_output(eng, name)
#define engine_delete_output(eng, output) (eng)->delete_output(eng, output)
#define engine_get_tick(eng)              (eng)->get_tick(eng)
#define _engine_send_buff(eng)            (eng)->_send_buff(eng)
#define engine_reset_pulse(eng)           (eng)->reset_pulse(eng)

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
  engine_ctx_t     *engine;
  output_t         *output;
  track_t          *track;
  uint_t           loop_start;
  uint_t           loop_len;
  bool_t           mute;
  list_iterator_t  current_tickev;
  pthread_rwlock_t lock;
  list_t           trash;
  bool_t           deleted;
  bool_t           need_sync;
  list_t           req_list;
  pthread_mutex_t  req_lock;
  byte_t           notes_on_state[256];
} track_ctx_t;

void       trackreq_play_midicev(track_ctx_t *trackctx, midicev_t *midicev);
void       trackreq_play_pendings(track_ctx_t *trackctx);
trackreq_t *trackreq_getnext_req(list_t *req_list);
#define    free_trackreq(reqlist) (free_list_node((reqlist), free))
uint_t     trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick);
void       play_trackctx(uint_t tick,
                         track_ctx_t *track_ctx,
                         bool_t *ev_to_drain);

track_ctx_t *engine_create_trackctx(engine_ctx_t *engine, char *name);
bool_t      engine_delete_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
track_ctx_t *engine_copy_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
void        engine_read_midifile(engine_ctx_t *engine, midifile_t *midifile);
void        engine_save_project(engine_ctx_t *engine, char *file_path);
void        engine_prepare_tracklist(engine_ctx_t *ctx);
void        engine_clean_tracklist(engine_ctx_t *ctx);
void        _engine_free_trash(engine_ctx_t *ctx);

void   play_all_tracks_pending_notes(engine_ctx_t *ctx);
void   play_all_tracks_ev(engine_ctx_t *ctx);
bool_t output_evlist(output_t *output,
                     list_t *seqevlist,
                     byte_t *notes_on_state);
bool_t output_pending_notes(output_t *output, byte_t *notes_on_state);

bool_t nns_init_engine(engine_ctx_t *ctx, char *name);

# include "ev_iterator.h"

typedef struct
{
  list_iterator_t evit;
  list_iterator_t tickit;
} trash_ctn_t;

void            trackctx_event2trash(track_ctx_t *traxkctx,
                                     ev_iterator_t *ev_iterator);

#endif
