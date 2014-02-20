#include "./engine.h"
/* #include "tool/tool.h" */
#include "debug_tool/debug_tool.h"
/* #include "midi/midifile.h" */
/* #include "asound/aseq_tool.h" */

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

void engine_free_trash(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;
  /* clockloop_t     *looph = NULL; */
  uint_t          tick;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx->trash.len
          && pthread_rwlock_trywrlock(&(track_ctx->lock)) == 0)
        {
          free_list_node(&(track_ctx->trash), _free_trash_ctn);
          tick = ctx->looph.clocktick.number % track_ctx->len;
          goto_next_available_tick(&(track_ctx->current_tickev), tick);
          pthread_rwlock_unlock(&(track_ctx->lock));
        }
    }
}

void play_all_tracks_pending_notes(engine_ctx_t *ctx)
{
    track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      play_track_pending_notes(track_ctx);
    }
}

void play_all_tracks_ev(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  clockloop_t     *looph     = &(ctx->looph);
  list_iterator_t trackit;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx->deleted == TRUE)
        iter_node_del(&trackit, _free_trackctx); /* /!\ Memory corruption while asking tracklist */
      if (iter_node(&trackit) != NULL)
        track_ctx = iter_node_ptr(&trackit);
      else
        break;
      play_trackev(looph->clocktick.number, track_ctx);
    }
}

void set_engine_rq(engine_ctx_t *ctx, engine_rq rq)
{
  /* pthread_rwlock_rdlock(&(ctx->rq.lock)); */
  ctx->rq = rq;
  /* pthread_rwlock_unlock(&(ctx->rq.lock)); */
}

engine_rq get_engine_rq(engine_ctx_t *ctx)
{
  return ctx->rq;
  /* engine_rq rq; */

  /* pthread_rwlock_rdlock(&(ctx->rq.lock)); */
  /* rq = ctx->rq; */
  /* pthread_rwlock_unlock(&(ctx->rq.lock)); */
  /* return rq; */
}

clock_req_t engine_cb(void *arg)
{
  engine_ctx_t    *ctx       = (engine_ctx_t *) arg;
  /* track_ctx_t     *track_ctx = NULL; */
  /* clockloop_t     *looph     = &(ctx->looph); */
  /* list_iterator_t trackit; */

  if (get_engine_rq(ctx) == engine_stop)
    {
      debug("engine: Got stop request\n");
      engine_free_trash(ctx);
      play_all_tracks_pending_notes(ctx);
      ctx->isrunning = FALSE;
      return STOP;
    }
  play_all_tracks_ev(ctx);
  engine_free_trash(ctx);
  return CONTINUE;
}

void stop_engine(engine_ctx_t *ctx)
{
  debug("stopping for engine thread end\n");
  set_engine_rq(ctx, engine_stop);
  pthread_join(ctx->thread_id, NULL);
}

void wait_engine(engine_ctx_t *ctx)
{
  debug("waiting for engine thread end\n");
  pthread_join(ctx->thread_id, NULL);
}

bool_t engine_isrunning(engine_ctx_t *ctx)
{
  bool_t isrunning;

  /* pthread_rwlock_rdlock(&(ctx->info.lock)); */
  isrunning = ctx->isrunning;
  /* pthread_rwlock_unlock(&(ctx->info.lock)); */
  return isrunning;
}

void _free_port(void *addr)
{
  aseqport_ctx_t *aseqport_ctx = (aseqport_ctx_t  *) addr;

  free_aseqport(aseqport_ctx);
}

bool_t engine_del_port(engine_ctx_t *ctx, aseqport_ctx_t *aseqportctx)
{
  list_iterator_t aseqportit;
  aseqport_ctx_t *aseqport_ctx = NULL;

  for (iter_init(&aseqportit, &(ctx->aseqport_list));
       iter_node(&aseqportit) != NULL;
       iter_next(&aseqportit))
    {
      aseqport_ctx = iter_node_ptr(&aseqportit);
      if (aseqport_ctx == aseqportctx)
        {
          iter_node_del(&aseqportit, _free_port);
          /* free_aseqport(aseqport_ctx); */
          return TRUE;
        }
    }
  return FALSE;
}

aseqport_ctx_t *engine_create_aport(engine_ctx_t *ctx, char *name)
{
  aseqport_ctx_t  *aseqport_ctx = myalloc(sizeof (aseqport_ctx_t));

  aseqport_ctx = create_aseqport_ctx(ctx->aseqh, name);
  push_to_list_tail(&(ctx->aseqport_list), aseqport_ctx);
  return aseqport_ctx;
}

bool_t engine_del_track(engine_ctx_t *ctx, track_ctx_t *trackctx)
{
  list_iterator_t trackit;
  track_ctx_t *track_ctx = NULL;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      if (track_ctx == trackctx)
        {
          if (track_ctx->is_handled == TRUE)
            track_ctx->deleted = TRUE;
          else
            iter_node_del(&trackit, _free_trackctx);
          /* free_track(track_ctx); */
          return TRUE;
        }
    }
  return FALSE;
}

