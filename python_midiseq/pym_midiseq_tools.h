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


#ifndef __PYM_MTOOLS
#define __PYM_MTOOLS
#include "loop_engine/engine.h"

PyObject *sel_noteonoff_repr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             byte_t note_min,
                             byte_t note_max);
PyObject *sel_noteonoff_evwr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             byte_t note_min,
                             byte_t note_max);
PyObject *sel_ctrl_evwr(track_ctx_t *trackctx,
                        byte_t channel,
                        uint_t tick_min,
                        uint_t tick_max,
                        byte_t ctrl_num);
PyObject *getall_noteonoff_repr(list_t *tickev_list, byte_t channel);
PyObject *getall_midicev_repr(list_t *tickev_list,
                              byte_t channel,
                              uint_t tickmin,
                              uint_t tickmax);
PyObject *getall_event_repr(list_t *tickev_list);
PyObject *add_evrepr_list(track_ctx_t *trackctx, PyObject *pylist);
void     delete_evwr_list(track_ctx_t *trackctx, PyObject *pylist);
PyObject *try_gen_evwr_list(track_ctx_t *trackctx, PyObject *pylist);

/* PyObject *get_event_list_repr(list_t *); */
/* PyObject *get_note_list_repr(list_t *tickev_list, byte_t channel); */
/* PyObject *get_note_list_selection(list_t *tickev_list, */
/*                                   byte_t channel, */
/*                                   uint_t tick_min, */
/*                                   uint_t tick_max, */
/*                                   char note_min, */
/*                                   char note_max); */
#endif
