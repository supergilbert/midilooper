/* Copyright 2012-2014 Gilbert Romer */

/* This file is part of gmidilooper. */

/* gmidilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* gmidilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include <Python.h>

#include "asound/aseq.h"
#include "seqtool/seqtool.h"
#include "seqtool/ev_iterator.h"
#include "./pym_midiseq_evwr.h"
#include "debug_tool/debug_tool.h"

PyObject *build_evrepr(uint_t tick, midicev_t *midicev)
{
  PyObject *ret = NULL;

  switch (midicev->type)
    {
    case NOTEOFF:
    case NOTEON:
      ret = Py_BuildValue("(iiiii)",
                          tick,
                          midicev->chan,
                          midicev->type,
                          midicev->event.note.num,
                          midicev->event.note.val);
      break;
    case CONTROLCHANGE:
      ret = Py_BuildValue("(iiiii)",
                          tick,
                          midicev->chan,
                          midicev->type,
                          midicev->event.ctrl.num,
                          midicev->event.ctrl.val);
      break;
    case PITCHWHEELCHANGE:
      ret = Py_BuildValue("(iiiii)",
                          tick,
                          midicev->chan,
                          midicev->type,
                          midicev->event.pitchbend.Lval,
                          midicev->event.pitchbend.Hval);
      break;
    default:
      output_error("Unsupported midi channel event type: %i\n", midicev->type);
      ret = Py_BuildValue("(iii)",
                          tick,
                          midicev->chan,
                          midicev->type);
    }
  return ret;
}

PyObject *getall_event_repr(list_t *tickev_list)
{
  PyObject      *ret_obj = NULL;
  ev_iterator_t evit;
  midicev_t     *midicev = NULL;

  ret_obj = PyList_New(0);
  if (tickev_list->len > 0)
    {
      midicev = evit_init_midiallchannel(&evit, tickev_list);
      while (midicev)
        {
          PyList_Append(ret_obj, build_evrepr(evit.tick, midicev));
          midicev = evit_next_midiallchannel(&evit);
        }
    }
  return ret_obj;
}

PyObject *getall_midicev_repr(list_t *tickev_list,
                              byte_t channel,
                              uint_t tickmin,
                              uint_t tickmax)
{
  PyObject      *ret_obj = NULL;
  ev_iterator_t evit;
  midicev_t     *midicev = NULL;

  ret_obj = PyList_New(0);
  if (tickev_list->len > 0)
    {
      midicev = evit_init_midicev(&evit, tickev_list, channel);
      while (midicev && evit.tick < tickmin)
        midicev = evit_next_midicev(&evit, channel);
      while (midicev && evit.tick <= tickmax)
        {
          PyList_Append(ret_obj, build_evrepr(evit.tick, midicev));
          midicev = evit_next_midicev(&evit, channel);
        }
    }
  return ret_obj;
}

PyObject *getall_noteonoff_repr(list_t *tickev_list, byte_t channel)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  PyObject      *ret_obj = NULL;
  PyObject      *ev_repr = NULL;
  /* seqev_t       *seqev   = NULL; */
  midicev_t     *midicev_noteon = NULL, *midicev_noteoff = NULL;

  ret_obj = PyList_New(0);

  if (tickev_list)
    {
      midicev_noteon = evit_init_noteon(&evit_noteon, tickev_list, channel);
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

bool_t mcev_is_in_list(list_t *mcev_list, midicev_t *mcev)
{
  node_t    *node = mcev_list->head;

  while (node != NULL)
    {
      if (node->addr == mcev)
        return TRUE;
      node = node->next;
    }
  return FALSE;
}

static midicev_t *_evit_next_noteoff_num_excl(ev_iterator_t *evit_noteoff,
                                              byte_t channel,
                                              byte_t num,
                                              list_t *exclude_list)
{
  midicev_t *note_off = NULL;

  for (note_off = evit_next_noteoff_num(evit_noteoff,
                                        channel,
                                        num);
       note_off != NULL
         && mcev_is_in_list(exclude_list, note_off);
       note_off = evit_next_noteoff_num(evit_noteoff,
                                        channel,
                                        num));
  return note_off;
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
  midicev_t     *midicev_noteon = NULL;
  midicev_t     *midicev_noteoff = NULL;
  list_t        added_noteoff;

  bzero(&added_noteoff, sizeof (list_t));

  if (trackctx)
    {
      midicev_noteon = evit_init_noteon(&evit_noteon,
                                        &(trackctx->track->tickev_list),
                                        channel);

      while (midicev_noteon)
        {
          if (evit_noteon.tick < tick_max &&
              note_min <= midicev_noteon->event.note.num &&
              midicev_noteon->event.note.num <= note_max)
            {
              evit_copy(&evit_noteon, &evit_noteoff);
              midicev_noteoff = _evit_next_noteoff_num_excl(&evit_noteoff,
                                                            midicev_noteon->chan,
                                                            midicev_noteon->event.note.num,
                                                            &added_noteoff);
              if (midicev_noteoff != NULL && tick_min < evit_noteoff.tick)
                {
                  ev_repr = build_noteonoff(&evit_noteon, &evit_noteoff, trackctx);
                  PyList_Append(ret_obj, ev_repr);
                  push_to_list_tail(&added_noteoff, midicev_noteoff);
                }
              /* else */
              /*   msg; */
            }
          midicev_noteon = evit_next_noteon(&evit_noteon, channel);
        }
    }
  free_list_node(&added_noteoff, NULL);
  return ret_obj;
}


PyObject *sel_noteonoff_repr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             byte_t note_min,
                             byte_t note_max)
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
                             byte_t note_min,
                             byte_t note_max)
{
  return _sel_noteonoff(trackctx,
                        channel,
                        tick_min,
                        tick_max,
                        note_min,
                        note_max,
                        _build_noteonoff_evwr);
}


