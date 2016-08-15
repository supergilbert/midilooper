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

#include "asound/aseq.h"
#include "asound/aseq_tool.h"
#include "debug_tool/debug_tool.h"
#include "./pym_midiseq_output.h"

static void midiseq_output_dealloc(PyObject *obj)
{
  midiseq_outputObject *self = (midiseq_outputObject *) obj;

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *midiseq_output_getname(PyObject *obj, PyObject *args)
{
  midiseq_outputObject *self = (midiseq_outputObject *) obj;

  return Py_BuildValue("s", output_get_name(self->output));
}

static PyObject *midiseq_output_setname(PyObject *obj, PyObject *args)
{
  midiseq_outputObject *self = (midiseq_outputObject *) obj;
  char *name = NULL;

  if (!PyArg_ParseTuple(args, "s", &name))
    {
      output_error("Problem with argument");
      return NULL;
    }
  output_set_name(self->output, name);
  Py_RETURN_NONE;
}

static PyObject *midiseq_output_repr(PyObject *obj)
{
  midiseq_outputObject *self = (midiseq_outputObject *) obj;

  return PyString_FromFormat("%s",
                             output_get_name(self->output));
  /* midiseq_outputObject *self = (midiseq_outputObject *) obj; */
  /* return PyString_FromFormat("%i:%i '%s'", */
  /*                            snd_seq_port_info_get_client(self->output->info), */
  /*                            snd_seq_port_info_get_port(self->output->info), */
  /*                            snd_seq_port_info_get_name(self->output->info)); */
}

static PyMethodDef midiseq_output_methods[] = {
  {"get_name", midiseq_output_getname, METH_NOARGS,
   "Return alsa seq port name"},
  {"set_name", midiseq_output_setname, METH_VARARGS,
   "Set alsa seq port name"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_outputType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "midiseq.output",             /* tp_name */
    sizeof(midiseq_outputObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    midiseq_output_dealloc,       /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_compare */
    midiseq_output_repr,          /* tp_repr */
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
    midiseq_output_methods,       /* tp_methods */
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

PyObject *create_midiseq_output(output_t *output)
{
  midiseq_outputObject *obj = NULL;

  obj = (midiseq_outputObject *) PyObject_New(midiseq_outputObject,
                                               &midiseq_outputType);
  obj->output = output;
  /* Py_INCREF(obj); */
  return (PyObject *) obj;
}

PyTypeObject *init_midiseq_outputType(void)
{
  if (PyType_Ready(&midiseq_outputType) < 0)
    return NULL;
  Py_INCREF(&midiseq_outputType);
  return &midiseq_outputType;
}
