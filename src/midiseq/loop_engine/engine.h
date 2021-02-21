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


#ifndef __ENGINE_H
#define __ENGINE_H

#include <pthread.h>

#include "midi/midifile.h"

typedef struct
{
  msq_bool_t used;
  midicev_t  midicev;
} midireq_t;

typedef struct midioutput
{
  void            *hdl;
  list_t          req_list;
  pthread_mutex_t req_lock;
  const char      *(*get_name)(void *hdl);
  void            (*set_name)(void *hdl, const char *name);
  /* is_handled should disapear */
  msq_bool_t      (*write)(struct midioutput *output,
                           midicev_t *midicev);
  /* _write must be always serialized and in the engine thread */
  msq_bool_t      (*_write)(struct midioutput *output, midicev_t *midicev);
} output_t;

#define output_get_name(output)        (output)->get_name((output)->hdl)
#define output_set_name(output, name)  (output)->set_name((output)->hdl, name)
#define output_write(output, midicev)           \
  (output)->write(output, midicev)
#define _output_write(output, midicev) (output)->_write(output, midicev)

void output_add_req(output_t *output, midicev_t *midicev);

typedef struct
{
  byte_t vals[2];
  list_t tracks;
} binding_t;

typedef enum
  {
    MSQ_MIDI_WAIT_NONE = 0,
    MSQ_MIDI_WAIT_NOTE,
    MSQ_MIDI_WAIT_PROG,
    MSQ_MIDI_WAIT_CTRL,
    MSQ_MIDI_WAIT_ENABLE
  } msq_binding_state_t;

typedef struct
{
  /* TODO: Modify rec_val check 255 api with an enum to filter prog and note
     midi binding rec */
  msq_binding_state_t state;
  byte_t              shr_rec_vals[2];
  msq_bool_t          midib_updating;
  msq_bool_t          midib_reading;
  list_t              mute_notepress;
  list_t              mute_programpress;
  list_t              mute_controlchg;
  list_t              mute_keypress;
  list_t              rec_notepress;
  list_t              rec_programpress;
  list_t              rec_controlchg;
  list_t              rec_keypress;
} msq_bindings_t;

typedef struct
{
  uint      tick;
  midicev_t ev;
} midirec_t;

typedef struct
{
  midirec_t  *buff;
  midirec_t  *last;
  midirec_t  *wptr;
  midirec_t  *rptr;
  msq_bool_t max;
} midiringbuffer_t;

void             free_midiringbuff(midiringbuffer_t *mrb);
midiringbuffer_t *init_midiringbuff(uint_t size);
msq_bool_t       mrb_write(midiringbuffer_t *rbuff,
                           uint tick,
                           midicev_t *ev);
msq_bool_t       mrb_read(midiringbuffer_t *rbuff,
                          uint *tick,
                          midicev_t *ev);

typedef enum {
  NOSAVE_RQ = 0,
  SAVE_TPL_RQ,
  SAVE_RQ,
  SAVE_N_QUIT_RQ
} saverq_t;

#define MSQ_ENG_ALSA 0
#define MSQ_ENG_JACK 1

typedef struct engine_ctx_s
{
  char             *name;
  byte_t           type;
  void             *hdl;
  list_t           output_list;
  list_t           track_list;
  msq_bool_t       rec;
  void             *track_rec;
  midiringbuffer_t *rbuff;
  uint_t           ppq;               /* Pulse per quater note (beat) */
  uint_t           tempo;             /* Quater note in micro second */
  msq_bindings_t   bindings;
  msq_bool_t       mute_state_changed; /* Ask to update interface */
  msq_bool_t       rec_state_changed;  /* Ask for recording track */
  saverq_t         saverq;
  char             savepath[256];
  msq_bool_t       (*is_running)(struct engine_ctx_s *engine);
  void             (*destroy_hdl)(struct engine_ctx_s *engine);
  void             (*start)(struct engine_ctx_s *engine);
  void             (*stop)(struct engine_ctx_s *engine);
  void             (*create_output)(struct engine_ctx_s *engine,
                                    output_t *output,
                                    const char *name);
  void             (*delete_output_node)(struct engine_ctx_s *engine,
                                         output_t *output);
  uint_t           (*get_tick)(struct engine_ctx_s *engine);
  void             (*set_tick)(struct engine_ctx_s *engine, uint_t tick);
  void             (*set_tempo)(struct engine_ctx_s *engine, uint_t ms);
} engine_ctx_t;

