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

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#ifndef ASEQ_TOOL_H
#define ASEQ_TOOL_H

#include "asound/aseq.h"
#include "midi/midiev_inc.h"
#include "loop_engine/engine.h"

#define alsa_drain_output(engine_ctx) (snd_seq_drain_output((engine_ctx)->aseqh))


msq_bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port);
void       aseq_to_mcev(snd_seq_event_t *snd_ev, midicev_t *mcev);
msq_bool_t aseq_output_evlist(output_t *output,
                              list_t *seqevlist,
                              byte_t *notes_on_state);
msq_bool_t aseq_output_write(output_t *output,
                             midicev_t *midicev);
msq_bool_t _aseq_output_write(output_t *output, midicev_t *midicev);

#endif
