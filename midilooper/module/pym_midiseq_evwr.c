#include <Python.h>
#include "seqtool/seqtool.h"
#include "midi/midiev_inc.h"
#include "debug_tool/debug_tool.h"

#include "./pym_midiseq_evwr.h"
#include "./pym_midiseq_tools.h"

PyObject *build_evrepr(uint_t tick, midicev_t *midicev)
{
  if (midicev->type == NOTEOFF || midicev->type == NOTEON)
    return Py_BuildValue("(iiiii)",
                         tick,
                         midicev->chan,
                         midicev->type,
                         midicev->event.note.num,
                         midicev->event.note.val);
  else
    {
      output_error("Unsupported midi channel event type: %i\n", midicev->type);
      return Py_BuildValue("(iii)",
                           tick,
                           midicev->chan,
                           midicev->type);
    }
}

static void midiseq_evwr_dealloc(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

  self->ob_type->tp_free((PyObject*)self);
}

bool_t evwr_check(midiseq_evwrObject *evwr)
{
  pthread_rwlock_rdlock(&(evwr->trackctx->lock));
  if (evit_check(&(evwr->evit), &(evwr->trackctx->track->tickev_list)))
    {
      pthread_rwlock_unlock(&(evwr->trackctx->lock));
      return TRUE;
    }
  else
    {
      pthread_rwlock_unlock(&(evwr->trackctx->lock));
      return FALSE;
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

/* static PyObject *midiseq_evwr_del_event(PyObject *obj, PyObject *args) */
/* { */
/*   midiseq_evwrObject *self = (midiseq_evwrObject *) obj; */

/*   if (!evit_check(&(self->evit))) */
/*     return NULL; */

/*   iter_node_del(&(self->evit.seqevit), free_seqev); */
/*   if (self->evit.seqevit.list->len <= 0) */
/*     { */
/*       iter_node_del(&(self->evit.tickit), free_tickev); */
/*       evit_tick_head(&(self->evit)); */
/*     } */
/*   Py_RETURN_NONE; */
/* } */

/* TODO */
/* static PyObject *midiseq_evwr_repr(midiseq_evwrObject *self) */
/* { */
/*   return PyString_FromFormat("midiseq evwr type"); */
/* } */

PyObject *midiseq_evwr_copy(PyObject *obj, PyObject *args);

static PyMethodDef midiseq_evwr_methods[] = {
  {"get_event", midiseq_evwr_getevent, METH_NOARGS,
   "Get the event representation as a tuple of integer (tick, channel, note_type, note, val)"},
  /* {"_del_event", midiseq_evwr_del_event, METH_NOARGS, */
  /*  "Delete the current event of the track (/!\\ Never use this function when engine is running it is really not thread safe and will surely make memory corruption)"}, */
  {"_copy", midiseq_evwr_copy, METH_NOARGS,
   "Create a clone of the event evwr (/!\\ Caution with memory corruption)"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_evwrType = {
    PyObject_HEAD_INIT(NULL)
    0,                          /* ob_size */
    "midiseq.evwr",             /* tp_name */
    sizeof(midiseq_evwrObject), /* tp_basicsize */
    0,                          /* tp_itemsize */
    midiseq_evwr_dealloc,       /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
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
