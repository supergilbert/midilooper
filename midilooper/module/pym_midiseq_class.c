#include <Python.h>
#include "debug_tool/debug_tool.h"
#include "./engine.h"
#include "./pym_midiseq_track.h"


typedef struct {
  PyObject_HEAD
  midiseq_trackObject *pytrack;
  engine_ctx_t *engine_ctx;
} midiseq_Object;


static void midiseq_dealloc(PyObject *obj)
{
  trace_func;
  midiseq_Object *self = (midiseq_Object *) obj;

  if (self->engine_ctx != NULL)
    free_engine_ctx(self->engine_ctx);
  if (self->pytrack != NULL)
    Py_DECREF(self->pytrack);
  self->ob_type->tp_free((PyObject*)self);
  trace_func;
}


static int midiseq_init(midiseq_Object *self,
                              PyObject *args,
                              PyObject *kwds)
{
  trace_func;
  char *aport_name = "midiseq_output";
  char *tmp = NULL;

  if (args != NULL)
    {
      if (!PyArg_ParseTuple(args, "s", &tmp))
        return -1;
      if (tmp != NULL)
        aport_name = tmp;
    }
  self->engine_ctx = init_engine_ctx(aport_name);
  return 0;
}


static PyObject *midiseq_settickpos(PyObject *obj,
                                    PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  uint_t tickpos = 0;

  if (args == NULL)
    return NULL;
  if (!PyArg_ParseTuple(args , "i", &tickpos))
    return NULL;
  self->engine_ctx->looph.clocktick.number = tickpos;

  Py_RETURN_NONE;
}


static PyObject *midiseq_gettickpos(PyObject *obj,
                                    PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  return Py_BuildValue("i", self->engine_ctx->looph.clocktick.number);
}


static PyObject *midiseq_getppq(PyObject *obj,
                                PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  return Py_BuildValue("i", self->engine_ctx->ppq);
}

static PyObject *midiseq_setppq(PyObject *obj,
                                PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  uint_t ppq = 0;

  if (args == NULL)
    return NULL;
  if (!PyArg_ParseTuple(args , "i", &ppq))
    return NULL;
  if (ppq < 48 || ppq > 960)
    return NULL;
  debug("set ppq=%d\n", ppq);
  self->engine_ctx->ppq = ppq;

  Py_RETURN_NONE;
}


static PyObject *midiseq_setbpm(PyObject *obj,
                                PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  uint_t bpm;

  if (args == NULL)
    return NULL;
  if (!PyArg_ParseTuple(args , "i", &bpm))
    return NULL;
  if (bpm < 40 || bpm > 300)
    return NULL;
  debug("midiseq_setbpm: %d bpm\n", bpm);
  set_bpmnppq_to_timespec(&(self->engine_ctx->looph.res),
                          self->engine_ctx->ppq,
                          bpm);
  Py_RETURN_NONE;
}


static PyObject *midiseq_setms(PyObject *obj,
                                PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  uint_t ms;

  if (args == NULL)
    return NULL;
  if (!PyArg_ParseTuple(args , "i", &ms))
    return NULL;
  if (ms < 200000 || ms > 1500000)
    return NULL;
  debug("midiseq_setms: %dms\n", ms);
  set_msnppq_to_timespec(&(self->engine_ctx->looph.res),
                         self->engine_ctx->ppq,
                         ms);
  Py_RETURN_NONE;
}


