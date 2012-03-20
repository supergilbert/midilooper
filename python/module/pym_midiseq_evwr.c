#include <Python.h>
#include "seqtool/seqtool.h"
#include "midi/midiev_inc.h"

typedef struct {
  PyObject_HEAD
  list_iterator_t tickit;
  list_iterator_t evit;
} midiseq_evwrObject;

static void midiseq_evwr_dealloc(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *build_event_repr(seqev_t *ev)
{
  midicev_t *midicev = NULL;

  if (ev->type == MIDICEV)
    {
      midicev = (midicev_t *) ev->addr;
      if (midicev->type == NOTEOFF || midicev->type == NOTEON)
        return Py_BuildValue("(iiii)",
                             midicev->type,
                             midicev->chan,
                             midicev->event.note.num,
                             midicev->event.note.val);
    }
  return Py_BuildValue("s", "Unsupported");
}

static PyObject *midiseq_evwr_getevent(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  seqev_t *ev = NULL;

  if (iter_node(&(self->evit)) != NULL)
    {
      ev = iter_node_ptr(&(self->evit));
      return build_event_repr(ev);
    }
  Py_RETURN_NONE;
}

static void goto_next_tick(midiseq_evwrObject *self)
{
  tickev_t *tickev = NULL;

  if (iter_node(&(self->tickit)) != NULL)
    {
      iter_next(&(self->tickit));
      tickev = (tickev_t *) iter_node(&(self->tickit));
      if (tickev != NULL)
	iter_init(&(self->evit), &(tickev->seqev_list));
    }
}

static PyObject *midiseq_evwr_next(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

  printf("in evwr next self:%p\n", self);
  if (iter_node(&(self->evit)) != NULL)
    {
      iter_next(&(self->evit));
      if (iter_node(&(self->evit)) != NULL)
        return obj;
      else
	{
	  goto_next_tick(self);
	  if (iter_node(&(self->evit)) != NULL)
	    return obj;
	}
    }
  PyErr_SetNone(PyExc_StopIteration);
  return NULL;
}

static PyObject *midiseq_evwr_iter(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

   if (iter_node(&(self->evit)) != NULL)
     return obj;
   /* ??? */
   return NULL;
}

static PyMethodDef midiseq_evwr_methods[] = {
  {"get_event", midiseq_evwr_getevent, METH_NOARGS,
   "Get the event representation as a tuple of integer"},
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
    midiseq_evwr_iter,          /* tp_iter */
    midiseq_evwr_next,          /* tp_iternext */
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

PyObject *create_midiseq_evwr(track_t *track)
{
  tickev_t *tickev = NULL;
  midiseq_evwrObject *evwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                                                 &midiseq_evwrType);

  iter_init(&(evwr->tickit), &(track->tickev_list));
  tickev = (tickev_t *) iter_node_ptr(&(evwr->tickit));
  iter_init(&(evwr->evit), &(tickev->seqev_list));
  return (PyObject *) evwr;
}

PyTypeObject *init_midiseq_evwrType(void)
{
  if (PyType_Ready(&midiseq_evwrType) < 0)
    return NULL;
  Py_INCREF(&midiseq_evwrType);
  return &midiseq_evwrType;
}
