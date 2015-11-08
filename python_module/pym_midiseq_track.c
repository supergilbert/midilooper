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


#include "./pym_midiseq_track.h"
#include "./pym_midiseq_output.h"
#include "./pym_midiseq_tools.h"

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

static PyObject *midiseq_track_play_note(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  uint_t channel = 0, type = 0, num = 0, val = 0;
  midicev_t mcev;

  if (!PyArg_ParseTuple(args, "iiii", &channel, &type, &num, &val))
    {
      output_error("Problem with argument");
      return NULL;
    }

  if (type != NOTEOFF && type != NOTEON)
    {
      output_error("unknown or unsupported type %s", type);
      return NULL;
    }

  if (self->trackctx->engine &&
      self->trackctx->output != NULL)
    {
      mcev.chan = channel;
      mcev.type = type;
      mcev.event.note.num = num;
      mcev.event.note.val = val;
      output_write(self->trackctx->output,
                   &mcev);
    }
  Py_RETURN_NONE;
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

  return Py_BuildValue("i", self->trackctx->loop_len);
}

static PyObject *midiseq_track_get_start(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  return Py_BuildValue("i", self->trackctx->loop_start);
}

static PyObject *midiseq_track_get_loop_pos(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  unsigned int        tick;

  if (!PyArg_ParseTuple(args, "i", &tick))
    {
      output_error("Problem with argument");
      return NULL;
    }
  return Py_BuildValue("i", trackctx_loop_pos(self->trackctx, tick));
}

static PyObject *midiseq_track_set_len(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  unsigned int        len;

  if (!PyArg_ParseTuple(args, "i", &len))
    {
      output_error("Problem with argument");
      return NULL;
    }
  self->trackctx->loop_len = len;
  self->trackctx->need_sync = TRUE;
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_set_start(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  unsigned int        start;

  if (!PyArg_ParseTuple(args, "i", &start))
    {
      output_error("Problem with argument");
      return NULL;
    }
  self->trackctx->loop_start = start;
  self->trackctx->need_sync = TRUE;
  Py_RETURN_NONE;
}

#include "asound/aseq_tool.h"

static PyObject *midiseq_track_mute(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->mute == FALSE)
    _trackctx_mute(self->trackctx);
  Py_RETURN_NONE;
}
static PyObject *midiseq_track_unmute(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  self->trackctx->mute = FALSE;
  Py_RETURN_NONE;
}
static PyObject *midiseq_track_toggle_mute(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  trackctx_toggle_mute(self->trackctx);
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_get_mute_state(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->mute == TRUE)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject *midiseq_track_add_evrepr_list(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  PyObject *pylist = NULL, *ret = NULL;

  pthread_rwlock_rdlock(&(self->trackctx->lock));
  if (!PyArg_ParseTuple(args, "O", &pylist))
    {
      output_error("Problem with argument");
      return NULL;
    }
  ret = add_evrepr_list(self->trackctx, pylist);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret;
}

static PyObject *midiseq_track_get_output(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->output == NULL)
    Py_RETURN_NONE;
  else
    return create_midiseq_output(self->trackctx->output);
}

