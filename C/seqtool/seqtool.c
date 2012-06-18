#include "seqtool/seqtool.h"
#include <stdlib.h>
#include <strings.h>

#include "debug_tool/debug_tool.h"


void dumpaddr_seqevlist(list_t *seqev_list)
{
  list_iterator_t evit;

  for (iter_init(&evit, seqev_list);
       iter_node(&evit);
       iter_next(&evit))
    output("ev node addr = %p\n", iter_node(&evit));
}

void free_seqev(void *addr)
{
  seqev_t *seqev = (seqev_t *) addr;

  if (seqev)
    {
      switch (seqev->type)
        {
        case ASEQTYPE:
        case MIDICEV:
          free(seqev->addr);
          break;
        default:
          break;
        }
      free(seqev);
    }
}

void free_tickev(void *addr)
{
  tickev_t *tickev = (tickev_t *) addr;

  if (tickev == NULL)
    return;
  free_list_node(&(tickev->seqev_list), free_seqev);
  free(tickev);
}

void clear_tickev_list(list_t *tickev_list)
{
  free_list_node(tickev_list, free_tickev);
  tickev_list->len = 0;
}

void free_track(void *addr)
{
  track_t *track = (track_t *) addr;

  if (track)
    {
      free_list_node(&(track->tickev_list), free_tickev);
      if (track->name != NULL)
        free(track->name);
      free(track);
    }
}

tickev_t *_create_new_tick_ev(uint_t tick)
{
  tickev_t     *tickev = myalloc(sizeof (tickev_t));

  bzero(tickev, sizeof (tickev_t));
  tickev->tick = tick;
  tickev->todel = TRUE;
  return tickev;
}

tickev_t *_search_or_add_tickev(track_t *track, uint_t tick)
{
  list_iterator_t iter;
  tickev_t *tickev = NULL;

  iter_init(&iter, &(track->tickev_list));
  while (iter_node(&iter))
    {
      tickev = iter_node_ptr(&iter);
      if (tickev->tick == tick)
        return tickev;
      else if (tickev->tick > tick)
        {
          tickev = _create_new_tick_ev(tick);
          iter_push_before(&iter, (void *) tickev);
          return tickev;
        }
      iter_next(&iter);
    }
  tickev = _create_new_tick_ev(tick);
  push_to_list_tail(&(track->tickev_list), tickev);
  return tickev;
}

/* Search in track for  tick event set tick time if the tick time doesnt
   exit it create a new one */
tickev_t *_get_or_add_tickev(track_t *track, uint_t tick)
{
  tickev_t *tickev = NULL;

  if (LIST_HEAD(&(track->tickev_list)) == NULL)
    {
      tickev = _create_new_tick_ev(tick);
      push_to_list(&(track->tickev_list), (void *) tickev);
      return tickev;
    }
  return _search_or_add_tickev(track, tick);
}

void add_new_seqev(track_t *track,
                   uint_t tick,
                   void *addr,
                   seqevtype_t type)
{
  tickev_t *tickev = NULL;
  seqev_t *seqev;

  if (track == NULL)
    {
      output_error(ERROR_FMT"track == NULL", ERROR_ARG);
      return;
    }
  tickev = _get_or_add_tickev(track, tick);

  seqev = myalloc(sizeof (seqev_t));
  bzero(seqev, sizeof (seqev_t));
  seqev->addr = addr;
  seqev->type = type;
  seqev->todel = FALSE;
  push_to_list_tail(&(tickev->seqev_list), (void *) seqev);
}

node_t *search_ticknode(list_t *tickev_list, uint_t tick)
{
  tickev_t      *tickev = NULL;
  list_iterator_t iter;

  iter_init(&iter, tickev_list);
  while (iter_node(&iter))
    {
      tickev = iter_node_ptr(&iter);
      if (tickev->tick == tick)
        return iter_node(&iter);
      iter_next(&iter);
    }
  return NULL;
}
