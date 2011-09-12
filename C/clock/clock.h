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
    CONTINUE = 0,
    STOP
  } clock_req_t;

typedef clock_req_t (*clockloop_cb)(void *arg);

typedef struct
{
  clocktick_t     clocktick;
  clockloop_cb    cb_func;
  void            *cb_arg;
  struct timespec res;
} clockloop_t;

bool_t clockloop(clockloop_t *looph);

void free_clockloop_struct(clockloop_t *clockloop);
void set_bpmnppq_to_timespec(struct timespec *res, uint_t bpm, uint_t ppq);
void set_msnppq_to_timespec(struct timespec *res, uint_t ms, uint_t ppq);
void set_clockloop_bpm_ppq(clockloop_t *clockloop,
                           uint_t bpm,
                           uint_t ppq,
                           clockloop_cb cb_func,
                           void *cb_arg);
void set_clockloop_ms_ppq(clockloop_t *clockloop,
                          uint_t ms,
                          uint_t ppq,
                          clockloop_cb cb_func,
                          void *cb_arg);

#endif
