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


#include "midi/midiev_inc.h"
#include "debug_tool/debug_tool.h"
#include <string.h>

void msq_mcev_tick_list_add(list_t *mcev_tick_list,
                            uint_t tick,
                            midicev_t *mcev)
{
  msq_mcev_tick_t *mcev_tick = malloc(sizeof (msq_mcev_tick_t));

  mcev_tick->tick = tick;
  memcpy(&(mcev_tick->mcev), mcev, sizeof (midicev_t));

  push_to_list_tail(mcev_tick_list, mcev_tick);
}

msq_mcev_tick_t *_msq_mcev_tick_list_pop_prev_noteon(list_t *mcev_tick_list,
                                                     uint_t tick,
                                                     byte_t channel,
                                                     byte_t num)
{
  list_iterator_t iter;
  msq_mcev_tick_t *mcev_tick;

  for (iter_init(&iter, mcev_tick_list);
       iter_node(&iter);
       iter_next(&iter))
    {
      mcev_tick = iter_node_ptr(&iter);
      if (mcev_tick->tick < tick
          && mcev_tick->mcev.chan == channel
          && mcev_tick->mcev.event.note.num == num)
        {
          iter_node_del(&iter, NULL);
          return mcev_tick;
        }
    }
  return NULL;
}

void msq_mcev_tick_list_clear(list_t *mcev_tick_list)
{
  free_list_node(mcev_tick_list, free);
}

