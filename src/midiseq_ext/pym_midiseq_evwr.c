/* Copyright 2012-2016 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include <Python.h>
#include "seqtool/seqtool.h"
#include "midi/midiev_inc.h"
#include "debug_tool/debug_tool.h"

#include "./pym_midiseq_evwr.h"
#include "./pym_midiseq_tools.h"

static void midiseq_evwr_dealloc(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

  Py_TYPE(self)->tp_free((PyObject*)self);
}

msq_bool_t evwr_check(midiseq_evwrObject *evwr)
{
  pthread_rwlock_rdlock(&(evwr->trackctx->lock));
  if (evit_check(&(evwr->evit), &(evwr->trackctx->track->tickev_list)))
    {
      pthread_rwlock_unlock(&(evwr->trackctx->lock));
      return MSQ_TRUE;
    }
  else
    {
      pthread_rwlock_unlock(&(evwr->trackctx->lock));
      return MSQ_FALSE;
    }
}

static PyObject *midiseq_evwr_getevent(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  seqev_t *seqev = NULL;
  PyObject *pylist_ev = NULL;

#ifndef __ROUGH
  if (!evwr_check(self))
    return NULL;
#endif

  seqev = evit_get_seqev(&(self->evit));
  if (seqev && seqev->type == MIDICEV)
    {
      pylist_ev = build_evrepr(self->evit.tick, (midicev_t *) seqev->addr);
      if (pylist_ev)
        return pylist_ev;
    }
  Py_RETURN_NONE;
}

static PyObject *midiseq_evwr_set_note_vel(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  seqev_t            *seqev = NULL;
  midicev_t          *midicev = NULL;
  uint_t             value;

#ifndef __ROUGH
  if (!evwr_check(self))
    return NULL;
#endif

  if (!PyArg_ParseTuple(args, "i", &value))
    {
      output_error("In %s (%s:%d) Problem with argument",
                   __FUNCTION__, __FILE__, __LINE__);
      return NULL;
    }

  seqev = evit_get_seqev(&(self->evit));
  midicev = (midicev_t *) seqev->addr;
  if (midicev->type == NOTEON || midicev->type == NOTEOFF)
    midicev->event.note.val = value;
  Py_RETURN_NONE;
}

PyObject *midiseq_evwr_copy(PyObject *obj, PyObject *args);

static PyMethodDef midiseq_evwr_methods[] = {
  {"get_event", midiseq_evwr_getevent, METH_NOARGS,
   "Get the event representation as a tuple of integer (tick, channel, note_type, note, val)"},
  {"set_note_vel", midiseq_evwr_set_note_vel, METH_VARARGS,
   "Change the note velocity"},
  /* {"_del_event", midiseq_evwr_del_event, METH_NOARGS, */
  /*  "Delete the current event of the track (/!\\ Never use this function when engine is running it is really not thread safe and will surely make memory corruption)"}, */
  {"_copy", midiseq_evwr_copy, METH_NOARGS,
   "Create a clone of the event evwr (/!\\ Caution with memory corruption)"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_evwrType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "midiseq.evwr",             /* tp_name */
    sizeof(midiseq_evwrObject), /* tp_basicsize */
    0,                          /* tp_itemsize */
    midiseq_evwr_dealloc,       /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Tick event wrapper",       /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    midiseq_evwr_methods,       /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
};

PyObject *midiseq_evwr_copy(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *evwr = (midiseq_evwrObject *) obj;
  midiseq_evwrObject *newevwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                                                    &midiseq_evwrType);

  memcpy(&(newevwr->evit), &(evwr->evit), sizeof(list_iterator_t));
  return (PyObject *) newevwr;
}

PyObject *build_evwr(track_ctx_t *trackctx)
{
  midiseq_evwrObject *evwr = NULL;

  evwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                             &midiseq_evwrType);
  evit_init(&(evwr->evit), &(trackctx->track->tickev_list));
  evwr->trackctx = trackctx;
  return (PyObject *) evwr;
}

PyObject *build_evwr_from_evit(ev_iterator_t *evit, track_ctx_t *trackctx)
{
  midiseq_evwrObject *evwr = NULL;

  evwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                             &midiseq_evwrType);
  evit_copy(evit, &(evwr->evit));
  evwr->trackctx = trackctx;
  return (PyObject *) evwr;
}

PyTypeObject *init_midiseq_evwrType(void)
{
  if (PyType_Ready(&midiseq_evwrType) < 0)
    return NULL;
  Py_INCREF(&midiseq_evwrType);
  return &midiseq_evwrType;
}
