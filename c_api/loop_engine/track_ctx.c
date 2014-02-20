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

void _play_if_noteoff(seqev_t *seqev,
                     aseqport_ctx_t *aseqport_ctx)
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
          alsa_play_midicev(aseqport_ctx, mcev);
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
    _play_if_noteoff(seqev, trackctx->aseqport_ctx);
  bcopy(&(ev_iterator->seqevit), &(ctn->evit), sizeof (list_iterator_t));
  bcopy(&(ev_iterator->tickit), &(ctn->tickit), sizeof (list_iterator_t));

  /* bcopy(evit, &(ctn->evit), sizeof (list_iterator_t)); */
  /* bcopy(tickit, &(ctn->tickit), sizeof (list_iterator_t)); */

  push_to_list_tail(&(trackctx->trash), ctn);
  trackctx->need_sync = TRUE;
}


void play_trackev(uint_t tick, track_ctx_t *track_ctx)
{
  tickev_t        *tickev    = NULL;
  /* static bool_t   to_reload  = FALSE; */

  tick = tick % track_ctx->len;

  /* reload of the iterator if some node has been deleted
     and reload again after the trash has been empty */
  if (track_ctx->need_sync == TRUE)
    {
      goto_next_available_tick(&(track_ctx->current_tickev), tick);
      if (track_ctx->trash.len == 0)
        track_ctx->need_sync = FALSE;
    }

  if (iter_node(&(track_ctx->current_tickev)) == NULL)
    {
      if (track_ctx->current_tickev.list != NULL)
        goto_next_available_tick(&(track_ctx->current_tickev), track_ctx->loop_start);
      /* else */
      /*   output_error("Current tick event list = NULL"); */
      return;
    }
  tickev = (tickev_t *) iter_node_ptr(&(track_ctx->current_tickev));

  /* The loop goes to the last tick and play the last tick +1
     with the first one */
  if (tickev && (tickev->tick % track_ctx->len) == tick)
    {
      if (track_ctx->mute == FALSE)
        alsa_play_seqevlist(track_ctx->aseqport_ctx,
                            &(tickev->seqev_list),
                            track_ctx->pending_notes);
      /* play the first tick if last one has been detected */
      if (tickev->tick == track_ctx->len)
        {
          goto_next_available_tick(&(track_ctx->current_tickev), track_ctx->loop_start);
          tickev = (tickev_t *) iter_node_ptr(&(track_ctx->current_tickev));
          if (tickev->tick == 0)
            {
              if (track_ctx->mute == FALSE)
                alsa_play_seqevlist(track_ctx->aseqport_ctx,
                                    &(tickev->seqev_list),
                                    track_ctx->pending_notes);
              iter_next_available_tick(&(track_ctx->current_tickev));
            }
        }
      else
        iter_next_available_tick(&(track_ctx->current_tickev));

      /* if no more event go to head */
      if (iter_node(&(track_ctx->current_tickev)) == NULL)
        {
          if (track_ctx->current_tickev.list != NULL)
            goto_next_available_tick(&(track_ctx->current_tickev), track_ctx->loop_start);
          return;
        }

      /* temp (may not occur event musnt be greater than len) */
      tickev = iter_node_ptr(&(track_ctx->current_tickev));
      if (tickev->tick > track_ctx->len)
          goto_next_available_tick(&(track_ctx->current_tickev), track_ctx->loop_start);
    }
  return;
}

void play_track_pending_notes(track_ctx_t *track_ctx)
{
  alsa_play_pending_notes(track_ctx->aseqport_ctx,
                          track_ctx->pending_notes);
}
