/* Copyright 2012-2014 Gilbert Romer */

/* This file is part of gmidilooper. */

/* gmidilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* gmidilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>. */


#ifndef ASEQ_TOOL_H
#define ASEQ_TOOL_H

#include "asound/aseq.h"
#include "midi/midiev_inc.h"
#include "loop_engine/engine.h"

#define alsa_drain_output(engine_ctx) (snd_seq_drain_output((engine_ctx)->aseqh))


bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port);
bool_t aseq_output_evlist(output_t *output,
                          list_t *seqevlist);
bool_t aseq_output_pending_notes(output_t *output);
bool_t aseq_output_ev(output_t *output, midicev_t *midicev);

#endif
