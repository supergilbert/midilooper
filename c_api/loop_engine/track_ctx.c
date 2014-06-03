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


uint_t trackctx_loop_pos(track_ctx_t *track_ctx, uint_t tick)
{
  tick = tick % track_ctx->loop_len;
  if (tick < track_ctx->loop_start)
    tick += track_ctx->loop_len;
  return tick;
}

void play_trackctx(uint_t tick, track_ctx_t *track_ctx)
{
  tickev_t        *tickev    = NULL;
  /* static bool_t   to_reload  = FALSE; */

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

  if (tick == (track_ctx->loop_start + track_ctx->loop_len - 1))
    play_track_pending_notes(track_ctx);

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
            alsa_play_seqevlist(track_ctx->aseqport_ctx,
                                &(tickev->seqev_list),
                                track_ctx->pending_notes);

          iter_next_available_tick(&(track_ctx->current_tickev));
        }
      else if (iter_node(&(track_ctx->current_tickev)) == NULL)
        /* if no more event go to head */
        {
          if (track_ctx->current_tickev.list != NULL)
            trackctx_restart_loop(track_ctx);
        }
      else if (tickev->tick >= track_ctx->loop_len)
        /* if in loop end go to head */
        trackctx_restart_loop(track_ctx);
    }
  return;
}
