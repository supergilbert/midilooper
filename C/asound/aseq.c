#include "asound/aseq.h"
#include "debug_tool/debug_tool.h"

snd_seq_t     *create_aseqh(char *name)
{
  snd_seq_t     *handle = NULL;
  int         err = 0;

  err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK);
  if (0 > err)
    {
      output_error("problem while creating alsa handler:\n%s\n", snd_strerror(err));
      return NULL;
    }
  snd_seq_set_client_name(handle, name);
  return handle;
}

aseqport_ctx_t  *create_aseqport_ctx(snd_seq_t *handle, char *name)
{
  aseqport_ctx_t        *aseq = NULL;

  aseq = myalloc(sizeof (aseqport_ctx_t));
  aseq->handle = handle;
  aseq->output_port = _create_aseq_port(aseq, name);
  snd_seq_port_info_malloc(&(aseq->info));
  snd_seq_get_port_info(handle, aseq->output_port, aseq->info);
  return aseq;
}

void free_aseqh(snd_seq_t *handle)
{
  int err = 0;

  err = snd_seq_close(handle);
  if (0 != err)
    output_error("problem while closing alsa seq handler\n%s\n", snd_strerror(err));
}

void free_aseqport(aseqport_ctx_t *aseq)
{
  int err = 0;

  err = snd_seq_delete_port(aseq->handle, aseq->output_port);
  if (0 != err)
    output_error("problem while deleting alsa port\n%s\n", snd_strerror(err));
  snd_seq_port_info_free(aseq->info);
  free(aseq);
}

const char *aseqport_name(aseqport_ctx_t *aseq)
{
  return snd_seq_port_info_get_name(aseq->info);
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
