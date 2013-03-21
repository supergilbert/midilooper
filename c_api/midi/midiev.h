#ifndef __MIDIEV_H
#define __MIDIEV_H

#include "midi/midiev_inc.h"
#include "seqtool/seqtool.h"

/* #define _add_new_midicev(track, tick, mcev)             \ */
/*   add_new_seqev(track, tick, (void *) mcev, MIDICEV) */

void add_new_midicev(track_t *track, uint_t tick, midicev_t *mcev);
void copy_midicev_to_track(track_t *track, uint_t tick, midicev_t *mcev);
track_t *merge_all_track(char *name, list_t *track_list);
void copy_track(track_t *track_src, track_t *track_dst);

#endif
