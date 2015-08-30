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


#include "./engine.h"
#include "debug_tool/debug_tool.h"

void _free_trash_ctn(void *addr)
{
  trash_ctn_t *ctn = (trash_ctn_t *) addr;

  iter_node_del(&(ctn->evit), free_seqev);
  if (ctn->evit.list->len <= 0)
    iter_node_del(&(ctn->tickit), free_tickev);
  free(ctn);
}

void _free_trackctx(void *addr)
{
  track_ctx_t *trackctx = (track_ctx_t  *) addr;

  pthread_rwlock_destroy(&(trackctx->lock));
  if (trackctx->trash.len)
    free_list_node(&(trackctx->trash), _free_trash_ctn);
  free_track(trackctx->track);
  free(trackctx);
}

void _engine_free_trash(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;

  iter_init(&trackit, &(ctx->track_list));
  while (iter_node(&trackit) != NULL)
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx->deleted == TRUE)
        iter_node_del(&trackit, _free_trackctx); /* /!\ Memory corruption while asking tracklist */
      else
        {
          if (track_ctx->trash.len &&
              pthread_rwlock_trywrlock(&(track_ctx->lock)) == 0)
            {
              free_list_node(&(track_ctx->trash), _free_trash_ctn);
              track_ctx->need_sync = TRUE;
              pthread_rwlock_unlock(&(track_ctx->lock));
            }
          iter_next(&trackit);
        }
    }
}

void play_tracks_pending_notes(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx->deleted == FALSE
          && track_ctx->output != NULL)
        output_pending_notes(track_ctx->output, track_ctx->notes_on_state);
    }
}

void play_tracks(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx  = NULL;
  list_iterator_t trackit;
  uint_t          tick = engine_get_tick(ctx);

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx->deleted != TRUE)
        play_trackctx(tick, track_ctx);
    }
}

bool_t engine_delete_trackctx(engine_ctx_t *ctx, track_ctx_t *trackctx)
{
  list_iterator_t trackit;
  track_ctx_t     *track_ctx = NULL;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx == trackctx)
        {
          if (engine_is_running(ctx) == TRUE)
            track_ctx->deleted = TRUE;
          else
            iter_node_del(&trackit, _free_trackctx);
          /* free_track(track_ctx); */
          return TRUE;
        }
    }
  return FALSE;
}

track_ctx_t *_engine_gen_trackctx(engine_ctx_t *ctx,
                                  track_t *track,
                                  uint_t loop_start,
                                  uint_t loop_len)
{
  track_ctx_t *trackctx = myalloc(sizeof (track_ctx_t));

  trackctx->engine = ctx;
  trackctx->track = track;
  trackctx->loop_start = loop_start;
  trackctx->loop_len = loop_len;
  trackctx->mute = FALSE;
  trackctx->deleted = FALSE;
  pthread_rwlock_init(&(trackctx->lock), NULL);
  iter_init(&(trackctx->current_tickev), &(trackctx->track->tickev_list));
  return trackctx;
}

#include <string.h>
track_ctx_t *engine_copy_trackctx(engine_ctx_t *ctx,
                                   track_ctx_t *trackctx_src)
{
  track_t      *track = myalloc(sizeof (track_t));
  track_ctx_t  *trackctx_dst = NULL;

  copy_track_list(trackctx_src->track, track);
  if (trackctx_src->track->name)
    track->name = strdup(trackctx_src->track->name);
  trackctx_dst = _engine_gen_trackctx(ctx,
                                      track,
                                      trackctx_src->loop_start,
                                      trackctx_src->loop_len);
  push_to_list_tail(&(ctx->track_list), trackctx_dst);
  return trackctx_dst;
}

track_ctx_t *engine_copyadd_miditrack(engine_ctx_t *ctx, midifile_track_t *mtrack)
{
  track_ctx_t *trackctx = NULL;
  track_t     *track = myalloc(sizeof (track_t));
  uint_t      loop_start;
  uint_t      loop_len;

  copy_track_list(&(mtrack->track), track);
  if (mtrack->track.name)
    track->name = strdup(mtrack->track.name);
  loop_start = mtrack->sysex_loop_start;
  if (mtrack->sysex_loop_len == 0)
    loop_len = ctx->ppq * 4;
  else
    loop_len = mtrack->sysex_loop_len;
  trackctx = _engine_gen_trackctx(ctx,
                                  track,
                                  loop_start,
                                  loop_len);
  push_to_list_tail(&(ctx->track_list), trackctx);
  return trackctx;
}

typedef struct
{
  int  id;
  void *output;
} tmpport_cache_t;

