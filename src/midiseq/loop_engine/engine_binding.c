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

#include "./engine.h"

void free_binding(void *addr)
{
  binding_t *binding = addr;

  free_list_node(&(binding->tracks), NULL);
  free(binding);
}

void engine_clear_all_bindings(engine_ctx_t *engine)
{
  free_list_node(&(engine->bindings.mute_keypress),
                 free_binding);
  free_list_node(&(engine->bindings.rec_keypress),
                 free_binding);

  engine->bindings.midib_updating = MSQ_TRUE;
  while (engine->bindings.midib_reading == MSQ_TRUE)
    usleep(100000);
  free_list_node(&(engine->bindings.mute_notepress),
                 free_binding);
  free_list_node(&(engine->bindings.mute_programpress),
                 free_binding);
  free_list_node(&(engine->bindings.rec_notepress),
                 free_binding);
  free_list_node(&(engine->bindings.rec_programpress),
                 free_binding);
  engine->bindings.midib_updating = MSQ_FALSE;
}

list_t *_search_bindings_one_val(list_t *bindings, byte_t val)
{
  list_iterator_t iter = {};
  binding_t *binding = NULL;

  for (iter_init(&iter, bindings);
       iter_node(&iter) != NULL;
       iter_next(&iter))
    {
      binding = (binding_t *) iter_node_ptr(&iter);
      if (binding->vals[0] == val)
        {
          if (binding->tracks.len > 0)
            return &(binding->tracks);
          else
            return NULL;
        }
    }
  return NULL;
}

list_t *_search_bindings_two_val(list_t *bindings, byte_t val1, byte_t val2)
{
  list_iterator_t iter = {};
  binding_t *binding = NULL;

  for (iter_init(&iter, bindings);
       iter_node(&iter) != NULL;
       iter_next(&iter))
    {
      binding = (binding_t *) iter_node_ptr(&iter);
      if (binding->vals[0] == val1 && binding->vals[1] == val2)
        {
          if (binding->tracks.len > 0)
            return &(binding->tracks);
          else
            return NULL;
        }
    }
  return NULL;
}

void _add_binding_one_val(list_t *bindings, byte_t val, track_ctx_t *track_ctx)
{
  list_iterator_t iter;
  list_t          *tracks = NULL;
  track_ctx_t     *track_ctx_ptr = NULL;
  binding_t       *binding = NULL;

  tracks = _search_bindings_one_val(bindings, val);
  if (tracks != NULL)
    {
      /* Check if binding already exist */
      for (iter_init(&iter, tracks);
           iter_node(&iter) != NULL;
           iter_next(&iter))
        {
          track_ctx_ptr = iter_node_ptr(&iter);
          if (track_ctx_ptr == track_ctx)
            return;
        }
      push_to_list_tail(tracks, track_ctx);
    }
  else
    {
      binding = myalloc(sizeof (binding_t));
      binding->vals[0] = val;
      binding->vals[1] = 0;
      push_to_list_tail(&(binding->tracks), track_ctx);
      push_to_list_tail(bindings, binding);
    }
}

void _add_binding_two_val(list_t *bindings,
                          byte_t val1,
                          byte_t val2,
                          track_ctx_t *track_ctx)
{
  list_iterator_t iter;
  list_t          *tracks = NULL;
  track_ctx_t     *track_ctx_ptr = NULL;
  binding_t       *binding = NULL;

  tracks = _search_bindings_two_val(bindings, val1, val2);
  if (tracks != NULL)
    {
      /* Check if binding already exist */
      for (iter_init(&iter, tracks);
           iter_node(&iter) != NULL;
           iter_next(&iter))
        {
          track_ctx_ptr = iter_node_ptr(&iter);
          if (track_ctx_ptr == track_ctx)
            return;
        }
      push_to_list_tail(tracks, track_ctx);
    }
  else
    {
      binding = myalloc(sizeof (binding_t));
      binding->vals[0] = val1;
      binding->vals[1] = val2;
      push_to_list_tail(&(binding->tracks), track_ctx);
      push_to_list_tail(bindings, binding);
    }
}

void _del_track_bindings(list_t *bindings, track_ctx_t *track_ctx)
{
  list_iterator_t iter_tracks;
  list_iterator_t iter_bindings;
  track_ctx_t     *track_ctx_ptr = NULL;
  binding_t       *binding = NULL;

  for (iter_init(&iter_bindings, bindings);
       iter_node(&iter_bindings) != NULL;
       iter_next(&iter_bindings))
    {
      binding = iter_node_ptr(&iter_bindings);
      for (iter_init(&iter_tracks, &(binding->tracks));
           iter_node(&iter_tracks) != NULL;
           iter_next(&iter_tracks))
        {
          track_ctx_ptr = iter_node_ptr(&iter_tracks);
          if (track_ctx_ptr == track_ctx)
            {
              iter_node_del(&iter_tracks, NULL);
              break;
            }
        }
    }
}

