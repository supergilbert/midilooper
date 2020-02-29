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

#include "wbe_glfw_inc.h"
#include "pbt_pixel_buffer_inc.h"
#include "wbe_gl.h"

#define wbe_pbw_make_context(_pbw) wbe_window_make_context((_pbw)->win_be)

typedef struct
{
  wbe_window_t *win_be;
  pbt_pixbuf_t buffer;
  GLuint texture_id;
} wbe_pbw_t;
