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


#include "midi/midiev_inc.h"
#include "debug_tool/debug_tool.h"

uint_t		get_midi_channel_event(midicev_t *chan_ev, byte_t *buffer)
{
  byte_t        type = *buffer >> 4;

  debug_midi("midi channel event type: %s %X\n", midicmd_to_str(type), type);
  switch (type)
    {
    case NOTEOFF:
      chan_ev->type = type;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.note.num = buffer[1];
      chan_ev->event.note.val = buffer[2];
      debug_midi("*** NOTEOFF Event (channel:%d num:%d val:%d) ***",
                 chan_ev->chan,
                 chan_ev->event.note.num,
                 chan_ev->event.note.val);
      return 3;

    case NOTEON:
      chan_ev->event.note.val = buffer[2];
      if (chan_ev->event.note.val > 0)
        chan_ev->type = type;
      else
        chan_ev->type = NOTEOFF;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.note.num = buffer[1];
      debug_midi("*** NOTEON Event (channel:%d num:%d val:%d) ***",
                 chan_ev->chan,
                 chan_ev->event.note.num,
                 chan_ev->event.note.val);
      return 3;

    case CONTROLCHANGE:
      chan_ev->type = type;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.ctrl.num = buffer[1];
      chan_ev->event.ctrl.val = buffer[2];
      /* debug_midi("*** Controller Event ***\n" */
      /*       "\tchannel: %d\n" */
      /*       "\tnum: %d\n" */
      /*       "\tval: %d\n", */
      /*       chan_ev->chan, */
      /*       chan_ev->event.ctrl.num, */
      /*       chan_ev->event.ctrl.val); */
      return 3;

    case KEYAFTERTOUCH:
      chan_ev->type = type;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.aftertouch.num = buffer[1];
      chan_ev->event.aftertouch.val = buffer[2];
      /* debug_midi("*** Controller Event ***\n" */
      /*       "\tchannel: %d\n" */
      /*       "\tnum: %d\n" */
      /*       "\tval: %d\n", */
      /*       chan_ev->chan, */
      /*       chan_ev->event.ctrl.num, */
      /*       chan_ev->event.ctrl.val); */
      return 3;

    case PROGRAMCHANGE:
      chan_ev->type = type;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.prg_chg = buffer[1];
      /* debug_midi("*** Program change Event ***\n" */
      /*       "\tchannel: %d\n" */
      /*       "\tval: %d\n", */
      /*       chan_ev->chan, */
      /*       chan_ev->event.prg_chg); */
      return 2;

    case CHANNELAFTERTOUCH:
      chan_ev->type = type;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.chan_aftertouch = buffer[1];
      /* debug_midi("*** Program change Event ***\n" */
      /*       "\tchannel: %d\n" */
      /*       "\tval: %d\n", */
      /*       chan_ev->chan, */
      /*       chan_ev->event.prg_chg); */
      return 2;

    case PITCHWHEELCHANGE:
      chan_ev->type = type;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.pitchbend.Lval = buffer[1];
      chan_ev->event.pitchbend.Hval = buffer[2];
      /* debug_midi("*** Program change Event ***\n" */
      /*       "\tchannel: %d\n" */
      /*       "\tval: %d\n", */
      /*       chan_ev->chan, */
      /*       chan_ev->event.prg_chg); */
      return 3;

    default:
      output_error("\nUNKNOWN channel event command=%X (ptr=%p)\n", type, buffer);
    }
  return 0;
}


#define _SETBIT(byte, bitnb)   ((byte) = ((byte) | (1 << (bitnb))))
#define _UNSETBIT(byte, bitnb) ((byte) = ((byte) & (~(1 << (bitnb)))))

void set_pending_note(byte_t *pending_notes,
                      byte_t channel,
                      byte_t num)
{
  uint_t idx    = ((channel * 128) + num) / 8;
  uint_t bitnum = ((channel * 128) + num) % 8;

  _SETBIT(pending_notes[idx], bitnum);
}

void unset_pending_note(byte_t *pending_notes, byte_t channel, byte_t num)
{
  uint_t idx    = ((channel * 128) + num) / 8;
  uint_t bitnum = ((channel * 128) + num) % 8;

  _UNSETBIT(pending_notes[idx], bitnum);
}


#define _GETBIT(byte, bitnb)   (((byte) >> (bitnb)) & 1 ? TRUE : FALSE)

bool_t is_pending_notes(byte_t *pending_notes, byte_t channel, byte_t num)
{
  uint_t idx    = ((channel * 128) + num) / 8;
  uint_t bitnum = ((channel * 128) + num) % 8;

  return _GETBIT(pending_notes[idx], bitnum);
}


void update_pending_notes(byte_t *pending_notes, midicev_t *midicev)
{
  if (midicev->type == NOTEON)
    set_pending_note(pending_notes,
                     midicev->chan,
                     midicev->event.note.num);
  else if (midicev->type == NOTEOFF)
    unset_pending_note(pending_notes,
                       midicev->chan,
                       midicev->event.note.num);
}