track_ctx_t  *engine_copyadd_miditrack(engine_ctx_t *ctx, midifile_track_t *mtrack)
{
  track_ctx_t       *trackctx = myalloc(sizeof (track_ctx_t));

  trackctx->track = myalloc(sizeof (track_t));
  copy_track(&(mtrack->track), trackctx->track);
  if (mtrack->track.name)
    trackctx->track->name = strdup(mtrack->track.name);

  if (mtrack->sysex_len == 0)
    trackctx->len = ctx->ppq * 4;
  else
    trackctx->len = mtrack->sysex_len;
  trackctx->mute = FALSE;
  trackctx->loop_start = 0;
  trackctx->is_handled = FALSE;
  trackctx->deleted = FALSE;

  pthread_rwlock_init(&(trackctx->lock), NULL);
  iter_init(&(trackctx->current_tickev), &(trackctx->track->tickev_list));
  push_to_list_tail(&(ctx->track_list), trackctx);

  return trackctx;
}

typedef struct
{
  int            id;
  aseqport_ctx_t *aseq;
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

  for (iter_init(&portit, &(midifile->info.portinfo_list));
       iter_node(&portit) != NULL;
       iter_next(&portit))
    {
      portinfo = iter_node_ptr(&portit);
      portcache = myalloc(sizeof (tmpport_cache_t));
      portcache->aseq = engine_create_aport(ctx, portinfo->name);
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
                  trackctx->aseqport_ctx = portcache->aseq;
                  break;
                }
            }
        }
    }
  free_list_node(&tmpport, free);

}

track_ctx_t  *engine_new_track(engine_ctx_t *ctx, char *name)
{
  track_ctx_t *trackctx = myalloc(sizeof (track_ctx_t));
  track_t     *track = myalloc(sizeof (track_t));

  track->name = strdup(name);
  trackctx->track = track;
  trackctx->len = ctx->ppq * 4;
  trackctx->mute = FALSE;
  trackctx->loop_start = 0;
  if (ctx->isrunning == TRUE)
    trackctx->is_handled = TRUE;
  else
    trackctx->is_handled = FALSE;
  trackctx->deleted = FALSE;
  pthread_rwlock_init(&(trackctx->lock), NULL);
  iter_init(&(trackctx->current_tickev), &(trackctx->track->tickev_list));
  push_to_list_tail(&(ctx->track_list), trackctx);
  return trackctx;
}

void free_engine_ctx(engine_ctx_t *ctx)
{
  if (engine_isrunning(ctx))
    stop_engine(ctx);
  pthread_rwlock_destroy(&(ctx->lock));
  /* pthread_rwlock_destroy(&(ctx->info.lock)); */

  free_list_node(&(ctx->aseqport_list), _free_port);
  free_list_node(&(ctx->track_list), _free_trackctx);
  free_aseqh(ctx->aseqh);
  /* snd_seq_client_info_free(ctx->aseqinfo); */
  free(ctx);
  /* free_clockloop_struct(ctx->looph); */
}

void engine_setbpm(engine_ctx_t *ctx, uint_t bpm)
{
  set_bpmnppq_to_timespec(&(ctx->looph.res),
                          ctx->ppq,
                          bpm);
}

engine_ctx_t *init_engine_ctx(char *name)
{
  engine_ctx_t *ctx = myalloc(sizeof (engine_ctx_t));
  ctx->aseqh = create_aseqh(name);
  /* snd_seq_client_info_malloc(&(ctx->aseqinfo)); */
  ctx->looph.cb_func = engine_cb;
  ctx->looph.cb_arg = ctx;
  ctx->isrunning = FALSE;
  ctx->rq = engine_stop;
  ctx->ppq = 192;
  engine_setbpm(ctx, 120);
  /* pthread_rwlock_init(&(ctx->info.lock), NULL); */
  pthread_rwlock_init(&(ctx->lock), NULL);
  /* pthread_attr_init(&(ctx->thread_attr)); */
  return ctx;
}

void init_engine_trackev(engine_ctx_t *ctx)
{
  track_ctx_t     *track_ctx = NULL;
  list_iterator_t trackit;

  for (iter_init(&trackit, &(ctx->track_list));
       iter_node(&trackit) != NULL;
       iter_next(&trackit))
    {
      track_ctx = iter_node_ptr(&trackit);
      track_ctx->is_handled = TRUE;
      goto_next_available_tick(&(track_ctx->current_tickev),
                               track_ctx->loop_start);
      /* iter_head(&(track_ctx->current_tickev)); */
    }
}

void unset_engine_trackev_handling(engine_ctx_t *ctx)
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
      if (iter_node(&trackit) != NULL)
        track_ctx = iter_node_ptr(&trackit);
      else
        break;
      track_ctx->is_handled = FALSE;
    }
}

void *engine_thread_wrapper(void *arg)
{
  engine_ctx_t *ctx = (engine_ctx_t *) arg;

  set_engine_rq(ctx, engine_cont);
  /* pthread_rwlock_wrlock(&(ctx->info.lock)); */
  ctx->isrunning = TRUE;
  /* pthread_rwlock_unlock(&(ctx->info.lock)); */
  init_engine_trackev(ctx);
  ctx->thread_ret = clockloop(&(ctx->looph));
  unset_engine_trackev_handling(ctx);
  /* engine_free_trash(ctx); */
  return &(ctx->thread_ret);
}

bool_t start_engine(engine_ctx_t *ctx)
{
  if (engine_isrunning(ctx))
    {
      output_error("Can not start engine it is already running\n");
      return FALSE;
    }
  /* if (iter_node(&(ctx->track_ctx.current_tickev)) == NULL) */
  /*   { */
  /*     output_error("Can not start engine with no events on track\n"); */
  /*     return FALSE; */
  /*   } */
  ctx->looph.clocktick.number = 0;
  ctx->rq = engine_cont;
  pthread_create(&(ctx->thread_id), NULL, engine_thread_wrapper, ctx);
  usleep(200000);
  return TRUE;
}
