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


#ifndef __MIDIEV_INC_H
#define __MIDIEV_INC_H

#include "tool/tool.h"

/* Midi event type list */
enum {
  MSQ_MIDI_NOTEOFF = 8,
  MSQ_MIDI_NOTEON,
  MSQ_MIDI_KEYAFTERTOUCH,
  MSQ_MIDI_CONTROLCHANGE,
  MSQ_MIDI_PROGRAMCHANGE,
  MSQ_MIDI_CHANNELAFTERTOUCH,
  MSQ_MIDI_PITCHWHEELCHANGE,
  MSQ_MIDI_SYSTEM
};

typedef struct
{
  byte_t type, chan;
  union
  {
    struct { byte_t num, val; }   note;
    struct { byte_t num, val; }   aftertouch;
    struct { byte_t num, val; }   ctrl;
    byte_t                        prg_chg;
    byte_t                        chan_aftertouch;
    struct { byte_t Lval, Hval; } pitchbend;
  }  event;
} midicev_t;

#define midicmd_to_str(cmd)                                             \
  ((cmd) == MSQ_MIDI_NOTEOFF ?           "NOTEOFF" :                    \
   (cmd) == MSQ_MIDI_NOTEON ?            "NOTEON" :                     \
   (cmd) == MSQ_MIDI_KEYAFTERTOUCH ?     "KEYAFTERTOUCH" :              \
   (cmd) == MSQ_MIDI_CONTROLCHANGE ?     "CONTROLCHANGE" :              \
   (cmd) == MSQ_MIDI_PROGRAMCHANGE ?     "PROGRAMCHANGE" :              \
   (cmd) == MSQ_MIDI_CHANNELAFTERTOUCH ? "CHANNELAFTERTOUCH" :          \
   (cmd) == MSQ_MIDI_PITCHWHEELCHANGE ?  "PITCHWHEELCHANGE" :           \
   "UNKNOWN")

/* #define midievtype_to_str(buf)                                          \ */
/*   ((buf) >> 4 == NOTEOFF ? "NOTEOFF" :                                  \ */
/*    (buf) >> 4 == NOTEON ? "NOTEON" :                                    \ */
/*    (buf) >> 4 == KEYAFTERTOUCH ? "KEYAFTERTOUCH" :                      \ */
/*    (buf) >> 4 == CONTROLCHANGE ? "CONTROLCHANGE" :                      \ */
/*    (buf) >> 4 == PROGRAMCHANGE ? "PROGRAMCHANGE" :                      \ */
/*    (buf) >> 4 == CHANNELAFTERTOUCH ? "CHANNELAFTERTOUCH" :              \ */
/*    (buf) >> 4 == PITCHWHEELCHANGE ? "PITCHWHEELCHANGE" :                \ */
/*    (buf) == 0xFF ? "META-EVENT" :                                       \ */
/*    "UNKNOWN")d */


/* Meta event list */
#define ME_SEQUENCENUMBER       0x00
#define ME_TEXTEVENT            0x01
#define ME_COPYRIGHTNOTICE      0x02
#define ME_NAME                 0x03
#define ME_CUEPOINT             0x07
#define ME_ENDOFTRACK           0x2F
#define ME_SETTEMPO             0x51
#define ME_SMPTE_OFFSET         0x54
#define ME_TIMESIGNATURE        0x58
#define ME_KEYSIGNATURE         0x59
#define ME_SEQUENCERSPECIFIC    0x7F

#define midime_to_str(type) ((type) == ME_SEQUENCENUMBER ? "ME_SEQUENCENUMBER" : \
                             (type) == ME_TEXTEVENT ? "ME_TEXTEVENT" :	\
                             (type) == ME_COPYRIGHTNOTICE ? "ME_COPYRIGHTNOTICE" : \
                             (type) == ME_NAME ? "ME_NAME" :		\
                             (type) == ME_CUEPOINT ? "ME_CUEPOINT" :	\
                             (type) == ME_ENDOFTRACK ? "ME_ENDOFTRACK" : \
                             (type) == ME_SETTEMPO  ? "ME_SETTEMPO" :   \
                             (type) == ME_SMPTE_OFFSET ? "ME_SMPTE_OFFSET" : \
			     (type) == ME_TIMESIGNATURE ? "ME_TIMESIGNATURE" : \
                             (type) == ME_KEYSIGNATURE  ? "ME_KEYSIGNATURE" : \
			     "UNKNOWN")

typedef struct
{
  byte_t        type;
  uint_t        size;
  byte_t        data[256];
  uint_t        val;
} midimev_t;

/* Handled MMC */
#define MMC_STOP  0x01
#define MMC_PLAY  0x02
#define MMC_FWD   0x04
#define MMC_RWD   0x05
#define MMC_RECS  0x06
#define MMC_PAUSE 0x09

uint_t get_midibuf_deltatime(byte_t *);
uint_t get_varlen_from_idx(byte_t *buffer, uint_t *offset);
uint_t get_varlen_from_ptr(byte_t **buffer);
/* uint_t get_deltatime_from_ptr(byte_t **); /\* a modifier *\/ */

uint_t     get_midi_meta_event(midimev_t *meta_ev, byte_t *buffer);
uint_t     get_midi_channel_event(midicev_t *chan_ev, byte_t *buffer);
void       set_pending_note(byte_t *pending_notes, byte_t channel, byte_t num);
void       unset_pending_note(byte_t *pending_notes, byte_t channel, byte_t num);
void       update_pending_notes(byte_t *noteson_state, midicev_t *midicev);
msq_bool_t is_pending_notes(byte_t *pending_notes, byte_t channel, byte_t num);
msq_bool_t compare_midicev(midicev_t *mcev1, midicev_t *mcev2);


#endif
