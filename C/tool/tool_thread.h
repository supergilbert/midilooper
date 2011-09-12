#ifndef __TOOL_THREAD_H
#define __TOOL_THREAD_H

typedef struct
{
  int                   thread_id;
  pthread_attr_t        thread_attr;
}       thread_ctx_t;

#endif
