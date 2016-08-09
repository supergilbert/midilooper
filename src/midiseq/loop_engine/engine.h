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

typedef struct
{
  byte_t rec_note;
  bool_t midib_updating;
  bool_t midib_reading;
  list_t notepress;
  list_t keypress;
} bindings_t;

typedef struct
{
  uint      tick;
  midicev_t ev;
} midirec_t;

typedef struct
{
  midirec_t *buff;
  midirec_t *last;
  midirec_t *wptr;
  midirec_t *rptr;
  bool_t    max;
} midiringbuffer_t;

void             free_midiringbuff(midiringbuffer_t *mrb);
midiringbuffer_t *init_midiringbuff(uint_t size);
bool_t           mrb_write(midiringbuffer_t *rbuff, uint tick, midicev_t *ev);
bool_t           mrb_read(midiringbuffer_t *rbuff, uint *tick, midicev_t *ev);

typedef struct engine_ctx
{
  void             *hdl;
  list_t           output_list;
  list_t           track_list;
  bool_t           rec;
  void             *track_rec;
  midiringbuffer_t *rbuff;
  uint_t           ppq;               /* Pulse per quater note (beat) */
  uint_t           tempo;             /* Quater note in micro second */
  bindings_t       bindings;
  bool_t           mute_state_changed; /* Ask to update interface */
  bool_t           (*is_running)(struct engine_ctx *engine);
  void             (*destroy_hdl)(struct engine_ctx *engine);
  void             (*start)(struct engine_ctx *engine);
  void             (*stop)(struct engine_ctx *engine);
  void             (*create_output)(struct engine_ctx *engine,
                                    output_t *output,
                                    const char *name);
  void             (*delete_output_node)(struct engine_ctx *engine,
                                         output_t *output);
  uint_t           (*get_tick)(struct engine_ctx *engine);
  void             (*set_tempo)(struct engine_ctx *engine, uint_t ms);
} engine_ctx_t;

#define engine_is_running(eng)    (eng)->is_running(eng)
#define engine_destroy_hdl(eng)   (eng)->destroy_hdl(eng)
#define engine_start(eng)         (eng)->start(eng)
#define engine_stop(eng)          (eng)->stop(eng)
#define engine_get_tick(eng)      (eng)->get_tick(eng)
#define engine_set_tempo(eng, ms) (eng)->set_tempo(eng, ms)

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
  bool_t           has_changed;
  bool_t           mute;
  list_iterator_t  current_tickev;
  pthread_rwlock_t lock;
  list_t           trash;
  bool_t           deleted;
  bool_t           play_pending_notes;
  byte_t           notes_on_state[256];
} track_ctx_t;

void   _trackctx_mute(track_ctx_t *track_ctx);
void   trackctx_toggle_mute(track_ctx_t *track_ctx);
uint_t trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick);
void   play_trackctx(uint_t tick,
                     track_ctx_t *track_ctx);

void engine_flush_rbuff(engine_ctx_t *engine);

track_ctx_t *engine_create_trackctx(engine_ctx_t *engine, char *name);
bool_t      engine_delete_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
track_ctx_t *engine_copy_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
void        engine_read_midifile(engine_ctx_t *engine, midifile_t *midifile);
void        engine_save_project(engine_ctx_t *engine, char *file_path);
void        gen_miditrack_info(char *retstr,
                                engine_ctx_t *ctx,
                                track_ctx_t *trackctx);
void        engine_prepare_tracklist(engine_ctx_t *ctx);
void        engine_clean_tracklist(engine_ctx_t *ctx);
void        _engine_free_trash(engine_ctx_t *ctx);

typedef struct
{
  byte_t val;
  list_t tracks;
} binding_t;

void engine_del_track_bindings(engine_ctx_t *engine, track_ctx_t *track_ctx);
void engine_clear_all_bindings(engine_ctx_t *engine);
void engine_call_notepress_b(engine_ctx_t *engine, byte_t key);
void _add_binding(list_t *bindings, byte_t val, track_ctx_t *track_ctx);
void engine_add_notebinding(engine_ctx_t *engine,
                            byte_t key,
                            track_ctx_t *track_ctx);
void engine_call_keypress_b(engine_ctx_t *engine, byte_t key);
void engine_add_keybinding(engine_ctx_t *engine,
                           byte_t key,
                           track_ctx_t *track_ctx);

void play_outputs_reqs(engine_ctx_t *ctx);
void play_tracks_pending_notes(engine_ctx_t *ctx);
void play_tracks(engine_ctx_t *ctx);

void output_play_reqlist(output_t *output);
void output_evlist(output_t *output,
                   list_t *seqevlist,
                   byte_t *notes_on_state);
void output_pending_notes(output_t *output, byte_t *notes_on_state);

byte_t engine_get_sysex_mmc(engine_ctx_t *ctx, byte_t *sysex, uint_t size);

bool_t nns_init_engine(engine_ctx_t *ctx, char *name);
bool_t jbe_init_engine(engine_ctx_t *ctx, char *name);

bool_t init_engine(engine_ctx_t *engine, char *name, int type);
void   uninit_engine(engine_ctx_t *engine);

typedef struct
{
  list_iterator_t evit;
  list_iterator_t tickit;
} trash_ctn_t;

#include "seqtool/ev_iterator.h"

void            trackctx_event2trash(track_ctx_t *traxkctx,
                                     ev_iterator_t *ev_iterator);

#endif
