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
const char     *aseqport_name(aseqport_ctx_t *aseq);

#include "midi/midiev_inc.h"

bool_t set_aseqev(midicev_t *chnev, snd_seq_event_t *ev, int port);

#endif
