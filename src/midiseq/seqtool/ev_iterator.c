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


#include "asound/aseq.h"
#include "seqtool/seqtool.h"
#include "./ev_iterator.h"

void dump_evit(ev_iterator_t *evit)
{
  printf("evit:%p tick:%d tickit->node:%p sevit->node:%p\n",
         evit, evit->tick, evit->tickit.node, evit->seqevit.node);
  dump_seqev((seqev_t *)evit->seqevit.node->addr);
}

seqev_t *_it_next_seqev(list_iterator_t *seqevit)
{
  seqev_t *ev = NULL;


  for (iter_next(seqevit);
       iter_node(seqevit) != NULL;
       iter_next(seqevit))
    {
      ev = (seqev_t *) iter_node_ptr(seqevit);
      if (ev->deleted == MSQ_FALSE)
        return ev;
    }
  return NULL;
}

/* #include "debug_tool/debug_tool.h" */
/* #define print_trace(format, ...) output(TRACE_FMT format "\n", TRACE_ARG, ##__VA_ARGS__) */

seqev_t *_evit_seqevit_head(list_iterator_t *seqevit)
{
  seqev_t *ev = NULL;

  iter_head(seqevit);
  if (iter_node(seqevit))
    {
      ev = (seqev_t *) iter_node_ptr(seqevit);
      if (ev->deleted == MSQ_TRUE)
        {
          ev = _it_next_seqev(seqevit);
        }
    }
  return ev;
}

seqev_t *_evit_seqevit_init(list_iterator_t *seqevit, list_t *seqev_list)
{
  iter_init(seqevit, seqev_list);
  return _evit_seqevit_head(seqevit);
}


seqev_t *evit_next_tick(ev_iterator_t *ev_iterator)
{
  tickev_t *tickev = NULL;
  seqev_t  *seqev  = NULL;


  if (iter_node(&(ev_iterator->tickit)))
    {
      for (iter_next(&(ev_iterator->tickit));
           iter_node(&(ev_iterator->tickit)) != NULL;
           iter_next(&(ev_iterator->tickit)))
        {
          tickev = (tickev_t *) iter_node_ptr(&(ev_iterator->tickit));
          if (tickev->deleted == MSQ_FALSE)
            {
              seqev = _evit_seqevit_init(&(ev_iterator->seqevit), &(tickev->seqev_list));
              if (seqev)
                {
                  ev_iterator->tick = tickev->tick;
                  return seqev;
                }
            }
        }
    }
  return NULL;
}

seqev_t *evit_tick_head(ev_iterator_t *ev_iterator)
{
  tickev_t *tickev = NULL;
  seqev_t  *seqev  = NULL;

  iter_head(&(ev_iterator->tickit));
  if (iter_node(&(ev_iterator->tickit)))
    {
      tickev = (tickev_t *) iter_node_ptr(&(ev_iterator->tickit));
      if (tickev->deleted == MSQ_TRUE)
        {
          seqev = evit_next_tick(ev_iterator);
        }
      else
        {
          seqev = _evit_seqevit_init(&(ev_iterator->seqevit), &(tickev->seqev_list));
          ev_iterator->tick = tickev->tick;
        }
    }
  return seqev;
}

seqev_t *evit_init(ev_iterator_t *ev_iterator, list_t *tickev_list)
{
  iter_init(&(ev_iterator->tickit), tickev_list);
  return evit_tick_head(ev_iterator);
}

midicev_t *evit_init_midiallchannel(ev_iterator_t *ev_iterator,
                                    list_t *tickev_list)
{
  midicev_t *midicev = NULL;
  seqev_t   *seqev   = NULL;

  seqev = evit_init(ev_iterator, tickev_list);
  if (seqev)
    {
      if (seqev->type == MIDICEV)
        midicev = seqev->addr;
      else
        midicev = evit_next_midiallchannel(ev_iterator);
    }
  return midicev;
}

midicev_t *evit_init_midicev(ev_iterator_t *ev_iterator,
                             list_t *tickev_list,
                             byte_t channel)
{
  midicev_t *midicev = NULL;

  midicev = evit_init_midiallchannel(ev_iterator, tickev_list);
  if (midicev && midicev->chan != channel)
    midicev = evit_next_midicev(ev_iterator, channel);
  return midicev;
}

midicev_t *evit_init_noteon(ev_iterator_t *ev_iterator,
                            list_t *tickev_list,
                            byte_t channel)
{
  midicev_t *midicev = evit_init_midicev(ev_iterator, tickev_list, channel);

  if (midicev && midicev->type != NOTEON)
    midicev = evit_next_noteon(ev_iterator, channel);
  return midicev;
}

midicev_t *evit_first_noteon(ev_iterator_t *ev_iterator, byte_t channel)
{
  seqev_t   *seqev = evit_tick_head(ev_iterator);
  midicev_t *mcev  = NULL;

  if (seqev->type == MIDICEV)
    {
      mcev = seqev->addr;
      if (mcev->chan == channel)
        return mcev;
    }
  else
    mcev = evit_next_noteon(ev_iterator, channel);
  return mcev;
}


