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

#include "engine.h"
#include "debug_tool/debug_tool.h"
#include "jack/jack_backend.h"

typedef struct
{
  jack_client_t  *client;
  jack_nframes_t cur_frame;
  uint_t         tick;
} jbe_hdl_t;

bool_t jbe_is_running(engine_ctx_t *ctx)
{
  jbe_hdl_t *hdl = (jbe_hdl_t *) ctx->hdl;
  jack_transport_state_t state = jack_transport_query(hdl->client, NULL);

  if (state == JackTransportRolling
      || state == JackTransportLooping
      || state == JackTransportStarting)
	  return TRUE;
  return FALSE;
}

void jbe_stop(engine_ctx_t *ctx)
{
  jbe_hdl_t *hdl = (jbe_hdl_t *) ctx->hdl;

  jack_transport_stop(hdl->client);
}

void jbe_free_output_node(void *addr)
{
  output_t     *joutput = (output_t *) addr;
  jbe_output_t *jackoutput = (jbe_output_t *) joutput->hdl;

  free_jbe_output(jackoutput);
  free(joutput);
}

void jbe_destroy_hdl(engine_ctx_t *ctx)
{
  jbe_hdl_t *hdl = (jbe_hdl_t *) ctx->hdl;

  if (engine_is_running(ctx))
    engine_stop(ctx);

  output_error("Not entirely implemented");
  jack_client_close(hdl->client);
  free(hdl);
}

bool_t jbe_start(engine_ctx_t *ctx)
{
  jbe_hdl_t *hdl = (jbe_hdl_t *) ctx->hdl;

  jack_transport_start(hdl->client);
  return TRUE;
}

void jbe_init_output(engine_ctx_t *ctx, output_t *output, const char *name)
{
  jbe_hdl_t *be_hdl = (jbe_hdl_t *) ctx->hdl;

  output->hdl      = create_jack_output(be_hdl->client,
                                        name,
                                        &(be_hdl->cur_frame));
  output->get_name = jbe_output_get_name;
  output->set_name = jbe_output_set_name;
  output->write    = jbe_output_write;
  output->_write   = _jbe_output_write;
}

uint_t jbe_get_tick(engine_ctx_t *ctx)
{
  jbe_hdl_t       *be_hdl = (jbe_hdl_t *) ctx->hdl;

  return be_hdl->tick;
}

void jbe_set_tempo(engine_ctx_t *ctx, uint_t ms)
{
  ctx->tempo = ms;
  /* jbe_hdl_t       *be_hdl = (jbe_hdl_t *) ctx->hdl; */
  /* jack_position_t position; */

  /* jack_transport_query(be_hdl->client, &position); */
}

#include <jack/midiport.h>
void jbe_prepare_outputs(list_t *output_list, jack_nframes_t nframes)
{
  list_iterator_t output_it;
  output_t        *output = NULL;
  jbe_output_t    *jbe_output = NULL;

  for (iter_init(&output_it, output_list);
       iter_node(&output_it) != NULL;
       iter_next(&output_it))
    {
      output = iter_node_ptr(&output_it);
      jbe_output = (jbe_output_t *) output->hdl;
      jbe_output->jack_buffer = jack_port_get_buffer(jbe_output->port, nframes);
      jack_midi_clear_buffer(jbe_output->jack_buffer);
    }
}

void jbe_handle_transport(engine_ctx_t *ctx, jack_nframes_t nframes)
{
  jbe_hdl_t              *be_hdl = (jbe_hdl_t *) ctx->hdl;
  jack_position_t        position;
  jack_nframes_t         tick_frame;
  uint64_t               tick64;
  static bool_t          started = FALSE;

  switch (jack_transport_query(be_hdl->client, &position))
    {
    case JackTransportRolling:
    case JackTransportLooping:
    case JackTransportStarting:
      /* Getting fist tick in the current buffer*/
      tick64 = position.frame
        * (uint64_t) ctx->ppq
        * (uint64_t) 1000000
        / (uint64_t) (position.frame_rate
                      * (uint64_t) ctx->tempo);
      if ((((uint64_t) (position.frame
                        * (uint64_t) 1000000
                        * (uint64_t) ctx->ppq))
           % ((uint64_t) (position.frame_rate
                          * (uint64_t) ctx->tempo))) == 0)
        be_hdl->cur_frame = 0;
      else
        {
          tick64++;
          tick_frame = (uint64_t) tick64
            * (uint64_t) ctx->tempo
            * position.frame_rate
            / ((uint64_t) ctx->ppq
               * (uint64_t) 1000000);
          be_hdl->cur_frame = tick_frame - position.frame;
        }

      /* Handling missing tick when started */
      if (started == TRUE)
        {
          if (be_hdl->tick != 0 && be_hdl->tick != tick64)
             output_error("!!! Missing tick !!! "
                         "last:%llu diff current:%llu (nframes:%llu)",
                         be_hdl->tick,
                         tick64,
                         nframes);
        }
      started = TRUE;

      /* Handling all tick in buffer */
      be_hdl->tick = tick64;
      while (be_hdl->cur_frame < nframes)
        {
          play_tracks(ctx);
          tick64++;
          tick_frame = tick64 * ctx->tempo * position.frame_rate
            / (ctx->ppq * 1000000);
          be_hdl->cur_frame = tick_frame - position.frame;
          be_hdl->tick = tick64;
        }
      break;
    case JackTransportStopped:
      if (started == TRUE)
        {
          play_tracks_pending_notes(ctx);
          be_hdl->tick = 0;
          be_hdl->cur_frame = 0;
          started = FALSE;
        }
      break;
    default:
      break;
    }
}


int jbe_process_cb(jack_nframes_t nframes, void *ctx_ptr)
{
  engine_ctx_t *engine = ctx_ptr;
  jbe_hdl_t       *be_hdl = (jbe_hdl_t *) engine->hdl;

  be_hdl->cur_frame = 0;

  jbe_prepare_outputs(&(engine->output_list), nframes);

  play_outputs_reqs(engine);

  jbe_handle_transport(engine, nframes);

  return 0;
}

void jack_shutdown (void *arg)
{
  output_error("Not implemented brutal exit");
  exit (1);
}

bool_t jbe_init_engine(engine_ctx_t *ctx, char *name)
{
  jbe_hdl_t *hdl = NULL;
  jack_client_t *client = create_jackh(name);

  if (client == NULL)
    return FALSE;

  ctx->destroy_hdl      = jbe_destroy_hdl;
  ctx->is_running       = jbe_is_running;
  ctx->start            = jbe_start;
  ctx->stop             = jbe_stop;
  ctx->init_output      = jbe_init_output;
  ctx->free_output_node = jbe_free_output_node;
  ctx->get_tick         = jbe_get_tick;
  ctx->set_tempo        = jbe_set_tempo;

  hdl = myalloc(sizeof (jbe_hdl_t));
  hdl->client = client;
  ctx->hdl   = hdl;

  jack_set_process_callback(hdl->client,
                            jbe_process_cb,
                            ctx);
  jack_on_shutdown(hdl->client, jack_shutdown, 0);

  if (jack_activate (client)) {
    output_error("Cannot activate jack client");
    engine_destroy_hdl(ctx);
    return FALSE;
  }

  return TRUE;
}