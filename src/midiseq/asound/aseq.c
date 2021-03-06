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


#include "asound/aseq.h"
#include "debug_tool/debug_tool.h"

snd_seq_t     *create_aseqh(char *name)
{
  snd_seq_t     *handle = NULL;
  int         err = 0;

  err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
  if (0 > err)
    {
      output_error("Problem while creating alsa handler:\n%s",
                   snd_strerror(err));
      output_warning("(Is snd_seq module unabled ?)");
      return NULL;
    }
  snd_seq_set_client_name(handle, name);
  return handle;
}

void free_aseqh(snd_seq_t *handle)
{
  int err = 0;

  err = snd_seq_close(handle);
  snd_config_update_free_global();
  if (0 != err)
    output_error("problem while closing alsa seq handler\n%s\n",
                 snd_strerror(err));
}

aseq_output_t  *create_aseq_output(snd_seq_t *handle,
                                   const char *name,
                                   msq_bool_t *ev_to_drain,
                                   msq_bool_t *is_running)
{
  aseq_output_t        *aseqoutput = myalloc(sizeof (aseq_output_t));

  aseqoutput->ev_to_drain = ev_to_drain;
  aseqoutput->is_running = is_running;
  aseqoutput->handle = handle;
  aseqoutput->port =
    snd_seq_create_simple_port(aseqoutput->handle,
                               name,
                               SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                               SND_SEQ_PORT_TYPE_APPLICATION);
  snd_seq_port_info_malloc(&(aseqoutput->info));
  snd_seq_get_port_info(handle, aseqoutput->port, aseqoutput->info);
  return aseqoutput;
}

void free_aseq_output(aseq_output_t *output)
{
  int err = 0;

  err = snd_seq_delete_port(output->handle, output->port);
  if (0 != err)
    output_error("problem while deleting alsa port\n%s\n", snd_strerror(err));
  snd_seq_port_info_free(output->info);
  free(output);
}

const char *aseq_output_get_name(void *addr)
{
  aseq_output_t *output = (aseq_output_t *) addr;

  return snd_seq_port_info_get_name(output->info);
}

void aseq_output_set_name(void *addr, const char *name)
{
  aseq_output_t *output = (aseq_output_t *) addr;
  int err = 0;

  snd_seq_port_info_set_name(output->info, name);
  err = snd_seq_set_port_info(output->handle, output->port, output->info);
  if (0 != err)
    output_error("problem while setting alsa port info\n%s\n",
                 snd_strerror(err));
}
