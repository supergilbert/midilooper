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

#include <pbt_type.h>
#include <pbt_font_inc.h>
/* #include <pbt_event_handler_inc.h> */
#include <pbt_window_gadget.h>
#include <loop_engine/engine.h>
#include <wbe_glfw.h>

EXTERN_C_BEGIN

typedef struct
{
  unsigned int default_margin;
  unsigned int default_separator;
  pbt_wgt_theme_t theme;
  unsigned char *play_color;
  unsigned char *rec_color;
  unsigned char *highlight_color;
  pbt_pixbuf_t play_button_imgs[6];
  pbt_pixbuf_t stop_button_imgs[3];
  pbt_pixbuf_t rec_button_imgs[6];
  pbt_pixbuf_t plus_button_imgs[3];
  pbt_pixbuf_t minus_button_imgs[3];
  pbt_pixbuf_t track_mute_imgs[6];
} msq_gui_theme_t;

#define msq_theme_window_fg(msq_theme) (msq_theme)->theme.window_fg
#define msq_theme_window_bg(msq_theme) (msq_theme)->theme.window_bg
#define msq_theme_frame_fg(msq_theme) (msq_theme)->theme.frame_fg
#define msq_theme_frame_bg(msq_theme) (msq_theme)->theme.frame_bg
#define msq_theme_wgt_normal_fg(msq_theme) (msq_theme)->theme.wgt_normal_fg
#define msq_theme_wgt_normal_bg(msq_theme) (msq_theme)->theme.wgt_normal_bg
#define msq_theme_wgt_activated_fg(msq_theme)   \
  (msq_theme)->theme.wgt_activated_fg
#define msq_theme_wgt_activated_bg(msq_theme)   \
  (msq_theme)->theme.wgt_activated_bg
#define msq_theme_wgt_hovered_fg(msq_theme) (msq_theme)->theme.wgt_hovered_fg
#define msq_theme_wgt_hovered_bg(msq_theme) (msq_theme)->theme.wgt_hovered_bg

#define msq_theme_cursor_arrow(msq_theme) (msq_theme)->theme.cursor_arrow
#define msq_theme_cursor_pencil(msq_theme) (msq_theme)->theme.cursor_pencil
#define msq_theme_cursor_ideam(msq_theme) (msq_theme)->theme.cursor_ibeam
#define msq_theme_cursor_finger(msq_theme) (msq_theme)->theme.cursor_finger
#define msq_theme_cursor_grab(msq_theme) (msq_theme)->theme.cursor_grab
#define msq_theme_cursor_grabbing(msq_theme) (msq_theme)->theme.cursor_grabbing
#define msq_theme_cursor_hresize(msq_theme) (msq_theme)->theme.cursor_hresize
#define msq_theme_cursor_vresize(msq_theme) (msq_theme)->theme.cursor_vresize

typedef struct
{
  unsigned short value;
  unsigned char color[4];
} gradient_value_t;

typedef struct
{
  engine_ctx_t *engine_ctx;
  msq_gui_theme_t *global_theme;
  unsigned int width;
  pbt_wgt_t wgt;
} msq_transport_tempo_t;

typedef struct
{
  pbt_ggt_ctnr_t root_ctnr;
  pbt_wgt_button_t play;
  pbt_wgt_button_t stop;
  pbt_wgt_button_t rec;
  engine_ctx_t *engine_ctx;
  pbt_wgt_button_t tempo_dec;
  pbt_wgt_button_t tempo_inc;
  msq_transport_tempo_t tempo_wgt;
  pbt_wgt_t wgt;
  pbt_ggt_node_t child;
  list_t transport_childs;
} msq_transport_iface_t;

typedef struct
{
  pbt_ggt_ctnr_t root_ctnr;
  pbt_wgt_button_t play;
  pbt_wgt_button_t stop;
  pbt_wgt_button_t rec;
  pbt_wgt_t wgt;
  pbt_ggt_node_t child;
  msq_transport_iface_t *parent;
} msq_transport_child_t;

typedef enum
  {
   LIST,
   FILE_BROWSER,
   STRING,
   STRING_INPUT
  } msq_dialog_type_t;

#include "tool/tool.h"

typedef void (*msq_dialog_idx_result_cb_t)(size_t idx, void *addr);

typedef void (*msq_dialog_str_result_cb_t)(char *str, void *addr);

typedef double msq_time_t;

typedef struct
{
  msq_bool_t                 activated;
  msq_bool_t                 need_popup;
  msq_bool_t                 need_update;
  msq_dialog_type_t          type;
  char                       *str;
  char                       **str_list;
  size_t                     str_list_len;
  msq_dialog_idx_result_cb_t result_idx_cb;
  msq_dialog_str_result_cb_t result_str_cb;
  void                       *arg_addr;
} msq_dialog_iface_t;

typedef struct
{
  unsigned int left, right, top, bottom;
  unsigned char *color;
  pbt_ggt_node_t child;
  pbt_ggt_t ggt;
} msq_margin_ggt_t;

typedef struct
{
  msq_gui_theme_t *global_theme;
  unsigned char *smooth_line_color;
  unsigned char *piano_white_color;
  unsigned char *piano_black_color;
  unsigned char *selection_color;
  gradient_value_t *gradient_value_list;
  unsigned int gradient_len;
  unsigned int piano_width;
  unsigned int timeline_height;
} track_editor_theme_t;

#define DEFAULT_FONT_SIZE 12


#define msq_button_init(_button,                                \
                        _pixbuf_released,                       \
                        _pixbuf_pressed,                        \
                        _pixbuf_hovered,                        \
                        _theme,                                 \
                        _cb,                                    \
                        _cb_arg)                                \
  pbt_wgt_button_init((_button),                                \
                      (_pixbuf_released),                       \
                      (_pixbuf_pressed),                        \
                      (_pixbuf_hovered),                        \
                      msq_theme_wgt_normal_bg((_theme)),        \
                      msq_theme_wgt_activated_bg((_theme)),     \
                      msq_theme_wgt_hovered_bg((_theme)),       \
                      msq_theme_cursor_finger((_theme)),        \
                      msq_theme_cursor_arrow((_theme)),         \
                      (_cb),                                    \
                      (_cb_arg))

#define msq_button_plus_init(_button, _theme, _cb, _cb_arg)     \
  msq_button_init((_button),                                    \
                  &((_theme)->plus_button_imgs[0]),             \
                  &((_theme)->plus_button_imgs[1]),             \
                  &((_theme)->plus_button_imgs[2]),             \
                  (_theme),                                     \
                  (_cb),                                        \
                  (_cb_arg))

#define msq_button_minus_init(_button, _theme, _cb, _cb_arg)    \
  msq_button_init((_button),                                    \
                  &((_theme)->minus_button_imgs[0]),            \
                  &((_theme)->minus_button_imgs[1]),            \
                  &((_theme)->minus_button_imgs[2]),            \
                  (_theme),                                     \
                  (_cb),                                        \
                  (_cb_arg))

EXTERN_C_END
