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

#include <jack/jack.h>

#include "./jack/jack_backend.h"
#include "./debug_tool/debug_tool.h"

jack_client_t *create_jackh(const char *name)
{
  jack_status_t status;
  jack_client_t *client = jack_client_open(name,
                                           JackNoStartServer,
                                           &status,
                                           NULL);
  char          *alternate_name = NULL;

  if (client == NULL)
    {
      output_error("jack_client_open() failed, status = 0x%2.0x\n",
                   status);
      if (status & JackServerFailed)
        output_error("Unable to connect to JACK server\n");
      return NULL;
    }
  if (status & JackServerStarted) {
    output("JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    alternate_name = jack_get_client_name(client);
    output_warning("unique name `%s' assigned\n", alternate_name);
  }
  return client;
}

jbe_output_t *create_jack_output(jack_client_t *client,
                                 const char *name,
                                 jack_nframes_t *cur_frame)
{
  jbe_output_t *output = myalloc(sizeof (jbe_output_t));

  output->port = jack_port_register(client,
                                    name,
                                    JACK_DEFAULT_MIDI_TYPE,
                                    JackPortIsTerminal|JackPortIsOutput,
                                    0);
  output->client = client;
  output->jack_buffer = NULL;
  output->cur_frame = cur_frame;
  return output;
}

void free_jbe_output(jbe_output_t *jbe)
{
  jack_port_unregister(jbe->client, jbe->port);
  free(jbe);
}

const char *jbe_output_get_name(void *hdl)
{
  jbe_output_t *output      = (jbe_output_t *) hdl;
  const char   *output_name = jack_port_name(output->port);

  while (output_name)
    {
      if (*output_name == ':')
        {
          output_name++;
          break;
        }
      else
        output_name++;
    }
  return output_name;
}

void jbe_output_set_name(void *hdl, const char *name)
{
  jbe_output_t *output = (jbe_output_t *) hdl;

#ifdef __MLP_OLD_JACK
  jack_port_set_name(output->port, name);
#else
  jack_port_rename(output->client, output->port, name);
#endif
}

bool_t jbe_output_write(struct midioutput *output,
                        midicev_t *midicev)
{
  output_add_req(output, midicev);
  return TRUE;
}

#include "midi/midi_tool.h"
#include <jack/midiport.h>
bool_t _jbe_output_write(struct midioutput *output, midicev_t *midicev)
{
  jbe_output_t *jbe_output = (jbe_output_t *) output->hdl;
  byte_t       data[3];

  if (convert_midicev_to_mididata(midicev, data) == FALSE)
    return FALSE;
  if (jack_midi_event_write(jbe_output->jack_buffer, *(jbe_output->cur_frame), data, 3))
    return FALSE;
  return TRUE;
}
