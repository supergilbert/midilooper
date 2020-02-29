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

#include <GLFW/glfw3.h>

#include "wbe_type.h"

#define WBE_SET_BIT(data, num, wbe_bool) ((data) = ((data) & ~(1 << (num))) \
                                          | ((wbe_bool) << (num)))

#define WBE_GET_BIT(data, num) (((data) & 1 << (num)) != 0 ?    \
                                WBE_TRUE : WBE_FALSE)

typedef GLFWwindow wbe_window_t;

/* Caution when updating keys, update WBE_KEY_LEN and WBE_KEYMAP_LEN */
typedef enum
  {
   WBE_KEY_ESCAPE,
   WBE_KEY_SPACE,
   WBE_KEY_BSPACE,
   WBE_KEY_SUPPR,
   WBE_KEY_ENTER,
   WBE_KEY_SHIFT,
   WBE_KEY_CONTROL,
   WBE_KEY_ALT,
   WBE_KEY_SUPER,
   WBE_KEY_LEFT,
   WBE_KEY_RIGHT,
   WBE_KEY_UP,
   WBE_KEY_DOWN,
   WBE_KEY_A
  } wbe_key_t;

#define WBE_KEY_LEN 40  /* = (WBE_KEY_A + 26) */

#define WBE_KEYMAP_LEN 5 /* = (WBE_KEY_LEN / 8) */

typedef unsigned char wbe_byte_t;

#define wbe_key_pressed(_keys, _idx)            \
  WBE_GET_BIT((_keys)[(_idx) / 8], (_idx) % 8)

#define wbe_key_pressedA(_keys, _alphau)                        \
  wbe_key_pressed((_keys), WBE_KEY_A + ((_alphau) - 'A'))

/* #define wbe_key_presseda(_keys, _alphau)                        \ */
/*   wbe_key_pressed((_keys), WBE_KEY_A + ((_alphau) - 'a')) */

typedef struct
{
  unsigned int buttons;
  unsigned char mods;
  wbe_byte_t keys[WBE_KEYMAP_LEN];
  int xpos;
  int ypos;
} wbe_window_input_t;

#define wbe_window_input_has_mod(_wbe_win_input, _key_mod)      \
  WBE_GET_BIT((_wbe_win_input)->mods, (_key_mod))

typedef void (*wbe_window_input_cb_t)(wbe_window_input_t *winev, void *arg);

typedef struct wbe_window_input_cb_node_s
{
  wbe_window_input_cb_t             callback;
  void                              *arg;
  struct wbe_window_input_cb_node_s *next;
} wbe_window_input_cb_node_t;

typedef void (*wbe_window_resize_cb_t)(unsigned int width,
                                       unsigned int height,
                                       void *arg);

typedef struct wbe_window_resize_cb_node_s
{
  wbe_window_resize_cb_t             callback;
  void                               *arg;
  struct wbe_window_resize_cb_node_s *next;
} wbe_window_resize_cb_node_t;

typedef void (*wbe_window_focus_cb_t)(wbe_bool_t focused, void *arg);

typedef struct wbe_window_focus_cb_node_s
{
  wbe_window_focus_cb_t             callback;
  void                              *arg;
  struct wbe_window_focus_cb_node_s *next;
} wbe_window_focus_cb_node_t;

typedef void (*wbe_window_refresh_cb_t)(void *arg);

typedef struct wbe_window_refresh_cb_node_s
{
  wbe_window_refresh_cb_t             callback;
  void                                *arg;
  struct wbe_window_refresh_cb_node_s *next;
} wbe_window_refresh_cb_node_t;

typedef struct wbe_window_evhdl_node_s
{
  wbe_window_t                   *win;
  wbe_bool_t                     new_input;
  wbe_window_input_t             input;
  wbe_window_input_cb_node_t     *input_head;
  wbe_bool_t                     resized;
  unsigned int                   width;
  unsigned int                   height;
  wbe_window_resize_cb_node_t    *resize_head;
  wbe_bool_t                     focus_changed;
  wbe_bool_t                     focus_state;
  wbe_window_focus_cb_node_t     *focus_head;
  wbe_bool_t                     need_refresh;
  wbe_window_refresh_cb_node_t   *refresh_head;
  struct wbe_window_evhdl_node_s *next;
} wbe_window_evhdl_node_t;

typedef enum
  {
   WBE_KEY_LAYOUT_EN,
   WBE_KEY_LAYOUT_FR
  } wbe_key_layout_t;

typedef struct
{
  wbe_bool_t              initialised;
  wbe_window_evhdl_node_t *winevhdl_head;
  wbe_key_layout_t        key_layout;
  wbe_window_t            *first_win;
} wbe_window_backend_t;

typedef GLFWcursor wbe_cursor_t;