#include <stdio.h>

void _tracklist_call_mute(list_t *tracks)
{
  list_iterator_t iter;
  track_ctx_t     *track_ctx = NULL;

  for (iter_init(&iter, tracks);
       iter_node(&iter) != NULL;
       iter_next(&iter))
    {
      track_ctx = iter_node_ptr(&iter);
      trackctx_toggle_mute(track_ctx);
    }
}

void _tracklist_call_first_track_rec(engine_ctx_t *engine_ctx,
                                     list_t *tracks)
{
  track_ctx_t     *track_ctx = NULL;

  track_ctx = tracks->head->addr;
  if (engine_ctx->rec == MSQ_TRUE)
    {
      if (engine_ctx->track_rec == track_ctx)
        engine_ctx->rec = MSQ_FALSE;
      else
        engine_ctx->track_rec = track_ctx;
    }
  else
    {
      engine_ctx->rec = MSQ_TRUE;
      engine_ctx->track_rec = track_ctx;
    }
}

void _safe_add_binding_one_val(engine_ctx_t *engine,
                               list_t *binding_list,
                               byte_t val,
                               track_ctx_t *track_ctx)
{
  engine->bindings.midib_updating = MSQ_TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == MSQ_TRUE)
    usleep(100000);
  _add_binding_one_val(binding_list, val, track_ctx);
  engine->bindings.midib_updating = MSQ_FALSE;
}

void _safe_add_one_track_binding_one_val(engine_ctx_t *engine,
                                         list_t *binding_list,
                                         byte_t val,
                                         track_ctx_t *track_ctx)
{
  list_t    *tracks = NULL;
  binding_t *binding = NULL;

  engine->bindings.midib_updating = MSQ_TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == MSQ_TRUE)
    usleep(100000);

  tracks = _search_bindings_one_val(binding_list, val);
  if (tracks != NULL)
    tracks->head->addr = track_ctx;
  else
    {
      binding = myalloc(sizeof (binding_t));
      binding->vals[0] = val;
      binding->vals[1] = 0;
      push_to_list_tail(&(binding->tracks), track_ctx);
      push_to_list_tail(binding_list, binding);
    }

  engine->bindings.midib_updating = MSQ_FALSE;
}

void _safe_add_binding_two_val(engine_ctx_t *engine,
                               list_t *binding_list,
                               byte_t val1,
                               byte_t val2,
                               track_ctx_t *track_ctx)
{
  engine->bindings.midib_updating = MSQ_TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == MSQ_TRUE)
    usleep(100000);
  _add_binding_two_val(binding_list, val1, val2, track_ctx);
  engine->bindings.midib_updating = MSQ_FALSE;
}

void _safe_add_one_track_binding_two_val(engine_ctx_t *engine,
                                         list_t *binding_list,
                                         byte_t val1,
                                         byte_t val2,
                                         track_ctx_t *track_ctx)
{
  list_t          *tracks = NULL;
  binding_t       *binding = NULL;

  engine->bindings.midib_updating = MSQ_TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == MSQ_TRUE)
    usleep(100000);

  tracks = _search_bindings_two_val(binding_list, val1, val2);
  if (tracks != NULL)
    tracks->head->addr = track_ctx;
  else
    {
      binding = myalloc(sizeof (binding_t));
      binding->vals[0] = val1;
      binding->vals[1] = val2;
      push_to_list_tail(&(binding->tracks), track_ctx);
      push_to_list_tail(binding_list, binding);
    }

  engine->bindings.midib_updating = MSQ_FALSE;
}

void engine_del_track_bindings(engine_ctx_t *engine,
                               track_ctx_t *track_ctx)
{
  engine->bindings.midib_updating = MSQ_TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == MSQ_TRUE)
    usleep(100000);

  _del_track_bindings(&(engine->bindings.mute_notepress), track_ctx);
  _del_track_bindings(&(engine->bindings.mute_programpress), track_ctx);
  _del_track_bindings(&(engine->bindings.mute_controlchg), track_ctx);
  _del_track_bindings(&(engine->bindings.rec_notepress), track_ctx);
  _del_track_bindings(&(engine->bindings.rec_programpress), track_ctx);
  _del_track_bindings(&(engine->bindings.rec_controlchg), track_ctx);

  engine->bindings.midib_updating = MSQ_FALSE;

  _del_track_bindings(&(engine->bindings.mute_keypress), track_ctx);
  _del_track_bindings(&(engine->bindings.rec_keypress), track_ctx);
}

