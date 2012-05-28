#ifndef __PYM_TICKEVWR
#define __PYM_TICKEVWR

#include <Python.h>
#include "seqtool/seqtool.h"

PyObject *create_midiseq_evwr(track_t *track);

typedef struct {
  PyObject_HEAD
  list_iterator_t tickit;
  list_iterator_t evit;
  bool_t          evit_started;
} midiseq_evwrObject;


#endif
