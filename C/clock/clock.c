#include <stdlib.h>
#include <stdio.h>
#include "clock.h"
#include "debug_tool/debug_tool.h"
#include <strings.h>

static void	add_time(struct timespec *t1, struct timespec *t2)
{
  t1->tv_sec += t2->tv_sec;
  t1->tv_nsec += t2->tv_nsec;
  if (t1->tv_nsec >= 1000000000)
    {
      t1->tv_sec++;
      t1->tv_nsec -= 1000000000;
    }
}

/* static void	del_time(struct timespec *t1, struct timespec *t2) */
/* { */
/*   t1->tv_sec -= t2->tv_sec; */
/*   t1->tv_nsec -= t2->tv_nsec; */
/*   if (t1->tv_nsec < 0) */
/*     { */
/*       t1->tv_sec--; */
/*       t1->tv_nsec += 1000000000; */
/*     } */
/* } */

void next_tick(clocktick_t *tick, struct timespec *res, clockid_t clkid)
{
  tick->number++;
  add_time(&(tick->time), res);
  clock_nanosleep(clkid, TIMER_ABSTIME, &(tick->time), NULL);
  return;
}

bool_t _clockloop(clocktick_t *tick, struct timespec *res, clockloop_cb cb_func, void *cb_arg)
{
  clockid_t	clkid = CLOCK_REALTIME;
  clock_req_t   clkreq = 0;

  debug("%s: start loop with res at %ds %dns\n",
        __FUNCTION__, res->tv_sec, res->tv_nsec);
  if (0 != clock_gettime(clkid, &(tick->time)))
    {
      perror("line: __LINE__ function: __FUNCTION__");
      return FALSE;
    }
  do
    {
      clkreq = cb_func(cb_arg);
      next_tick(tick, res, clkid);
    }  while (clkreq != STOP);
  return TRUE;
}

bool_t clockloop(clockloop_t *looph)
{
  return _clockloop(&(looph->clocktick),
                    &(looph->res),
                    looph->cb_func,
                    looph->cb_arg);
}

/* void free_clockloop_struct(clockloop_t *loop) */
/* { */
/*   free(loop); */
/* } */

/* /!\ Caution ppm must not be zero */
void set_bpmnppq_to_timespec(struct timespec *res, uint_t ppq, uint_t bpm)
{
  uint_t ppm = bpm * ppq; /* /!\ bpm and ppq musnt be zero */
  double tmp;

  debug("%s: bpm=%d ppq=%d\n", __FUNCTION__, bpm, ppq);
  res->tv_sec = 60 / ppm;
  tmp = 1000000000;
  tmp = 60 * tmp;
  tmp = (tmp / ppm);
  res->tv_nsec = tmp;
  res->tv_nsec = res->tv_nsec % 1000000000;
  debug("%s: setting time res at %ds %dns\n",
        __FUNCTION__, res->tv_sec, res->tv_nsec);
}

/* /!\ Caution ppq must not be zero */
void set_msnppq_to_timespec(struct timespec *res, uint_t ppq, uint_t ms)
{
  uint_t pulse_nsec = (ms * 1000) / ppq;

  debug("%s: ms=%d ppq=%d\n", __FUNCTION__, ms, ppq);
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

void set_clockloop_bpm_ppq(clockloop_t *clockloop,
                           uint_t bpm,
                           uint_t ppq,
                           clockloop_cb cb_func,
                           void *cb_arg)
{
  set_bpmnppq_to_timespec(&(clockloop->res), ppq, bpm);
  _set_callback(clockloop, cb_func, cb_arg);
}

void set_clockloop_ms_ppq(clockloop_t *clockloop,
                          uint_t ms,
                          uint_t ppq,
                          clockloop_cb cb_func,
                          void *cb_arg)
{
  set_msnppq_to_timespec(&(clockloop->res), ppq, ms);
  _set_callback(clockloop, cb_func, cb_arg);
}

/* clockloop_t *init_clockloop_bpm_ppq(uint_t bpm, uint_t ppq, clockloop_cb cb_func, void *cb_arg) */
/* { */
/*   clockloop_t *clockloop = myalloc(sizeof (clockloop_t)); */

/*   set_clockloop_bpm_ppq(clockloop, bpm, ppq, cb_func, cb_arg); */
/*   return clockloop; */
/* } */

/* clockloop_t *init_clockloop_ms_ppq(uint_t ms, uint_t ppq, clockloop_cb cb_func, void *cb_arg) */
/* { */
/*   clockloop_t *clockloop = myalloc(sizeof (clockloop_t)); */
/*   double pulse_nsec = (double) ms; */

/*   pulse_nsec = (pulse_nsec * 1000) / ((double) ppq); */

/*   debug("%s: ms=%d ppq=%d\n", __FUNCTION__, ms, ppq); */
/*   clockloop->res.tv_sec = (time_t) pulse_nsec / 1000000000; */
/*   clockloop->res.tv_nsec = (long) pulse_nsec % 1000000000; */
/*   debug("%s: setting time res at %ds %dns\n", __FUNCTION__, clockloop->res.tv_sec, clockloop->res.tv_nsec); */
/*   clockloop->cb_func = cb_func; */
/*   clockloop->cb_arg = cb_arg; */
/*   return clockloop; */
/* } */