#define engine_is_running(eng)     (eng)->is_running(eng)
#define engine_destroy_hdl(eng)    (eng)->destroy_hdl(eng)
#define engine_start(eng)          (eng)->start(eng)
#define engine_stop(eng)           (eng)->stop(eng)
#define engine_get_tick(eng)       (eng)->get_tick(eng)
#define engine_set_tick(eng, tick) (eng)->set_tick(eng, tick)
#define engine_set_tempo(eng, ms)  (eng)->set_tempo(eng, ms)

output_t   *engine_create_output(engine_ctx_t *ctx, const char *name);
msq_bool_t engine_delete_output(engine_ctx_t *ctx, output_t *output);

typedef struct
{
  engine_ctx_t     *engine;
  output_t         *output;
  track_t          *track;
  uint_t           loop_start;
  uint_t           loop_len;
  msq_bool_t       need_sync;
  msq_bool_t       has_changed;
  msq_bool_t       mute;
  list_iterator_t  current_tickev;
  pthread_rwlock_t lock;
  list_t           trash;
  msq_bool_t       deleted;
  msq_bool_t       play_pending_notes;
  byte_t           notes_on_state[256];
} track_ctx_t;

void   _trackctx_mute(track_ctx_t *track_ctx);
void   trackctx_toggle_mute(track_ctx_t *track_ctx);
uint_t trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick);
void   play_trackctx(uint_t tick,
                     track_ctx_t *track_ctx);

void engine_flush_rbuff(engine_ctx_t *engine);

track_ctx_t *engine_create_trackctx(engine_ctx_t *engine, char *name);
msq_bool_t  engine_delete_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
track_ctx_t *engine_copy_trackctx(engine_ctx_t *engine, track_ctx_t *trackctx);
void        engine_read_midifile(engine_ctx_t *engine, midifile_t *midifile);
void        engine_save_project(engine_ctx_t *engine,
                                const char *file_path,
                                msq_bool_t is_template);
midifile_t *engine_gen_midifile_struct(engine_ctx_t *ctx);
/* void gen_miditrack_info(char *retstr, */
/*                         engine_ctx_t *ctx, */
/*                         track_ctx_t *trackctx); */
void        engine_prepare_tracklist(engine_ctx_t *ctx);
void        engine_clean_tracklist(engine_ctx_t *ctx);
void        _engine_free_trash(engine_ctx_t *ctx);

void engine_del_track_bindings(engine_ctx_t *engine, track_ctx_t *track_ctx);
void engine_clear_all_bindings(engine_ctx_t *engine);
void engine_call_notepress_b(engine_ctx_t *engine, byte_t key);
void engine_call_programpress_b(engine_ctx_t *engine, byte_t key);
void engine_call_controlchg_b(engine_ctx_t *engine, byte_t ctrl, byte_t val);
void _add_binding_one_val(list_t *bindings, byte_t val, track_ctx_t *track_ctx);
void _add_binding_two_val(list_t *bindings,
                          byte_t val1,
                          byte_t val2,
                          track_ctx_t *track_ctx);
void _safe_add_binding_one_val(engine_ctx_t *engine,
                               list_t *binding_list,
                               byte_t val,
                               track_ctx_t *track_ctx);
void _safe_add_one_track_binding_one_val(engine_ctx_t *engine,
                                         list_t *binding_list,
                                         byte_t val,
                                         track_ctx_t *track_ctx);
void _safe_add_binding_two_val(engine_ctx_t *engine,
                               list_t *binding_list,
                               byte_t val1,
                               byte_t val2,
                               track_ctx_t *track_ctx);
void _safe_add_one_track_binding_two_val(engine_ctx_t *engine,
                                         list_t *binding_list,
                                         byte_t val1,
                                         byte_t val2,
                                         track_ctx_t *track_ctx);
#define engine_add_mute_keybinding(_engine, _key, _track_ctx)           \
  _safe_add_binding_one_val((_engine),                                  \
                            &((_engine)->bindings.mute_keypress),       \
                            (_key),                                     \
                            (_track_ctx))
#define engine_add_mute_notebinding(_engine, _note, _track_ctx)         \
  _safe_add_binding_one_val((_engine),                                  \
                            &((_engine)->bindings.mute_notepress),      \
                            (_note),                                    \
                            (_track_ctx))
