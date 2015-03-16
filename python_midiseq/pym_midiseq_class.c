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
#include "debug_tool/debug_tool.h"
#include "./loop_engine/engine.h"
#include "./pym_midiseq_track.h"
#include "./pym_midiseq_file.h"


typedef struct {
  PyObject_HEAD
  /* midiseq_trackObject *pytrack; */
  engine_ctx_t engine_ctx;
} midiseq_Object;


static void midiseq_dealloc(PyObject *obj)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  if (self->engine_ctx.hdl != NULL)
    engine_destroy(&(self->engine_ctx));
  /* if (self->pytrack != NULL) */
  /*   Py_DECREF(self->pytrack); */
  self->ob_type->tp_free((PyObject*)self);
}


static int midiseq_init(midiseq_Object *self,
                        PyObject *args,
                        PyObject *kwds)
{
  char *aport_name = "midiseq_output";
  char *tmp = NULL;

  if (args != NULL)
    {
      if (!PyArg_ParseTuple(args, "s", &tmp))
        return -1;
      if (tmp != NULL)
        aport_name = tmp;
    }
  self->engine_ctx.hdl = NULL;
  if (nns_init_engine(&(self->engine_ctx), aport_name))
    return 0;
  else
    return -1;
}


static PyObject *midiseq_save(PyObject *obj,
                              PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  char           *filename = NULL;

  if (args == NULL)
    return NULL;
  if (!PyArg_ParseTuple(args , "s", &filename))
    return NULL;
  engine_save_project(&(self->engine_ctx), filename);
  Py_RETURN_NONE;
}

static PyObject *midiseq_gettickpos(PyObject *obj,
                                    PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  return Py_BuildValue("i", engine_get_tick(&(self->engine_ctx)));
}


static PyObject *midiseq_getppq(PyObject *obj,
                                PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  return Py_BuildValue("i", self->engine_ctx.ppq);
}

static PyObject *midiseq_gettempo(PyObject *obj,
                                  PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  return Py_BuildValue("i", self->engine_ctx.tempo);
}


static PyObject *midiseq_settempo(PyObject *obj,
                                  PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  uint_t ms;

  if (args == NULL)
    return NULL;
  if (!PyArg_ParseTuple(args , "i", &ms))
    return NULL;
  if (ms < 288461 || ms > 1500000)
    return NULL;
  self->engine_ctx.tempo = ms;
  engine_reset_pulse(&(self->engine_ctx));
  Py_RETURN_NONE;
}


