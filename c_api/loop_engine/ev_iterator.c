#include "asound/aseq.h"
#include "seqtool/seqtool.h"
#include "./ev_iterator.h"

void dump_evit(ev_iterator_t *evit)
{
  printf("evit:%p tick:%d tickit->node:%p sevit->node:%p\n",
         evit, evit->tick, evit->tickit.node, evit->seqevit.node);
}

seqev_t *_it_next_seqev(list_iterator_t *seqevit)
{
  seqev_t *ev = NULL;


  for (iter_next(seqevit);
       iter_node(seqevit) != NULL;
       iter_next(seqevit))
    {
      ev = (seqev_t *) iter_node_ptr(seqevit);
      if (ev->deleted == FALSE)
        return ev;
    }
  return NULL;
}

/* #include "debug_tool/debug_tool.h" */
/* #define print_trace(format, ...) output(TRACE_FMT format "\n", TRACE_ARG, ##__VA_ARGS__) */

seqev_t *_evit_seqev_head(list_iterator_t *seqevit)
{
  seqev_t *ev = NULL;

  iter_head(seqevit);
  if (iter_node(seqevit))
    {
      ev = (seqev_t *) iter_node_ptr(seqevit);
      if (ev->deleted == TRUE)
        {
          ev = _it_next_seqev(seqevit);
        }
    }
  return ev;
}

seqev_t *_evit_seqevit_init(list_iterator_t *seqevit, list_t *seqev_list)
{
  iter_init(seqevit, seqev_list);
  return _evit_seqev_head(seqevit);
}


seqev_t *evit_next_tick(ev_iterator_t *ev_iterator)
{
  tickev_t *tickev = NULL;
  seqev_t  *seqev  = NULL;


  if (iter_node(&(ev_iterator->tickit)))
    {
      for (iter_next(&(ev_iterator->tickit));
           iter_node(&(ev_iterator->tickit)) != NULL;
           iter_next(&(ev_iterator->tickit)))
        {
          tickev = (tickev_t *) iter_node_ptr(&(ev_iterator->tickit));
          if (tickev->deleted == FALSE)
            {
              seqev = _evit_seqevit_init(&(ev_iterator->seqevit), &(tickev->seqev_list));
              if (seqev)
                {
                  ev_iterator->tick = tickev->tick;
                  return seqev;
                }
            }
        }
    }
  return NULL;
}

seqev_t *evit_tick_head(ev_iterator_t *ev_iterator)
{
  tickev_t *tickev = NULL;
  seqev_t  *seqev  = NULL;

  iter_head(&(ev_iterator->tickit));
  if (iter_node(&(ev_iterator->tickit)))
    {
      tickev = (tickev_t *) iter_node_ptr(&(ev_iterator->tickit));
      if (tickev->deleted == TRUE)
        {
          seqev = evit_next_tick(ev_iterator);
        }
      else
        {
          seqev = _evit_seqevit_init(&(ev_iterator->seqevit), &(tickev->seqev_list));
          ev_iterator->tick = tickev->tick;
        }
    }
  return seqev;
}


seqev_t *evit_init(ev_iterator_t *ev_iterator, list_t *tickev_list)
{

  iter_init(&(ev_iterator->tickit), tickev_list);
  return evit_tick_head(ev_iterator);
}


seqev_t *evit_get_seqev(ev_iterator_t *ev_iterator)
{
  if (iter_node(&(ev_iterator->tickit)) && iter_node(&(ev_iterator->seqevit)))
    return (seqev_t *) iter_node_ptr(&(ev_iterator->seqevit));
  return NULL;
}

/* seqev_t *_evit_next_seqev(ev_iterator_t *ev_iterator) */
/* { */
/*   return _it_next_seqev(&(ev_iterator->seqevit)); */
/* } */


seqev_t *evit_next_seqev(ev_iterator_t *ev_iterator)
{
  seqev_t *ev = NULL;

  if (iter_node(&(ev_iterator->tickit)) && iter_node(&(ev_iterator->seqevit)))
    {
      ev = _it_next_seqev(&(ev_iterator->seqevit));
      if (ev == NULL)
        ev = evit_next_tick(ev_iterator);
    }
  return ev;
}

midicev_t *evit_next_midiallchannel(ev_iterator_t *ev_iterator)
{
  seqev_t   *ev = NULL;

  for (ev = evit_next_seqev(ev_iterator);
       ev != NULL;
       ev = evit_next_seqev(ev_iterator))
    if (ev->type == MIDICEV)
      return (midicev_t *) ev->addr;
  return NULL;
}

midicev_t *evit_next_midicev(ev_iterator_t *ev_iterator, byte_t channel)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_midiallchannel(ev_iterator);
       midicev != NULL;
       midicev = evit_next_midiallchannel(ev_iterator))
    if (midicev->chan == channel)
      return midicev;
  return NULL;
}


midicev_t *evit_next_midicev_type(ev_iterator_t *ev_iterator, byte_t channel, byte_t type)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_midicev(ev_iterator, channel);
       midicev != NULL;
       midicev = evit_next_midicev(ev_iterator, channel))
    {
      if (midicev->type == type)
        return midicev;
    }
  return NULL;
}


midicev_t *evit_next_noteoff_num(ev_iterator_t *ev_iterator, byte_t channel, byte_t num)
{
  midicev_t *midicev = NULL;

  for (midicev = evit_next_noteoff(ev_iterator, channel);
       midicev != NULL;
       midicev = evit_next_noteoff(ev_iterator, channel))
    {
      if (midicev->event.note.num == num)
        return midicev;
    }
  return NULL;
}


void evit_copy(ev_iterator_t *evit_src, ev_iterator_t *evit_dst)
{
  iter_copy(&(evit_src->tickit), &(evit_dst->tickit));
  iter_copy(&(evit_src->seqevit), &(evit_dst->seqevit));
  evit_dst->tick = evit_src->tick;
}

void evit_del_event(ev_iterator_t *ev_iterator)
{
  iter_node_del(&(ev_iterator->seqevit), free_seqev);
  if (ev_iterator->seqevit.list->len <= 0)
    iter_node_del(&(ev_iterator->tickit), free_tickev);
}

#include "debug_tool/debug_tool.h"

bool_t _evit_check(ev_iterator_t *evit, list_t *tickev_list)
{
  ev_iterator_t evit_ptr;
  seqev_t       *ev;

  if (evit->tickit.list != tickev_list)
    {
      output_error("Tick event list mismatch");
      return FALSE;
    }
  for (ev = evit_init(&evit_ptr, tickev_list);
       ev;
       ev = evit_next_tick(&evit_ptr))
    {
      if (evit_ptr.tick < evit->tick)
        continue;
      else
        {
          if (evit_ptr.tick != evit->tick)
            {
              break;
            }
          if (iter_node(&(evit_ptr.tickit)) != iter_node(&(evit->tickit)))
            {
              break;
            }
          if (iter_node(&(evit_ptr.seqevit)) == iter_node(&(evit->seqevit)))
            return TRUE;
          else
            while ((ev = _it_next_seqev(&(evit_ptr.seqevit))) != NULL)
              {
                if (iter_node(&(evit_ptr.seqevit)) == iter_node(&(evit->seqevit)))
                  return TRUE;
              }
          output_error("sequencer event iterator mismatch");
          return FALSE;
        }
    }
  output_error("tick iterator mismatch");
  return FALSE;
}
