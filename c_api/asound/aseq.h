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

typedef struct  aseqport_ctx_s
{
  snd_seq_t           *handle;
  int                 output_port;
  snd_seq_port_info_t *info;
}               aseqport_ctx_t;

#define _create_aseq_port(aseq, name)   snd_seq_create_simple_port((aseq)->handle, (name), \
                                                                   SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, \
                                                                   SND_SEQ_PORT_TYPE_APPLICATION)
/* SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, */
/*  SND_SEQ_PORT_TYPE_MIDI_GENERIC); */


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

snd_seq_t      *create_aseqh(char *name);
aseqport_ctx_t *create_aseqport_ctx(snd_seq_t *handle, char *name);
void           free_aseqport(aseqport_ctx_t *aseq);
void           free_aseqh(snd_seq_t *handle);
const char     *aseqport_get_name(aseqport_ctx_t *aseq);
void           aseqport_set_name(aseqport_ctx_t *aseq, char *name);

#endif
