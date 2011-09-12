#include "asound/aseq.h"

snd_seq_t     *create_aseqh(char *name)
{
  snd_seq_t     *handle = NULL;

  if (0 > snd_seq_open(&handle, "default", SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK))
    return NULL;
  snd_seq_set_client_name(handle, name);
  return handle;
}

aseq_ctx_t  *init_aseq(char *name)
{
  aseq_ctx_t        *aseq = NULL;
  snd_seq_t     *handle = create_aseqh(name);

  if (!handle)
    return NULL;

  aseq = myalloc(sizeof (aseq_ctx_t));
  aseq->handle = handle;
  aseq->output_port = _create_aseq_port(aseq, "output_1");
  return aseq;
}

void free_aseq(aseq_ctx_t *aseq)
{
  snd_seq_delete_port(aseq->handle, aseq->output_port);
  snd_seq_close(aseq->handle);
  free(aseq);
}

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
