#include <Python.h>

#include "asound/aseq.h"
#include "seqtool/seqtool.h"
#include "loop_engine/ev_iterator.h"
#include "./pym_midiseq_evwr.h"
#include "debug_tool/debug_tool.h"

PyObject *getall_event_repr(list_t *tickev_list)
{
  PyObject      *ret_obj = NULL;
  ev_iterator_t evit;
  seqev_t       *seqev   = NULL;
  midicev_t     *midicev = NULL;

  ret_obj = PyList_New(0);
  if (tickev_list->len > 0)
    {
      seqev = evit_init(&evit, tickev_list);
      if (seqev->type == MIDICEV)
          midicev = seqev->addr;
      else
        midicev = evit_next_midiallchannel(&evit);
      while (midicev)
        {
          PyList_Append(ret_obj, build_evrepr(evit.tick, midicev));
          midicev = evit_next_midiallchannel(&evit);
        }
    }
  return ret_obj;
}


PyObject *getall_noteonoff_repr(list_t *tickev_list, byte_t channel)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  PyObject      *ret_obj = NULL;
  PyObject      *ev_repr = NULL;
  seqev_t       *seqev   = NULL;
  midicev_t     *midicev_noteon = NULL, *midicev_noteoff = NULL;

  ret_obj = PyList_New(0);

  if (tickev_list)
    {
      seqev = evit_init(&evit_noteon, tickev_list);
      if (seqev)
        {
          if (seqev->type == MIDICEV)
            {
              midicev_noteon = seqev->addr;
              if (midicev_noteon->type != NOTEON)
                midicev_noteon = evit_next_noteon(&evit_noteon, channel);
            }
          else
            midicev_noteon = evit_next_noteon(&evit_noteon, channel);
          while (midicev_noteon)
            {
              evit_copy(&evit_noteon, &evit_noteoff);
              midicev_noteoff = evit_next_noteoff_num(&evit_noteoff,
                                                      channel,
                                                      midicev_noteon->event.note.num);
              if (midicev_noteoff)
                {
                  ev_repr = PyList_New(0);
                  PyList_Append(ev_repr, build_evrepr(evit_noteon.tick, midicev_noteon));
                  PyList_Append(ev_repr, build_evrepr(evit_noteoff.tick, midicev_noteoff));
                  PyList_Append(ret_obj, ev_repr);
                }
              midicev_noteon = evit_next_noteon(&evit_noteon, channel);
            }
        }
    }
  return ret_obj;
}


/* bool_t _noteonoff_match_selection(uint_t tick_min, */
/*                                   uint_t tick_max, */
/*                                   byte_t note_min, */
/*                                   byte_t note_max, */
/*                                   uint_t tick_on, */
/*                                   uint_t tick_off, */
/*                                   byte_t note) */
/* { */
/*   if ((tick_min <= tick_off && tick_on <= tick_max) && */
/*       (note_min <= note && note <= note_max)) */
/*       return TRUE; */
/*   return FALSE; */
/* } */

/* #define _evit_get_midicev(evit) ((midicev_t *) (evit)->seqevit.node->addr) */

/* PyObject *_get_noteonoff_repr(ev_iterator_t *evit_noteon, */
/*                               midicev_t *midicev_noteon, */
/*                               uint_t tick_min, */
/*                               uint_t tick_max, */
/*                               byte_t note_min, */
/*                               byte_t note_max) */
/* { */
/*   ev_iterator_t evit_noteoff; */
/*   midicev_t     *midicev_noteoff = NULL; */
/*   PyObject      *ev_repr = NULL; */

/*   evit_copy(evit_noteon, &evit_noteoff); */
/*   midicev_noteoff = evit_next_noteoff_num(&evit_noteoff, */
/*                                           midicev_noteon->chan, */
/*                                           midicev_noteon->event.note.num); */
/*   if (midicev_noteoff) */
/*     { */
/*       if (_noteonoff_match_selection(tick_min, */
/*                                      tick_max, */
/*                                      note_min, */
/*                                      note_max, */
/*                                      evit_noteon->tick, */
/*                                      evit_noteoff.tick, */
/*                                      midicev_noteon->event.note.num)) */
/*         { */
/*           ev_repr = PyList_New(0); */
/*           PyList_Append(ev_repr, build_evrepr(evit_noteon->tick, midicev_noteon)); */
/*           PyList_Append(ev_repr, build_evrepr(evit_noteoff.tick, midicev_noteoff)); */
/*         } */
/*     } */
/*   return ev_repr; */
/* } */


