#ifndef __PYM_TRACK
#define __PYM_TRACK

#include <Python.h>

#include "seqtool/seqtool.h"

PyObject *create_midiseq_track(track_t *track);

/* track header */
typedef struct {
    PyObject_HEAD
    track_t     *track;
} midiseq_trackObject;


#endif
