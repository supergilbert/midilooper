#include <Python.h>
#include "pym_midiseq.h"
#include "asound/aseq.h"

/* track_t  *midifile_to_onetrack(char *filename) */
/* { */
/*   int fd; */
/*   midifile_t *midifile; */
/*   track_t *thetrack = NULL; */

/*   fd = open(filename, O_RDONLY); */
/*   if (fd == -1) */
/*     { */
/*       output_error("Problem with file \"%s\": %s\n", filename, strerror(errno)); */
/*       return NULL; */
/*     } */
/*   debug("Reading file %s\n", filename); */
/*   midifile = read_midifile_fd(fd); */
/*   if (midifile == NULL) */
/*     { */
/*       output_error("Error while reading file %s", filename); */
/*       return NULL; */
/*     } */
/*   if (midifile->info.tempo  == 0) */
/*     { */
/*       midifile->info.tempo = 500000; */
/*       output_error("!!! tempo=0 changing it to %d\n", midifile->info.tempo); */
/*     } */
/*   debug("tempo=%i (bpm=%i)\nppq=%i\n", */
/*         midifile->info.tempo, */
/*         60000000 / midifile->info.tempo, */
/*         midifile->info.ppq); */
/*   thetrack = merge_all_track(filename, &(midifile->track_list)); */
/*   free_midifile_struct(midifile); */
/*   return thetrack; */
/* } */

/* static PyObject *read_midifile(PyObject *self, PyObject *args) */
/* { */
/*   char *filename = NULL; */
/*   track_t *track = NULL; */
/*   midiseq_trackObject *pyobj = NULL; */

/*   filename = PyString_AsString(args); */
/*   if (filename == NULL) */
/*     return NULL; */
/*   track = midifile_to_onetrack(filename); */
/*   /\* pyobj = (midiseq_trackObject *) PyObject_New(midiseq_trackObject, midiseq_trackType); *\/ */
/*   pyobj = PyObject_New(midiseq_trackObject, &midiseq_trackType); */
/*   pyobj->track = track; */
/*   return (PyObject *) pyobj; */
/* } */

/* static PyObject *track_to_midioutput(PyObject *self, PyObject *args) */
/* { */
  
/* } */

static PyMethodDef midiseq_module_methods[] = {
  /* {"midifile_to_track", read_midifile, METH_O, */
   /* "read a midifile and return a single track"}, */
  {NULL, NULL, 0, NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initmidiseq(void)
{
  PyObject* midiseq_module;

  midiseq_module = Py_InitModule3("midiseq", midiseq_module_methods,
                                  "Example module that creates an extension type.");
  PyModule_AddObject(midiseq_module, "midiseq", (PyObject *) init_midiseq_Type());
  PyModule_AddObject(midiseq_module, "evwr", (PyObject *) init_midiseq_evwrType());
  PyModule_AddObject(midiseq_module, "track", (PyObject *) init_midiseq_trackType());
  PyModule_AddObject(midiseq_module, "midifile", (PyObject *) init_midiseq_fileType());
}
