#ifndef __PYM_TRACK
#define __PYM_TRACK

#include <Python.h>
#include "seqtool/seqtool.h"

/* track header */
typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    track_t     *track;
} midiseq_trackObject;

PyTypeObject *get_midiseq_trackType(void);

#endif
