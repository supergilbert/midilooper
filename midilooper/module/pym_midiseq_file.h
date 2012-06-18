#ifndef __PYM_MFILE_H
#define __PYM_MFILE_H

#include "midi/midifile.h"

/* file header */
typedef struct {
    PyObject_HEAD
    midifile_t *midifile;
} midiseq_fileObject;

#endif
