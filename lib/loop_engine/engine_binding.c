#include "./engine.h"

typedef struct
{
  byte_t val;
  list_t tracks;
} binding_t;

void free_binding(void *addr)
{
  binding_t *binding = addr;

  free_list_node(&(binding->tracks), NULL);
}

void engine_clear_all_bindings(engine_ctx_t *engine)
{
  free_list_node(&(engine->bindings.keypress),
                 free_binding);

  engine->bindings.midib_updating = TRUE;
  while (engine->bindings.midib_reading == TRUE)
    usleep(100000);
  free_list_node(&(engine->bindings.notepress),
                 free_binding);
  engine->bindings.midib_updating = FALSE;
}

list_t *_search_bindings(list_iterator_t *iter, byte_t val)
{
  binding_t *binding = NULL;

  while (iter_node(iter) != NULL)
    {
      binding = (binding_t *) iter_node_ptr(iter);
      if (binding->val == val)
        return &(binding->tracks);
      iter_next(iter);
    }
  return NULL;
}

void _add_binding(list_t *bindings, byte_t val, track_ctx_t *track_ctx)
{
  list_iterator_t iter;
  list_t          *tracks = NULL;
  track_ctx_t     *track_ctx_ptr = NULL;
  binding_t       *binding = NULL;

  iter_init(&iter, bindings);
  tracks = _search_bindings(&iter, val);
  if (tracks != NULL)
    {
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
      binding->val = val;
      push_to_list_tail(&(binding->tracks), track_ctx);
      iter_push_after(&iter, binding);
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
              return;
            }
        }
    }
}

bool_t _call_binding(list_t *bindings, byte_t val)
{
  list_iterator_t iter;
  list_t          *tracks = NULL;
  track_ctx_t     *track_ctx = NULL;
  bool_t          mute_state_changed = FALSE;

  iter_init(&iter, bindings);
  tracks = _search_bindings(&iter, val);
  if (tracks != NULL)
    {
      for (iter_init(&iter, tracks);
           iter_node(&iter) != NULL;
           iter_next(&iter))
        {
          track_ctx = iter_node_ptr(&iter);
          trackctx_toggle_mute(track_ctx);
          mute_state_changed = TRUE;
        }
    }
  return mute_state_changed;
}

void engine_add_notebinding(engine_ctx_t *engine,
                            byte_t note,
                            track_ctx_t *track_ctx)
{
  engine->bindings.midib_updating = TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == TRUE)
    usleep(100000);
  _add_binding(&(engine->bindings.notepress), note, track_ctx);
  engine->bindings.midib_updating = FALSE;
}

void engine_del_track_bindings(engine_ctx_t *engine,
                               track_ctx_t *track_ctx)
{
  engine->bindings.midib_updating = TRUE;
  /* Waiting if midi input port call bindings */
  while (engine->bindings.midib_reading == TRUE)
    usleep(100000);
  _del_track_bindings(&(engine->bindings.notepress), track_ctx);
  engine->bindings.midib_updating = FALSE;

  _del_track_bindings(&(engine->bindings.keypress), track_ctx);
}

void engine_call_notepress_b(engine_ctx_t *engine, byte_t note)
{
  engine->bindings.midib_reading = TRUE;
  if (engine->bindings.midib_updating != TRUE)
    engine->mute_state_changed = _call_binding(&(engine->bindings.notepress),
                                               note);
  engine->bindings.midib_reading = FALSE;
}

void engine_add_keybinding(engine_ctx_t *engine,
                           byte_t key,
                           track_ctx_t *track_ctx)
{
  _add_binding(&(engine->bindings.keypress), key, track_ctx);
}

void engine_call_keypress_b(engine_ctx_t *engine, byte_t key)
{
  engine->mute_state_changed = _call_binding(&(engine->bindings.keypress), key);
}
