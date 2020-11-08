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


#ifndef JACK_BE_H
#define JACK_BE_H

#include <jack/jack.h>

#include "loop_engine/engine.h"

#define MIDI_BUFF_LEN 1600

typedef struct
{
  jack_client_t  *client;
  jack_port_t    *port;
  void           *jack_buffer;
  jack_nframes_t *cur_frame;
} jbe_output_t;

jack_client_t *create_jackh(const char *name);
jbe_output_t  *create_jack_output(jack_client_t *client,
                                  const char *name,
                                  jack_nframes_t *cur_frame);
void          free_jbe_output(jbe_output_t *jbe);
const char    *jbe_output_get_name(void *hdl);
void          jbe_output_set_name(void *hdl, const char *name);
msq_bool_t    jbe_output_write(struct midioutput *output,
                               midicev_t *midicev);
msq_bool_t    _jbe_output_write(struct midioutput *output, midicev_t *midicev);

#endif