/* PyObject *_get_noteonoff_evwr(ev_iterator_t *evit_noteon, */
/*                               midicev_t *midicev_noteon, */
/*                               uint_t tick_min, */
/*                               uint_t tick_max, */
/*                               byte_t note_min, */
/*                               byte_t note_max) */
/* { */
/*   ev_iterator_t evit_noteoff; */
/*   midicev_t     *midicev_noteoff = NULL; */
/*   PyObject      *ev_repr = NULL; */


/*   evit_copy(evit_noteon, &evit_noteoff); */
/*   midicev_noteoff = evit_next_noteoff_num(&evit_noteoff, */
/*                                           midicev_noteon->chan, */
/*                                           midicev_noteon->event.note.num); */
/*   if (midicev_noteoff) */
/*     { */
/*       if (_noteonoff_match_selection(tick_min, */
/*                                      tick_max, */
/*                                      note_min, */
/*                                      note_max, */
/*                                      evit_noteon->tick, */
/*                                      evit_noteoff.tick, */
/*                                      midicev_noteon->event.note.num)) */
/*         { */
/*           ev_repr = PyList_New(0); */
/*           PyList_Append(ev_repr, build_evwr(evit_noteon)); */
/*           PyList_Append(ev_repr, build_evwr(&evit_noteoff)); */
/*         } */
/*     } */
/*   return ev_repr; */
/* } */


/* typedef PyObject *(*get_noteonoff_func_t)(ev_iterator_t *evit_noteon, */
/*                                           midicev_t *midicev_noteon, */
/*                                           uint_t tick_min, */
/*                                           uint_t tick_max, */
/*                                           byte_t note_min, */
/*                                           byte_t note_max); */

PyObject *_build_noteonoff_evwr(ev_iterator_t *evit_noteon,
                                ev_iterator_t *evit_noteoff,
                                track_ctx_t *trackctx)
{
  PyObject      *ev_repr = NULL;

  ev_repr = PyList_New(0);
  PyList_Append(ev_repr, build_evwr_from_evit(evit_noteon, trackctx));
  PyList_Append(ev_repr, build_evwr_from_evit(evit_noteoff, trackctx));
  return ev_repr;
}

#define _evit_get_midicev(evit)                                 \
  ((midicev_t *) ((seqev_t *) evit->seqevit.node->addr)->addr)
PyObject *_build_noteonoff_repr(ev_iterator_t *evit_noteon,
                                ev_iterator_t *evit_noteoff,
                                track_ctx_t *unused)
{
  PyObject      *ev_repr = NULL;

  ev_repr = PyList_New(0);
  PyList_Append(ev_repr, build_evrepr(evit_noteon->tick, _evit_get_midicev(evit_noteon)));
  PyList_Append(ev_repr, build_evrepr(evit_noteoff->tick, _evit_get_midicev(evit_noteoff)));
  return ev_repr;
}

typedef PyObject *(*build_noteonoff_func_t)(ev_iterator_t *evit_noteon,
                                            ev_iterator_t *evit_noteoff,
                                            track_ctx_t *trackctx);

PyObject *_sel_noteonoff(track_ctx_t *trackctx,
                         byte_t channel,
                         uint_t tick_min,
                         uint_t tick_max,
                         byte_t note_min,
                         byte_t note_max,
                         build_noteonoff_func_t build_noteonoff)
{
  ev_iterator_t evit_noteon;
  ev_iterator_t evit_noteoff;
  PyObject      *ret_obj = PyList_New(0);
  PyObject      *ev_repr = NULL;
  seqev_t       *seqev   = NULL;
  midicev_t     *midicev_noteon = NULL;
  midicev_t     *midicev_noteoff = NULL;

  if (trackctx)
    {
      seqev = evit_init(&evit_noteon, &(trackctx->track->tickev_list));
      if (seqev)
        {
          if (seqev->type == MIDICEV)
            {
              midicev_noteon = seqev->addr;
              if (midicev_noteon->type != NOTEON)
                midicev_noteon = evit_next_noteon(&evit_noteon, channel);
            }
          else
            midicev_noteon = evit_next_noteon(&evit_noteon, channel);
          while (midicev_noteon)
            {
              if (evit_noteon.tick <= tick_max &&
                  note_min <= midicev_noteon->event.note.num &&
                  midicev_noteon->event.note.num <= note_max)
                {
                  evit_copy(&evit_noteon, &evit_noteoff);
                  midicev_noteoff = evit_next_noteoff_num(&evit_noteoff,
                                                          midicev_noteon->chan,
                                                          midicev_noteon->event.note.num);
                  if (midicev_noteoff != NULL && tick_min <= evit_noteoff.tick)
                    {
                      ev_repr = build_noteonoff(&evit_noteon, &evit_noteoff, trackctx);
                      PyList_Append(ret_obj, ev_repr);
                    }
                  /* else */
                  /*   msg; */
                }
              midicev_noteon = evit_next_noteon(&evit_noteon, channel);
            }
        }
    }
  return ret_obj;
}


