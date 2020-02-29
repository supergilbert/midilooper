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
#include "pbt_window_gadget_inc.h"

void pbt_wgt_set_default_theme_color(pbt_wgt_theme_t *theme);

void pbt_wgt_default_theme_cursor_init(pbt_wgt_theme_t *theme);

void pbt_wgt_default_theme_cursor_destroy(pbt_wgt_theme_t *theme);

void pbt_wgt_gl_refresh_rect(pbt_wgt_t *wgt,
                             unsigned int xpos,
                             unsigned int ypos,
                             unsigned int width,
                             unsigned int height);

#define pbt_wgt_gl_refresh(wgt)                                         \
  pbt_wgt_gl_refresh_rect((wgt), 0, 0, pbt_ggt_width(wgt), pbt_ggt_height(wgt))

void pbt_wgt_gl_fillrect(pbt_wgt_t *wgt,
                         unsigned int xpos,
                         unsigned int ypos,
                         unsigned int width,
                         unsigned int height,
                         unsigned char *color);

void pbt_wgt_gl_draw_line(pbt_wgt_t *wgt,
                          unsigned int xpos1,
                          unsigned int ypos1,
                          unsigned int xpos2,
                          unsigned int ypos2,
                          unsigned char *color);


void pbt_wgt_evnode_destroy(pbt_ggt_t *ggt);

void _pbt_wgt_draw(pbt_wgt_t *wgt);

#define _pbt_wgt_draw(_wgt) pbt_ggt_draw(_wgt)

#define pbt_wgt_draw(_wgt) _pbt_wgt_draw(&((_wgt)->wgt))

void _pbt_wgt_vsplitted_area_init_gg(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_ggt_t *child_ggt1,
                                     pbt_ggt_t *child_ggt2,
                                     unsigned int ggt2_size,
                                     unsigned char *color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor);

#define pbt_wgt_vsplitted_area_init_gg(_splitted_area,  \
                                       _child1,         \
                                       _child2,         \
                                       _child2_size,    \
                                       _color,          \
                                       _entered_cursor, \
                                       _grab_cursor,    \
                                       _leave_cursor)   \
  _pbt_wgt_vsplitted_area_init_gg((_splitted_area),     \
                                  &((_child1)->ggt),    \
                                  &((_child2)->ggt),    \
                                  (_child2_size),       \
                                  (_color),             \
                                  (_entered_cursor),    \
                                  (_grab_cursor),       \
                                  (_leave_cursor))

void _pbt_wgt_vsplitted_area_init_wg(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_wgt_t *wgt1,
                                     pbt_ggt_t *ggt2,
                                     unsigned int ggt2_size,
                                     unsigned char *color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor);

#define pbt_wgt_vsplitted_area_init_wg(_splitted_area,          \
                                       _child1,                 \
                                       _child2,                 \
                                       _child2_size,            \
                                       _color,                  \
                                       _entered_cursor,         \
                                       _grab_cursor,            \
                                       _leave_cursor)           \
  _pbt_wgt_vsplitted_area_init_wg((_splitted_area),             \
                                  &((_child1)->wgt.ggt),        \
                                  &((_child2)->ggt),            \
                                  (_child2_size),               \
                                  (_color),                     \
                                  (_entered_cursor),            \
                                  (_grab_cursor),               \
                                  (_leave_cursor))

void _pbt_wgt_vsplitted_area_init_gw(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_ggt_t *ggt1,
                                     pbt_wgt_t *wgt2,
                                     unsigned int ggt2_size,
                                     unsigned char *color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor);

#define pbt_wgt_vsplitted_area_init_gw(_splitted_area,          \
                                       _child1,                 \
                                       _child2,                 \
                                       _child2_size,            \
                                       _color,                  \
                                       _entered_cursor,         \
                                       _grab_cursor,            \
                                       _leave_cursor)           \
  _pbt_wgt_vsplitted_area_init_gw((_splitted_area),             \
                                  &((_child1)->ggt),            \
                                  &((_child2)->wgt.ggt),        \
                                  (_child2_size),               \
                                  (_color),                     \
                                  (_entered_cursor),            \
                                  (_grab_cursor),               \
                                  (_leave_cursor))

void _pbt_wgt_vsplitted_area_init_ww(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_wgt_t *wgt1,
                                     pbt_wgt_t *wgt2,
                                     unsigned int ggt2_size,
                                     unsigned char *color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor);

#define pbt_wgt_vsplitted_area_init_ww(_splitted_area,          \
                                       _child1,                 \
                                       _child2,                 \
                                       _child2_size,            \
                                       _color,                  \
                                       _entered_cursor,         \
                                       _grab_cursor,            \
                                       _leave_cursor)           \
  _pbt_wgt_vsplitted_area_init_ww((_splitted_area),             \
                                  &((_child1)->wgt.ggt),        \
                                  &((_child2)->wgt.ggt),        \
                                  (_child2_size),               \
                                  (_color),                     \
                                  (_entered_cursor),            \
                                  (_grab_cursor),               \
                                  (_leave_cursor))

void pbt_wgt_button_init(pbt_wgt_button_t *button,
                         pbt_pixbuf_t *pb_released,
                         pbt_pixbuf_t *pb_pressed,
                         pbt_pixbuf_t *pb_hovered,
                         unsigned char *bg_released,
                         unsigned char *bg_pressed,
                         unsigned char *bg_hovered,
                         wbe_cursor_t *entered_cursor,
                         wbe_cursor_t *leave_cursor,
                         pbt_wgt_cb_t cb,
                         void *cb_arg);

void pbt_wgt_pb_button_init(pbt_wgt_button_t *wgt,
                            pbt_render_pixbuf_cb_t render_pixbuf,
                            pbt_wgt_theme_t *theme,
                            pbt_wgt_cb_t cb,
                            void *cb_arg);

void _pbt_wgt_label_button_init(pbt_wgt_button_t *button,
                                const char *label,
                                pbt_font_t *font,
                                unsigned char *normal_fg,
                                unsigned char *normal_bg,
                                unsigned char *activated_fg,
                                unsigned char *activated_bg,
                                unsigned char *hovered_fg,
                                unsigned char *hovered_bg,
                                wbe_cursor_t *entered_cursor,
                                wbe_cursor_t *leave_cursor,
                                pbt_wgt_cb_t cb,
                                void *cb_arg);

void pbt_wgt_label_button_init(pbt_wgt_button_t *wgt,
                               const char *label,
                               pbt_wgt_theme_t *theme,
                               pbt_wgt_cb_t cb,
                               void *cb_arg);

void pbt_wgt_hscrollbar_init(pbt_wgt_scrollbar_t *wgt,
                             pbt_adj_t *adj,
                             unsigned int width,
                             unsigned char *fg_color,
                             unsigned char *bg_color,
                             pbt_wgt_cb_t cb,
                             void *cb_arg);

void pbt_wgt_vscrollbar_init(pbt_wgt_scrollbar_t *wgt,
                             pbt_adj_t *adj,
                             unsigned int width,
                             unsigned char *fg_color,
                             unsigned char *bg_color,
                             pbt_wgt_cb_t cb,
                             void *cb_arg);

unsigned int pbt_ggt_return_zero(pbt_ggt_t *ggt);

void pbt_ggt_memcpy_area(pbt_ggt_t *ggt, pbt_pbarea_t *pbarea);
