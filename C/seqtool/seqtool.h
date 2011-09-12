#ifndef __SEQ_TOOL_H
#define __SEQ_TOOL_H

#include "tool/tool.h"

typedef enum {
  UNDEFINED = 0,
  ASEQTYPE,
  MIDICEV
} seqevtype_t;

typedef struct
{
  seqevtype_t   type;
  void          *addr;
} seqev_t;

typedef struct
{
  uint_t	tick;
  list_t        seqev_list;
}		tickev_t;

typedef struct
{
  /* + d'info */
  char          *name;
  list_t        tickev_list;
}	        track_t;

void add_new_seqev(track_t *track,
                   uint_t tick,
                   void *addr,
                   seqevtype_t type);
#define add_new_

typedef void (*free_seqev_addr_func)(void *addr);

void free_track(void *addr);

#endif
