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

void track_editor_destroy(track_editor_t *track_editor);
void track_editor_init(track_editor_t *track_editor,
                       track_editor_theme_t *theme,
                       msq_dialog_iface_t *dialog_iface,
                       msq_transport_iface_t *transport_iface,
                       pbt_ggt_t *track_node_ggt,
                       track_ctx_t *track_ctx,
                       unsigned int qn_size,
                       unsigned int note_height,
                       unsigned int quantize,
                       unsigned char note_scale);
void msq_track_draw_current_tick_pos(track_editor_t *track_editor);
void clear_progress_line(track_editor_t *track_editor);
void msq_draw_vggts(msq_vggts_t *vggts);

uint_t msq_get_track_loop_tick(track_ctx_t *track_ctx, uint_t tick);

msq_bool_t note_collision(note_t *note,
                          track_editor_ctx_t *editor_ctx,
                          msq_bool_t filter_selection);

void _history_evit_add_midicev(list_t *history,
                               ev_iterator_t *evit,
                               unsigned int tick,
                               midicev_t *mcev);

void _history_add_note(track_editor_ctx_t *editor_ctx,
                       note_t *note);


void _history_add_mark(list_t *history);

EXTERN_C_END