void _engine_call_bindings_one_val(engine_ctx_t *engine,
                                   list_t *bindings_mute,
                                   list_t *bindings_rec,
                                   byte_t val)
{
  list_t          *tracks = NULL;

  engine->bindings.midib_reading = MSQ_TRUE;
  if (engine->bindings.midib_updating != MSQ_TRUE)
    {
      tracks = _search_bindings_one_val(bindings_mute, val);
      if (tracks != NULL)
        {
          _tracklist_call_mute(tracks);
          engine->mute_state_changed = MSQ_TRUE;
        }
      tracks = _search_bindings_one_val(bindings_rec, val);
      if (tracks != NULL)
        {
          _tracklist_call_first_track_rec(engine, tracks);
          engine->rec_state_changed = MSQ_TRUE;
        }
    }
  engine->bindings.midib_reading = MSQ_FALSE;
}

void engine_call_notepress_b(engine_ctx_t *engine, byte_t note)
{
  _engine_call_bindings_one_val(engine,
                                &(engine->bindings.mute_notepress),
                                &(engine->bindings.rec_notepress),
                                note);
}

void engine_call_programpress_b(engine_ctx_t *engine, byte_t program)
{
  _engine_call_bindings_one_val(engine,
                                &(engine->bindings.mute_programpress),
                                &(engine->bindings.rec_programpress),
                                program);
}

void engine_call_controlchg_b(engine_ctx_t *engine,
                              byte_t ctrl_num,
                              byte_t val)
{
  list_t          *tracks = NULL;

  engine->bindings.midib_reading = MSQ_TRUE;
  if (engine->bindings.midib_updating != MSQ_TRUE)
    {
      tracks = _search_bindings_two_val(&(engine->bindings.mute_controlchg),
                                        ctrl_num,
                                        val);
      if (tracks != NULL)
        {
          _tracklist_call_mute(tracks);
          engine->mute_state_changed = MSQ_TRUE;
        }
      tracks = _search_bindings_two_val(&(engine->bindings.rec_controlchg),
                                        ctrl_num,
                                        val);
      if (tracks != NULL)
        {
          _tracklist_call_first_track_rec(engine, tracks);
          engine->rec_state_changed = MSQ_TRUE;
        }
    }
  engine->bindings.midib_reading = MSQ_FALSE;
}

msq_bool_t engine_call_keypress_b(engine_ctx_t *engine, byte_t key)
{
  list_t          *tracks = NULL;

  tracks = _search_bindings_one_val(&(engine->bindings.mute_keypress), key);
  if (tracks != NULL)
    {
      _tracklist_call_mute(tracks);
      engine->mute_state_changed = MSQ_TRUE;
    }
  tracks = _search_bindings_one_val(&(engine->bindings.rec_keypress), key);
  if (tracks != NULL)
    {
      _tracklist_call_first_track_rec(engine,
                                      tracks);
      engine->rec_state_changed = MSQ_TRUE;
    }
  return engine->mute_state_changed || engine->rec_state_changed;
}

#include <string.h>

size_t _fill_byte_array_w_track_bindings_one_val(byte_t *byte_array,
                                                 size_t max_sz,
                                                 list_t *bindings,
                                                 track_ctx_t *trackctx)
{
  list_iterator_t iter_binding, iter_track;
  uint_t          idx = 0;
  track_ctx_t     *trackctx_ptr = NULL;
  binding_t       *binding = NULL;

  memset(byte_array, 0, max_sz);
  for (iter_init(&iter_binding, bindings), idx = 0;
       iter_node(&iter_binding) && idx < max_sz;
       iter_next(&iter_binding))
    {
      binding = iter_node_ptr(&iter_binding);
      for (iter_init(&iter_track, &(binding->tracks));
           iter_node(&iter_track);
           iter_next(&iter_track))
        {
          trackctx_ptr = iter_node_ptr(&iter_track);
          if (trackctx == trackctx_ptr)
            {
              byte_array[idx] = binding->vals[0];
              idx++;
              break;
            }
        }
    }
  return idx;
}

size_t _fill_byte_array_w_track_bindings_two_val(byte_t *byte_array,
                                                 size_t max_sz,
                                                 list_t *bindings,
                                                 track_ctx_t *trackctx)
{
  list_iterator_t iter_binding, iter_track;
  size_t          idx = 0;
  track_ctx_t     *trackctx_ptr = NULL;
  binding_t       *binding = NULL;

  memset(byte_array, 0, max_sz);
  for (iter_init(&iter_binding, bindings), idx = 0;
       iter_node(&iter_binding) && idx < max_sz;
       iter_next(&iter_binding))
    {
      binding = iter_node_ptr(&iter_binding);
      for (iter_init(&iter_track, &(binding->tracks));
           iter_node(&iter_track);
           iter_next(&iter_track))
        {
          trackctx_ptr = iter_node_ptr(&iter_track);
          if (trackctx == trackctx_ptr)
            {
              byte_array[idx] = binding->vals[0];
              byte_array[idx + 1] = binding->vals[1];
              idx += 2;
              break;
            }
        }
    }
  return idx;
}
