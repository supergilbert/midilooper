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


#ifndef __PYM_TRACK
#define __PYM_TRACK

#include <Python.h>

#include "loop_engine/engine.h"

PyObject *create_pym_track(track_ctx_t *trackctx);

/* track header */
typedef struct {
    PyObject_HEAD
    track_ctx_t *trackctx;
} midiseq_trackObject;


#endif
