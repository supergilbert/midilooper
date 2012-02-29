#include <Python.h>
#include "seqtool/seqtool.h"
#include "midi/midiev_inc.h"

typedef struct {
  PyObject_HEAD
  list_iterator_t *tickit;
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

static PyObject *midiseq_evwr_gotohead(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  seqev_t *ev = NULL;

  iter_init(&(self->evit), self->evit.list);
  if (iter_node(&(self->evit)) != NULL)
    {
      ev = iter_node_ptr(&(self->evit));
      return build_event_repr(ev);
    }
  Py_RETURN_NONE;
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

static PyObject *midiseq_evwr_next(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  seqev_t *ev = NULL;


  printf("in evwr next self:%p\n", self);
  if (iter_node(&(self->evit)) != NULL)
    {
      iter_next(&(self->evit));
      if (iter_node(&(self->evit)) != NULL)
        {
          ev = iter_node_ptr(&(self->evit));
          /* if (ev) */
          return build_event_repr(ev);
        }
    }
  Py_RETURN_NONE;
}

#include "seqtool/seqtool.h"
static PyObject *midiseq_evwr_delevent(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  tickev_t           *tickev = NULL;

  iter_node_del(&(self->evit), free_seqev);
  if (self->evit.list->len <= 0)
    {
      iter_node_del(self->tickit, free_tickev);
      if (self->tickit->list->len > 0)
        {
          tickev = iter_node_ptr(self->tickit);
          iter_init(&(self->evit), &(tickev->seqev_list));
        }
    }
  Py_RETURN_NONE;
}

static PyMethodDef midiseq_evwr_methods[] = {
  {"goto_head", midiseq_evwr_gotohead, METH_NOARGS, "Go to first tick event"},
  {"get_event", midiseq_evwr_getevent, METH_NOARGS,
   "Get the event representation as a tuple of integer"},
  {"del_event", midiseq_evwr_delevent, METH_NOARGS, "delete the current event"},
  {"next", midiseq_evwr_next, METH_NOARGS, "Go to next tick"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_evwrType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "midiseq.evwr",          /* tp_name */
    sizeof(midiseq_evwrObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    midiseq_evwr_dealloc,    /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_compare */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Tick event wrapper",        /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    midiseq_evwr_methods,    /* tp_methods */
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    0,                           /* tp_new */
};

PyObject *create_midiseq_evwr(list_iterator_t *tickit)
{
  tickev_t *tickev = (tickev_t *) iter_node_ptr(tickit);
  midiseq_evwrObject *evwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                                                 &midiseq_evwrType);

  evwr->tickit = tickit;
  printf("Initialising evwr iterator with addr:%p\n", evwr);
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
