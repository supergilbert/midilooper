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


#include <stdlib.h>
#include <stdio.h>
#include "clock.h"
#include "debug_tool/debug_tool.h"
#include <strings.h>

static void add_time(struct timespec *t1, struct timespec *t2)
{
  t1->tv_sec += t2->tv_sec;
  t1->tv_nsec += t2->tv_nsec;
  if (t1->tv_nsec >= 1000000000)
    {
      t1->tv_sec++;
      t1->tv_nsec -= 1000000000;
    }
}

void next_tick(clocktick_t *tick, struct timespec *res, clockid_t clkid)
{
  tick->number++;
  add_time(&(tick->time), res);
  clock_nanosleep(clkid, TIMER_ABSTIME, &(tick->time), NULL);
  return;
}

void dump_clockres(clockid_t clkid)
{
  struct timespec res;

  clock_getres(clkid, &res);
  output("Clock resolution: %ds %dns\n", res.tv_sec, res.tv_nsec);
}

msq_bool_t _clockloop(clocktick_t *tick, struct timespec *res, clockloop_cb cb_func, void *cb_arg)
{
  clockid_t clkid = CLOCK_REALTIME;

  debug("%s: start loop with res at %ds %dns\n",
        __FUNCTION__, res->tv_sec, res->tv_nsec);
  if (0 != clock_gettime(clkid, &(tick->time)))
    {
      perror("line: __LINE__ function: __FUNCTION__");
      return MSQ_FALSE;
    }
  while (CLOCK_STOP != cb_func(cb_arg))
    next_tick(tick, res, clkid);
  return MSQ_TRUE;
}

msq_bool_t clockloop(clockloop_t *looph)
{
  return _clockloop(&(looph->clocktick),
                    &(looph->res),
                    looph->cb_func,
                    looph->cb_arg);
}

/* /!\ Caution ppq must not be zero */
void set_msnppq_to_timespec(struct timespec *res, uint_t ppq, uint_t ms)
{
  uint_t pulse_nsec = (ms * 1000) / ppq;

  res->tv_sec = pulse_nsec / 1000000000;
  res->tv_nsec = (long) pulse_nsec % 1000000000;
  debug("%s: setting time res at %ds %dns\n",
        __FUNCTION__, res->tv_sec, res->tv_nsec);
}

void _set_callback(clockloop_t *clockloop,
                   clockloop_cb cb_func,
                   void *cb_arg)
{
  clockloop->cb_func = cb_func;
  clockloop->cb_arg = cb_arg;
}
