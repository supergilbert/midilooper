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

/* You should have received a copy of the GNU Gneneral Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */

#pragma once

#include "pbt_event_handler_inc.h"
#include "wbe_glfw_inc.h"

void pbt_evh_clear(pbt_evh_t *evh);

void pbt_evh_del_node(pbt_evh_t *evh, pbt_ggt_t *ggt);

void pbt_evh_add_set_focus_cb(pbt_evh_t *evh,
                              pbt_ggt_t *gadget,
                              pbt_evh_input_cb_t set_focus_cb,
                              void *set_focus_arg);

void pbt_evh_add_unset_focus_cb(pbt_evh_t *evh,
                                pbt_ggt_t *gadget,
                                pbt_evh_input_cb_t unset_focus_cb,
                                void *unset_focus_arg);

void pbt_evh_add_enter_cb(pbt_evh_t *evh,
                          pbt_ggt_t *gadget,
                          pbt_evh_notify_cb_t enter_cb,
                          void *enter_arg);

void pbt_evh_add_leave_cb(pbt_evh_t *evh,
                          pbt_ggt_t *gadget,
                          pbt_evh_notify_cb_t leave_cb,
                          void *leave_arg);

pbt_bool_t pbt_evh_handle_input_in(pbt_evh_t *evh,
                                   unsigned int buttons,
                                   int xpos,
                                   int ypos);

pbt_bool_t pbt_evh_handle_input_out(pbt_evh_t *evh,
                                    unsigned int buttons,
                                    int xpos,
                                    int ypos);

void pbt_evh_handle_notify(pbt_evh_t *evh, int xpos, int ypos);

void pbt_evh_handle(pbt_evh_t *evh, wbe_window_input_t *winev);
