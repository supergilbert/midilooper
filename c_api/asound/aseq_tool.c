#include "loop_engine/engine.h"
/* #include "seqtool/seqtool.h" */
/* #include "midi/midiev_inc.h" */
#include "asound/aseq.h"


bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port)
{
  bzero(ev, sizeof (snd_seq_event_t));
  switch (chnev->type)
    {
    case NOTEOFF:
      ASEQ_SETNOTEOFFEV(ev,
                        port,
                        chnev->chan,
                        chnev->event.note.num,
                        chnev->event.note.val);
      break;
    case NOTEON:
      ASEQ_SETNOTEONEV(ev,
                       port,
                       chnev->chan,
                       chnev->event.note.num,
                       chnev->event.note.val);
      break;
    case KEYAFTERTOUCH:
      ASEQ_SETKEYAFTERTOUCHEV(ev,
                              port,
                              chnev->chan,
                              chnev->event.aftertouch.num,
                              chnev->event.aftertouch.val);
      break;
    case CONTROLCHANGE:
      ASEQ_SETCONTROLCHANGEEV(ev,
                              port,
                              chnev->chan,
                              chnev->event.ctrl.num,
                              chnev->event.ctrl.val);
      break;
    case PROGRAMCHANGE:
      ASEQ_SETPROGRAMCHANGEEV(ev,
                              port,
                              chnev->chan,
                              chnev->event.prg_chg);
      break;
    case CHANNELAFTERTOUCH:
      ASEQ_SETCHANNELAFTERTOUCHEV(ev,
                                  port,
                                  chnev->chan,
                                  chnev->event.chan_aftertouch);
      break;
    case PITCHWHEELCHANGE:
      ASEQ_SETPITCHWHEELCHANGEEV(ev,
                                 port,
                                 chnev->chan,
                                 chnev->event.pitchbend.Lval);
      break;
    default:
      fprintf(stderr, "Unsuported event\n");
      return FALSE;
    }
  return TRUE;
}


bool_t alsa_play_midicev(aseqport_ctx_t *aseq_ctx, midicev_t *midicev)
{
  snd_seq_event_t aseqev;

  if (set_aseqev(midicev, &aseqev, aseq_ctx->output_port))
    {
      snd_seq_event_output(aseq_ctx->handle, &aseqev);
      snd_seq_drain_output(aseq_ctx->handle);
      return TRUE;
    }
  return FALSE;
}


void alsa_play_seqevlist(aseqport_ctx_t *aseq_ctx, list_t *seqevlist, byte_t *pending_notes)
{
  snd_seq_event_t aseqev;
  list_iterator_t iter;
  bool_t          ev_to_drain = FALSE;
  seqev_t         *seqev = NULL;
  midicev_t       *midicev = NULL;

  if (aseq_ctx == NULL)
    return;
  for (iter_init(&iter, seqevlist);
       iter_node(&iter);
       iter_next(&iter))
    {
      seqev = (seqev_t *) iter_node_ptr(&(iter));
      if (seqev->deleted == FALSE && seqev->type == MIDICEV)
        {
          midicev = (midicev_t *) seqev->addr;
          if (set_aseqev(midicev, &aseqev, aseq_ctx->output_port))
            update_pending_notes(pending_notes, midicev);
          snd_seq_event_output(aseq_ctx->handle, &aseqev);
          ev_to_drain = TRUE;
        }
    }
  if (ev_to_drain)
    snd_seq_drain_output(aseq_ctx->handle);
}

void alsa_play_pending_notes(aseqport_ctx_t *aseq_ctx, byte_t *pending_notes)
{
  bool_t          ev_to_drain = FALSE;
  uint_t          note_idx, channel_idx;
  midicev_t       mcev;
  snd_seq_event_t aseqev;

  mcev.type = NOTEOFF;
  mcev.event.note.val = 0;

  for (channel_idx = 0;
       channel_idx < 16;
       channel_idx++)
    for (note_idx = 0;
         note_idx < 128;
         note_idx++)
      {
        if (is_pending_notes(pending_notes, channel_idx, note_idx))
          {
            mcev.chan      = channel_idx;
            mcev.event.note.num = note_idx;
            set_aseqev(&mcev, &aseqev, aseq_ctx->output_port);
            snd_seq_event_output(aseq_ctx->handle, &aseqev);
            ev_to_drain = TRUE;
            unset_pending_note(pending_notes, channel_idx, note_idx);
          }
      }
  if (ev_to_drain)
    snd_seq_drain_output(aseq_ctx->handle);
}
