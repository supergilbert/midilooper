#include "beta_engine/engine.h"
#include "tool/tool.h"
#include "debug_tool/debug_tool.h"
#include "midi/midifile.h"

void	play_midiev(list_t *seqevlist, aseq_ctx_t *aseq_ctx)
{
  snd_seq_event_t aseqev;
  list_iterator_t iter;
  bool_t          ev_to_drain = FALSE;
  seqev_t         *seqev = NULL;

  for (iter_init(&iter, seqevlist);
       iter_node(&iter);
       iter_next(&iter))
    {
      seqev = (seqev_t *) iter_node_ptr(&(iter));
      if (seqev->type == MIDICEV)
        {
          set_aseqev((midicev_t *) seqev->addr,
                     &aseqev,
                     aseq_ctx->output_port);
          snd_seq_event_output(aseq_ctx->handle, &aseqev);
          ev_to_drain = TRUE;
        }
    }
  if (ev_to_drain)
    snd_seq_drain_output(aseq_ctx->handle);
}

engine_rq get_engine_rq(engine_ctx_t *ctx)
{
  engine_rq rq;

  pthread_rwlock_rdlock(&(ctx->rq.lock));
  rq = ctx->rq.rq;
  pthread_rwlock_unlock(&(ctx->rq.lock));
  return rq;
}

void set_engine_rq(engine_ctx_t *ctx, engine_rq rq)
{
  pthread_rwlock_rdlock(&(ctx->rq.lock));
  ctx->rq.rq = rq;
  pthread_rwlock_unlock(&(ctx->rq.lock));
}

clock_req_t engine_cb(void *arg)
{
  engine_ctx_t  *ctx = (engine_ctx_t *) arg;
  aseq_ctx_t    *aseq_ctx  = ctx->aseq_ctx;
  track_ctx_t   *track_ctx = &(ctx->track_ctx);
  clockloop_t   *looph     = &(ctx->looph);
  tickev_t      *tickev;

  tickev = iter_node_ptr(&(track_ctx->current_tickev));
  /* debug("@ tick %i waiting for tick %i\r", looph->clocktick.number, tickev->tick); */

  if (tickev->tick == looph->clocktick.number)
    {
      /* debug("\ngot tick number %i at %ds %dns\n", */
      /*       tickev->tick, */
      /*       looph->clocktick.time.tv_sec, */
      /*       looph->clocktick.time.tv_nsec); */
      play_midiev(&(tickev->seqev_list), aseq_ctx);
      iter_next(&(track_ctx->current_tickev));
      if (iter_node(&(track_ctx->current_tickev)) == NULL)
        {
          debug("End of track\n");
          pthread_rwlock_wrlock(&(ctx->info.lock));
          ctx->info.isrunning = FALSE;
          pthread_rwlock_unlock(&(ctx->info.lock));
          set_engine_rq(ctx, engine_stop);
          return STOP;
        }
    }

  if (get_engine_rq(ctx) == engine_stop)
    {
      debug("engine: Got stop request\n");
      pthread_rwlock_wrlock(&(ctx->info.lock));
      ctx->info.isrunning = FALSE;
      pthread_rwlock_unlock(&(ctx->info.lock));
      return STOP;
    }
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

  pthread_rwlock_rdlock(&(ctx->info.lock));
  isrunning = ctx->info.isrunning;
  pthread_rwlock_unlock(&(ctx->info.lock));
  return isrunning;
}

void free_engine_ctx(engine_ctx_t *ctx)
{
  if (engine_isrunning(ctx))
    stop_engine(ctx);
  pthread_rwlock_destroy(&(ctx->rq.lock));
  pthread_rwlock_destroy(&(ctx->info.lock));
  free_aseq(ctx->aseq_ctx);
  free(ctx);
  /* free_clockloop_struct(ctx->looph); */
}

engine_ctx_t *init_engine(char *aport_name)
{
  engine_ctx_t *ctx = myalloc(sizeof (engine_ctx_t));
  ctx->aseq_ctx = init_aseq(aport_name);
  ctx->looph.cb_func = engine_cb;
  ctx->looph.cb_arg = ctx;
  ctx->info.isrunning = FALSE;
  ctx->rq.rq = engine_stop;
  pthread_rwlock_init(&(ctx->info.lock), NULL);
  pthread_rwlock_init(&(ctx->rq.lock), NULL);
  /* pthread_attr_init(&(ctx->thread_attr)); */
  return ctx;
}

void *engine_thread_wrapper(void *arg)
{
  engine_ctx_t *ctx = (engine_ctx_t *) arg;

  set_engine_rq(ctx, engine_cont);

  pthread_rwlock_wrlock(&(ctx->info.lock));
  ctx->info.isrunning = TRUE;
  pthread_rwlock_unlock(&(ctx->info.lock));

  ctx->thread_ret = clockloop(&(ctx->looph));
  return &(ctx->thread_ret);
}

bool_t start_engine(engine_ctx_t *ctx)
{
  if (engine_isrunning(ctx))
    {
      output_error("Can not start engine it is already running\n");
      return FALSE;
    }
  if (iter_node(&(ctx->track_ctx.current_tickev)) == NULL)
    {
      output_error("Can not start engine with no events on track\n");
      return FALSE;
    }
  ctx->looph.clocktick.number = 0;
  ctx->rq.rq = engine_cont;
  pthread_create(&(ctx->thread_id), NULL, engine_thread_wrapper, ctx);
  usleep(200000);
  return TRUE;
}
