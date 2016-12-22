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

static struct PyModuleDef midiseq_module = {
	.m_base = PyModuleDef_HEAD_INIT,
	.m_name = "midiseq",										/* name of module */
	.m_doc = "Provide midi sequencer API.", /* module documentation, may be NULL */
	.m_size = -1,			 /* size of per-interpreter state of the module, */
	/* or -1 if the module keeps state in global variables. */
	.m_methods = midiseq_module_methods,
	.m_slots = NULL,
	.m_traverse = NULL,
	.m_clear = NULL,
	.m_free = NULL
};

PyMODINIT_FUNC PyInit_midiseq(void)
{
  PyObject *midiseq_module_po = PyModule_Create(&midiseq_module);
  PyObject *new_object = NULL;

  new_object = (PyObject *) init_midilooper_Type();
  if (new_object == NULL)
    return NULL;
  PyModule_AddObject(midiseq_module_po, "midilooper", (PyObject *) new_object);

  new_object = (PyObject *) init_midiseq_evwrType();
  if (new_object == NULL)
    return NULL;
  PyModule_AddObject(midiseq_module_po, "evwr", (PyObject *) new_object);

  new_object = (PyObject *) init_midiseq_trackType();
  if (new_object == NULL)
    return NULL;
  PyModule_AddObject(midiseq_module_po, "track", (PyObject *) new_object);

  new_object = (PyObject *) init_midiseq_fileType();
  if (new_object == NULL)
    return NULL;
  PyModule_AddObject(midiseq_module_po, "midifile", (PyObject *) new_object);

  new_object = (PyObject *) init_midiseq_outputType();
  if (new_object == NULL)
    return NULL;
  PyModule_AddObject(midiseq_module_po, "port", (PyObject *) new_object);
  return midiseq_module_po;
}
