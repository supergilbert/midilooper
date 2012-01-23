#include <Python.h>
#include "seqtool/seqtool.h"

typedef struct {
  PyObject_HEAD
  list_iterator_t tickit;
} midiseq_tickevwrObject;

static void midiseq_tickevwr_dealloc(PyObject *obj)
{
  midiseq_tickevwrObject *self = (midiseq_tickevwrObject *) obj;

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *midiseq_tickevwr_gotohead(PyObject *obj, PyObject *args)
{
  midiseq_tickevwrObject *self = (midiseq_tickevwrObject *) obj;
  tickev_t *tickev = NULL;

  iter_init(&(self->tickit), self->tickit.list);
  tickev = iter_node_ptr(&(self->tickit));
  return Py_BuildValue("i", tickev->tick);
}

static PyObject *midiseq_tickevwr_gettick(PyObject *obj, PyObject *args)
{
  midiseq_tickevwrObject *self = (midiseq_tickevwrObject *) obj;
  tickev_t *tickev = iter_node_ptr(&(self->tickit));

  return Py_BuildValue("i", tickev->tick);
}

static PyObject *midiseq_tickevwr_next(PyObject *obj, PyObject *args)
{
  midiseq_tickevwrObject *self = (midiseq_tickevwrObject *) obj;
  tickev_t *tickev = NULL;

  iter_next(&(self->tickit));
  tickev = (tickev_t *) iter_node_ptr(&(self->tickit));
  if (tickev)
    return Py_BuildValue("i", tickev->tick);
  else
    return Py_None;
}

#include "./pym_midiseq_evwr.h"

static PyObject *midiseq_tickevwr_get_evwr(PyObject *obj, PyObject *args)
{
  midiseq_tickevwrObject *self = (midiseq_tickevwrObject *) obj;
  tickev_t *tickev = (tickev_t *) iter_node_ptr(&(self->tickit));

  return create_midiseq_evwr(tickev);
}

static PyMethodDef midiseq_tickevwr_methods[] = {
  {"gotohead", midiseq_tickevwr_gotohead, METH_NOARGS, "Go to first tick event"},
  {"gettick", midiseq_tickevwr_gettick, METH_NOARGS, "Get tick number"},
  {"next", midiseq_tickevwr_next, METH_NOARGS, "Go to next tick"},
  {"get_evwr", midiseq_tickevwr_get_evwr, METH_NOARGS, "Get event wrapper"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_tickevwrType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "midiseq.tickevwr",          /* tp_name */
    sizeof(midiseq_tickevwrObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    midiseq_tickevwr_dealloc,    /* tp_dealloc */
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
    midiseq_tickevwr_methods,    /* tp_methods */
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

PyObject *create_midiseq_tickevwr(track_t *track)
{
  midiseq_tickevwrObject *tickevwr = (midiseq_tickevwrObject *) PyObject_New(midiseq_tickevwrObject,
                                                                             &midiseq_tickevwrType);

  printf("Creating object tickevwr\n");
  iter_init(&(tickevwr->tickit), &(track->tickev_list));
  return (PyObject *) tickevwr;
}

PyTypeObject *init_midiseq_tickevwrType(void)
{
  if (PyType_Ready(&midiseq_tickevwrType) < 0)
    return NULL;
  Py_INCREF(&midiseq_tickevwrType);
  return &midiseq_tickevwrType;
}
