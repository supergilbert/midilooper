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

#include "pbt_gadget_inc.h"
#include "wbe_glfw_inc.h"

typedef pbt_bool_t (*pbt_evh_input_cb_t)(pbt_ggt_t *gadget,
                                         wbe_window_input_t *winev,
                                         void *arg);

typedef void (*pbt_evh_notify_cb_t)(void *arg);

typedef struct pbt_ggt_evh_node_s
{
  pbt_ggt_t *ggt;
  pbt_evh_input_cb_t set_focus_cb;
  void *set_focus_arg;
  pbt_evh_input_cb_t unset_focus_cb;
  void *unset_focus_arg;
  pbt_evh_notify_cb_t enter_cb;
  void *enter_arg;
  pbt_evh_notify_cb_t leave_cb;
  void *leave_arg;
  struct pbt_ggt_evh_node_s *next;
} pbt_evh_node_t;

typedef struct
{
  pbt_evh_node_t *head;
  pbt_evh_node_t *input_wgt;
  pbt_evh_node_t *entered_wgt;
} pbt_evh_t;
