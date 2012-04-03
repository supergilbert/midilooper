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
  /* pthread_rwlock_t      lock; */
  bool_t isrunning;
} engine_info_t;

typedef struct
{
  aseqport_ctx_t  *aseqport_ctx;
  track_t         *track;
  list_iterator_t current_tickev;
}       track_ctx_t;

typedef struct
{
  pthread_t             thread_id;
  /* pthread_attr_t        thread_attr; */
  bool_t                thread_ret;
  pthread_rwlock_t      lock;
  engine_rq             rq;
  engine_info_t         info;
  track_ctx_t           track_ctx;
  aseqport_ctx_t        *aseqport_ctx;
  snd_seq_t             *aseqh;
  clockloop_t           looph;
  uint_t                ppq;
}       engine_ctx_t;

engine_ctx_t    *init_engine(char *aport_name);
void            free_engine_ctx(engine_ctx_t *ctx);
bool_t          start_engine(engine_ctx_t *ctx);
void            stop_engine(engine_ctx_t *ctx);
void            wait_engine(engine_ctx_t *ctx);
clock_req_t     engine_cb(void *arg);
bool_t          engine_isrunning(engine_ctx_t *ctx);

#endif
