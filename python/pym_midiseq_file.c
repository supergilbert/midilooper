#include <Python.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pym_midiseq_track.h"
#include "debug_tool/debug_tool.h"

#include "midi/midifile.h"

/* file header */
typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    midifile_t *midifile;
} midiseq_fileObject;

static PyObject *midiseq_file_gettempo(PyObject *self, PyObject *args)
{
  midiseq_fileObject *pobj = (midiseq_fileObject *) self;

  return Py_BuildValue("i", pobj->midifile->info.tempo);
}

static PyObject *midiseq_file_getppq(PyObject *self, PyObject *args)
{
  midiseq_fileObject *pobj = (midiseq_fileObject *) self;

  return Py_BuildValue("i", pobj->midifile->info.ppq);
}

static PyObject *midiseq_file_getname(PyObject *self, PyObject *args)
{
  midiseq_fileObject *pobj = (midiseq_fileObject *) self;

  return Py_BuildValue("s", pobj->midifile->info.name);

}

static PyObject *midiseq_file_getnumber_of_track(PyObject *self,
                                                  PyObject *args)
{
  midiseq_fileObject *pobj = (midiseq_fileObject *) self;

  return Py_BuildValue("i", pobj->midifile->number_of_track);
}

static PyObject *midiseq_file_getmergedtrack(PyObject *self, PyObject *args)
{
  midiseq_fileObject *pobj = (midiseq_fileObject *) self;
  track_t     *track = merge_all_track("blabla", &(pobj->midifile->track_list));
  midiseq_trackObject *pytrack = PyObject_New(midiseq_trackObject,
                                              get_midiseq_trackType());

  pytrack->track = track;
  return (PyObject *) pytrack;
}

static PyMethodDef midiseq_file_methods[] = {
  {"gettempo", midiseq_file_gettempo, METH_NOARGS, ""},
  {"getms", midiseq_file_gettempo, METH_NOARGS, ""},
  {"getppq", midiseq_file_getppq, METH_NOARGS, ""},
  {"getname", midiseq_file_getname, METH_NOARGS, ""},
  {"get_number_of_track", midiseq_file_getnumber_of_track, METH_NOARGS, ""},
  {"getmergedtrack", midiseq_file_getmergedtrack, METH_NOARGS, ""},
  {NULL, NULL, 0, NULL}
};

static void midiseq_file_dealloc(PyObject *obj)
{
  trace_func;
  midiseq_fileObject *self = (midiseq_fileObject *) obj;

  if (self->midifile != NULL)
    free_midifile(self->midifile);
  self->ob_type->tp_free((PyObject*)self);
  trace_func;
}

midifile_t *read_midifile_path(char *filepath)
{
  int        fd = 0;
  midifile_t *midifile = NULL;
  fd = open(filepath, O_RDONLY);
  if (fd == -1)
    {
      output_error("Problem with file \"%s\": %s\n", filepath, strerror(errno));
      return NULL;
    }
  midifile = read_midifile_fd(fd);
  return midifile;
}

static int midiseq_file_init(midiseq_fileObject *self,
                                   PyObject *args,
                                   PyObject *kwds)
{
  char *filepath = NULL;
  trace_func;

  if (args == NULL)
    return -1;
  trace_func;

  if (!PyArg_ParseTuple(args, "s", &filepath))
    return -1;

  trace_func;
  if (filepath == NULL)
    return -1;

  self->midifile = read_midifile_path(filepath);
  trace_func;
  debug("filepath=%s self->midifile=%p\n", filepath, self->midifile);
  if (self->midifile == NULL)
    return -1;

  trace_func;
  /* return (PyObject *) NULL; */
  return 0;
}

static PyTypeObject midiseq_fileType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "midiseq.midifile",         /* tp_name */
    sizeof(midiseq_fileObject),  /* tp_basicsize */
    0,                           /* tp_itemsize */
    midiseq_file_dealloc,        /* tp_dealloc */
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
    "file objects",              /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    midiseq_file_methods,        /* tp_methods */
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    (initproc) midiseq_file_init, /* tp_init */
    0,                           /* tp_alloc */
    PyType_GenericNew,           /* tp_new */
};

PyTypeObject *init_midiseq_fileType(void)
{
  if (PyType_Ready(&midiseq_fileType) < 0)
    return NULL;
  Py_INCREF(&midiseq_fileType);
  return &midiseq_fileType;
}
