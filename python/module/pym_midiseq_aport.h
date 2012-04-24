#ifndef __PYM_APORT
#define __PYM_APORT

#include "asound/aseq.h"

typedef struct {
    PyObject_HEAD
    aseqport_ctx_t *aport;
} midiseq_aportObject;

PyObject *create_midiseq_aport(aseqport_ctx_t *aport);


#endif
