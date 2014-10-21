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


#include "asound/aseq.h"
#include "debug_tool/debug_tool.h"

snd_seq_t     *create_aseqh(char *name)
{
  snd_seq_t     *handle = NULL;
  int         err = 0;

  err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK);
  if (0 > err)
    {
      output_error("problem while creating alsa handler:\n%s\n", snd_strerror(err));
      return NULL;
    }
  snd_seq_set_client_name(handle, name);
  return handle;
}

aseqport_ctx_t  *create_aseqport_ctx(snd_seq_t *handle, char *name)
{
  aseqport_ctx_t        *aseq = NULL;

  aseq = myalloc(sizeof (aseqport_ctx_t));
  aseq->handle = handle;
  aseq->output_port = _create_aseq_port(aseq, name);
  snd_seq_port_info_malloc(&(aseq->info));
  snd_seq_get_port_info(handle, aseq->output_port, aseq->info);
  return aseq;
}

void free_aseqh(snd_seq_t *handle)
{
  int err = 0;

  err = snd_seq_close(handle);
  if (0 != err)
    output_error("problem while closing alsa seq handler\n%s\n", snd_strerror(err));
}

void free_aseqport(aseqport_ctx_t *aseq)
{
  int err = 0;

  err = snd_seq_delete_port(aseq->handle, aseq->output_port);
  if (0 != err)
    output_error("problem while deleting alsa port\n%s\n", snd_strerror(err));
  snd_seq_port_info_free(aseq->info);
  free(aseq);
}

const char *aseqport_get_name(aseqport_ctx_t *aseq)
{
  return snd_seq_port_info_get_name(aseq->info);
}

void aseqport_set_name(aseqport_ctx_t *aseq, char *name)
{
  int err = 0;

  snd_seq_port_info_set_name(aseq->info, name);
  err = snd_seq_set_port_info(aseq->handle, aseq->output_port, aseq->info);
  if (0 != err)
    output_error("problem while setting alsa port info\n%s\n", snd_strerror(err));

}
