/* Copyright 2012-2016 Gilbert Romer */

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


#include "midi/midiev.h"
#include "debug_tool/debug_tool.h"

void dump_tickev(tickev_t *tickev)
{
  list_iterator_t seqevit;

  output("At tick %d %s got %d event(s)\n",
         tickev->tick,
         tickev->deleted == MSQ_TRUE ? "(\033[31mdeleted\033[0m)" : "(not deleted)",
         tickev->seqev_list.len);
  for (iter_init(&seqevit, &(tickev->seqev_list));
       iter_node(&seqevit) != NULL;
       iter_next(&seqevit))
    dump_seqev((seqev_t *) iter_node_ptr(&(seqevit)));
}

void dump_track(track_t *track)
{
  list_iterator_t tickit;

  output("track \"%s\" got %d tick event(s)\n", track->name, track->tickev_list.len);
  for (iter_init(&tickit, &(track->tickev_list));
       iter_node(&tickit) != NULL;
       iter_next(&tickit))
      dump_tickev((tickev_t *) iter_node_ptr(&tickit));
}
