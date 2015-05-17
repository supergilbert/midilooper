#include "engine.h"
#include "clock/clock.h"
#include "asound/aseq_tool.h"
#include "asound/aseq_tool.h"
#include "debug_tool/debug_tool.h"

typedef enum
  {
    engine_rq_cont = 0,
    engine_rq_stop,
    engine_rq_desactivate
  } engine_rq;

typedef struct
{
  bool_t           is_running;
  pthread_rwlock_t lock;
  clockloop_t      looph;
  snd_seq_t        *aseqh;
  int              remote_input;
  engine_rq        rq;
  pthread_t        thread_id;
  bool_t           thread_ret;
  bool_t           ev_to_drain;
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

void nns_free_output_node(void *addr)
{
  output_t      *output = (output_t *) addr;
  aseq_output_t *aseqoutput = (aseq_output_t  *) output->hdl;

  free_aseq_output(aseqoutput);
  free(output);
}

void nns_stop(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  hdl->rq = engine_rq_stop;
  hdl->looph.clocktick.number = 0;
}

void nns_destroy_hdl(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  if (nns_is_running(ctx))
    {
      nns_stop(ctx);
      pthread_join(hdl->thread_id, NULL);
    }
  free_aseqh(hdl->aseqh);
  pthread_rwlock_destroy(&(hdl->lock));
  free(hdl);
}

void nns_start(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  if (hdl->rq == engine_rq_stop)
    hdl->rq = engine_rq_cont;
  else if (hdl->rq == engine_rq_cont)
    hdl->rq = engine_rq_stop;
}

void nns_init_output(engine_ctx_t *ctx, output_t *output, const char *name)
{
  nns_hdl_t     *hdl = (nns_hdl_t *) ctx->hdl;

  output->hdl      = create_aseq_output(hdl->aseqh, name, &(hdl->ev_to_drain), &(hdl->is_running));
  output->get_name = aseq_output_get_name;
  output->set_name = aseq_output_set_name;
  output->write    = aseq_output_write;
  output->_write   = _aseq_output_write;
}

uint_t nns_get_tick(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  return hdl->looph.clocktick.number;
}

void _nns_flush_evbuff(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  snd_seq_drain_output(hdl->aseqh);
}

void nns_set_tempo(engine_ctx_t *ctx, uint_t ms)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  ctx->tempo = ms;
  set_msnppq_to_timespec(&(hdl->looph.res),
                         ctx->ppq,
                         ctx->tempo);
}

bool_t nns_drain_output(nns_hdl_t *hdl)
{
  if (hdl->ev_to_drain)
    {
      snd_seq_drain_output(hdl->aseqh);
      hdl->ev_to_drain = FALSE;
      return TRUE;
    }
  else
    return FALSE;
}

void nns_handle_input(engine_ctx_t *ctx)
{
  nns_hdl_t       *hdl  = (nns_hdl_t *) ctx->hdl;
  snd_seq_event_t *ev   = NULL;

  while (snd_seq_event_input_pending(hdl->aseqh, 1) > 0)
    {
      snd_seq_event_input(hdl->aseqh, &ev);
      if (ev->dest.port == hdl->remote_input)
        {
          switch (ev->type)
            {
            case SND_SEQ_EVENT_NOTEON:
              if (ev->data.note.velocity != 0)
                {
                  engine_call_notepress_b(ctx, ev->data.note.note);
                  break;
                }
            case SND_SEQ_EVENT_NOTEOFF:
              if (ctx->bindings.rec_note == 255)
                ctx->bindings.rec_note = ev->data.note.note;
              break;
            case SND_SEQ_EVENT_SYSEX:
              engine_handle_sysex(ctx, ev->data.ext.ptr, ev->data.ext.len);
              break;
            default:
              output("%s: %d %d %d\n", __FUNCTION__, ev->type, SND_SEQ_EVENT_NOTEON, SND_SEQ_EVENT_NOTEOFF);
            }
        }
    }
}

clock_req_t _nns_engine_cb(void *arg)
{
  engine_ctx_t *ctx = (engine_ctx_t *) arg;
  nns_hdl_t    *hdl = (nns_hdl_t *) ctx->hdl;

  nns_handle_input(ctx);

  if (hdl->rq != engine_rq_cont)
    {
      debug("engine: Got stop request\n");
      pthread_rwlock_wrlock(&(hdl->lock));
      _engine_free_trash(ctx);
      play_outputs_reqs(ctx);
      play_tracks_pending_notes(ctx);
      nns_drain_output(hdl);
      hdl->is_running = FALSE;
      pthread_rwlock_unlock(&(hdl->lock));
      return CLOCK_STOP;
    }

  play_outputs_reqs(ctx);
  play_tracks(ctx);
  nns_drain_output(hdl);

  _engine_free_trash(ctx);
  return CLOCK_CONTINUE;
}

void *nns_thread_wrapper(void *arg)
{
  engine_ctx_t *ctx = (engine_ctx_t *) arg;
  nns_hdl_t    *hdl = (nns_hdl_t *) ctx->hdl;

  hdl->looph.clocktick.number = 0;
  while (hdl->rq != engine_rq_desactivate)
    {
      nns_handle_input(ctx);
      if (hdl->rq == engine_rq_cont)
        {
          debug("engine: Got continue request\n");
          pthread_rwlock_wrlock(&(hdl->lock));
          hdl->is_running = TRUE;
          pthread_rwlock_unlock(&(hdl->lock));
          engine_prepare_tracklist(ctx);
          hdl->thread_ret = clockloop(&(hdl->looph));
          engine_clean_tracklist(ctx);
        }
      usleep(15000);
    }
  return &(hdl->thread_ret);
}

bool_t nns_init_engine(engine_ctx_t *ctx, char *name)
{
  nns_hdl_t *hdl = NULL;
  snd_seq_t *aseqh = create_aseqh(name);

  if (aseqh == NULL)
    return FALSE;

  ctx->destroy_hdl      = nns_destroy_hdl;
  ctx->is_running       = nns_is_running;
  ctx->start            = nns_start;
  ctx->stop             = nns_stop;
  ctx->init_output      = nns_init_output;
  ctx->free_output_node = nns_free_output_node;
  ctx->get_tick         = nns_get_tick;
  ctx->set_tempo        = nns_set_tempo;

  hdl = myalloc(sizeof (nns_hdl_t));
  pthread_rwlock_init(&(hdl->lock), NULL);
  hdl->aseqh         = aseqh;
  hdl->looph.cb_func = _nns_engine_cb;
  hdl->looph.cb_arg  = ctx;
  hdl->rq            = engine_rq_stop;
  hdl->is_running    = FALSE;
  hdl->remote_input  =
    snd_seq_create_simple_port(hdl->aseqh,
                               "remote",
                               SND_SEQ_PORT_CAP_WRITE
                               |SND_SEQ_PORT_CAP_SUBS_WRITE,
                               SND_SEQ_PORT_TYPE_APPLICATION);
  ctx->hdl = hdl;

  ctx->ppq = 192;
  engine_set_tempo(ctx, 500);
  pthread_create(&(hdl->thread_id), NULL, nns_thread_wrapper, ctx);
  usleep(200000);
  return TRUE;
}
