#include <Python.h>

#include "asound/aseq.h"

#include "./pym_midiseq_aport.h"

static void midiseq_aport_dealloc(PyObject *obj)
{
  midiseq_aportObject *self = (midiseq_aportObject *) obj;

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *midiseq_aport_getname(PyObject *obj, PyObject *args)
{
  midiseq_aportObject *self = (midiseq_aportObject *) obj;

  return Py_BuildValue("s", aseqport_name(self->aport));
}

static PyObject *midiseq_aport_repr(PyObject *obj)
{
  midiseq_aportObject *self = (midiseq_aportObject *) obj;
  return PyString_FromFormat("%i:%i '%s'",
                             snd_seq_port_info_get_client(self->aport->info),
                             snd_seq_port_info_get_port(self->aport->info),
                             snd_seq_port_info_get_name(self->aport->info));
}

static PyMethodDef midiseq_aport_methods[] = {
  {"get_name", midiseq_aport_getname, METH_NOARGS,
   "Return alsa seq port name"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_aportType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "midiseq.aport",             /* tp_name */
    sizeof(midiseq_aportObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    midiseq_aport_dealloc,       /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_compare */
    midiseq_aport_repr,          /* tp_repr */
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
    midiseq_aport_methods,       /* tp_methods */
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

PyObject *create_midiseq_aport(aseqport_ctx_t *aport)
{
  midiseq_aportObject *obj = NULL;

  obj = (midiseq_aportObject *) PyObject_New(midiseq_aportObject,
                                               &midiseq_aportType);
  obj->aport = aport;
  /* Py_INCREF(obj); */
  return (PyObject *) obj;
}

PyTypeObject *init_midiseq_aportType(void)
{
  if (PyType_Ready(&midiseq_aportType) < 0)
    return NULL;
  Py_INCREF(&midiseq_aportType);
  return &midiseq_aportType;
}