PyObject *sel_noteonoff_repr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             char note_min,
                             char note_max)
{
  return _sel_noteonoff(trackctx,
                        channel,
                        tick_min,
                        tick_max,
                        note_min,
                        note_max,
                        _build_noteonoff_repr);
}


PyObject *sel_noteonoff_evwr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             char note_min,
                             char note_max)
{
  return _sel_noteonoff(trackctx,
                        channel,
                        tick_min,
                        tick_max,
                        note_min,
                        note_max,
                        _build_noteonoff_evwr);
}

#include "loop_engine/engine.h"

void trackctx_del_event(track_ctx_t *track_ctx,
                        ev_iterator_t *ev_iterator)
{
  evit_del_event(ev_iterator);
}

void delete_evwr_list(track_ctx_t *trackctx, PyObject *pylist)
{
  Py_ssize_t list_len;
  Py_ssize_t idx;
  midiseq_evwrObject *evwrobj = NULL;

  void (*del_func)(track_ctx_t *, ev_iterator_t *);

  if (trackctx->is_handled == TRUE)
    del_func = trackctx_event2trash;
  else
    del_func = trackctx_del_event;

  for (list_len = PyList_GET_SIZE(pylist),
         idx = 0;
       idx < list_len;
       idx++)
    {
      evwrobj = (midiseq_evwrObject *) PyList_GetItem(pylist, idx);
      del_func(trackctx, &(evwrobj->evit));
    }
}

#include "seqtool/seqtool.h"

void _evit_add_midicev(ev_iterator_t *evit, uint_t tick, midicev_t *mcev)
{
  node_t   *tick_node = search_or_add_ticknode(evit->tickit.list, tick);
  tickev_t *tickev    = tick_node->addr;
  seqev_t  *seqev     = alloc_seqev(mcev, MIDICEV);
  node_t   *seq_node  = NULL;

  if (mcev->type == NOTEOFF)
    seq_node = push_to_list(&(tickev->seqev_list), (void *) seqev);
  else
    seq_node = push_to_list_tail(&(tickev->seqev_list), (void *) seqev);

  evit->tickit.node  = tick_node;
  evit->seqevit.list = &(tickev->seqev_list);
  evit->seqevit.node = seq_node;
  evit->tick         = tickev->tick;
}

PyObject *add_pyevrepr(track_ctx_t *trackctx, PyObject *pyevrepr)
{
  uint_t        type, tick;
  Py_ssize_t    repr_sz;
  midicev_t     *mcev;
  PyObject      *obj = NULL;
  ev_iterator_t evit;

  evit_init(&evit, &(trackctx->track->tickev_list));

  if (!PyTuple_Check(pyevrepr))
    {
      output_error("Event representation is not a tuple");
      return NULL;
    }

  repr_sz = PyTuple_GET_SIZE(pyevrepr);
  if (repr_sz != 5)
    {
      output_error("Unsupported event representation (list size %d mismatch)", repr_sz);
      return NULL;
    }

  obj = PyTuple_GetItem(pyevrepr, 2);
  type = PyInt_AS_LONG(obj);
  switch (type)
    {
    case NOTEON:
    case NOTEOFF:
      mcev = myalloc(sizeof (midicev_t));
      obj = PyTuple_GetItem(pyevrepr, 1);
      mcev->chan = PyInt_AS_LONG(obj);
      mcev->type = type;
      obj = PyTuple_GetItem(pyevrepr, 3);
      mcev->event.note.num = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 4);
      mcev->event.note.val = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 0);
      tick = PyInt_AS_LONG(obj);
      _evit_add_midicev(&evit, tick, mcev);
      obj = build_evwr_from_evit(&evit, trackctx);
      /* build_evwr <------------------------------- avec evit_add_midicev */
      /* add_new_midicev(track, tick, mcev); <------ on degage ca */
      break;
    default:
      output_error("Unsupported event type");
    }
  return obj;
}

PyObject *add_evrepr_list(track_ctx_t *trackctx, PyObject *pylist)
{
  Py_ssize_t list_len;
  Py_ssize_t idx;
  PyObject   *evrepr, *evwr, *list;

  list = PyList_New(0);
  for (list_len = PyList_GET_SIZE(pylist),
         idx = 0;
       idx < list_len;
       idx++)
    {
      evrepr = PyList_GetItem(pylist, idx);
      evwr = add_pyevrepr(trackctx, evrepr);
      PyList_Append(list, evwr);
    }
  return list;
}
