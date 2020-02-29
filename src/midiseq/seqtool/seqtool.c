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


#include "seqtool/seqtool.h"
#include <stdlib.h>
#include <strings.h>

#include "debug_tool/debug_tool.h"


void dumpaddr_seqevlist(list_t *seqev_list)
{
  list_iterator_t evit;

  for (iter_init(&evit, seqev_list);
       iter_node(&evit);
       iter_next(&evit))
    output("ev node addr = %p\n", iter_node(&evit));
}


void free_seqev(void *addr)
{
  seqev_t *seqev = (seqev_t *) addr;

  if (seqev)
    {
      switch (seqev->type)
        {
        case ASEQTYPE:
        case MIDICEV:
          free(seqev->addr);
          break;
        default:
          break;
        }
      free(seqev);
    }
}


void free_tickev(void *addr)
{
  tickev_t *tickev = (tickev_t *) addr;

  if (tickev == NULL)
    return;
  free_list_node(&(tickev->seqev_list), free_seqev);
  free(tickev);
}


void clear_tickev_list(list_t *tickev_list)
{
  free_list_node(tickev_list, free_tickev);
  tickev_list->len = 0;
}


void free_track(void *addr)
{
  track_t *track = (track_t *) addr;

  if (track)
    {
      free_list_node(&(track->tickev_list), free_tickev);
      if (track->name != NULL)
        free(track->name);
      free(track);
    }
}


tickev_t *_create_new_tick_ev(uint_t tick)
{
  tickev_t     *tickev = myalloc(sizeof (tickev_t));

  bzero(tickev, sizeof (tickev_t));
  tickev->tick = tick;
  tickev->deleted = MSQ_FALSE;
  return tickev;
}


/* Search in track for  tick event set tick time if the tick time doesnt
   exit it create a new one */
node_t *search_or_add_ticknode(list_t *tickev_list, uint_t tick)
{
  list_iterator_t iter;
  tickev_t *tickev = NULL;

  if (LIST_HEAD(tickev_list) == NULL)
    {
      tickev = _create_new_tick_ev(tick);
      return push_to_list(tickev_list, (void *) tickev);
    }

  iter_init(&iter, tickev_list);
  while (iter_node(&iter))
    {
      tickev = iter_node_ptr(&iter);
      if (tickev->tick == tick)
        {
          tickev->deleted = MSQ_FALSE;
          return iter_node(&iter);
        }
      else if (tickev->tick > tick)
        {
          tickev = _create_new_tick_ev(tick);
          return iter_push_before(&iter, (void *) tickev);
        }
      iter_next(&iter);
    }
  tickev = _create_new_tick_ev(tick);
  return push_to_list_tail(tickev_list, tickev);
}

tickev_t *search_or_add_tickev(track_t *track, uint_t tick)
{
  node_t *ticknode = search_or_add_ticknode(&(track->tickev_list), tick);
  return (tickev_t *) ticknode->addr;
}

seqev_t *alloc_seqev(void *addr,
                     seqevtype_t type)
{
  seqev_t *seqev;

  seqev = myalloc(sizeof (seqev_t));
  bzero(seqev, sizeof (seqev_t));
  seqev->addr = addr;
  seqev->type = type;
  seqev->deleted = MSQ_FALSE;
  return seqev;
}


void add_new_seqev_tail(track_t *track,
                        uint_t tick,
                        void *addr,
                        seqevtype_t type)
{
  tickev_t *tickev = NULL;
  seqev_t *seqev;

  tickev = search_or_add_tickev(track, tick);
  seqev = alloc_seqev(addr, type);
  push_to_list_tail(&(tickev->seqev_list), (void *) seqev);
}


void add_new_seqev_head(track_t *track,
                        uint_t tick,
                        void *addr,
                        seqevtype_t type)
{
  tickev_t *tickev = NULL;
  seqev_t *seqev;

  tickev = search_or_add_tickev(track, tick);
  seqev = alloc_seqev(addr, type);
  push_to_list(&(tickev->seqev_list), (void *) seqev);
}


node_t *search_ticknode(list_t *tickev_list, uint_t tick)
{
  tickev_t      *tickev = NULL;
  list_iterator_t iter;

  iter_init(&iter, tickev_list);
  while (iter_node(&iter))
    {
      tickev = iter_node_ptr(&iter);
      if (tickev->tick == tick)
        return iter_node(&iter);
      iter_next(&iter);
    }
  return NULL;
}


void goto_next_available_tick(list_iterator_t *tickit, uint_t tick)
{
  tickev_t *tickev = NULL;

  for (iter_head(tickit);
       iter_node(tickit);
       iter_next(tickit))
    {
      tickev = (tickev_t *) iter_node_ptr(tickit);
      if (tickev->tick >= tick && tickev->deleted == MSQ_FALSE)
        return;
    }
}


void iter_next_available_tick(list_iterator_t *tickit)
{
  tickev_t *tickev = NULL;

  for (iter_next(tickit);
       iter_node(tickit);
       iter_next(tickit))
    {
      tickev = (tickev_t *) iter_node_ptr(tickit);
      if (tickev->deleted == MSQ_FALSE)
        return;
    }
}
