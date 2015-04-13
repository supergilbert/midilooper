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


#include "loop_engine/engine.h"
/* #include "seqtool/seqtool.h" */
/* #include "midi/midiev_inc.h" */
#include "asound/aseq.h"


bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port)
{
  int value = 0;

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
      value = chnev->event.pitchbend.Hval << 7;
      value += chnev->event.pitchbend.Lval;
      ASEQ_SETPITCHWHEELCHANGEEV(ev,
                                 port,
                                 chnev->chan,
                                 value);
      break;
    default:
      fprintf(stderr, "Unsuported event\n");
      return FALSE;
    }
  return TRUE;
}

bool_t _aseq_output_write(output_t *output, midicev_t *midicev)
{
  aseq_output_t   *aseqoutput = (aseq_output_t *) output->hdl;
  snd_seq_event_t aseqev;

  if (set_aseqev(midicev, &aseqev, aseqoutput->port))
    {
      snd_seq_event_output(aseqoutput->handle, &aseqev);
      *(aseqoutput->ev_to_drain) = TRUE;
      return TRUE;
    }
  return FALSE;
}

bool_t aseq_output_ev_n_drain(output_t *output, midicev_t *midicev)
{
  aseq_output_t   *aseqoutput = (aseq_output_t *) output->hdl;
  snd_seq_event_t aseqev;

  if (set_aseqev(midicev, &aseqev, aseqoutput->port))
    {
      snd_seq_event_output(aseqoutput->handle, &aseqev);
      snd_seq_drain_output(aseqoutput->handle);
      return TRUE;
    }
  return FALSE;
}

bool_t aseq_output_write(output_t *output,
                         midicev_t *midicev)
{
  aseq_output_t   *aseqoutput = (aseq_output_t *) output->hdl;

  if (*(aseqoutput->is_running) == TRUE)
    output_add_req(output, midicev);
  else
    return aseq_output_ev_n_drain(output, midicev);
  return TRUE;
}
