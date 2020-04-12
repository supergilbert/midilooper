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

/* You should have received a copy of the GNU Gneneral Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */

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
  msq_bool_t       is_running;
  pthread_rwlock_t lock;
  clockloop_t      looph;
  snd_seq_t        *aseqh;
  int              remote_input;
  int              record_input;
  engine_rq        rq;
  pthread_t        thread_id;
  msq_bool_t       thread_ret;
  msq_bool_t       ev_to_drain;
  output_t         *del_output_req;
  char             *add_output_req;
  aseq_output_t    *add_output_req_res;
} nns_hdl_t;

msq_bool_t nns_is_running(engine_ctx_t *ctx)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;
  msq_bool_t     isrunning;

  pthread_rwlock_rdlock(&(hdl->lock));
  isrunning = hdl->is_running;
  pthread_rwlock_unlock(&(hdl->lock));
  return isrunning;
}

void nns_delete_output_node(engine_ctx_t *ctx, output_t *output)
{
  nns_hdl_t     *hdl = (nns_hdl_t *) ctx->hdl;
  aseq_output_t *aseqoutput = (aseq_output_t  *) output->hdl;

  pthread_rwlock_rdlock(&(hdl->lock));
  if (hdl->is_running)
    {
      hdl->del_output_req = output;
      pthread_rwlock_unlock(&(hdl->lock));
      while (hdl->del_output_req != NULL)
        usleep(100000);
    }
  else
    {
      free_aseq_output(aseqoutput);
      free(output);
      pthread_rwlock_unlock(&(hdl->lock));
    }
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

  hdl->rq = engine_rq_desactivate;
  pthread_join(hdl->thread_id, NULL);
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

void nns_create_output(engine_ctx_t *ctx, output_t *output, const char *name)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  pthread_rwlock_rdlock(&(hdl->lock));
  if (hdl->is_running)
    {
      hdl->add_output_req = (char *) name;
      pthread_rwlock_unlock(&(hdl->lock));
      while (hdl->add_output_req != NULL)
        usleep(100000);
      output->hdl = hdl->add_output_req_res;
      hdl->add_output_req_res = NULL;
    }
  else
    {
      output->hdl = create_aseq_output(hdl->aseqh,
                                       name,
                                       &(hdl->ev_to_drain),
                                       &(hdl->is_running));
      pthread_rwlock_unlock(&(hdl->lock));
    }
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

void nns_set_tick(engine_ctx_t *ctx, uint_t tick)
{
  nns_hdl_t *hdl = (nns_hdl_t *) ctx->hdl;

  hdl->looph.clocktick.number = tick;
  engine_set_tracks_need_sync(ctx);
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

msq_bool_t nns_drain_output(nns_hdl_t *hdl)
{
  if (hdl->ev_to_drain)
    {
      snd_seq_drain_output(hdl->aseqh);
      hdl->ev_to_drain = MSQ_FALSE;
      return MSQ_TRUE;
    }
  else
    return MSQ_FALSE;
}

void nns_handle_input(engine_ctx_t *ctx)
{
  nns_hdl_t       *hdl  = (nns_hdl_t *) ctx->hdl;
  snd_seq_event_t *snd_ev   = NULL;
  midicev_t       mcev;

  while (snd_seq_event_input_pending(hdl->aseqh, 1) > 0)
    {
      snd_seq_event_input(hdl->aseqh, &snd_ev);
      if (snd_ev->dest.port == hdl->remote_input)
        {
          switch (snd_ev->type)
            {
            case SND_SEQ_EVENT_NOTEON:
              if (snd_ev->data.note.velocity != 0)
                {
                  engine_call_notepress_b(ctx, snd_ev->data.note.note);
                  break;
                }
            case SND_SEQ_EVENT_NOTEOFF:
              if (ctx->bindings.rec_note == 255)
                ctx->bindings.rec_note = snd_ev->data.note.note;
              break;
            case SND_SEQ_EVENT_SYSEX:
              switch (engine_get_sysex_mmc(ctx, snd_ev->data.ext.ptr, snd_ev->data.ext.len))
                {
                case MMC_STOP:
                  nns_stop(ctx);
                  break;
                case MMC_PAUSE:
                  nns_start(ctx);
                  break;
                case MMC_RECS:
                  engine_toggle_rec(ctx);
                  break;
                default:
                  break;
                }
              break;
            default:
              /* output("%s: %d %d %d\n", __FUNCTION__, snd_ev->type, SND_SEQ_EVENT_NOTEON, SND_SEQ_EVENT_NOTEOFF); */
              ;
            }
        }
      else if (nns_is_running(ctx) == MSQ_TRUE
               && ctx->rec == MSQ_TRUE
               && ctx->track_rec != NULL
               && snd_ev->dest.port == hdl->record_input
               && (snd_ev->type == SND_SEQ_EVENT_NOTEON
                   || snd_ev->type == SND_SEQ_EVENT_NOTEOFF
                   || snd_ev->type == SND_SEQ_EVENT_CONTROLLER
                   || snd_ev->type == SND_SEQ_EVENT_PITCHBEND))
        {
          aseq_to_mcev(snd_ev, &mcev);
          mrb_write(ctx->rbuff, engine_get_tick(ctx), &mcev);
        }
    }
}

void _nnshdl_add_del_output_req(nns_hdl_t *hdl)
{
  aseq_output_t *aseqoutput = NULL;

  if (hdl->add_output_req != NULL)
    {
      hdl->add_output_req_res = create_aseq_output(hdl->aseqh,
                                                   hdl->add_output_req,
                                                   &(hdl->ev_to_drain),
                                                   &(hdl->is_running));
      hdl->add_output_req = NULL;
    }
  if (hdl->del_output_req != NULL)
    {
      aseqoutput = (aseq_output_t  *) hdl->del_output_req->hdl;
      free_aseq_output(aseqoutput);
      free(hdl->del_output_req);
      hdl->del_output_req = NULL;
    }
}

clock_req_t _nns_engine_cb(void *arg)
{
  engine_ctx_t  *ctx = (engine_ctx_t *) arg;
  nns_hdl_t     *hdl = (nns_hdl_t *) ctx->hdl;

  nns_handle_input(ctx);

  if (hdl->rq != engine_rq_cont)
    {
      debug("engine: Got stop request\n");
      pthread_rwlock_wrlock(&(hdl->lock));
      _engine_free_trash(ctx);
      play_outputs_reqs(ctx);
      play_tracks_pending_notes(ctx);
      nns_drain_output(hdl);
      _nnshdl_add_del_output_req(hdl);
      hdl->is_running = MSQ_FALSE;
      pthread_rwlock_unlock(&(hdl->lock));
      return CLOCK_STOP;
    }

  play_outputs_reqs(ctx);
  play_tracks(ctx);
  nns_drain_output(hdl);

  _nnshdl_add_del_output_req(hdl);

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
          hdl->is_running = MSQ_TRUE;
          pthread_rwlock_unlock(&(hdl->lock));
          engine_prepare_tracklist(ctx);
          hdl->thread_ret = clockloop(&(hdl->looph));
          engine_clean_tracklist(ctx);
        }
      usleep(15000);
    }
  return &(hdl->thread_ret);
}

