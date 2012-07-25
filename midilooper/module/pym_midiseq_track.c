#include "./pym_midiseq_track.h"
#include "./pym_midiseq_aport.h"

#include "debug_tool/debug_tool.h"

/* static int midiseq_track_init(midiseq_trackObject *self, */
/*                                PyObject *args, */
/*                                PyObject *kwds) */
/* { */
/*   track_t       *track = NULL; */
/*   char          *name = NULL; */

/*   track = myalloc(sizeof (track_t)); */
/*   bzero(track, sizeof (track_t)); */

/*   if (args != NULL) */
/*     { */
/*       if (!PyArg_ParseTuple(args, "s", &name)) */
/*         return -1; */
/*       if (name == NULL) */
/*         name = "no track name"; */
/*     } */
/*   track->name = strdup(name); */
/*   self->track = track; */
/*   return 0; */
/* } */

static void midiseq_track_dealloc(PyObject *obj)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  /* if (self->track != NULL) { */
  /*   free_track(self->track); */
  /* } */
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *midiseq_track_get_name(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  char *track_name = "no track name";

  if (self->trackctx->track->name)
    track_name = self->trackctx->track->name;
  return Py_BuildValue("s", track_name);
}

static PyObject *midiseq_track_set_name(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  char *name = NULL;
  char *tmp;

  if (!PyArg_ParseTuple(args, "s", &name))
    {
      output_error("track_set_name: Problem with argument");
      return NULL;
    }
  tmp = self->trackctx->track->name;
  self->trackctx->track->name = strdup(name);
  if (tmp)
    free(tmp);
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_get_len(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  return Py_BuildValue("i", self->trackctx->len);
}

static PyObject *midiseq_track_set_len(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  unsigned int        len;

  if (!PyArg_ParseTuple(args, "i", &len))
    {
      output_error("track_add_note_event: Problem with argument");
      return NULL;
    }
  self->trackctx->len = len;
  Py_RETURN_NONE;
}

static PyObject *midiseq_toggle_mute(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  self->trackctx->mute = self->trackctx->mute ? FALSE : TRUE;
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_get_mute_state(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->mute == TRUE)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

#include "midi/midiev_inc.h"

static PyObject *midiseq_track_add_note_event(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  uint_t channel = 0, tick = 0, type = 0, num = 0, val = 0;
  midicev_t *mcev;

  if (!PyArg_ParseTuple(args, "iiiii", &tick, &type, &channel, &num, &val))
    {
      output_error("track_add_note_event: Problem with argument");
      return NULL;
    }

  if (type != NOTEOFF && type != NOTEON)
    return NULL;

  mcev = myalloc(sizeof (midicev_t));
  mcev->chan = channel;
  mcev->type = type;
  mcev->event.note.num = num;
  mcev->event.note.val = val;
  add_new_seqev(self->trackctx->track, tick, mcev, MIDICEV);
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_clear(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  clear_tickev_list(&(self->trackctx->track->tickev_list));
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_set_port(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  midiseq_aportObject *pyaport = NULL;

  if (!PyArg_ParseTuple(args , "O", &pyaport))
    return NULL;
  if (Py_None == (PyObject *) pyaport)
    self->trackctx->aseqport_ctx = NULL;
  else
    self->trackctx->aseqport_ctx = pyaport->aport;
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_has_port(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  midiseq_aportObject *pyaport = NULL;

  if (!PyArg_ParseTuple(args , "O", &pyaport))
    return NULL;
  if (self->trackctx->aseqport_ctx == pyaport->aport)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject *midiseq_track_lock(PyObject *obj,
                                    PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  pthread_rwlock_rdlock(&(self->trackctx->lock));
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_unlock(PyObject *obj,
                                      PyObject *args)
{
  midiseq_trackObject     *self = (midiseq_trackObject *) obj;

  pthread_rwlock_unlock(&(self->trackctx->lock));
  Py_RETURN_NONE;
}

#include "pym_midiseq_evwr.h"

static PyObject *midiseq_track_event2trash(PyObject *obj,
                                           PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  midiseq_evwrObject  *evwr = NULL;

  if (!PyArg_ParseTuple(args , "O", &evwr))
    return NULL;
  trackctx_event2trash(self->trackctx,
                       &(evwr->tickit),
                       &(evwr->evit));
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_ishandled(PyObject *obj,
                                         PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->is_handled == TRUE)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject *midiseq_track_repr(PyObject *obj)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  return Py_BuildValue("s", self->trackctx->track->name);
}

static PyMethodDef midiseq_track_methods[] = {
  {"get_name", midiseq_track_get_name, METH_NOARGS, "Get track name"},
  {"set_name", midiseq_track_set_name, METH_VARARGS, "Set track name"},
  {"get_len", midiseq_track_get_len, METH_NOARGS, "Get track len"},
  {"get_mute_state", midiseq_track_get_mute_state, METH_NOARGS, "Get mute state"},
  {"set_len", midiseq_track_set_len, METH_VARARGS, "Set track len"},
  {"set_port", midiseq_track_set_port, METH_VARARGS, "Set track port"},
  {"toggle_mute", midiseq_toggle_mute, METH_NOARGS, "Toggle mute state"},
  {"has_port", midiseq_track_has_port, METH_VARARGS, "Check if track has port"},
  {"add_note_event", midiseq_track_add_note_event, METH_VARARGS, "Add a note event"},
  {"lock", midiseq_track_lock, METH_NOARGS, "Lock track"},
  {"unlock", midiseq_track_unlock, METH_NOARGS, "Unlock track"},
  {"event2trash", midiseq_track_event2trash, METH_VARARGS, "Put event to the trash"},
  {"is_handled", midiseq_track_ishandled, METH_NOARGS, "Get if the track is handled"},
  {"clear", midiseq_track_clear, METH_NOARGS, "free all track list to use with caution (/!\\ not while running for now)"},
  {NULL, NULL, 0, NULL}
};

#include "./pym_midiseq_evwr.h"

static PyObject *midiseq_track_get_iter(PyObject *obj)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  return create_midiseq_evwr(self->trackctx->track);
}

static PyTypeObject midiseq_trackType = {
    PyObject_HEAD_INIT(NULL)
    0,                             /* ob_size */
    "midiseq.track",               /* tp_name */
    sizeof(midiseq_trackObject),   /* tp_basicsize */
    0,                             /* tp_itemsize */
    midiseq_track_dealloc,         /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    midiseq_track_repr,            /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    "track objects",               /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    midiseq_track_get_iter,        /* tp_iter */
    0,                             /* tp_iternext */
    midiseq_track_methods,         /* tp_methods */
    0,                             /* tp_members */
    0,                             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    0,                             /* tp_init */
    0,                             /* tp_alloc */
    PyType_GenericNew,             /* tp_new */
};

PyObject *create_midiseq_track(track_ctx_t *trackctx)
{
  midiseq_trackObject *pytrack = PyObject_New(midiseq_trackObject,
                                              &midiseq_trackType);

  pytrack->trackctx = trackctx;
  return (PyObject *) pytrack;
}

PyTypeObject *init_midiseq_trackType(void)
{
  if (PyType_Ready(&midiseq_trackType) < 0)
    return NULL;
  Py_INCREF(&midiseq_trackType);
  return &midiseq_trackType;
}
