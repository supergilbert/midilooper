/* Copyright 2012-2016 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include "loop_engine/engine.h"
/* #include "seqtool/seqtool.h" */
/* #include "midi/midiev_inc.h" */
#include "asound/aseq.h"


msq_bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port)
{
  bzero(ev, sizeof (snd_seq_event_t));
  switch (chnev->type)
    {
    case NOTEOFF:
      ASEQ_SETNOTEOFFEV(ev,
                        port,
                        chnev->chan,
                        chnev->event.note.num,
                        chnev->event.note.vel);
      break;
    case NOTEON:
      ASEQ_SETNOTEONEV(ev,
                       port,
                       chnev->chan,
                       chnev->event.note.num,
                       chnev->event.note.vel);
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
                                 chnev->event.pitchbend.Lval,
                                 chnev->event.pitchbend.Hval);
      break;
    default:
      fprintf(stderr, "Unsuported event\n");
      return MSQ_FALSE;
    }
  return MSQ_TRUE;
}

void aseq_to_mcev(snd_seq_event_t *snd_ev, midicev_t *mcev)
{
  bzero(mcev, sizeof (midicev_t));
  switch (snd_ev->type)
    {
    case SND_SEQ_EVENT_NOTEON:
      mcev->type = NOTEON;
      mcev->chan = snd_ev->data.note.channel;
      mcev->event.note.num = snd_ev->data.note.note;
      mcev->event.note.vel = snd_ev->data.note.velocity;
      break;
    case SND_SEQ_EVENT_NOTEOFF:
      mcev->type = NOTEOFF;
      mcev->chan = snd_ev->data.note.channel;
      mcev->event.note.num = snd_ev->data.note.note;
      mcev->event.note.vel = snd_ev->data.note.velocity;
      break;
    case SND_SEQ_EVENT_CONTROLLER:
      mcev->type = CONTROLCHANGE;
      mcev->chan = snd_ev->data.control.channel;
      mcev->event.ctrl.num = snd_ev->data.control.param;
      mcev->event.ctrl.val = snd_ev->data.control.value;
      break;
    case SND_SEQ_EVENT_PITCHBEND:
      mcev->type = PITCHWHEELCHANGE;
      mcev->chan = snd_ev->data.control.channel;
      mcev->event.pitchbend.Lval = snd_ev->data.control.param;
      mcev->event.pitchbend.Hval = snd_ev->data.control.value;
      break;
    default:
      ;
    }
}

msq_bool_t _aseq_output_write(output_t *output, midicev_t *midicev)
{
  aseq_output_t   *aseqoutput = (aseq_output_t *) output->hdl;
  snd_seq_event_t aseqev;

  if (set_aseqev(midicev, &aseqev, aseqoutput->port))
    {
      snd_seq_event_output(aseqoutput->handle, &aseqev);
      *(aseqoutput->ev_to_drain) = MSQ_TRUE;
      return MSQ_TRUE;
    }
  return MSQ_FALSE;
}

msq_bool_t aseq_output_ev_n_drain(output_t *output, midicev_t *midicev)
{
  aseq_output_t   *aseqoutput = (aseq_output_t *) output->hdl;
  snd_seq_event_t aseqev;

  if (set_aseqev(midicev, &aseqev, aseqoutput->port))
    {
      snd_seq_event_output(aseqoutput->handle, &aseqev);
      snd_seq_drain_output(aseqoutput->handle);
      return MSQ_TRUE;
    }
  return MSQ_FALSE;
}

msq_bool_t aseq_output_write(output_t *output,
                             midicev_t *midicev)
{
  aseq_output_t   *aseqoutput = (aseq_output_t *) output->hdl;

  if (*(aseqoutput->is_running) == MSQ_TRUE)
    output_add_req(output, midicev);
  else
    return aseq_output_ev_n_drain(output, midicev);
  return MSQ_TRUE;
}
