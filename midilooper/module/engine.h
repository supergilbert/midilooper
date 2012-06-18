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
  aseqport_ctx_t   *aseqport_ctx;
  track_t          *track;
  uint_t           loop_start;
  list_iterator_t  current_tickev;
  pthread_rwlock_t lock;
  list_t           trash;
  bool_t           is_handled;
  bool_t           to_del;
  bool_t           ev_todel;
} track_ctx_t;

typedef struct
{
  pthread_t             thread_id;
  /* pthread_attr_t        thread_attr; */
  bool_t                thread_ret;
  pthread_rwlock_t      lock;
  engine_rq             rq;
  engine_info_t         info;
  list_t                track_list;
  /* track_ctx_t           track_ctx; */
  list_t                aseqport_list;
  /* aseqport_ctx_t        *aseqport_ctx; */
  snd_seq_t             *aseqh;
  clockloop_t           looph;
  uint_t                ppq;
}       engine_ctx_t;

engine_ctx_t    *init_engine_ctx(char *aname);
void            free_engine_ctx(engine_ctx_t *ctx);
bool_t          start_engine(engine_ctx_t *ctx);
void            stop_engine(engine_ctx_t *ctx);
void            wait_engine(engine_ctx_t *ctx);
clock_req_t     engine_cb(void *arg);
bool_t          engine_isrunning(engine_ctx_t *ctx);
aseqport_ctx_t  *engine_create_aport(engine_ctx_t *ctx, char *name);
bool_t          engine_del_port(engine_ctx_t *ctx,
                                aseqport_ctx_t *aseqportctx);
bool_t          engine_del_track(engine_ctx_t *ctx, track_ctx_t *trackctx);
track_ctx_t     *engine_new_track(engine_ctx_t *ctx, char *name);
void            engine_copy_tracklist(engine_ctx_t *ctx, list_t *tracklist);
void            trackctx_event2trash(track_ctx_t *traxkctx,
                                     list_iterator_t *tickit,
                                     list_iterator_t *evit);

void engine_save_project(engine_ctx_t *ctx, char *file_path);

#endif