static PyObject *midiseq_start(PyObject *obj,
                               PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  Py_BEGIN_ALLOW_THREADS;
  engine_start(&(self->engine_ctx));
  Py_END_ALLOW_THREADS;
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *midiseq_stop(PyObject *obj,
                              PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  engine_stop(&(self->engine_ctx));
  Py_RETURN_NONE;
}


static PyObject *midiseq_isrunning(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  if (engine_is_running(&(self->engine_ctx)))
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

#include "./pym_midiseq_track.h"

static PyObject *midiseq_deltrack(PyObject *obj,
                                  PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  midiseq_trackObject *pytrack = NULL;

  if (!PyArg_ParseTuple(args , "O", &pytrack))
    return NULL;

  if (engine_delete_trackctx(&(self->engine_ctx), pytrack->trackctx) == FALSE)
    return NULL;
  Py_RETURN_NONE;
}

static PyObject *midiseq_newtrack(PyObject *obj,
                                  PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  char           *name = NULL;
  track_ctx_t    *trackctx = NULL;

  if (!PyArg_ParseTuple(args , "s", &name))
    return NULL;
  trackctx = engine_create_trackctx(&(self->engine_ctx), name);
  return create_pym_track(trackctx);
}

static PyObject *midiseq_copy_track(PyObject *obj,
                                    PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_trackObject *pytrack = NULL;
  track_ctx_t         *trackctx = NULL;

  if (!PyArg_ParseTuple(args , "O", &pytrack))
    return NULL;

  trackctx = engine_copy_trackctx(&(self->engine_ctx), pytrack->trackctx);
  return create_pym_track(trackctx);
}

static PyObject *midiseq_gettracks(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object  *self = (midiseq_Object *) obj;
  list_iterator_t iter;
  track_ctx_t    *trackctx = NULL;
  PyObject        *tracklist = PyList_New(0);


  for (iter_init(&iter, &(self->engine_ctx.track_list));
       iter_node(&iter);
       iter_next(&iter))
    {
      trackctx = iter_node_ptr(&iter);
      PyList_Append(tracklist, create_pym_track(trackctx));
    }
  return tracklist;
}

#include "./pym_midiseq_output.h"

static PyObject *midiseq_newoutput(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  char           *name = NULL;
  output_t       *output = NULL;

  if (!PyArg_ParseTuple(args , "s", &name))
    return NULL;
  output = engine_create_output(&(self->engine_ctx), name);
  return create_midiseq_output(output);
}

static PyObject *midiseq_deloutput(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_outputObject *port = NULL;

  if (!PyArg_ParseTuple(args , "O", &port))
    return NULL;
  if (engine_delete_output(&(self->engine_ctx), port->output) == FALSE)
    return NULL;
  Py_RETURN_NONE;
}

static PyObject *midiseq_getoutputs(PyObject *obj,
                                    PyObject *args)
{
  midiseq_Object  *self = (midiseq_Object *) obj;
  list_iterator_t iter;
  output_t        *output = NULL;
  PyObject        *outputlist = PyList_New(0);


  for (iter_init(&iter, &(self->engine_ctx.output_list));
       iter_node(&iter);
       iter_next(&iter))
    {
      output = iter_node_ptr(&iter);
      PyList_Append(outputlist, create_midiseq_output(output));
    }
  return outputlist;
}

static PyObject *midiseq_read_msqfile_tracks(PyObject *obj,
                                               PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_fileObject  *mfile = NULL;

  if (!PyArg_ParseTuple(args , "O", &mfile))
    return NULL;
  engine_read_midifile(&(self->engine_ctx), mfile->midifile);

  Py_RETURN_NONE;
}

/* Warning: Not thread safe */
static PyObject *midiseq_move_track_before(PyObject *obj,
                                           PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_trackObject *target = NULL;
  midiseq_trackObject *topaste = NULL;
  list_iterator_t     iterator;

  if (!PyArg_ParseTuple(args , "OO", &target, &topaste))
    return NULL;
  iter_init(&iterator, &(self->engine_ctx.track_list));
  if (!iter_move_to_addr(&iterator, topaste->trackctx))
    return NULL;
  iter_node_del(&iterator, NULL);
  if (!iter_move_to_addr(&iterator, target->trackctx))
    return NULL;
  iter_push_before(&iterator, topaste->trackctx);
  Py_RETURN_NONE;
}

/* Warning: Not thread safe */
static PyObject *midiseq_move_track_after(PyObject *obj,
                                          PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_trackObject *target = NULL;
  midiseq_trackObject *topaste = NULL;
  list_iterator_t     iterator;

  if (!PyArg_ParseTuple(args , "OO", &target, &topaste))
    return NULL;
  iter_init(&iterator, &(self->engine_ctx.track_list));
  if (!iter_move_to_addr(&iterator, topaste->trackctx))
    return NULL;
  iter_node_del(&iterator, NULL);
  if (!iter_move_to_addr(&iterator, target->trackctx))
    return NULL;
  iter_push_after(&iterator, topaste->trackctx);
  Py_RETURN_NONE;
}

/* Warning: Not thread safe */
static PyObject *midiseq_move_port_before(PyObject *obj,
                                          PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_outputObject *target = NULL;
  midiseq_outputObject *topaste = NULL;
  list_iterator_t     iterator;

  if (!PyArg_ParseTuple(args , "OO", &target, &topaste))
    return NULL;
  iter_init(&iterator, &(self->engine_ctx.output_list));
  if (!iter_move_to_addr(&iterator, topaste->output))
    return NULL;
  iter_node_del(&iterator, NULL);
  if (!iter_move_to_addr(&iterator, target->output))
    return NULL;
  iter_push_before(&iterator, topaste->output);
  Py_RETURN_NONE;
}

/* Warning: Not thread safe */
static PyObject *midiseq_move_port_after(PyObject *obj,
                                         PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_outputObject *target = NULL;
  midiseq_outputObject *topaste = NULL;
  list_iterator_t     iterator;

  if (!PyArg_ParseTuple(args , "OO", &target, &topaste))
    return NULL;
  iter_init(&iterator, &(self->engine_ctx.output_list));
  if (!iter_move_to_addr(&iterator, topaste->output))
    return NULL;
  iter_node_del(&iterator, NULL);
  if (!iter_move_to_addr(&iterator, target->output))
    return NULL;
  iter_push_after(&iterator, topaste->output);
  Py_RETURN_NONE;
}

static PyMethodDef midiseq_methods[] = {
  {"read_msqfile", midiseq_read_msqfile_tracks, METH_VARARGS,
   "Copy the track of a midifile"},
  {"save", midiseq_save, METH_VARARGS,
   "Save the sequence information as a midifile"},
  {"getppq", midiseq_getppq, METH_NOARGS,
   "Get sequencer pulsation per quarter note"},
  {"settempo", midiseq_settempo, METH_VARARGS,
   "Set sequencer tempo in micro-second"},
  {"gettempo", midiseq_gettempo, METH_NOARGS,
   "Get sequencer tempo in micro-second"},
  {"gettickpos", midiseq_gettickpos, METH_NOARGS,
   "Get sequencer tick position"},
  {"start", midiseq_start, METH_NOARGS,
   "Start the engine"},
  {"stop", midiseq_stop, METH_NOARGS,
   "Stop the engine"},
  {"isrunning", midiseq_isrunning, METH_NOARGS,
   "Get if engine is running"},
  {"newoutput", midiseq_newoutput, METH_VARARGS,
   "Create new sequencer output port"},
  {"deloutput", midiseq_deloutput, METH_VARARGS,
   "Del sequencer output port"},
  {"getoutputs", midiseq_getoutputs, METH_NOARGS,
   "Get output ports list"},
  {"newtrack", midiseq_newtrack, METH_VARARGS,
   "Create new track"},
  {"copy_track", midiseq_copy_track, METH_VARARGS,
   "Copy an existing track"},
  {"deltrack", midiseq_deltrack, METH_VARARGS,
   "Delete a track"},
  {"gettracks", midiseq_gettracks, METH_NOARGS,
   "Create new sequencer track"},
  {"move_track_before", midiseq_move_track_before, METH_VARARGS,
   "Warning: Not thread safe"},
  {"move_track_after", midiseq_move_track_after, METH_VARARGS,
   "Warning: Not thread safe"},
  {"move_port_before", midiseq_move_port_before, METH_VARARGS,
   "Warning: Not thread safe"},
  {"move_port_after", midiseq_move_port_after, METH_VARARGS,
   "Warning: Not thread safe"},
  //  {"getinfo", midiseq_getinfo, METH_NOARGS, "get track info"},
  /* {"getname", midiseq_readtrack, METH_NOARGS, "get track name"}, */
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                      /* ob_size */
    "midiseq.midiseq",      /* tp_name */
    sizeof(midiseq_Object), /* tp_basicsize */
    0,                      /* tp_itemsize */
    midiseq_dealloc,        /* tp_dealloc */
    0,                      /* tp_print */
    0,                      /* tp_getattr */
    0,                      /* tp_setattr */
    0,                      /* tp_compare */
    0,                      /* tp_repr */
    0,                      /* tp_as_number */
    0,                      /* tp_as_sequence */
    0,                      /* tp_as_mapping */
    0,                      /* tp_hash */
    0,                      /* tp_call */
    0,                      /* tp_str */
    0,                      /* tp_getattro */
    0,                      /* tp_setattro */
    0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,     /* tp_flags */
    "midi sequencer with alsa ctx objects", /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    midiseq_methods,        /* tp_methods */
    0,                      /* tp_members */
    0,                      /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    (initproc) midiseq_init,           /* tp_init */
    0,                      /* tp_alloc */
    PyType_GenericNew,      /* tp_new */
};

PyTypeObject *init_midiseq_Type(void)
{
  if (PyType_Ready(&midiseq_Type) < 0)
    return NULL;
  Py_INCREF(&midiseq_Type);
  return &midiseq_Type;
}
