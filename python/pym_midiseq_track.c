#include "./pym_midiseq_track.h"
/* #include "midi/midifile.h" */
#include "debug_tool/debug_tool.h"

#include "./pym_midiseq_track.h"

static void midiseq_track_dealloc(PyObject *obj)
{
  trace_func;
  midiseq_trackObject *self = (midiseq_trackObject *) obj;
  if (self->track != NULL) {
    free_track(self->track);
  }
  self->ob_type->tp_free((PyObject*)self);
  trace_func;
}

static PyObject *midiseq_track_getname(PyObject *self, PyObject *args)
{
  midiseq_trackObject *trackpy = (midiseq_trackObject *) self;
  char *track_name = "no track name";

  if (trackpy->track && trackpy->track->name)
    track_name = trackpy->track->name;
  return Py_BuildValue("s", track_name);
}

static PyMethodDef midiseq_track_methods[] = {
  //  {"getinfo", midiseq_getinfo, METH_NOARGS, "get track info"},
  {"getname", midiseq_track_getname, METH_NOARGS, "get track name"},
  {NULL, NULL, 0, NULL}
};

static PyTypeObject midiseq_trackType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "midiseq.track",             /* tp_name */
    sizeof(midiseq_trackObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    midiseq_track_dealloc,       /* tp_dealloc */
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
    "track objects",             /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    midiseq_track_methods,       /* tp_methods */
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

PyTypeObject *get_midiseq_trackType(void) { return &midiseq_trackType; }

PyTypeObject *init_midiseq_trackType(void)
{
  if (PyType_Ready(&midiseq_trackType) < 0)
    return NULL;
  Py_INCREF(&midiseq_trackType);
  return &midiseq_trackType;
}
