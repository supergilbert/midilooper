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


#ifndef ASEQ_H
#define ASEQ_H

#include <alsa/asoundlib.h>
#include "seqtool/seqtool.h"

/* #warning beaucoup de chose a reflechir voir meme a tester direct */
/* #warning fonction de creation du handler alsa, du port + creation et envoi dev aseq */

typedef struct
{
  snd_seq_t           *handle;
  int                 port;
  snd_seq_port_info_t *info;
}               aseq_output_t;

#define ASEQ_SETNOTEOFFEV(seqev, port, channel, note, velocity)         \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_noteoff((seqev), (channel), (note), (velocity)),      \
   snd_seq_ev_set_direct(seqev))

#define ASEQ_SETNOTEONEV(seqev, port, channel, note, velocity)          \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_noteon((seqev), (channel), (note), (velocity)),       \
   snd_seq_ev_set_direct(seqev))

#define ASEQ_SETKEYAFTERTOUCHEV(seqev, port, channel, note, velocity)   \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_keypress((seqev), (channel), (note), (velocity)),     \
   snd_seq_ev_set_direct(seqev))

#define ASEQ_SETCONTROLCHANGEEV(seqev, port, channel, num, value)       \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_controller((seqev), (channel), (num), (value)),       \
   snd_seq_ev_set_direct(seqev))

#define ASEQ_SETPROGRAMCHANGEEV(seqev, port, channel, value)            \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_pgmchange((seqev), (channel), (value)),               \
   snd_seq_ev_set_direct(seqev))

#define ASEQ_SETCHANNELAFTERTOUCHEV(seqev, port, channel, value)        \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_chanpress((seqev), (channel), (value)),               \
   snd_seq_ev_set_direct(seqev))

#define ASEQ_SETPITCHWHEELCHANGEEV(seqev, port, channel, value)         \
  (snd_seq_ev_clear(seqev),                                             \
   snd_seq_ev_set_source((seqev), (port)),                              \
   snd_seq_ev_set_subs(seqev),                                          \
   snd_seq_ev_set_pitchbend((seqev), (channel), (value)),               \
   snd_seq_ev_set_direct(seqev))

snd_seq_t     *create_aseqh(char *name);
aseq_output_t *create_aseq_output(snd_seq_t *handle, char *name);
void          free_aseq_output(aseq_output_t *aseq);
void          free_aseqh(snd_seq_t *handle);
const char    *aseq_output_get_name(void *output);
void          aseq_output_set_name(void *output, char *name);

#endif