#define engine_add_mute_programbinding(_engine, _val, _track_ctx)       \
  _safe_add_binding_one_val((_engine),                                  \
                            &((_engine)->bindings.mute_programpress),   \
                            (_val),                                     \
                            (_track_ctx))
#define engine_add_mute_controlbinding(_engine, _ctrl_num, _val, _track_ctx) \
  _safe_add_binding_two_val((_engine),                                  \
                            &((_engine)->bindings.mute_controlchg),     \
                            (_ctrl_num),                                \
                            (_val),                                     \
                            (_track_ctx))
#define engine_add_rec_keybinding(_engine, _key, _track_ctx)            \
  _safe_add_one_track_binding_one_val((_engine),                        \
                                      &((_engine)->bindings.rec_keypress), \
                                      (_key),                           \
                                      (_track_ctx))
#define engine_add_rec_notebinding(_engine, _note, _track_ctx)          \
  _safe_add_one_track_binding_one_val((_engine),                        \
                                      &((_engine)->bindings.rec_notepress), \
                                      (_note),                          \
                                      (_track_ctx))
#define engine_add_rec_programbinding(_engine, _val, _track_ctx)        \
  _safe_add_one_track_binding_one_val((_engine),                        \
                                      &((_engine)->bindings.rec_programpress), \
                                      (_val),                           \
                                      (_track_ctx))
#define engine_add_rec_controlbinding(_engine, _ctrl_num, _val, _track_ctx) \
  _safe_add_one_track_binding_two_val((_engine),                        \
                                      &((_engine)->bindings.rec_controlchg), \
                                      (_ctrl_num),                      \
                                      (_val),                           \
                                      (_track_ctx))

msq_bool_t engine_call_keypress_b(engine_ctx_t *engine, byte_t key);
size_t _fill_byte_array_w_track_bindings_one_val(byte_t *byte_array,
                                                 size_t max_sz,
                                                 list_t *bindings,
                                                 track_ctx_t *trackctx);
size_t _fill_byte_array_w_track_bindings_two_val(byte_t *byte_array,
                                                 size_t max_sz,
                                                 list_t *bindings,
                                                 track_ctx_t *trackctx);

void free_output_list(engine_ctx_t *ctx);

void play_outputs_reqs(engine_ctx_t *ctx);
void play_tracks_pending_notes(engine_ctx_t *ctx);
void play_tracks(engine_ctx_t *ctx);
void engine_set_tracks_need_sync(engine_ctx_t *ctx);

void output_play_reqlist(output_t *output);
void output_evlist(output_t *output,
                   list_t *seqevlist,
                   byte_t *notes_on_state);
void output_pending_notes(output_t *output, byte_t *notes_on_state);

byte_t     engine_get_sysex_mmc(engine_ctx_t *ctx, byte_t *sysex, uint_t size);
void engine_toggle_rec(engine_ctx_t *ctx);

msq_bool_t nns_init_engine(engine_ctx_t *ctx, char *name);
msq_bool_t jbe_init_engine(engine_ctx_t *ctx, char *name);

msq_bool_t init_engine(engine_ctx_t *engine,
                       char *name,
                       byte_t type);
void       uninit_engine(engine_ctx_t *engine);

typedef struct
{
  list_iterator_t evit;
  list_iterator_t tickit;
} trash_ctn_t;

#include "seqtool/ev_iterator.h"

void trackctx_event2trash(track_ctx_t *traxkctx,
                          ev_iterator_t *ev_iterator);

void trackctx_set_name(track_ctx_t *traxkctx, const char *name);

void gen_key_bindings_str(char *kb_str,
                          byte_t *notes,
                          size_t notes_sz);

void gen_midinote_bindings_str(char *mnb_str,
                               byte_t *notes,
                               size_t notes_sz);

void gen_midiprogram_bindings_str(char *mpb_str,
                                  byte_t *programs,
                                  size_t programs_sz);

void gen_midicontrol_bindings_str(char *mpb_str,
                                  byte_t *controls,
                                  size_t controls_sz);

char **engine_gen_output_str_list(engine_ctx_t *engine_ctx,
                                  size_t *str_list_len);

output_t *engine_get_output(engine_ctx_t *engine_ctx,
                            size_t idx);

#endif
