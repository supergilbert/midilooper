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


#ifndef __EV_ITERATOR
#define __EV_ITERATOR

#include "midi/midiev_inc.h"

typedef struct {
  list_iterator_t tickit;
  list_iterator_t seqevit;
  uint_t          tick;
} ev_iterator_t;


void    dump_evit(ev_iterator_t *evit);
seqev_t *evit_init(ev_iterator_t *, list_t *tickev_list);
midicev_t *evit_init_midiallchannel(ev_iterator_t *ev_iterator,
                                    list_t *tickev_list);
midicev_t *evit_init_midicev(ev_iterator_t *ev_iterator,
                             list_t *tickev_list,
                             byte_t channel);
midicev_t *evit_init_noteon(ev_iterator_t *ev_iterator,
                            list_t *tickev_list,
                            byte_t channel);
seqev_t   *evit_tick_head(ev_iterator_t *);
midicev_t *evit_searchev(ev_iterator_t *evit, uint_t tick, midicev_t *mcev);
seqev_t   *evit_next_tick(ev_iterator_t *);
seqev_t   *evit_get_seqev(ev_iterator_t *);
seqev_t   *evit_next_seqev(ev_iterator_t *);
void      evit_copy(ev_iterator_t *evit_src, ev_iterator_t *evit_dst);
midicev_t *evit_next_midicev(ev_iterator_t *ev_iterator, byte_t channel);
midicev_t *evit_next_midicev_type(ev_iterator_t *,
                                  byte_t channel,
                                  byte_t type);
midicev_t *evit_next_midiallchannel(ev_iterator_t *ev_iterator);
void      evit_del_event(ev_iterator_t *ev_iterator);
void      evit_add_midicev(ev_iterator_t *evit, uint_t tick, midicev_t *mcev);

msq_bool_t _evit_check(ev_iterator_t *evit, list_t *tickev_list);
#ifdef __ROUGH
#define evit_check(evit, tickev_list) (TRUE)
#else
#define evit_check(evit, tickev_list) _evit_check(evit, (tickev_list))
#endif

#define evit_next_noteon(evit, chan)                    \
  (evit_next_midicev_type((evit), (chan), NOTEON))
#define evit_next_noteoff(evit, chan)                   \
  (evit_next_midicev_type((evit), (chan), NOTEOFF))
#define evit_next_ctrl(evit, chan)                              \
  (evit_next_midicev_type((evit), (chan), CONTROLCHANGE))
#define evit_next_pitch(evit, chan)                             \
  (evit_next_midicev_type((evit), (chan), PITCHWHEELCHANGE))

midicev_t *evit_next_noteoff_num(ev_iterator_t *ev_iterator,
                                 byte_t channel,
                                 byte_t num);
midicev_t *evit_next_ctrl_num(ev_iterator_t *ev_iterator,
                              byte_t channel,
                              byte_t ctrl_num);
midicev_t *evit_init_ctrl_num(ev_iterator_t *ev_iterator,
                              list_t *tickev_list,
                              byte_t channel,
                              byte_t ctrl_num);
midicev_t *evit_init_pitch(ev_iterator_t *ev_iterator,
                           list_t *tickev_list,
                           byte_t channel);

msq_bool_t note_collision(uint_t tick,
                          byte_t channel,
                          byte_t note,
                          list_t *tickev_list);

#endif