msq_bool_t nns_init_engine(engine_ctx_t *ctx, char *name)
{
  nns_hdl_t *hdl = NULL;
  snd_seq_t *aseqh = create_aseqh(name);

  if (aseqh == NULL)
    return MSQ_FALSE;

  ctx->ppq = 192;
  ctx->tempo = 500000;

  ctx->destroy_hdl        = nns_destroy_hdl;
  ctx->is_running         = nns_is_running;
  ctx->start              = nns_start;
  ctx->stop               = nns_stop;
  ctx->create_output      = nns_create_output;
  ctx->delete_output_node = nns_delete_output_node;
  ctx->get_tick           = nns_get_tick;
  ctx->set_tick           = nns_set_tick;
  ctx->set_tempo          = nns_set_tempo;

  hdl = myalloc(sizeof (nns_hdl_t));
  pthread_rwlock_init(&(hdl->lock), NULL);
  hdl->aseqh         = aseqh;
  hdl->looph.cb_func = _nns_engine_cb;
  hdl->looph.cb_arg  = ctx;
  hdl->rq            = engine_rq_stop;
  hdl->is_running    = MSQ_FALSE;
  hdl->remote_input  =
    snd_seq_create_simple_port(hdl->aseqh,
                               "remote",
                               SND_SEQ_PORT_CAP_WRITE
                               |SND_SEQ_PORT_CAP_SUBS_WRITE,
                               SND_SEQ_PORT_TYPE_APPLICATION);
  hdl->record_input  =
    snd_seq_create_simple_port(hdl->aseqh,
                               "record",
                               SND_SEQ_PORT_CAP_WRITE
                               |SND_SEQ_PORT_CAP_SUBS_WRITE,
                               SND_SEQ_PORT_TYPE_APPLICATION);
  ctx->hdl = hdl;

  set_msnppq_to_timespec(&(hdl->looph.res),
                         ctx->ppq,
                         ctx->tempo);
  pthread_create(&(hdl->thread_id), NULL, nns_thread_wrapper, ctx);
  usleep(200000);
  return MSQ_TRUE;
}
