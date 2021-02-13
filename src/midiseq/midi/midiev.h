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


#ifndef __MIDIEV_H
#define __MIDIEV_H

#include "midi/midiev_inc.h"
#include "seqtool/seqtool.h"

/* #define _add_new_midicev(track, tick, mcev)             \ */
/*   add_new_seqev(track, tick, (void *) mcev, MIDICEV) */

void dumpaddr_seqevlist(list_t *seqev_list);
void add_new_midicev(track_t *track, uint_t tick, midicev_t *mcev);
void copy_midicev_to_track(track_t *track, uint_t tick, midicev_t *mcev);
track_t *merge_all_track(char *name, list_t *track_list);
void copy_track_list(track_t *track_src, track_t *track_dst);

void dump_midicev(midicev_t *midicev);

void msq_mcev_tick_list_add(list_t *mcev_tick_list,
                            uint_t tick,
                            midicev_t *mcev);
msq_mcev_tick_t *_msq_mcev_tick_list_pop_prev_noteon(list_t *mcev_tick_list,
                                                     uint_t tick,
                                                     byte_t channel,
                                                     byte_t num);
void msq_mcev_tick_list_clear(list_t *mcev_tick_list);

#endif