void engine_read_midifile(engine_ctx_t *ctx, midifile_t *midifile)
{
  list_iterator_t     trackit;
  list_iterator_t     portit;
  list_t              tmpport = {NULL, NULL, 0};
  midifile_track_t    *mf_track = NULL;
  midifile_portinfo_t *portinfo = NULL;
  tmpport_cache_t     *portcache = NULL;
  track_ctx_t         *trackctx = NULL;

  ctx->ppq = midifile->info.ppq;
  engine_set_tempo(ctx, midifile->info.tempo);

  for (iter_init(&portit, &(midifile->info.portinfo_list));
       iter_node(&portit) != NULL;
       iter_next(&portit))
    {
      portinfo = iter_node_ptr(&portit);
      portcache = myalloc(sizeof (tmpport_cache_t));
      portcache->output = engine_create_output(ctx, portinfo->name);
      portcache->id = portinfo->id;
      push_to_list_tail(&tmpport, portcache);
    }

  for (iter_init(&trackit, &(midifile->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      mf_track = iter_node_ptr(&trackit);
      trackctx = engine_copyadd_miditrack(ctx, mf_track);
      if (mf_track->sysex_portid != -1)
        {
          for (iter_init(&portit, &tmpport);
               iter_node(&portit) != NULL;
               iter_next(&portit))
            {
              portcache = iter_node_ptr(&portit);
              if (portcache->id == mf_track->sysex_portid)
                {
                  trackctx->output = portcache->output;
                  break;
                }
            }
        }
    }
  free_list_node(&tmpport, free);
}

track_ctx_t  *engine_create_trackctx(engine_ctx_t *ctx, char *name)
{
  track_ctx_t *trackctx = NULL;
  track_t     *track = myalloc(sizeof (track_t));

  track->name = strdup(name);
  trackctx = _engine_gen_trackctx(ctx, track, 0, ctx->ppq * 4);
  push_to_list_tail(&(ctx->track_list), trackctx);
  return trackctx;
}

void engine_prepare_tracklist(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;
  uint_t          current_tick = engine_get_tick(ctx);

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      goto_next_available_tick(&(track_ctx->current_tickev),
                               current_tick);
      /* iter_head(&(track_ctx->current_tickev)); */
    }
}

void engine_clean_tracklist(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx->deleted == TRUE)
        iter_node_del(&trackit, _free_trackctx);
    }
}

void output_evlist(output_t *output,
                   list_t *seqevlist,
                   byte_t *notes_on_state)
{
  list_iterator_t iter;
  seqev_t         *seqev = NULL;
  midicev_t       *midicev = NULL;

  if (output == NULL)
    return;
  for (iter_init(&iter, seqevlist);
       iter_node(&iter);
       iter_next(&iter))
    {
      seqev = (seqev_t *) iter_node_ptr(&(iter));
      if (seqev->deleted == FALSE && seqev->type == MIDICEV)
        {
          midicev = (midicev_t *) seqev->addr;
          if (_output_write(output, midicev))
            update_pending_notes(notes_on_state, midicev);
        }
    }
}

void output_pending_notes(output_t *output, byte_t *notes_on_state)
{
  uint_t          note_idx, channel_idx;
  midicev_t       mcev;

  mcev.type = NOTEOFF;
  mcev.event.note.val = 0;

  for (channel_idx = 0;
       channel_idx < 16;
       channel_idx++)
    for (note_idx = 0;
         note_idx < 128;
         note_idx++)
      {
        if (is_pending_notes(notes_on_state, channel_idx, note_idx))
          {
            mcev.chan = channel_idx;
            mcev.event.note.num = note_idx;
            _output_write(output, &mcev);
            unset_pending_note(notes_on_state, channel_idx, note_idx);
          }
      }
}

void play_outputs_reqs(engine_ctx_t *ctx)
{
  list_iterator_t output_it;
  output_t        *output = NULL;

  for (iter_init(&output_it, &(ctx->output_list));
       iter_node(&output_it) != NULL;
       iter_next(&output_it))
    {
      output = iter_node_ptr(&output_it);
      output_play_reqlist(output);
    }
}

output_t *engine_create_output(engine_ctx_t *ctx, const char *name)
{
  output_t      *output = myalloc(sizeof (output_t));

  ctx->init_output(ctx, output, name);
  pthread_mutex_init(&(output->req_lock), NULL);
  push_to_list_tail(&(ctx->output_list), output);
  return output;
}

bool_t engine_delete_output(engine_ctx_t *ctx, output_t *output)
{
  list_iterator_t output_it;
  output_t        *ptr = NULL;

  for (iter_init(&output_it, &(ctx->output_list));
       iter_node(&output_it) != NULL;
       iter_next(&output_it))
    {
      ptr = iter_node_ptr(&output_it);
      if (ptr == output)
        {
          free_list_node(&(output->req_list), free);
          pthread_mutex_destroy(&(output->req_lock));
          iter_node_del(&output_it, ctx->free_output_node);
          return TRUE;
        }
    }
  return FALSE;
}

void engine_handle_sysex(engine_ctx_t *ctx, byte_t *sysex, uint_t size)
{
  if (sysex[1] == 0x7F && sysex[3] == 0x06)
    {
      switch (sysex[4])
        {
        case MMC_STOP:
          engine_stop(ctx);
          break;
        case MMC_PLAY:
        case MMC_PAUSE:
          engine_start(ctx);
          break;
        default:
          /* output_warning("Unhandled MMC (id:%d)", sysex[4]); */
          /* print_bin(stdout, sysex, size); */
          ;
        }
    }
}

bool_t init_engine(engine_ctx_t *engine, char *name, int type)
{
  bzero(engine, sizeof (engine_ctx_t));
  engine->rbuff = init_midiringbuff(400);

  if (type == 0)
    {
      if (nns_init_engine(engine, name) != TRUE)
        return FALSE;
    }
  else
    {
      if (jbe_init_engine(engine, name) != TRUE)
        return FALSE;
    }

  /* Setting pulsation to prevent error */
  engine->ppq   = 192;
  engine_set_tempo(engine, 500);
  return TRUE;
}

void uninit_engine(engine_ctx_t *engine)
{
  free_list_node(&(engine->output_list), engine->free_output_node);
  engine_destroy_hdl(engine);
  free_list_node(&(engine->track_list), _free_trackctx);
  engine_clear_all_bindings(engine);
  bzero(engine, sizeof (engine_ctx_t));
}
