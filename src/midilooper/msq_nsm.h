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

#include "msq_nsm_inc.h"

msq_bool_t msq_nsm_start_client(msq_nsm_client_t *msq_nsm_client,
                                const char *nsm_srv_url,
                                const char *app_name,
                                const char *exe_name,
                                msq_nsm_open_cb_t open_cb,
                                msq_nsm_quit_cb_t quit_cb,
                                msq_nsm_save_cb_t save_cb,
                                void *cb_arg);
