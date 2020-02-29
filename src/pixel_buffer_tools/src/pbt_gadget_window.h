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

#ifndef __PBT_GADGET_WINDOW_H_INCLUDED__
#define __PBT_GADGET_WINDOW_H_INCLUDED__

#include "pbt_window_gadget_inc.h"
#include "wbe_pbw.h"

void pbt_ggt_win_destroy(pbt_ggt_win_t *ggt_win);

void pbt_ggt_win_set_min_size(pbt_ggt_win_t *ggt_win);

pbt_bool_t _pbt_ggt_win_init(pbt_ggt_win_t *ggt_win,
                             const char *name,
                             pbt_ggt_t *root_ggt,
                             unsigned int width,
                             unsigned int height,
                             pbt_bool_t resizeable);

#define pbt_ggt_win_init(_ggt_win,              \
                         _name,                 \
                         _root_ggt,             \
                         _width,                \
                         _height,               \
                         _resizeable)           \
  _pbt_ggt_win_init((_ggt_win),                 \
                    (_name),                    \
                    &(_root_ggt)->ggt,          \
                    (_width),                   \
                    (_height),                  \
                    (_resizeable))

void pbt_ggt_win_init_child_ev(pbt_ggt_win_t *ggt_win, pbt_ggt_node_t *childs);

pbt_ggt_node_t *pbt_wgt_ctnr_add_ggt(pbt_ggt_win_t *ggt_win,
                                     pbt_ggt_ctnr_t *ctnr,
                                     pbt_ggt_t *ggt);

pbt_bool_t _pbt_wgt_win_init(pbt_ggt_win_t *ggt_win,
                             const char *name,
                             pbt_wgt_t *root_ggt,
                             unsigned int width,
                             unsigned int height,
                             pbt_bool_t resizeable);

#define pbt_wgt_win_init(_ggt_win,              \
                         _name,                 \
                         _root_wgt,             \
                         _width,                \
                         _height,               \
                         _resizeable)           \
  _pbt_wgt_win_init((_ggt_win),                 \
                    (_name),                    \
                    &(_root_wgt)->wgt,          \
                    (_width),                   \
                    (_height),                  \
                    (_resizeable))

#define pbt_ggt_win_put_buffer(ggt_win)            \
  wbe_pbw_put_buffer(&((ggt_win)->pb_win))


#define pbt_ggt_win_make_context(ggt_win)            \
  wbe_pbw_make_context(&((ggt_win)->pb_win))

#endif  /* __PBT_GADGET_WINDOW_H_INCLUDED__ */
