#include "engine.h"
#include "clock/clock.h"
#include "asound/aseq_tool.h"
#include "asound/aseq_tool.h"
#include "debug_tool/debug_tool.h"

typedef enum
  {
    engine_rq_cont = 0,
    engine_rq_stop
  } engine_rq;

typedef struct
{
  bool_t           is_running;
  pthread_rwlock_t lock;
  clockloop_t      looph;
  snd_seq_t        *aseqh;
  engine_rq        rq;
  pthread_t        thread_id;
  bool_t           thread_ret;
} nns_hdl_t;

bool_t nns_is_running(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;
  bool_t     isrunning;

  pthread_rwlock_rdlock(&(hdl->lock));
  isrunning = hdl->is_running;
  pthread_rwlock_unlock(&(hdl->lock));
  return isrunning;
}

void _free_aseq_output(void *addr)
{
  output_t      *output = (output_t *) addr;
  aseq_output_t *aseqoutput = (aseq_output_t  *) output->hdl;

  free_aseq_output(aseqoutput);
  free(output);
}



void nns_stop(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  debug("stopping for engine thread end\n");
  hdl->rq = engine_rq_stop;
  pthread_join(hdl->thread_id, NULL);
  hdl->looph.clocktick.number = 0;
}

void nns_destroy(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  if (nns_is_running(ctx))
    nns_stop(ctx);
  free_list_node(&(ctx->output_list), _free_aseq_output);
  free_aseqh(hdl->aseqh);
  pthread_rwlock_destroy(&(hdl->lock));
  free(hdl);
}

void *nns_thread_wrapper(void *arg)
{
  engine_ctx_t *ctx = (engine_ctx_t *) arg;
  nns_hdl_t    *hdl = (nns_hdl_t *) ctx->hdl;

  hdl->rq = engine_rq_cont;
  pthread_rwlock_wrlock(&(hdl->lock));
  hdl->is_running = TRUE;
  pthread_rwlock_unlock(&(hdl->lock));
  engine_prepare_tracklist(ctx);
  hdl->thread_ret = clockloop(&(hdl->looph));
  engine_clean_tracklist(ctx);
  /* engine_free_trash(ctx); */
  return &(hdl->thread_ret);
}

bool_t nns_start(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  if (hdl->is_running == TRUE)
    {
      output_error("Can not start engine it is already running\n");
      return FALSE;
    }
  hdl->looph.clocktick.number = 0;
  hdl->rq = engine_rq_cont;
  pthread_create(&(hdl->thread_id), NULL, nns_thread_wrapper, ctx);
  usleep(200000);
  return TRUE;
}

output_t *nns_create_output(engine_ctx_t *ctx, char *name)
{
  nns_hdl_t     *hdl = (nns_hdl_t *) ctx->hdl;
  output_t      *output = myalloc(sizeof (output_t));
  aseq_output_t *aseqoutput = create_aseq_output(hdl->aseqh, name);

  output->hdl                  = aseqoutput;
  output->get_id               = aseq_output_get_id;
  output->get_name             = aseq_output_get_name;
  output->set_name             = aseq_output_set_name;
  output->output_ev            = aseq_output_ev;
  output->output_evlist        = aseq_output_evlist;
  output->output_pending_notes = aseq_output_pending_notes;
  push_to_list_tail(&(ctx->output_list), output);
  return output;
}

bool_t nns_delete_output(engine_ctx_t *ctx, output_t *output)
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
          iter_node_del(&output_it, _free_aseq_output);
          return TRUE;
        }
    }
  return FALSE;
}

uint_t nns_get_tick(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  return hdl->looph.clocktick.number;
}

void _nns_drain_output(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  snd_seq_drain_output(hdl->aseqh);
}

void nns_reset_pulse(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  set_msnppq_to_timespec(&(hdl->looph.res),
                         ctx->ppq,
                         ctx->tempo);
}

clock_req_t _nns_engine_cb(void *arg)
{
  engine_ctx_t *ctx = (engine_ctx_t *) arg;
  nns_hdl_t    *hdl = (nns_hdl_t *) ctx->hdl;

  if (hdl->rq == engine_rq_stop)
    {
      debug("engine: Got stop request\n");
      pthread_rwlock_wrlock(&(hdl->lock));
      _engine_free_trash(ctx);
      /* play_all_tracks_req(ctx); */
      play_all_tracks_pending_notes(ctx);
      hdl->is_running = FALSE;
      pthread_rwlock_unlock(&(hdl->lock));
      return STOP;
    }
  play_all_tracks_ev(ctx);
  _engine_free_trash(ctx);
  return CONTINUE;
}

void nns_init_engine(engine_ctx_t *ctx, char *name)
{
  nns_hdl_t *hdl = NULL;

  ctx->destroy       = nns_destroy;
  ctx->is_running    = nns_is_running;
  ctx->start         = nns_start;
  ctx->stop          = nns_stop;
  ctx->create_output = nns_create_output;
  ctx->delete_output = nns_delete_output;
  ctx->get_tick      = nns_get_tick;
  ctx->_drain_output = _nns_drain_output;
  ctx->reset_pulse   = nns_reset_pulse;

  hdl = myalloc(sizeof (nns_hdl_t));
  pthread_rwlock_init(&(hdl->lock), NULL);
  hdl->aseqh = create_aseqh(name);
  hdl->aseqh = create_aseqh(name);
  hdl->looph.cb_func = _nns_engine_cb;
  hdl->looph.cb_arg = ctx;
  hdl->rq = engine_rq_stop;
  hdl->is_running = FALSE;

  ctx->ppq   = 192;
  ctx->tempo = 500;
  ctx->hdl   = hdl;
  engine_reset_pulse(ctx);
}


/* void set_engine_rq(engine_ctx_t *ctx, engine_rq rq) */
/* { */
/*   /\* pthread_rwlock_rdlock(&(ctx->rq.lock)); *\/ */
/*   ctx->rq = rq; */
/*   /\* pthread_rwlock_unlock(&(ctx->rq.lock)); *\/ */
/* } */

/* engine_rq get_engine_rq(engine_ctx_t *ctx) */
/* { */
/*   return ctx->rq; */
/*   /\* engine_rq rq; *\/ */

/*   /\* pthread_rwlock_rdlock(&(ctx->rq.lock)); *\/ */
/*   /\* rq = ctx->rq; *\/ */
/*   /\* pthread_rwlock_unlock(&(ctx->rq.lock)); *\/ */
/*   /\* return rq; *\/ */
/* } */

/* void wait_engine(engine_ctx_t *ctx) */
/* { */
/*   debug("waiting for engine thread end\n"); */
/*   pthread_join(ctx->thread_id, NULL); */
/* } */

/* aseqport_ctx_t *engine_create_aport(engine_ctx_t *ctx, char *name) */
/* { */
/*   aseqport_ctx_t  *aseqport_ctx = myalloc(sizeof (aseqport_ctx_t)); */

/*   aseqport_ctx = create_aseqport_ctx(ctx->aseqh, name); */
/*   push_to_list_tail(&(ctx->aseqport_list), aseqport_ctx); */
/*   return aseqport_ctx; */
/* } */
