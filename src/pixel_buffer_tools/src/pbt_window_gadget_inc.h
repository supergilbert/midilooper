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

#define PBT_SPLIT_AREA_SIZE 7

#include "pbt_gadget.h"
#include "pbt_gadget_window_inc.h"
#include "pbt_font_inc.h"

#define pbt_wgt_win_put_buffer(_wgt)                       \
  wbe_pbw_put_buffer(&((_wgt)->ggt_win->pb_win))

/* #define pbt_wgt_swap_buffer(_wgt)                               \ */
/*   wbe_window_swap_buffer((_wgt)->ggt_win->pb_win.win_be); */

typedef struct
{
  unsigned char *window_fg;
  unsigned char *window_bg;
  unsigned char *frame_fg;
  unsigned char *frame_bg;
  unsigned char *wgt_normal_fg;
  unsigned char *wgt_normal_bg;
  unsigned char *wgt_activated_fg;
  unsigned char *wgt_activated_bg;
  unsigned char *wgt_hovered_fg;
  unsigned char *wgt_hovered_bg;
  wbe_cursor_t *cursor_arrow;
  wbe_cursor_t *cursor_pencil;
  wbe_cursor_t *cursor_ibeam;
  wbe_cursor_t *cursor_finger;
  wbe_cursor_t *cursor_grab;
  wbe_cursor_t *cursor_grabbing;
  wbe_cursor_t *cursor_hresize;
  wbe_cursor_t *cursor_vresize;
  pbt_font_t font;
} pbt_wgt_theme_t;

typedef struct pbt_wgt_st
{
  void *priv;
  pbt_ggt_win_t *ggt_win;
  pbt_ggt_t ggt;
  void (*init_ev_cb)(struct pbt_wgt_st *wgt, pbt_ggt_win_t *ggt_win);
} pbt_wgt_t;

typedef struct
{
  pbt_ggt_node_t node[3];
  pbt_ggt_drawarea_t separator;
  /* pbt_pixbuf_t split_area_cache; */
  unsigned int last_ypos;
  unsigned char *color;
  wbe_cursor_t *entered_cursor;
  wbe_cursor_t *grab_cursor;
  wbe_cursor_t *leave_cursor;
  pbt_wgt_t wgt;
} pbt_wgt_splitted_area_t;

typedef enum
  {
   RELEASED,
   PRESSED,
   HOVERED
  } pbt_wgt_button_state_t;

typedef void (*pbt_wgt_cb_t)(void *arg);

typedef struct
{
  pbt_pixbuf_t *pb_released;
  pbt_pixbuf_t *pb_pressed;
  pbt_pixbuf_t *pb_hovered;
  unsigned char *bg_released;
  unsigned char *bg_pressed;
  unsigned char *bg_hovered;
  unsigned int width, height;
  pbt_wgt_button_state_t state;
  pbt_wgt_cb_t cb;
  void *cb_arg;
  wbe_cursor_t *entered_cursor;
  wbe_cursor_t *leave_cursor;
  pbt_wgt_t wgt;
} pbt_wgt_button_t;

typedef struct
{
  unsigned int max;
  unsigned int size;
  unsigned int pos;
} pbt_adj_t;

typedef struct
{
  pbt_adj_t *adj;
  int last_pos;
  unsigned int size;
  unsigned char *fg_color;
  unsigned char *bg_color;
  pbt_wgt_cb_t cb;
  void *cb_arg;
  pbt_wgt_t wgt;
} pbt_wgt_scrollbar_t;

#define _pbt_wgt_init_ev(wgt, ggt_win)          \
  (wgt)->init_ev_cb(wgt, ggt_win)

typedef void (*pbt_render_pixbuf_cb_t)(pbt_pixbuf_t *pixbuf,
                                       unsigned char *fg,
                                       unsigned char *bg);

#define _pbt_ggt_add_child_wgt(_ctnr, _child_wgt)                       \
  _pbt_ggt_add_child_ggt_type(&((_ctnr)->ggt), &((_child_wgt)->ggt), WIDGET)

#define pbt_ggt_add_child_wgt(_ctnr, _child)            \
  _pbt_ggt_add_child_wgt((_ctnr), &((_child)->wgt))

#define pbt_wgt_xpos(_wgt)                      \
  pbt_ggt_xpos(&((_wgt)->wgt))

#define pbt_wgt_ypos(_wgt)                      \
  pbt_ggt_ypos(&((_wgt)->wgt))

#define pbt_wgt_width(_wgt)                     \
  pbt_ggt_width(&((_wgt)->wgt))

#define pbt_wgt_height(_wgt)                    \
  pbt_ggt_height(&((_wgt)->wgt))
