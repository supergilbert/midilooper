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

#include "debug_tool/debug_tool.h"
#include "midi/midiev_inc.h"

bool_t write_midicev(byte_t *buf, midicev_t *midicev)
{
  bool_t ret = TRUE;

  switch (midicev->type)
    {
    case NOTEOFF:
    case NOTEON:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.note.num & 0xFF;
      buf[2] = midicev->event.note.val & 0xFF;
      break;
    case KEYAFTERTOUCH:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.aftertouch.num & 0xFF;
      buf[2] = midicev->event.aftertouch.val & 0xFF;
      break;
    case CONTROLCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.ctrl.num & 0xFF;
      buf[2] = midicev->event.ctrl.val & 0xFF;
      break;
    case PROGRAMCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.prg_chg;
      buf[2] = 0;
      break;
    case CHANNELAFTERTOUCH:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.chan_aftertouch;
      buf[2] = 0;
      break;
    case PITCHWHEELCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.pitchbend.Lval & 0xFF;
      buf[2] = midicev->event.pitchbend.Hval & 0xFF;
      break;
    default:
      output_error("Unexpected channel event type\n");
      ret = FALSE;
      break;
    }
  return ret;
}
