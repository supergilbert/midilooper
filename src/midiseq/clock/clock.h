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


#ifndef CLOCK_H
#define CLOCK_H

#include <time.h>
#include "tool/tool.h"

typedef struct
{
  struct timespec time;
  uint_t    number;
} clocktick_t;

typedef enum
  {
    CLOCK_CONTINUE = 0,
    CLOCK_STOP
  } clock_req_t;

typedef clock_req_t (*clockloop_cb)(void *arg);

typedef struct
{
  clocktick_t     clocktick;
  clockloop_cb    cb_func;
  void            *cb_arg;
  struct timespec res;
} clockloop_t;

msq_bool_t clockloop(clockloop_t *looph);

void free_clockloop_struct(clockloop_t *clockloop);
void set_msnppq_to_timespec(struct timespec *res, uint_t ppq, uint_t ms);

#endif