midicev_t *evit_next_ctrl_num(ev_iterator_t *ev_iterator,
                              byte_t channel,
                              byte_t ctrl_num)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_ctrl(ev_iterator, channel);
       midicev != NULL;
       midicev = evit_next_ctrl(ev_iterator, channel))
    if (midicev->event.ctrl.num == ctrl_num)
      return midicev;
  return NULL;
}

midicev_t *evit_init_ctrl_num(ev_iterator_t *ev_iterator,
                              list_t *tickev_list,
                              byte_t channel,
                              byte_t ctrl_num)
{
  midicev_t *midicev = evit_init_midicev(ev_iterator, tickev_list, channel);

  if (midicev && midicev->type != CONTROLCHANGE)
    midicev = evit_next_ctrl_num(ev_iterator, channel, ctrl_num);
  return midicev;
}

midicev_t *evit_init_pitch(ev_iterator_t *ev_iterator,
                           list_t *tickev_list,
                           byte_t channel)
{
  midicev_t *midicev = evit_init_midicev(ev_iterator, tickev_list, channel);

  if (midicev && midicev->type != PITCHWHEELCHANGE)
    midicev = evit_next_pitch(ev_iterator, channel);
  return midicev;
}

msq_bool_t compare_midicev_type(midicev_t *mcev1, midicev_t *mcev2)
{
  if (mcev1->type == mcev2->type && mcev1->chan == mcev2->chan)
    switch (mcev1->type)
      {
      case NOTEOFF:
      case NOTEON:
        if (mcev1->event.note.num == mcev2->event.note.num)
          return MSQ_TRUE;
        break;
      case KEYAFTERTOUCH:
        if (mcev1->event.aftertouch.num == mcev2->event.aftertouch.num)
          return MSQ_TRUE;
        break;
      case CONTROLCHANGE:
        if (mcev1->event.ctrl.num == mcev2->event.ctrl.num)
          return MSQ_TRUE;
        break;
      case PROGRAMCHANGE:
      case CHANNELAFTERTOUCH:
      case PITCHWHEELCHANGE:
        return MSQ_TRUE;
        break;
      default:
        return MSQ_FALSE;
      }
  return MSQ_FALSE;
}

midicev_t *evit_searchev(ev_iterator_t *evit, uint_t tick, midicev_t *mcev)
{
  seqev_t       *ev = NULL;
  midicev_t     *midicev = NULL;

  for (ev = evit_tick_head(evit);
       (ev && evit->tick <= tick);
       ev = evit_next_tick(evit))
    {
      if (evit->tick == tick)
        {
          if (ev->type == MIDICEV)
            {
              midicev = (midicev_t *) ev->addr;
              if (compare_midicev_type(midicev, mcev))
                return midicev;
            }
          for (ev = _it_next_seqev(&(evit->seqevit));
               ev;
               ev = _it_next_seqev(&(evit->seqevit)))
            {
              if (ev->type == MIDICEV)
                {
                  midicev = (midicev_t *) ev->addr;
                  if (compare_midicev_type(midicev, mcev))
                    return midicev;
                }
            }
          return NULL;
        }
    }
  return NULL;
}

seqev_t *evit_get_seqev(ev_iterator_t *ev_iterator)
{
  if (iter_node(&(ev_iterator->tickit)) && iter_node(&(ev_iterator->seqevit)))
    return (seqev_t *) iter_node_ptr(&(ev_iterator->seqevit));
  return NULL;
}

/* seqev_t *_evit_next_seqev(ev_iterator_t *ev_iterator) */
/* { */
/*   return _it_next_seqev(&(ev_iterator->seqevit)); */
/* } */


seqev_t *evit_next_seqev(ev_iterator_t *ev_iterator)
{
  seqev_t *ev = NULL;

  if (iter_node(&(ev_iterator->tickit)) && iter_node(&(ev_iterator->seqevit)))
    {
      ev = _it_next_seqev(&(ev_iterator->seqevit));
      if (ev == NULL)
        ev = evit_next_tick(ev_iterator);
    }
  return ev;
}

midicev_t *evit_next_midiallchannel(ev_iterator_t *ev_iterator)
{
  seqev_t   *ev = NULL;

  for (ev = evit_next_seqev(ev_iterator);
       ev != NULL;
       ev = evit_next_seqev(ev_iterator))
    if (ev->type == MIDICEV)
      return (midicev_t *) ev->addr;
  return NULL;
}

midicev_t *evit_next_midicev(ev_iterator_t *ev_iterator, byte_t channel)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_midiallchannel(ev_iterator);
       midicev != NULL;
       midicev = evit_next_midiallchannel(ev_iterator))
    if (midicev->chan == channel)
      return midicev;
  return NULL;
}


midicev_t *evit_next_midicev_type(ev_iterator_t *ev_iterator, byte_t channel, byte_t type)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_midicev(ev_iterator, channel);
       midicev != NULL;
       midicev = evit_next_midicev(ev_iterator, channel))
    if (midicev->type == type)
      return midicev;
  return NULL;
}


