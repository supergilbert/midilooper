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

/* You should have received a copy of the GNU Gneneral Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */

#pragma once

#include "lo/lo.h"
#include "tool/tool.h"
#include "pbt_tools.h"
#include <signal.h>

typedef enum
  {
   MSQ_NSM_WAIT_CONNECTION = 0,
   MSQ_NSM_CONNECTED,
   MSQ_NSM_ANNOUNCE_ERROR
  } msq_nsm_client_state_t;

typedef msq_bool_t (*msq_nsm_open_cb_t)(const char *project_path,
                                        const char *client_id,
                                        void *arg);
typedef void (*msq_nsm_quit_cb_t)(void *arg);
typedef msq_bool_t (*msq_nsm_save_cb_t)(void *arg);

typedef struct
{
  lo_server_thread       srv_th;
  lo_server              srv;
  lo_address             nsm_srv_addr;
  msq_nsm_client_state_t state;
  char                   *manager_name;
  char                   *project_path;
  char                   *display_name;
  char                   *clien_id;
  msq_nsm_open_cb_t      open_cb;
  msq_nsm_quit_cb_t      quit_cb;
  msq_nsm_save_cb_t      save_cb;
  void                   *cb_arg;
  pid_t                  pid;
  struct sigaction       old_action;
} msq_nsm_client_t;
