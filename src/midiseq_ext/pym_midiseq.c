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


#include "pym_midiseq.h"
#include "asound/aseq.h"

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
  PyModule_AddObject(midiseq_module, "aport", (PyObject *) init_midiseq_outputType());
}
