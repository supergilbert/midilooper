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

#include "msq_track_editor_inc.h"

#include <pbt_gadget_window.h>

#define track_editor_destroy(track_editor)              \
  pbt_ggt_win_destroy(&((track_editor)->ggt_win))
void track_editor_init(track_editor_t *track_editor,
                       track_editor_theme_t *theme,
                       msq_dialog_iface_t *dialog_iface,
                       msq_transport_iface_t *transport_iface,
                       track_ctx_t *track_ctx,
                       unsigned int qn_size,
                       unsigned int note_height,
                       unsigned int quantize,
                       unsigned char note_scale);
void draw_progress_line(track_editor_t *track_editor);
void clear_progress_line(track_editor_t *track_editor);
/* void pbt_evh_button_label_add(pbt_evh_t *evh, */
/*                               pbt_button_label_t *button, */
/*                               const char *label, */
/*                               msq_gui_theme_t *theme, */
/*                               pbt_clickh_cb_t cb, */
/*                               void *cb_arg); */
/* void pbt_evh_button_label_clear(pbt_button_label_t *button); */

EXTERN_C_END