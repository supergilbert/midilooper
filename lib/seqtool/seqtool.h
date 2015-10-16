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


#ifndef __SEQ_TOOL_H
#define __SEQ_TOOL_H

#include "tool/tool.h"

typedef enum {
  UNDEFINED = 0,
  ASEQTYPE,
  MIDICEV
} seqevtype_t;

typedef struct
{
  seqevtype_t   type;
  void          *addr;
  bool_t        deleted;
} seqev_t;

typedef struct
{
  uint_t	tick;
  list_t        seqev_list;
  bool_t        deleted;
}		tickev_t;

typedef struct
{
  /* + d'info */
  char          *name;
  list_t        tickev_list;
}	        track_t;


void add_new_seqev_head(track_t *track,
                        uint_t tick,
                        void *addr,
                        seqevtype_t type);

void add_new_seqev_tail(track_t *track,
                        uint_t tick,
                        void *addr,
                        seqevtype_t type);

typedef void (*free_seqev_addr_func)(void *addr);

void free_track(void *addr);
void clear_tickev_list(list_t *tickev_list);

void    dump_seqev(seqev_t *seqev);
void    free_seqev(void *addr);
void    free_tickev(void *addr);
node_t  *search_or_add_ticknode(list_t *tickev_list, uint_t tick);
seqev_t *alloc_seqev(void *addr, seqevtype_t type);
node_t  *search_ticknode(list_t *tickev_list, uint_t tick);
void    goto_next_available_tick(list_iterator_t *tickit, uint_t tick);
void    iter_next_available_tick(list_iterator_t *tickit);
bool_t  note_collision(list_t *tickev_list,
                       uint_t tick, byte_t channel, byte_t note,
                       uint_t *noteon_tick,
                       uint_t *noteoff_tick);

#endif
