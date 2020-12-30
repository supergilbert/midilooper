/* Copyright 2012-2020 Gilbert Romer */

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

#include "debug_tool/debug_tool.h"
#include "midi/midiev_inc.h"

msq_bool_t convert_midicev_to_mididata(midicev_t *midicev, byte_t *buf)
{
  msq_bool_t ret = MSQ_FALSE;

  switch (midicev->type)
    {
    case MSQ_MIDI_NOTEOFF:
    case MSQ_MIDI_NOTEON:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.note.num & 0xFF;
      buf[2] = midicev->event.note.val & 0xFF;
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_KEYAFTERTOUCH:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.aftertouch.num & 0xFF;
      buf[2] = midicev->event.aftertouch.val & 0xFF;
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_CONTROLCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.ctrl.num & 0xFF;
      buf[2] = midicev->event.ctrl.val & 0xFF;
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_PROGRAMCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.prg_chg;
      buf[2] = 0;
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_CHANNELAFTERTOUCH:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.chan_aftertouch;
      buf[2] = 0;
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_PITCHWHEELCHANGE:
      buf[0] = (midicev->type << 4) + midicev->chan;
      buf[1] = midicev->event.pitchbend.Lval & 0xFF;
      buf[2] = midicev->event.pitchbend.Hval & 0xFF;
      ret = MSQ_TRUE;
      break;
    default:
      output_error("Unexpected channel event type\n");
      break;
    }
  return ret;
}

msq_bool_t convert_mididata_to_midicev(byte_t *buf, midicev_t *midicev)
{
  msq_bool_t ret = MSQ_FALSE;
  byte_t     type = buf[0] >> 4;

  switch (type)
    {
    case MSQ_MIDI_NOTEOFF:
    case MSQ_MIDI_NOTEON:
      midicev->type = type;
      midicev->chan = buf[0] & 0xF;
      midicev->event.note.num = buf[1];
      midicev->event.note.val = buf[2];
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_KEYAFTERTOUCH:
      midicev->type = type;
      midicev->chan = buf[0] & 0xF;
      midicev->event.aftertouch.num = buf[1];
      midicev->event.aftertouch.val = buf[2];
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_CONTROLCHANGE:
      midicev->type = type;
      midicev->chan = buf[0] & 0xF;
      midicev->event.ctrl.num = buf[1];
      midicev->event.ctrl.val = buf[2];
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_PROGRAMCHANGE:
      midicev->type = type;
      midicev->chan = buf[0] & 0xF;
      midicev->event.prg_chg = buf[1];
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_CHANNELAFTERTOUCH:
      midicev->type = type;
      midicev->chan = buf[0] & 0xF;
      midicev->event.chan_aftertouch = buf[1];
      ret = MSQ_TRUE;
      break;
    case MSQ_MIDI_PITCHWHEELCHANGE:
      midicev->type = type;
      midicev->chan = buf[0] & 0xF;
      midicev->event.pitchbend.Lval = buf[1];
      midicev->event.pitchbend.Hval = buf[2];
      ret = MSQ_TRUE;
      break;
    default:
      output_error("Unexpected channel event type\n");
      break;
    }

  return ret;
}
