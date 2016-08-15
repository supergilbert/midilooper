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


#ifndef __MIDISEQ_PYM
#define __MIDISEQ_PYM

#include <Python.h>

PyTypeObject *init_midiseq_trackType(void);
PyTypeObject *init_midiseq_Type(void);
PyTypeObject *init_midiseq_fileType(void);
PyTypeObject *init_midiseq_evwrType(void);
PyTypeObject *init_midiseq_outputType(void);

#endif