static PyObject *midiseq_start(PyObject *obj,
                               PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  Py_BEGIN_ALLOW_THREADS;
  start_engine(self->engine_ctx);
  Py_END_ALLOW_THREADS;
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *midiseq_stop(PyObject *obj,
                               PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  stop_engine(self->engine_ctx);
  Py_RETURN_NONE;
}


static PyObject *midiseq_wait(PyObject *obj,
                               PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  wait_engine(self->engine_ctx);
  Py_RETURN_NONE;
}

static PyObject *midiseq_isrunning(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  if (self->engine_ctx->info.isrunning)
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

  if (engine_del_track(self->engine_ctx, pytrack->trackctx) == FALSE)
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
  trackctx = engine_create_track(self->engine_ctx, name);
  return create_midiseq_track(trackctx);
}

static PyObject *midiseq_gettracks(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object  *self = (midiseq_Object *) obj;
  list_iterator_t iter;
  track_ctx_t    *trackctx = NULL;
  PyObject        *tracklist = PyList_New(0);


  for (iter_init(&iter, &(self->engine_ctx->track_list));
       iter_node(&iter);
       iter_next(&iter))
    {
      trackctx = iter_node_ptr(&iter);
      PyList_Append(tracklist, create_midiseq_track(trackctx));
    }
  return tracklist;
}

#include "./pym_midiseq_aport.h"

static PyObject *midiseq_newoutput(PyObject *obj,
                                   PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  char           *name = NULL;
  aseqport_ctx_t *aport = NULL;

  if (!PyArg_ParseTuple(args , "s", &name))
    return NULL;
  aport = engine_create_aport(self->engine_ctx, name);
  return create_midiseq_aport(aport);
}

static PyObject *midiseq_delport(PyObject *obj,
                                 PyObject *args)
{
  midiseq_Object      *self = (midiseq_Object *) obj;
  midiseq_aportObject *port = NULL;
  /* aseqport_ctx_t      *aport = NULL; */

  if (!PyArg_ParseTuple(args , "O", &port))
    return NULL;
  if (engine_del_port(self->engine_ctx, port->aport) == FALSE)
    return NULL;
  Py_RETURN_NONE;
}

static PyObject *midiseq_getports(PyObject *obj,
                                 PyObject *args)
{
  midiseq_Object  *self = (midiseq_Object *) obj;
  list_iterator_t iter;
  aseqport_ctx_t  *aport = NULL;
  PyObject        *portlist = PyList_New(0);


  for (iter_init(&iter, &(self->engine_ctx->aseqport_list));
       iter_node(&iter);
       iter_next(&iter))
    {
      aport = iter_node_ptr(&iter);
      PyList_Append(portlist, create_midiseq_aport(aport));
    }
  return portlist;
}

static PyMethodDef midiseq_methods[] = {
  {"setppq", midiseq_setppq, METH_VARARGS,
   "Set sequencer pulsation per quater note"},
  {"getppq", midiseq_getppq, METH_NOARGS,
   "Get sequencer pulsation per quarter note"},
  {"setbpm", midiseq_setbpm, METH_VARARGS,
   "Set sequencer tempo in beat per minute"},
  {"settickpos", midiseq_settickpos, METH_VARARGS,
   "Set sequencer tick position"},
  {"gettickpos", midiseq_gettickpos, METH_NOARGS,
   "Get sequencer tick position"},
  {"setms", midiseq_setms, METH_VARARGS,
   "Set sequencer tempo in micro seconde"},
  /* {"settrack", midiseq_settrack, METH_VARARGS, */
  /*  "Set sequencer track"}, */
  {"start", midiseq_start, METH_NOARGS,
   "Start the engine"},
  {"stop", midiseq_stop, METH_NOARGS,
   "Stop the engine"},
  {"wait", midiseq_wait, METH_NOARGS,
   "Wait for engine thread end"},
  {"isrunning", midiseq_isrunning, METH_NOARGS,
   "Get if engine is running"},
  {"newoutput", midiseq_newoutput, METH_VARARGS,
   "Create new sequencer output port"},
  {"delport", midiseq_delport, METH_VARARGS,
   "Del sequencer output port"},
  {"getports", midiseq_getports, METH_NOARGS,
   "Get output ports list"},
  {"newtrack", midiseq_newtrack, METH_VARARGS,
   "Create new track"},
  {"deltrack", midiseq_deltrack, METH_VARARGS,
   "Delete a track"},
  {"gettracks", midiseq_gettracks, METH_NOARGS,
   "Create new sequencer track"},
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