static PyObject *midiseq_track_set_output(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  midiseq_outputObject *pyoutput = NULL;

  if (!PyArg_ParseTuple(args , "O", &pyoutput))
    return NULL;
  if (Py_None == (PyObject *) pyoutput)
    self->trackctx->output = NULL;
  else
    self->trackctx->output = pyoutput->output;
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_has_output(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  midiseq_outputObject *pyoutput = NULL;

  if (!PyArg_ParseTuple(args , "O", &pyoutput))
    return NULL;
  if (self->trackctx->output == pyoutput->output)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

/* static PyObject *midiseq_track_lock(PyObject *obj, */
/*                                     PyObject *args) */
/* { */
/*   midiseq_trackObject *self = (midiseq_trackObject *) obj; */

/*   output("!!! BAD BAD BAD !!!\n"); */
/*   pthread_rwlock_rdlock(&(self->trackctx->lock)); */
/*   Py_RETURN_NONE; */
/* } */

/* static PyObject *midiseq_track_unlock(PyObject *obj, */
/*                                       PyObject *args) */
/* { */
/*   midiseq_trackObject     *self = (midiseq_trackObject *) obj; */

/*   output("!!! BAD BAD BAD !!!\n"); */
/*   pthread_rwlock_unlock(&(self->trackctx->lock)); */
/*   Py_RETURN_NONE; */
/* } */

#include "pym_midiseq_evwr.h"

/* static PyObject *midiseq_track_event2trash(PyObject *obj, */
/*                                            PyObject *args) */
/* { */
/*   midiseq_trackObject *self = (midiseq_trackObject *) obj; */
/*   midiseq_evwrObject  *evwr = NULL; */

/*   if (!PyArg_ParseTuple(args , "O", &evwr)) */
/*     return NULL; */
/*   trackctx_event2trash(self->trackctx, */
/*                        &(evwr->evit.tickit), */
/*                        &(evwr->evit.seqevit)); */
/*   Py_RETURN_NONE; */
/* } */

static PyObject *midiseq_track_is_in_recmode(PyObject *obj,
                                             PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->engine->track_rec == self->trackctx)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject *midiseq_track_ishandled(PyObject *obj,
                                         PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->engine && engine_is_running(self->trackctx->engine) == TRUE)
    Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject *midiseq_track_has_changed(PyObject *obj,
                                         PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  if (self->trackctx->has_changed == TRUE)
    {
      self->trackctx->has_changed = FALSE;
      Py_RETURN_TRUE;
    }
  Py_RETURN_FALSE;
}


static PyObject *msq_track_delete_evwr_list(PyObject *obj,
                                            PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *pylist  = NULL;

  if (!PyArg_ParseTuple(args , "O", &pylist))
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  delete_evwr_list(self->trackctx, pylist);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  Py_RETURN_NONE;
}


static PyObject *msq_track_get_evwr_list(PyObject *obj,
                                         PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *pylist  = NULL, *evwr_list = NULL;

  if (!PyArg_ParseTuple(args , "O", &pylist))
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  evwr_list = try_gen_evwr_list(self->trackctx, pylist);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  if (evwr_list)
    return evwr_list;
  else
    Py_RETURN_NONE;
}


static PyObject *msq_track_getall_event(PyObject *obj,
                                        PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;

  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = getall_event_repr(&(self->trackctx->track->tickev_list));
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

static PyObject *msq_track_getall_noteonoff(PyObject *obj,
                                            PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  uint_t              channel;

  if (!PyArg_ParseTuple(args , "i", &channel))
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = getall_noteonoff_repr(&(self->trackctx->track->tickev_list),
                                  (byte_t) channel);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}
static PyObject *msq_track_getall_noteonoff_evwr(PyObject *obj,
                                                 PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  uint_t              channel;

  if (!PyArg_ParseTuple(args , "i", &channel))
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = getall_noteonoff_evwr(self->trackctx,
                                  (byte_t) channel);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

static PyObject *msq_track_getall_midicev(PyObject *obj,
                                          PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  int                 channel, tickmin, tickmax;

  if (!PyArg_ParseTuple(args , "iii", &channel, &tickmin, &tickmax))
    return NULL;
  if (tickmin < 0)
    tickmin = 0;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = getall_midicev_repr(&(self->trackctx->track->tickev_list),
                                (byte_t) channel,
                                (uint_t) tickmin,
                                (uint_t) tickmax);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

static PyObject *msq_track_sel_noteonoff_repr(PyObject *obj,
                                              PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  int                 channel, tick_min, tick_max, note_min, note_max;

  if (!PyArg_ParseTuple(args,
                        "iiiii",
                        &channel,
                        &tick_min,
                        &tick_max,
                        &note_min,
                        &note_max))
    return NULL;
  if (channel < 0 ||
      tick_min < 0 ||
      tick_max < 0 ||
      note_min < 0 ||
      note_max < 0)
    return NULL;
  /* output("debug: ### %i %i %i %i %i\n", */
  /*        channel, */
  /*        tick_min, */
  /*        tick_max, */
  /*        note_min, */
  /*        note_max); */
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = sel_noteonoff_repr(self->trackctx,
                               (byte_t) channel,
                               tick_min,
                               tick_max,
                               (byte_t) note_min,
                               (byte_t) note_max);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

static PyObject *msq_track_sel_noteonoff_evwr(PyObject *obj,
                                              PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  int                 channel, tick_min, tick_max, note_min, note_max;

  if (!PyArg_ParseTuple(args,
                        "iiiii",
                        &channel,
                        &tick_min,
                        &tick_max,
                        &note_min,
                        &note_max))
    return NULL;
  if (channel < 0 ||
      tick_min < 0 ||
      tick_max < 0 ||
      note_min < 0 ||
      note_max < 0)
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = sel_noteonoff_evwr(self->trackctx,
                               (byte_t) channel,
                               tick_min,
                               tick_max,
                               (byte_t) note_min,
                               (byte_t) note_max);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

static PyObject *msq_track_sel_ctrl_evwr(PyObject *obj,
                                         PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  int                 channel, tick_min, tick_max, ctrl_num;

  if (!PyArg_ParseTuple(args,
                        "iiii",
                        &channel,
                        &tick_min,
                        &tick_max,
                        &ctrl_num))
    return NULL;
  if (channel < 0 ||
      tick_min < 0 ||
      tick_max < 0 ||
      ctrl_num < 0)
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = sel_ctrl_evwr(self->trackctx,
                          (byte_t) channel,
                          tick_min,
                          tick_max,
                          (byte_t) ctrl_num);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

static PyObject *msq_track_sel_pitch_evwr(PyObject *obj,
                                          PyObject *args)
{
  midiseq_trackObject *self    = (midiseq_trackObject *) obj;
  PyObject            *ret_obj = NULL;
  int                 channel, tick_min, tick_max;

  if (!PyArg_ParseTuple(args,
                        "iii",
                        &channel,
                        &tick_min,
                        &tick_max))
    return NULL;
  if (channel < 0 ||
      tick_min < 0 ||
      tick_max < 0)
    return NULL;
  pthread_rwlock_rdlock(&(self->trackctx->lock));
  ret_obj = sel_pitch_evwr(self->trackctx,
                          (byte_t) channel,
                          tick_min,
                          tick_max);
  pthread_rwlock_unlock(&(self->trackctx->lock));
  return ret_obj;
}

void dump_track(track_t *);

static PyObject *midiseq_track_dump(PyObject *obj,
                                    PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  output("\ntrackctx:trash.len %d\n", self->trackctx->trash.len);
  dump_track(self->trackctx->track);
  Py_RETURN_NONE;
}

static PyMethodDef midiseq_track_methods[] = {
  {"get_name",              midiseq_track_get_name,          METH_NOARGS,  "Get track name"},
  {"set_name",              midiseq_track_set_name,          METH_VARARGS, "Set track name"},
  {"get_len",               midiseq_track_get_len,           METH_NOARGS,  "Get track len"},
  {"get_start",             midiseq_track_get_start,         METH_NOARGS,  "Get track start"},
  {"get_loop_pos",          midiseq_track_get_loop_pos,      METH_VARARGS, "Get track loop pos from a tick"},
  {"get_mute_state",        midiseq_track_get_mute_state,    METH_NOARGS,  "Get mute state"},
  {"set_len",               midiseq_track_set_len,           METH_VARARGS, "Set track len"},
  {"set_start",             midiseq_track_set_start,         METH_VARARGS, "Set track start"},
  {"get_output",            midiseq_track_get_output,        METH_NOARGS,  "Get track output"},
  {"set_output",            midiseq_track_set_output,        METH_VARARGS, "Set track output"},
  {"toggle_mute",           midiseq_track_toggle_mute,       METH_NOARGS,  "Toggle mute state"},
  {"mute",                  midiseq_track_mute,              METH_NOARGS,  "Set mute state"},
  {"unmute",                midiseq_track_unmute,            METH_NOARGS,  "Unset mute state"},
  {"has_output",            midiseq_track_has_output,        METH_VARARGS, "Check if track has output"},
  {"add_evrepr_list",       midiseq_track_add_evrepr_list,   METH_VARARGS, "Add a note list event representation"},
  {"is_in_recmode",         midiseq_track_is_in_recmode,     METH_NOARGS,  "Get if the track is in record mode"},
  {"is_handled",            midiseq_track_ishandled,         METH_NOARGS,  "Get if the track is handled"},
  {"has_changed",           midiseq_track_has_changed,       METH_NOARGS,  "Get if the track has changed (During record)"},
  {"_delete_evwr_list",     msq_track_delete_evwr_list,      METH_VARARGS, "Delete event wrapper list (Caution: never use event of other tracks)"},
  {"_get_evwr_list",        msq_track_get_evwr_list,         METH_VARARGS, "Delete event wrapper list (Caution: never use event of other tracks)"},
  {"play_note",             midiseq_track_play_note,         METH_VARARGS, "Play note on output"},
  {"getall_noteonoff",      msq_track_getall_noteonoff,      METH_VARARGS, "Get note list representation in python"},
  {"getall_noteonoff_evwr", msq_track_getall_noteonoff_evwr, METH_VARARGS, "Get note event wrapper list"},
  {"getall_midicev",        msq_track_getall_midicev,        METH_VARARGS, "Get note list representation in python"},
  {"getall_event",          msq_track_getall_event,          METH_VARARGS, "Get event list representation in python"},
  {"sel_noteonoff_evwr",    msq_track_sel_noteonoff_evwr,    METH_VARARGS, "select note (event wrapper list)"},
  {"sel_noteonoff_repr",    msq_track_sel_noteonoff_repr,    METH_VARARGS, "select note (event repr list)"},
  {"sel_ctrl_evwr",         msq_track_sel_ctrl_evwr,         METH_VARARGS, "select control event number (event wrapper list)"},
  {"sel_pitch_evwr",        msq_track_sel_pitch_evwr,        METH_VARARGS, "select pitch bend event (event wrapper list)"},
  {"_dump",                 midiseq_track_dump,              METH_NOARGS,  "Dump track event(s)"},
  {NULL,                    NULL,                            0,            NULL}
};


static PyObject *midiseq_track_repr(PyObject *obj)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  return Py_BuildValue("s",
                       self->trackctx->track->name ?
                       self->trackctx->track->name : "no track name");
}

#include "./pym_midiseq_evwr.h"

/* static PyObject *midiseq_track_get_iter(PyObject *obj) */
/* { */
/*   midiseq_trackObject *self = (midiseq_trackObject *) obj; */

/*   return create_midiseq_evwr(self->trackctx->track); */
/* } */

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
    0,                             /* tp_iter */
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

PyObject *create_pym_track(track_ctx_t *trackctx)
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
