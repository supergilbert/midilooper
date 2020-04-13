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

#include <pbt_gadget_window_inc.h>

typedef enum
  {
   MSQ_DEL,
   MSQ_DEL_NOTE,
   MSQ_ADD,
   MSQ_ADD_NOTE,
   MSQ_MARK
  } msq_history_type_t;

typedef struct
{
  msq_history_type_t type;
  unsigned int tick[2];
  midicev_t ev[2];
} msq_history_elt_t;

typedef struct
{
  track_editor_theme_t *theme;
  pbt_adj_t zoom_adj;
  pbt_adj_t hadj;
  pbt_adj_t vadj;
  unsigned char channel;
  unsigned char default_velocity;
  short default_pitch;
  unsigned char default_ctrl_val;
  unsigned int qn_size;
  unsigned int note_height;
  unsigned int quantize;        /* in tick */
  unsigned char note_scale;
  track_ctx_t *track_ctx;
  unsigned int vline_stride;    /* in tick */
  list_t selected_notes;
  uint_t selected_notes_min_tick;
  int tmp_coo[4];
  list_t history;
} track_editor_ctx_t;

typedef struct msq_tmp_tick_bar_st
{
  unsigned int tick;
  unsigned short velocity;
  struct msq_tmp_tick_bar_st *next;
} msq_tmp_tick_bar_t;

typedef enum
  {
   VALUE_NO_MODE,
   VALUE_WRITE_MODE,
   VALUE_WRITTING_MODE,
   VALUE_WRITTING_DEFAULT,
   VALUE_SELECT_MODE
  } msq_value_state_t;

typedef struct
{
  pbt_ggt_t *piano;
  pbt_ggt_t *grid;
  pbt_ggt_t *vscroll;
} msq_hggts_t;

typedef struct
{
  pbt_ggt_t *timeline;
  pbt_ggt_t *grid;
  pbt_ggt_t *value;
  pbt_ggt_t *hscroll;
  pbt_ggt_t *zoom;
} msq_vggts_t;

typedef struct
{
  track_editor_ctx_t *editor_ctx;
  msq_hggts_t *hggts;
  pbt_wgt_t wgt;
} msq_piano_wgt_t;

typedef enum
  {
   VALUE_NOTELEVEL_TYPE = 0,
   VALUE_PITCH_TYPE = 1,
   VALUE_TYPE_OFFSET
  } msq_value_type_t;

typedef struct
{
  track_editor_ctx_t *editor_ctx;
  msq_tmp_tick_bar_t *tmp_bar_head;
  unsigned char type;
  msq_value_state_t state;
  unsigned int tmp_coo[2];
  msq_vggts_t *vggts;
  pbt_wgt_t wgt;
  unsigned int selection_coo[2];
  msq_bool_t in_selection_mode;
} msq_value_wgt_t;

typedef enum
  {
   GRID_NO_MODE,
   GRID_CTRL_A_MODE,
   GRID_CTRL_C_MODE,
   GRID_CTRL_V_MODE,
   GRID_CTRL_X_MODE,
   GRID_CTRL_Z_MODE,
   GRID_WRITE_MODE,
   GRID_WRITTING_MODE,
   GRID_SELECT_NOTE_MODE,
   GRID_MOVE_NOTE_MODE,
   GRID_MOVE_NOTE_START_MODE,
   GRID_MOVE_NOTE_END_MODE,
   GRID_PASTE_MODE
  } msq_grid_state_t;

typedef struct
{
  track_editor_ctx_t *editor_ctx;
  msq_grid_state_t state;
  msq_hggts_t *hggts;
  msq_vggts_t *vggts;
  pbt_wgt_t wgt;
} msq_grid_wgt_t;

/* typedef struct */
/* { */
/*   pbt_ggt_ctnr_t hctnr; */
/*   pbt_wgt_button_t inc; */
/*   pbt_wgt_button_t dec; */
/*   pbt_ggt_drawarea_t label; */
/* } msq_grid_zoom_t */

typedef void (*msq_combobox_cb_t)(size_t combo_idx, void *arg);

typedef struct
{
  pbt_wgt_button_t button;
  msq_combobox_cb_t cb;
  void *cb_arg;
  size_t current_idx;
  char **list;
  size_t list_len;
  msq_dialog_iface_t *dialog_iface;
  pbt_pixbuf_t pb_released;
  pbt_pixbuf_t pb_pressed;
  pbt_pixbuf_t pb_hovered;
  msq_gui_theme_t *gui_theme;
  pbt_ggt_node_t button_node;
  pbt_ggt_t ggt;
} msq_combobox_t;

typedef struct
{
  pbt_ggt_win_t ggt_win;
  msq_margin_ggt_t margin;
  msq_dialog_iface_t *dialog_iface;
  msq_piano_wgt_t piano_wgt;
  pbt_ggt_ctnr_t hctnr_header;
  msq_grid_wgt_t grid_wgt;
  pbt_wgt_scrollbar_t hscrollbar_wgt;
  pbt_wgt_scrollbar_t vscrollbar_wgt;
  pbt_wgt_scrollbar_t hscrollbar_zoom_wgt;
  pbt_wgt_t timeline_wgt;
  msq_transport_child_t transport;
  pbt_wgt_button_t quantify_button;
  msq_combobox_t resolution_combobox;
  msq_combobox_t channel_combobox;
  msq_combobox_t value_type_combobox;
  pbt_wgt_t value_num_wgt;
  pbt_wgt_button_t value_num_dec;
  pbt_wgt_button_t value_num_inc;
  pbt_ggt_ctnr_t value_num_ctnr;
  pbt_ggt_ctnr_t value_vctnr_ggt;
  pbt_wgt_t value_vbar_wgt;
  msq_value_wgt_t value_wgt;
  pbt_ggt_drawarea_t value_empty_ggt;
  pbt_ggt_ctnr_t hctnr_up;
  pbt_ggt_ctnr_t hctnr_middle;
  pbt_ggt_ctnr_t hctnr_hscrollbar;
  pbt_ggt_ctnr_t hctnr_bottom;
  pbt_ggt_ctnr_t vctnr1;
  pbt_ggt_ctnr_t vctnr2;
  pbt_wgt_splitted_area_t splitted_area;
  track_editor_ctx_t editor_ctx;
  unsigned int value_min_height;
  msq_hggts_t hggts;
  msq_vggts_t vggts;
} track_editor_t;

#define track_editor_font(track_editor)                 \
  (&((track_editor)->theme->global_theme->font))

typedef struct
{
  unsigned char channel;
  unsigned char num;
  unsigned char val;
  unsigned int tick;
  unsigned int len;
} note_t;

typedef void (*evit_del_func_t)(track_ctx_t *, ev_iterator_t *);

EXTERN_C_END
