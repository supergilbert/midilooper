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

#include "pbt_cursor_inc.h"

EXTERN_C_BEGIN

void wbe_window_destroy(wbe_window_t *win);

wbe_bool_t wbe_window_init(wbe_window_t **win,
                           const char *name,
                           unsigned int width,
                           unsigned int height,
                           wbe_bool_t resizeable);

#define wbe_window_get_size(_window, _width, _height)   \
  glfwGetWindowSize((_window), (_width), (_height))

void wbe_window_map(wbe_window_t *win);

#define wbe_window_unmap(_window) glfwHideWindow(_window)

#define wbe_window_mapped(_window) (glfwGetWindowAttrib((_window),      \
                                                        GLFW_VISIBLE)   \
                                    != 0 ? GLFW_TRUE : GLFW_FALSE)

#define wbe_window_closed(_window) (glfwWindowShouldClose(_window)      \
                                    == GLFW_TRUE ? WBE_TRUE : WBE_FALSE)

#define WBE_SIZE_DONT_CARE GLFW_DONT_CARE

#define wbe_window_size_limits(_window, _xmin, _ymin, _xmax, _ymax)     \
  glfwSetWindowSizeLimits((_window),                                    \
                          (_xmin) != 0 ? (_xmin) : 1,                   \
                          (_ymin) != 0 ? (_ymin) : 1,                   \
                          (_xmax) != 0 ? (_xmax) : GLFW_DONT_CARE,      \
                          (_ymax) != 0 ? (_ymax) : GLFW_DONT_CARE)

#define wbe_window_get_position(_window, _xpos, _ypos)  \
  glfwGetWindowPos((_window), (_xpos), (_ypos))

#define wbe_window_set_position(_window, _xpos, _ypos)  \
  glfwSetWindowPos((_window), (_xpos), (_ypos))

#define _wbe_window_set_size(_window, _width, _height)  \
  glfwSetWindowSize((_window), (_width), (_height))

#define wbe_window_make_context(_window) glfwMakeContextCurrent(_window)

/* #define wbe_window_swap_buffer(_window) glfwSwapBuffers(_window) */

/* #define wbe_next_windows_double_buffer()        \ */
/*   glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE) */

#define wbe_next_windows_single_buffer()        \
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE)

#define wbe_wait_events()                       \
  glfwWaitEvents()

void wbe_window_backend_set_key_layout(wbe_key_layout_t key_layout);

wbe_bool_t wbe_window_backend_init(void);

void wbe_window_backend_destroy(void);

void wbe_backend_remove_win_event(wbe_window_t *win);

void wbe_window_add_input_cb(wbe_window_t *win,
                             wbe_window_input_cb_t winev_cb,
                             void *arg);

void wbe_window_add_resize_cb(wbe_window_t *win,
                              wbe_window_resize_cb_t winsz_cb,
                              void *arg);

void wbe_window_add_focus_cb(wbe_window_t *win,
                             wbe_window_focus_cb_t winfocus_cb,
                             void *arg);

void wbe_window_add_refresh_cb(wbe_window_t *win,
                               wbe_window_refresh_cb_t callback,
                               void *arg);

void wbe_window_handle_events(void);

#define wbe_cursor_destroy(_cursor) glfwDestroyCursor(_cursor)

wbe_cursor_t *wbe_cursor_init(pbt_cursor_t *pbt_cursor);

#define wbe_cursor_init_arrow()                 \
  glfwCreateStandardCursor(GLFW_ARROW_CURSOR)

#define wbe_cursor_init_ibeam()                 \
  glfwCreateStandardCursor(GLFW_IBEAM_CURSOR)

#define wbe_cursor_init_crosshair()                     \
  glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR)

#define wbe_cursor_init_hand()                  \
  glfwCreateStandardCursor(GLFW_HAND_CURSOR)

#define wbe_cursor_init_hresize()               \
  glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR)

#define wbe_cursor_init_vresize()               \
  glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR)

#define wbe_window_set_cursor(_win, _cursor)    \
  glfwSetCursor((_win), (_cursor))

EXTERN_C_END