bool_t compare_midicev(midicev_t *mcev1, midicev_t *mcev2)
{
  if (mcev1->type == mcev2->type && mcev1->chan == mcev2->chan)
    switch (mcev1->type)
      {
      case NOTEOFF:
      case NOTEON:
        if (mcev1->event.note.num == mcev2->event.note.num &&
            mcev1->event.note.val == mcev2->event.note.val)
          return TRUE;
        break;
      case KEYAFTERTOUCH:
        if (mcev1->event.aftertouch.num == mcev2->event.aftertouch.num &&
            mcev1->event.aftertouch.val == mcev2->event.aftertouch.val)
          return TRUE;
        break;
      case CONTROLCHANGE:
        if (mcev1->event.ctrl.num == mcev2->event.ctrl.num &&
            mcev1->event.ctrl.val == mcev2->event.ctrl.val)
          return TRUE;
        break;
      case PROGRAMCHANGE:
        if (mcev1->event.prg_chg == mcev2->event.prg_chg)
          return TRUE;
        break;
      case CHANNELAFTERTOUCH:
        if (mcev1->event.chan_aftertouch == mcev2->event.chan_aftertouch)
          return TRUE;
        break;
      case PITCHWHEELCHANGE:
        if (mcev1->event.pitchbend.Lval == mcev2->event.pitchbend.Lval &&
            mcev1->event.pitchbend.Hval == mcev2->event.pitchbend.Hval)
          return TRUE;
        break;
      default:
        return FALSE;
      }
  return FALSE;
}


/* #warning include a gerer */
#include "seqtool/seqtool.h"
#include <stdlib.h>
#include <strings.h>

void dump_seqev(seqev_t *seqev)
{
  midicev_t *midicev = NULL;

  output("\tseqev: addr=%p deleted=%s",
         seqev,
         seqev->deleted == TRUE ? "\033[31mTRUE\033[0m" : "FALSE");
  if (seqev->type == MIDICEV)
    {
      midicev = (midicev_t *) seqev->addr;
      output(" type=%s channel=%i", "MIDICEV", midicev->chan);
      switch (midicev->type)
        {
        case NOTEON:
          output(" | NOTEON  num=%hhd val=%hhd\n",
                 midicev->event.note.num,
                 midicev->event.note.val);
          break;
        case NOTEOFF:
          output(" | NOTEOFF num=%hhd val=%hhd\n",
                 midicev->event.note.num,
                 midicev->event.note.val);
          break;
        case CONTROLCHANGE:
          output(" | CONTROLCHANGE num=%hhd val=%hhd\n",
                 midicev->event.ctrl.num,
                 midicev->event.ctrl.val);
          break;
        case PITCHWHEELCHANGE:
          output(" | PITCHWHEELCHANGE Hval=%hhd Lval=%hhd\n",
                 midicev->event.pitchbend.Hval,
                 midicev->event.pitchbend.Lval);
          break;
        default:
          output(" | Unsupported event\n");
        }
    }
  else
    output("type=UNKNOWN\n");
}

void add_new_midicev(track_t *track, uint_t tick, midicev_t *mcev)
{
  if (mcev->type == NOTEOFF)
    add_new_seqev_head(track, tick, mcev, MIDICEV);
  else
    add_new_seqev_tail(track, tick, mcev, MIDICEV);
}

void copy_midicev_to_track(track_t *track, uint_t tick, midicev_t *mcev)
{
  midicev_t     *new_mcev = myalloc(sizeof (midicev_t));

  bcopy(mcev, new_mcev, sizeof (midicev_t));
  add_new_midicev(track, tick, new_mcev);
}

typedef struct
{
  uint_t  tick;
  track_t *track;
} list_copy_seqev_t;

void _list_copy_seqev(void *addr, void *arg)
{
  seqev_t *seqev = (seqev_t *) addr;
  list_copy_seqev_t *list_arg = arg;

  if (seqev->type == MIDICEV)
    {
      debug_midi("adding midi channel event\n");
      if (seqev->deleted == FALSE)
        copy_midicev_to_track(list_arg->track, list_arg->tick, seqev->addr);
    }
  else
    {
      output_warning(TRACE_FMT"Unsupported sequencer event\n", TRACE_ARG);
    }
}

void _list_copy_tickev(void *addr, void *track_addr)
{
  tickev_t *tickev = (tickev_t *) addr;
  track_t *track_dst = NULL;
  list_copy_seqev_t list_arg;

  if (tickev->deleted == FALSE)
    {
      track_dst = (track_t *) track_addr;
      list_arg.tick = tickev->tick;
      list_arg.track = track_dst;
      foreach_list_node(&(tickev->seqev_list), _list_copy_seqev, (void *) &list_arg);
    }
}

void copy_track_list(track_t *track_src, track_t *track_dst)
{
  foreach_list_node(&(track_src->tickev_list), _list_copy_tickev, (void *) track_dst);
}

void _list_copy_track(void *src_addr, void *dst_addr)
{
  track_t *track_src = (track_t *) src_addr;
  track_t *track_dst = (track_t *) dst_addr;

  copy_track_list(track_src, track_dst);
}

#include <string.h>
track_t *merge_all_track(char *name, list_t *track_list)
{
  track_t *track;

  if (LIST_HEAD(track_list) == NULL)
    return NULL;
  track = myalloc(sizeof (track_t));
  foreach_list_node(track_list, _list_copy_track, (void *) track);
  if (name != NULL)
    track->name = strdup(name);
  return track;
}
