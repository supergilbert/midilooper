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

#ifndef __WBE_PBW_H_INCLUDED__
#define __WBE_PBW_H_INCLUDED__

#include "wbe_pbw_inc.h"
#include "wbe_glfw.h"

void wbe_pbw_destroy(wbe_pbw_t *win);

wbe_bool_t wbe_pbw_init(wbe_pbw_t *win,
                        const char *name,
                        unsigned int width,
                        unsigned int height,
                        wbe_bool_t resizeable);

void _wbe_pbw_texture_load(wbe_pbw_t *win);

void wbe_pbw_put_buffer(wbe_pbw_t *win);

void wbe_pbw_refresh(wbe_pbw_t *win);

#define wbe_pbw_size_limits(_pbwin, _xmin, _ymin, _xmax, _ymax)         \
  wbe_window_size_limits((_pbwin)->win_be,                              \
                         (_xmin) != 0 ? (_xmin) : 1,                    \
                         (_ymin) != 0 ? (_ymin) : 1,                    \
                         (_xmax) != 0 ? (_xmax) : GLFW_DONT_CARE,       \
                         (_ymax) != 0 ? (_ymax) : GLFW_DONT_CARE)

#define wbe_pbw_add_resize_cb(_pbwin, _resize_cb, _resize_arg)          \
  wbe_window_add_resize_cb((_pbwin)->win_be, (_resize_cb), (_resize_arg))

void wbe_pbw_set_size(wbe_pbw_t *win,
                      unsigned int width,
                      unsigned int height);

#define wbe_pbw_backend_init() wbe_window_backend_init()

void wbe_pbw_backend_destroy(void);

#endif  /* __WBE_PBW_H_INCLUDED__ */
