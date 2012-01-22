#ifndef __ENGINE_H
#define __ENGINE_H

#include <pthread.h>

/* #include "seqtool/seqtool.h" */
#include "midi/midifile.h"
#include "clock/clock.h"
#include "asound/aseq.h"

typedef enum
  {
    engine_cont = 0,
    engine_stop
  }     engine_rq;

typedef struct
{
  pthread_rwlock_t      lock;
  bool_t                isrunning;
} engine_info_t;

typedef struct
{
  pthread_rwlock_t      lock;
  engine_rq             rq;
} engine_rq_t;

typedef struct
{
  track_t         *track;
  list_iterator_t current_tickev;
}       track_ctx_t;

typedef struct
{
  pthread_t             thread_id;
  /* pthread_attr_t        thread_attr; */
  bool_t                thread_ret;
  engine_info_t         info;
  engine_rq_t           rq;
  track_ctx_t           track_ctx;
  aseq_ctx_t            *aseq_ctx;
  clockloop_t           looph;
  uint_t                ppq;
}       engine_ctx_t;

engine_ctx_t    *init_engine(char *aport_name);
void            free_engine_ctx(engine_ctx_t *ctx);
bool_t          start_engine(engine_ctx_t *ctx);
void            stop_engine(engine_ctx_t *ctx);
void            wait_engine(engine_ctx_t *ctx);
clock_req_t     engine_cb(void *arg);

#endif