uint_t get_midi_channel_event(midicev_t *chan_ev, byte_t *buffer)
{
  byte_t        type = *buffer >> 4;

  debug_midi("midi channel event type: %s %X\n", midicmd_to_str(type), type);
  switch (type)
    {
    case MSQ_MIDI_NOTEOFF:
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

    case MSQ_MIDI_NOTEON:
      chan_ev->event.note.val = buffer[2];
      if (chan_ev->event.note.val > 0)
        chan_ev->type = type;
      else
        chan_ev->type = MSQ_MIDI_NOTEOFF;
      chan_ev->chan = *buffer & 0xF;
      /* chan_ev->chan--; */
      chan_ev->event.note.num = buffer[1];
      debug_midi("*** NOTEON Event (channel:%d num:%d val:%d) ***",
                 chan_ev->chan,
                 chan_ev->event.note.num,
                 chan_ev->event.note.val);
      return 3;

    case MSQ_MIDI_CONTROLCHANGE:
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

    case MSQ_MIDI_KEYAFTERTOUCH:
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

    case MSQ_MIDI_PROGRAMCHANGE:
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

    case MSQ_MIDI_CHANNELAFTERTOUCH:
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

    case MSQ_MIDI_PITCHWHEELCHANGE:
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


#define _GETBIT(byte, bitnb)   (((byte) >> (bitnb)) & 1 ? MSQ_TRUE : MSQ_FALSE)

msq_bool_t is_pending_notes(byte_t *pending_notes, byte_t channel, byte_t num)
{
  uint_t idx    = ((channel * 128) + num) / 8;
  uint_t bitnum = ((channel * 128) + num) % 8;

  return _GETBIT(pending_notes[idx], bitnum);
}


void update_pending_notes(byte_t *pending_notes, midicev_t *midicev)
{
  if (midicev->type == MSQ_MIDI_NOTEON)
    set_pending_note(pending_notes,
                     midicev->chan,
                     midicev->event.note.num);
  else if (midicev->type == MSQ_MIDI_NOTEOFF)
    unset_pending_note(pending_notes,
                       midicev->chan,
                       midicev->event.note.num);
}

msq_bool_t compare_midicev(midicev_t *mcev1, midicev_t *mcev2)
{
  if (mcev1->type == mcev2->type && mcev1->chan == mcev2->chan)
    switch (mcev1->type)
      {
      case MSQ_MIDI_NOTEOFF:
      case MSQ_MIDI_NOTEON:
        if (mcev1->event.note.num == mcev2->event.note.num &&
            mcev1->event.note.val == mcev2->event.note.val)
          return MSQ_TRUE;
        break;
      case MSQ_MIDI_KEYAFTERTOUCH:
        if (mcev1->event.aftertouch.num == mcev2->event.aftertouch.num &&
            mcev1->event.aftertouch.val == mcev2->event.aftertouch.val)
          return MSQ_TRUE;
        break;
      case MSQ_MIDI_CONTROLCHANGE:
        if (mcev1->event.ctrl.num == mcev2->event.ctrl.num &&
            mcev1->event.ctrl.val == mcev2->event.ctrl.val)
          return MSQ_TRUE;
        break;
      case MSQ_MIDI_PROGRAMCHANGE:
        if (mcev1->event.prg_chg == mcev2->event.prg_chg)
          return MSQ_TRUE;
        break;
      case MSQ_MIDI_CHANNELAFTERTOUCH:
        if (mcev1->event.chan_aftertouch == mcev2->event.chan_aftertouch)
          return MSQ_TRUE;
        break;
      case MSQ_MIDI_PITCHWHEELCHANGE:
        if (mcev1->event.pitchbend.Lval == mcev2->event.pitchbend.Lval &&
            mcev1->event.pitchbend.Hval == mcev2->event.pitchbend.Hval)
          return MSQ_TRUE;
        break;
      default:
        return MSQ_FALSE;
      }
  return MSQ_FALSE;
}


/* #warning include a gerer */
#include "seqtool/seqtool.h"
#include <stdlib.h>
#include <strings.h>

void dump_midicev(midicev_t *midicev)
{
  switch (midicev->type)
    {
    case MSQ_MIDI_NOTEON:
      output("NOTEON  num=%hhd val=%hhd\n",
             midicev->event.note.num,
             midicev->event.note.val);
      break;
    case MSQ_MIDI_NOTEOFF:
      output("NOTEOFF num=%hhd val=%hhd\n",
             midicev->event.note.num,
             midicev->event.note.val);
      break;
    case MSQ_MIDI_CONTROLCHANGE:
      output("CONTROLCHANGE num=%hhd val=%hhd\n",
             midicev->event.ctrl.num,
             midicev->event.ctrl.val);
      break;
    case MSQ_MIDI_PITCHWHEELCHANGE:
      output("PITCHWHEELCHANGE Hval=%hhd Lval=%hhd\n",
             midicev->event.pitchbend.Hval,
             midicev->event.pitchbend.Lval);
      break;
    case MSQ_MIDI_KEYAFTERTOUCH:
      output("KEYAFTERTOUCH num=%hhd val=%hhd\n",
             midicev->event.aftertouch.num,
             midicev->event.aftertouch.val);
      break;
    case MSQ_MIDI_PROGRAMCHANGE:
      output("PROGRAMCHANGE %hhd\n",
             midicev->event.prg_chg);
      break;
    case MSQ_MIDI_CHANNELAFTERTOUCH:
      output("CHANNELAFTERTOUCH %hhd\n",
             midicev->event.chan_aftertouch);
      break;
    default:
      output("Unsupported event\n");
    }
}

void dump_seqev(seqev_t *seqev)
{
  midicev_t *midicev = NULL;

  output("\tseqev: addr=%p deleted=%s",
         seqev,
         seqev->deleted == MSQ_TRUE ? "\033[31mTRUE\033[0m" : "FALSE");
  if (seqev->type == MIDICEV)
    {
      midicev = (midicev_t *) seqev->addr;
      output(" type=%s channel=%i | ", "MIDICEV", midicev->chan);
      dump_midicev(midicev);
    }
  else
    output("type=UNKNOWN\n");
}

void msq_dump_tickev(tickev_t *tickev)
{
  list_iterator_t seqevit;

  output("At tick %d %s got %d event(s)\n",
         tickev->tick,
         tickev->deleted == MSQ_TRUE ? "(\033[31mdeleted\033[0m)" : "(not deleted)",
         tickev->seqev_list.len);
  for (iter_init(&seqevit, &(tickev->seqev_list));
       iter_node(&seqevit) != NULL;
       iter_next(&seqevit))
    dump_seqev((seqev_t *) iter_node_ptr(&(seqevit)));
}

void msq_dump_track(track_t *track)
{
  list_iterator_t tickit;

  output("track \"%s\" got %d tick event(s)\n", track->name, track->tickev_list.len);
  for (iter_init(&tickit, &(track->tickev_list));
       iter_node(&tickit) != NULL;
       iter_next(&tickit))
      msq_dump_tickev((tickev_t *) iter_node_ptr(&tickit));
}

void add_new_midicev(track_t *track, uint_t tick, midicev_t *mcev)
{
  if (mcev->type == MSQ_MIDI_NOTEOFF)
    add_new_seqev_head(track, tick, mcev, MIDICEV);
  else
    add_new_seqev_tail(track, tick, mcev, MIDICEV);
}

void copy_midicev_to_track(track_t *track, uint_t tick, midicev_t *mcev)
{
  midicev_t     *new_mcev = myalloc(sizeof (midicev_t));

  memcpy(new_mcev, mcev, sizeof (midicev_t));
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
#ifdef DEBUG_MIDI_MODE
      output("copying_seqev\ttick:% 7d", list_arg->tick);
      dump_seqev(seqev);
#endif
      if (seqev->deleted == MSQ_FALSE)
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

  if (tickev->deleted == MSQ_FALSE)
    {
      track_dst = (track_t *) track_addr;
      list_arg.tick = tickev->tick;
      list_arg.track = track_dst;
      foreach_list_node(&(tickev->seqev_list), _list_copy_seqev, (void *) &list_arg);
    }
}

void copy_track_list(track_t *track_src, track_t *track_dst)
{
#ifdef DEBUG_MIDI_MODE
  output("\ncopying track %s\n", track_src->name);
#endif
  foreach_list_node(&(track_src->tickev_list), _list_copy_tickev, (void *) track_dst);
}

void _list_copy_track(void *src_addr, void *dst_addr)
{
  track_t *track_src = (track_t *) src_addr;
  track_t *track_dst = (track_t *) dst_addr;

  copy_track_list(track_src, track_dst);
}

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
