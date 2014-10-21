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
          trackreq_play_midicev(trackctx, mcev);
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

  if (tickev->seqev_list.len == 1)
    tickev->deleted = TRUE;

  seqev->deleted = TRUE;

  if (trackctx->aseqport_ctx != NULL)
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

bool_t play_trackreq(track_ctx_t *track_ctx)
{
  trackreq_t *req = NULL;
  bool_t     ev_to_drain = FALSE;

  for (req = trackreq_getnext_req(&(track_ctx->req_list));
       req;
       req = trackreq_getnext_req(&(track_ctx->req_list)))
    {
      switch (req->req)
        {
        case req_play_midicev:
          if (alsa_output_midicev(track_ctx->aseqport_ctx,
                                  &(req->midicev)))
            ev_to_drain = TRUE;
          break;
        case req_pending_notes:
          if (play_track_pending_notes(track_ctx))
            ev_to_drain = TRUE;
          break;
        default:
          fprintf(stderr, "Unknown track request:%d\n", req->req);
        }
      req->used = FALSE;
    }
  return ev_to_drain;
}

void play_trackctx(uint_t tick, track_ctx_t *track_ctx, bool_t *ev_to_drain)
{
  tickev_t *tickev  = NULL;
  /* static bool_t   to_reload  = FALSE; */
  uint_t   loop_end = track_ctx->loop_start + track_ctx->loop_len - 1;

#define trackctx_restart_loop(trackctx)                         \
  goto_next_available_tick(&((trackctx)->current_tickev),       \
                           (trackctx)->loop_start)

  if (play_trackreq(track_ctx))
    *ev_to_drain = TRUE;

  tick = trackctx_loop_pos(track_ctx, tick);

  if (track_ctx->need_sync == TRUE)
    /* reload of the iterator if some node has been deleted
       and reload again after the trash has been empty */
    {
      goto_next_available_tick(&(track_ctx->current_tickev), tick);
      if (track_ctx->trash.len == 0)
        track_ctx->need_sync = FALSE;
    }

  if (tick == loop_end)
    if (play_track_pending_notes(track_ctx))
      *ev_to_drain = TRUE;

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
            if (alsa_output_seqevlist(track_ctx->aseqport_ctx,
                                      &(tickev->seqev_list),
                                      track_ctx->pending_notes))
              *ev_to_drain = TRUE;
          iter_next_available_tick(&(track_ctx->current_tickev));
        }

      if (iter_node(&(track_ctx->current_tickev)) == NULL)
        /* if no more event go to head */
        {
          if (track_ctx->current_tickev.list != NULL)
            trackctx_restart_loop(track_ctx);
        }
      else if (tickev->tick >= loop_end)
        /* if in loop end go to head */
        trackctx_restart_loop(track_ctx);
    }
  return;
}
