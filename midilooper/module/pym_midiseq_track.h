#ifndef __PYM_TRACK
#define __PYM_TRACK

#include <Python.h>

#include "loop_engine/engine.h"

PyObject *create_midiseq_track(track_ctx_t *trackctx);

/* track header */
typedef struct {
    PyObject_HEAD
    track_ctx_t *trackctx;
} midiseq_trackObject;


#endif
