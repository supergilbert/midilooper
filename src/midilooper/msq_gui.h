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

EXTERN_C_BEGIN

/* #include <pbt_window_gadget.h> */

#include "loop_engine/engine.h"
#include "msq_gui_inc.h"

void gui_default_theme_destroy(msq_gui_theme_t *global_theme);

void gui_default_theme_init(msq_gui_theme_t *global_theme);

/* void destroy_track_editor_theme(track_editor_theme_t *theme); */

void track_editor_default_theme_init(track_editor_theme_t *theme,
                                     msq_gui_theme_t *global_theme);

#define msq_dialog_activate(_dialog_iface) (_dialog_iface)->activated = MSQ_TRUE

void _msq_free_list(char **str_list, size_t str_list_len);

void msq_dialog_desactivate(msq_dialog_iface_t *dialog_iface);


void msq_dialog_list(msq_dialog_iface_t *dialog_iface,
                     char **list,
                     size_t str_list_len,
                     msq_dialog_idx_result_cb_t result_idx_cb,
                     void *arg_addr);

void msq_dialog_filebrowser(msq_dialog_iface_t *dialog_iface,
                            msq_dialog_str_result_cb_t result_str_cb,
                            void *arg_addr);

void msq_dialog_string_input(msq_dialog_iface_t *dialog_iface,
                             msq_dialog_str_result_cb_t result_str_cb,
                             void *arg_addr);

void msq_dialog_text(msq_dialog_iface_t *dialog_iface,
                     const char *str);

void msq_transport_init(msq_transport_iface_t *transport_iface,
                        msq_gui_theme_t *global_theme,
                        engine_ctx_t *engine_ctx);

void msq_transport_child_init(msq_transport_child_t *transport_child,
                              msq_transport_iface_t *transport_iface);

void msq_dialog_result_idx(msq_dialog_iface_t *dialog_iface, size_t idx);

void msq_dialog_result_str(msq_dialog_iface_t *dialog_iface, char *str);

pbt_ggt_t *msq_margin_get_child(msq_margin_ggt_t *margin_ggt);

void msq_margin_ggt_init(msq_margin_ggt_t *ggt,
                         pbt_ggt_node_type_t type,
                         pbt_ggt_t *child,
                         unsigned int left,
                         unsigned int right,
                         unsigned int top,
                         unsigned int bottom,
                         unsigned char *color);

#define msq_margin_ggt_init_g(_ggt,             \
                              _child,           \
                              _left,            \
                              _right,           \
                              _top,             \
                              _bottom,          \
                              _color)           \
  msq_margin_ggt_init((_ggt),                   \
                      GADGET,                   \
                      (_child),                 \
                      (_left),                  \
                      (_right),                 \
                      (_top),                   \
                      (_bottom),                \
                      (_color))

#define msq_margin_ggt_init_w(_ggt,             \
                              _child,           \
                              _left,            \
                              _right,           \
                              _top,             \
                              _bottom,          \
                              _color)           \
  msq_margin_ggt_init((_ggt),                   \
                      WIDGET,                   \
                      &((_child)->ggt),         \
                      (_left),                  \
                      (_right),                 \
                      (_top),                   \
                      (_bottom),                \
                      (_color))

void msq_draw_veil(pbt_pbarea_t *pbarea,
                   unsigned int xmin,
                   unsigned int xmax,
                   unsigned int ymin,
                   unsigned int ymax);

EXTERN_C_END
