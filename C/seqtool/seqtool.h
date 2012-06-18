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
  bool_t        todel;
} seqev_t;

typedef struct
{
  uint_t	tick;
  list_t        seqev_list;
  bool_t        todel;
}		tickev_t;

typedef struct
{
  /* + d'info */
  uint_t        len;
  char          *name;
  list_t        tickev_list;
}	        track_t;

void dumpaddr_seqevlist(list_t *seqev_list);

void add_new_seqev(track_t *track,
                   uint_t tick,
                   void *addr,
                   seqevtype_t type);

typedef void (*free_seqev_addr_func)(void *addr);

void free_track(void *addr);
void clear_tickev_list(list_t *tickev_list);

node_t *search_ticknode(list_t *tickev_list, uint_t tick);
void free_seqev(void *addr);
void free_tickev(void *addr);

#endif