PyObject *sel_ctrl_evwr(track_ctx_t *trackctx,
                        byte_t channel,
                        uint_t tick_min,
                        uint_t tick_max,
                        byte_t ctrl_num)
{
  ev_iterator_t evit;
  PyObject      *ret_obj = PyList_New(0);
  PyObject      *evwr = NULL;
  midicev_t     *midicev = NULL;

  if (trackctx)
    {
      for (midicev = evit_init_ctrl_num(&evit,
                                             &(trackctx->track->tickev_list),
                                             channel,
                                             ctrl_num);
           midicev && evit.tick <= tick_max;
           midicev = evit_next_ctrl_num(&evit, channel, ctrl_num))
        if (tick_min <= evit.tick)
          {
            evwr = build_evwr_from_evit(&evit, trackctx);
            PyList_Append(ret_obj, evwr);
          }
    }

  return ret_obj;
}

PyObject *sel_pitch_evwr(track_ctx_t *trackctx,
                         byte_t channel,
                         uint_t tick_min,
                         uint_t tick_max)
{
  ev_iterator_t evit;
  PyObject      *ret_obj = PyList_New(0);
  PyObject      *evwr    = NULL;
  midicev_t     *midicev = NULL;

  if (trackctx)
    {
      for (midicev = evit_init_pitch(&evit,
                                     &(trackctx->track->tickev_list),
                                     channel);
           midicev && evit.tick <= tick_max;
           midicev = evit_next_pitch(&evit, channel))
        if (tick_min <= evit.tick)
          {
            evwr = build_evwr_from_evit(&evit, trackctx);
            PyList_Append(ret_obj, evwr);
          }
    }

  return ret_obj;
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

  if (trackctx->engine && engine_is_running(trackctx->engine) == TRUE)
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


bool_t gen_mcev_from_evrepr(PyObject *evrepr, uint_t *tick, midicev_t *mcev)
{
  Py_ssize_t    list_len = PyTuple_Size(evrepr);
  unsigned long chan, type, val1, val2;

  if (list_len < 5)
    return FALSE;
  /* todo: more test */
  *tick = (uint_t) PyLong_AsUnsignedLong(PyTuple_GetItem(evrepr, 0));
  chan  = PyLong_AsUnsignedLong(PyTuple_GetItem(evrepr, 1));
  type  = PyLong_AsUnsignedLong(PyTuple_GetItem(evrepr, 2));
  val1  = PyLong_AsUnsignedLong(PyTuple_GetItem(evrepr, 3));
  val2  = PyLong_AsUnsignedLong(PyTuple_GetItem(evrepr, 4));
  mcev->chan = (byte_t) chan;
  mcev->type = (byte_t) type;
  switch (type)
    {
    case NOTEOFF:
    case NOTEON:
      mcev->event.note.num = (byte_t) val1;
      mcev->event.note.val = (byte_t) val2;
      break;
    case CONTROLCHANGE:
      mcev->event.ctrl.num = (byte_t) val1;
      mcev->event.ctrl.val = (byte_t) val2;
      break;
    case PITCHWHEELCHANGE:
      mcev->event.pitchbend.Lval = (byte_t) val1;
      mcev->event.pitchbend.Hval = (byte_t) val2;
      break;
    default:
      ;
    }
  return TRUE;
}

PyObject *try_gen_evwr_list(track_ctx_t *trackctx, PyObject *pylist)
{
  Py_ssize_t    list_len;
  Py_ssize_t    idx;
  PyObject      *evrepr = NULL, *evwr = NULL, *ret = PyList_New(0);
  midicev_t     mcev;
  uint_t        tick = 0;
  ev_iterator_t evit;

  if (evit_init(&evit, &(trackctx->track->tickev_list)))
    for (list_len = PyList_GET_SIZE(pylist),
           idx = 0;
         idx < list_len;
         idx++)
      {
        evrepr = (PyObject *) PyList_GetItem(pylist, idx);
        if (gen_mcev_from_evrepr(evrepr, &tick, &mcev) == TRUE)
          {
            if (evit_searchev(&evit, tick, &mcev) != NULL)
              {
                evwr = build_evwr_from_evit(&evit, trackctx);
                PyList_Append(ret, evwr);
              }
            else
              output_error("Can not find tick %d %s Missing an event",
                           tick,
                           midicmd_to_str(mcev.type));
          }
        else
          output_error("Can not generate event repr. Missing an event");
      }
  else
    output_error("Track has no seqev");
  return ret;
}

PyObject *add_pyevrepr(track_ctx_t *trackctx, PyObject *pyevrepr)
{
  uint_t        type, tick;
  Py_ssize_t    repr_sz;
  midicev_t     mcev;
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
      obj = PyTuple_GetItem(pyevrepr, 1);
      mcev.chan = PyInt_AS_LONG(obj);
      mcev.type = type;
      obj = PyTuple_GetItem(pyevrepr, 3);
      mcev.event.note.num = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 4);
      mcev.event.note.val = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 0);
      tick = PyInt_AS_LONG(obj);
      evit_add_midicev(&evit, tick, &mcev);
      obj = build_evwr_from_evit(&evit, trackctx);
      break;
    case CONTROLCHANGE:
      obj = PyTuple_GetItem(pyevrepr, 1);
      mcev.chan = PyInt_AS_LONG(obj);
      mcev.type = type;
      obj = PyTuple_GetItem(pyevrepr, 3);
      mcev.event.ctrl.num = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 4);
      mcev.event.ctrl.val = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 0);
      tick = PyInt_AS_LONG(obj);
      evit_add_midicev(&evit, tick, &mcev);
      obj = build_evwr_from_evit(&evit, trackctx);
      break;
    case PITCHWHEELCHANGE:
      obj = PyTuple_GetItem(pyevrepr, 1);
      mcev.chan = PyInt_AS_LONG(obj);
      mcev.type = type;
      obj = PyTuple_GetItem(pyevrepr, 3);
      mcev.event.pitchbend.Lval = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 4);
      mcev.event.pitchbend.Hval = PyInt_AS_LONG(obj);
      obj = PyTuple_GetItem(pyevrepr, 0);
      tick = PyInt_AS_LONG(obj);
      evit_add_midicev(&evit, tick, &mcev);
      obj = build_evwr_from_evit(&evit, trackctx);
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
  list_len = PyList_GET_SIZE(list);
  if (list_len > 0)
    trackctx->has_changed = TRUE;
  return list;
}
