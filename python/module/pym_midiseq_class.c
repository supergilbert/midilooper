#include <Python.h>
#include "debug_tool/debug_tool.h"
#include "beta_engine/engine.h"
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
  char *aport_name = "midiseq_output";
  char *tmp = NULL;

  if (args != NULL)
    {
      if (!PyArg_ParseTuple(args, "s", &tmp))
        return -1;
      if (tmp != NULL)
        aport_name = tmp;
    }
  self->engine_ctx = init_engine(aport_name);
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

  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *midiseq_gettickpos(PyObject *obj,
                                    PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  return Py_BuildValue("i", self->engine_ctx->looph.clocktick.number);
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

  Py_INCREF(Py_None);
  return Py_None;
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
                          bpm,
                          self->engine_ctx->ppq);

  Py_INCREF(Py_None);
  return Py_None;
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
                         ms,
                         self->engine_ctx->ppq);
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *midiseq_settrack(PyObject *obj,
                                  PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;
  midiseq_trackObject *pytrack = NULL;

  trace_func;
  if (args == NULL)
    return NULL;
  trace_func;
  if (!PyArg_ParseTuple(args , "O", &pytrack))
    return NULL;
  trace_func;
  Py_INCREF(pytrack);
  self->engine_ctx->track_ctx.track = pytrack->track;
  if (self->pytrack != NULL)
    Py_DECREF(self->pytrack);
  self->pytrack = pytrack;
  iter_init(&(self->engine_ctx->track_ctx.current_tickev),
            &(self->engine_ctx->track_ctx.track->tickev_list));
  trace_func;
  Py_INCREF(Py_None);
  return Py_None;
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
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *midiseq_wait(PyObject *obj,
                               PyObject *args)
{
  midiseq_Object *self = (midiseq_Object *) obj;

  wait_engine(self->engine_ctx);
  Py_INCREF(Py_None);
  return Py_None;
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

static PyMethodDef midiseq_methods[] = {
  {"setppq", midiseq_setppq, METH_VARARGS,
   "Set sequencer pulsation per quater note"},
  {"setbpm", midiseq_setbpm, METH_VARARGS,
   "Set sequencer tempo in beat per minute"},
  {"settickpos", midiseq_settickpos, METH_VARARGS,
   "Set sequencer tick position"},
  {"gettickpos", midiseq_gettickpos, METH_NOARGS,
   "Get sequencer tick position"},
  {"setms", midiseq_setms, METH_VARARGS,
   "Set sequencer tempo in micro seconde"},
  {"settrack", midiseq_settrack, METH_VARARGS,
   "Set sequencer track"},
  {"start", midiseq_start, METH_NOARGS,
   "Start the engine"},
  {"stop", midiseq_stop, METH_NOARGS,
   "Stop the engine"},
  {"wait", midiseq_wait, METH_NOARGS,
   "Wait for engine thread end"},
  {"isrunning", midiseq_isrunning, METH_NOARGS,
   "Get if engine is running"},
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