midicev_t *evit_next_noteoff_num(ev_iterator_t *ev_iterator, byte_t channel, byte_t num)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_noteoff(ev_iterator, channel);
       midicev != NULL;
       midicev = evit_next_noteoff(ev_iterator, channel))
    if (midicev->event.note.num == num)
      return midicev;
  return NULL;
}


void evit_copy(ev_iterator_t *evit_src, ev_iterator_t *evit_dst)
{
  iter_copy(&(evit_src->tickit), &(evit_dst->tickit));
  iter_copy(&(evit_src->seqevit), &(evit_dst->seqevit));
  evit_dst->tick = evit_src->tick;
}

void evit_del_event(ev_iterator_t *ev_iterator)
{
  iter_node_del(&(ev_iterator->seqevit), free_seqev);
  if (ev_iterator->seqevit.list->len <= 0)
    iter_node_del(&(ev_iterator->tickit), free_tickev);
}

node_t *_add_new_midicev(list_t *seqev_list, midicev_t *midicev)
{
  seqev_t  *seqev     = alloc_seqev(midicev, MIDICEV);

  if (midicev->type == NOTEOFF)
    return push_to_list(seqev_list, (void *) seqev);
  else
    return push_to_list_tail(seqev_list, (void *) seqev);
}

node_t *search_or_add_midicev(list_t *seqev_list, midicev_t *midicev)
{
  list_iterator_t seqevit;
  seqev_t         *ev = NULL;
  midicev_t       *mcev = NULL;

  for (ev = _evit_seqevit_init(&seqevit, seqev_list);
       ev != NULL;
       ev = _it_next_seqev(&seqevit))
    if (ev->type == MIDICEV)
      {
        mcev = (midicev_t *) ev->addr;
        if (compare_midicev_type(midicev, mcev) == MSQ_TRUE)
          {
            /* TODO (function to copy only the necessary) */
            bcopy(midicev, mcev, sizeof (midicev_t));
            return seqevit.node;
          }
      }
  mcev = myalloc(sizeof (midicev_t));
  bcopy(midicev, mcev, sizeof (midicev_t));
  return _add_new_midicev(seqev_list, mcev);
}

void evit_add_midicev(ev_iterator_t *evit, uint_t tick, midicev_t *mcev)
{
  node_t   *tick_node = search_or_add_ticknode(evit->tickit.list, tick);
  tickev_t *tickev    = tick_node->addr;
  node_t   *seq_node  = search_or_add_midicev(&(tickev->seqev_list), mcev);

  evit->tickit.node  = tick_node;
  evit->seqevit.list = &(tickev->seqev_list);
  evit->seqevit.node = seq_node;
  evit->tick         = tickev->tick;
}

#include "debug_tool/debug_tool.h"

msq_bool_t _evit_check(ev_iterator_t *evit, list_t *tickev_list)
{
  ev_iterator_t evit_ptr;
  seqev_t       *ev;

  if (evit->tickit.list != tickev_list)
    {
      output_error("Tick event list mismatch");
      return MSQ_FALSE;
    }
  for (ev = evit_init(&evit_ptr, tickev_list);
       ev;
       ev = evit_next_tick(&evit_ptr))
    {
      if (evit_ptr.tick < evit->tick)
        continue;
      else
        {
          if (evit_ptr.tick != evit->tick)
            {
              break;
            }
          if (iter_node(&(evit_ptr.tickit)) != iter_node(&(evit->tickit)))
            {
              break;
            }
          if (iter_node(&(evit_ptr.seqevit)) == iter_node(&(evit->seqevit)))
            return MSQ_TRUE;
          else
            while ((ev = _it_next_seqev(&(evit_ptr.seqevit))) != NULL)
              {
                if (iter_node(&(evit_ptr.seqevit)) == iter_node(&(evit->seqevit)))
                  return MSQ_TRUE;
              }
          output_error("sequencer event iterator mismatch");
          return MSQ_FALSE;
        }
    }
  output_error("tick iterator mismatch");
  return MSQ_FALSE;
}

msq_bool_t note_collision(uint_t tick,
                          byte_t channel,
                          byte_t note,
                          list_t *tickev_list)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  midicev_t *noteon = evit_init_noteon(&evit_noteon,
                                       tickev_list,
                                       channel),
    *noteoff;

  while (noteon != NULL && evit_noteon.tick < tick)
    {
      if (noteon->event.note.num == note)
        {
          evit_copy(&evit_noteon, &evit_noteoff);
          noteoff = evit_next_noteoff_num(&evit_noteoff, channel, note);
          if (noteoff == NULL)
            {
              output_error("noteoff missing");
              return MSQ_FALSE;
            }
          if (evit_noteoff.tick > tick)
            return MSQ_TRUE;
        }
      noteon = evit_next_noteon(&evit_noteon, channel);
    }

  return MSQ_FALSE;
}
