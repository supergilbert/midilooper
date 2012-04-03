#include "./pym_midiseq_track.h"

#include "debug_tool/debug_tool.h"

static int midiseq_track_init(midiseq_trackObject *self,
                               PyObject *args,
                               PyObject *kwds)
{
  track_t       *track = NULL;
  char          *name = NULL;

  track = myalloc(sizeof (track_t));
  bzero(track, sizeof (track_t));

  if (args != NULL)
    {
      if (!PyArg_ParseTuple(args, "s", &name))
        return -1;
      if (name == NULL)
        name = "no track name";
    }
  track->name = strdup(name);
  self->track = track;
  return 0;
}

static void midiseq_track_dealloc(PyObject *obj)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  if (self->track != NULL) {
    free_track(self->track);
  }
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *midiseq_track_get_name(PyObject *self, PyObject *args)
{
  midiseq_trackObject *trackpy = (midiseq_trackObject *) self;
  char *track_name = "no track name";

  if (trackpy->track && trackpy->track->name)
    track_name = trackpy->track->name;
  return Py_BuildValue("s", track_name);
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
  add_new_seqev(self->track, tick, mcev, MIDICEV);
  Py_RETURN_NONE;
}

static PyObject *midiseq_track_clear(PyObject *obj, PyObject *args)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  clear_tickev_list(&(self->track->tickev_list));
  Py_RETURN_NONE;
}

static PyMethodDef midiseq_track_methods[] = {
  {"get_name", midiseq_track_get_name, METH_NOARGS, "Get track name"},
  {"add_note_event", midiseq_track_add_note_event, METH_VARARGS, "Add a note event"},
  {"clear", midiseq_track_clear, METH_NOARGS, "free all track list to use with caution (/!\\ not while running for now)"},
  {NULL, NULL, 0, NULL}
};

#include "./pym_midiseq_evwr.h"

static PyObject *midiseq_track_get_iter(PyObject *obj)
{
  midiseq_trackObject *self = (midiseq_trackObject *) obj;

  return create_midiseq_evwr(self->track);
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
    0,                             /* tp_repr */
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
    (initproc) midiseq_track_init, /* tp_init */
    0,                             /* tp_alloc */
    PyType_GenericNew,             /* tp_new */
};

PyObject *create_midiseq_track(track_t *track)
{
  midiseq_trackObject *pytrack = PyObject_New(midiseq_trackObject,
                                              &midiseq_trackType);

  pytrack->track = track;
  return (PyObject *) pytrack;
}

PyTypeObject *init_midiseq_trackType(void)
{
  if (PyType_Ready(&midiseq_trackType) < 0)
    return NULL;
  Py_INCREF(&midiseq_trackType);
  return &midiseq_trackType;
}
