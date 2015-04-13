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

typedef struct
{
  bool_t    used;
  midicev_t midicev;
} midireq_t;

typedef struct midioutput
{
  void            *hdl;
  list_t          req_list;
  pthread_mutex_t req_lock;
  const char      *(*get_name)(void *hdl);
  void            (*set_name)(void *hdl, const char *name);
  /* is_handled should disapear */
  bool_t          (*write)(struct midioutput *output,
                           midicev_t *midicev);
  /* _write must be always serialized and in the engine thread */
  bool_t          (*_write)(struct midioutput *output, midicev_t *midicev);
} output_t;

#define output_get_name(output)        (output)->get_name((output)->hdl)
#define output_set_name(output, name)  (output)->set_name((output)->hdl, name)
#define output_write(output, midicev)           \
  (output)->write(output, midicev)
#define _output_write(output, midicev) (output)->_write(output, midicev)

void output_add_req(output_t *output, midicev_t *midicev);

typedef struct engine_ctx
{
  void     *hdl;
  list_t   output_list;
  list_t   track_list;
  uint_t   ppq;                 /* Pulse per quater note (beat) */
  uint_t   tempo;               /* Quater note in micro second */
  bool_t   (*is_running)(struct engine_ctx *engine);
  void     (*destroy_hdl)(struct engine_ctx *engine);
  bool_t   (*start)(struct engine_ctx *engine);
  void     (*stop)(struct engine_ctx *engine);
  void     (*init_output)(struct engine_ctx *engine,
                          output_t *output,
                          const char *name);
  void     (*free_output_node)(void *addr);
  uint_t   (*get_tick)(struct engine_ctx *engine);
  void     (*set_tempo)(struct engine_ctx *engine, uint_t ms);
} engine_ctx_t;

#define engine_is_running(eng)                (eng)->is_running(eng)
#define engine_destroy_hdl(eng)               (eng)->destroy_hdl(eng)
#define engine_start(eng)                     (eng)->start(eng)
#define engine_stop(eng)                      (eng)->stop(eng)
#define engine_get_tick(eng)                  (eng)->get_tick(eng)
#define engine_set_tempo(eng, ms)            (eng)->set_tempo(eng, ms)

output_t *engine_create_output(engine_ctx_t *ctx, const char *name);
bool_t   engine_delete_output(engine_ctx_t *ctx, output_t *output);

typedef struct
{
  engine_ctx_t     *engine;
  output_t         *output;
  track_t          *track;
  uint_t           loop_start;
  uint_t           loop_len;
  bool_t           need_sync;
  bool_t           mute;
  list_iterator_t  current_tickev;
  pthread_rwlock_t lock;
  list_t           trash;
  bool_t           deleted;
  bool_t           play_pending_notes;
  byte_t           notes_on_state[256];
} track_ctx_t;

uint_t     trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick);
void       play_trackctx(uint_t tick,
                         track_ctx_t *track_ctx);

track_ctx_t *engine_create_trackctx(engine_ctx_t *engine, char *name);
bool_t      engine_delete_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
track_ctx_t *engine_copy_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
void        engine_read_midifile(engine_ctx_t *engine, midifile_t *midifile);
void        engine_save_project(engine_ctx_t *engine, char *file_path);
void        engine_prepare_tracklist(engine_ctx_t *ctx);
void        engine_clean_tracklist(engine_ctx_t *ctx);
void        _engine_free_trash(engine_ctx_t *ctx);

void   play_outputs_reqs(engine_ctx_t *ctx);
void   play_tracks_pending_notes(engine_ctx_t *ctx);
void   play_tracks(engine_ctx_t *ctx);

void   output_play_reqlist(output_t *output);
void   output_evlist(output_t *output,
                     list_t *seqevlist,
                     byte_t *notes_on_state);
void   output_pending_notes(output_t *output, byte_t *notes_on_state);

bool_t nns_init_engine(engine_ctx_t *ctx, char *name);
bool_t jbe_init_engine(engine_ctx_t *ctx, char *name);

bool_t init_engine(engine_ctx_t *engine, char *name, int type);
void   uninit_engine(engine_ctx_t *engine);


# include "ev_iterator.h"

typedef struct
{
  list_iterator_t evit;
  list_iterator_t tickit;
} trash_ctn_t;

void            trackctx_event2trash(track_ctx_t *traxkctx,
                                     ev_iterator_t *ev_iterator);

#endif
