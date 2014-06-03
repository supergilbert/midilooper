#ifndef ASEQ_TOOL_H
#define ASEQ_TOOL_H

#include "asound/aseq.h"

#include "midi/midiev_inc.h"

bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port);
void alsa_play_seqevlist(aseqport_ctx_t *aseq_ctx,
                         list_t *seqevlist,
                         byte_t *noteon_state);
bool_t alsa_play_midicev(aseqport_ctx_t *aseq_ctx, midicev_t *midicev);
void alsa_play_pending_notes(aseqport_ctx_t *aseq_ctx, byte_t *pending_notes);

#endif
