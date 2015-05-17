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
#include "asound/aseq_tool.h"

void dump_trash_ctn(trash_ctn_t *ctn)
{
  tickev_t *tickev = NULL;

  if (iter_node(&(ctn->tickit)) == NULL)
    return;

  tickev = (tickev_t *) iter_node_ptr(&(ctn->tickit));
  dumpaddr_seqevlist(ctn->evit.list);
  dumpaddr_seqevlist(&(tickev->seqev_list));
}

void _play_if_noteoff(track_ctx_t *trackctx,
                      seqev_t *seqev)
{
  midicev_t *mcev = NULL;
  /* snd_seq_event_t aseqev; */

  switch (seqev->type)
    {
    case ASEQTYPE:
    case MIDICEV:
      mcev = (midicev_t *) seqev->addr;
      if (mcev->type == NOTEOFF)
        {
          output_add_req(trackctx->output, mcev);
          /* set_aseqev(mcev, */
          /*            &aseqev, */
          /*            aseqport_ctx->output_port); */
          /* snd_seq_event_output(aseqport_ctx->handle, &aseqev); */
          /* snd_seq_drain_output(aseqport_ctx->handle); */
        }
      break;
    default:
      break;
    }
}

uint_t _seqev_len(tickev_t *tickev)
{
  list_iterator_t seqevit;
  seqev_t         *ev = NULL;
  uint_t          len = 0;

  for (iter_init(&seqevit, &(tickev->seqev_list));
       iter_node(&seqevit) != NULL;
       iter_next(&seqevit))
    {
      ev = (seqev_t *) iter_node_ptr(&seqevit);
      if (ev->deleted == FALSE)
        len++;
    }

  return len;
}

void trackctx_event2trash(track_ctx_t *trackctx,
                          ev_iterator_t *ev_iterator)
/* list_iterator_t *tickit, */
/* list_iterator_t *evit) */
{
  trash_ctn_t *ctn = myalloc(sizeof (trash_ctn_t));
  seqev_t     *seqev = (seqev_t *) iter_node_ptr(&(ev_iterator->seqevit));
  tickev_t    *tickev = (tickev_t *) iter_node_ptr(&(ev_iterator->tickit));
  /* seqev_t     *seqev = (seqev_t *) iter_node_ptr(evit); */
  /* tickev_t    *tickev = (tickev_t *) iter_node_ptr(tickit); */

  if (_seqev_len(tickev) == 1)
    tickev->deleted = TRUE;

  seqev->deleted = TRUE;

  if (trackctx->output != NULL)
    _play_if_noteoff(trackctx, seqev);
  bcopy(&(ev_iterator->seqevit), &(ctn->evit), sizeof (list_iterator_t));
  bcopy(&(ev_iterator->tickit), &(ctn->tickit), sizeof (list_iterator_t));

  /* bcopy(evit, &(ctn->evit), sizeof (list_iterator_t)); */
  /* bcopy(tickit, &(ctn->tickit), sizeof (list_iterator_t)); */

  push_to_list_tail(&(trackctx->trash), ctn);
  trackctx->need_sync = TRUE;
}


uint_t trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick)
{
  tick = tick % track_ctx->loop_len;
  while (tick < track_ctx->loop_start)
    tick += track_ctx->loop_len;
  return tick;
}

void play_trackctx(uint_t tick, track_ctx_t *track_ctx)
{
  tickev_t *tickev  = NULL;
  /* static bool_t   to_reload  = FALSE; */
  uint_t   last_pulse = track_ctx->loop_start + track_ctx->loop_len - 1;

  if (track_ctx->output == NULL)
    return;

#define trackctx_restart_loop(trackctx)                         \
  goto_next_available_tick(&((trackctx)->current_tickev),       \
                           (trackctx)->loop_start)

  tick = trackctx_loop_pos(track_ctx, tick);

  if (track_ctx->need_sync == TRUE)
    /* reload of the iterator if some node has been deleted
       and reload again after the trash has been empty */
    {
      goto_next_available_tick(&(track_ctx->current_tickev), tick);
      if (track_ctx->trash.len == 0)
        track_ctx->need_sync = FALSE;
    }

  if (tick == last_pulse || track_ctx->play_pending_notes)
    {
      output_pending_notes(track_ctx->output, track_ctx->notes_on_state);
      track_ctx->play_pending_notes = FALSE;
    }

  if (iter_node(&(track_ctx->current_tickev)) == NULL)
    {
      if (track_ctx->current_tickev.list != NULL)
        trackctx_restart_loop(track_ctx);
      /* else */
      /*   output_error("Current tick event list = NULL"); */
    }
  else
    {
      tickev = iter_node_ptr(&(track_ctx->current_tickev));
      if (tickev->tick == tick)
        {
          if (track_ctx->mute == FALSE)
            output_evlist(track_ctx->output,
                          &(tickev->seqev_list),
                          track_ctx->notes_on_state);
          iter_next_available_tick(&(track_ctx->current_tickev));
        }

      if (iter_node(&(track_ctx->current_tickev)) == NULL)
        /* if no more event go to head */
        {
          if (track_ctx->current_tickev.list != NULL)
            trackctx_restart_loop(track_ctx);
        }
      else if (tickev->tick >= last_pulse)
        /* if in loop end go to head */
        trackctx_restart_loop(track_ctx);
    }
}

void trackctx_toggle_mute(track_ctx_t *track_ctx)
{
  if (track_ctx->mute)
    track_ctx->mute = FALSE;
  else
    {
      track_ctx->mute = TRUE;
      if (track_ctx->output)
        if (track_ctx->engine && engine_is_running(track_ctx->engine))
          track_ctx->play_pending_notes = TRUE;
    }
}
