#ifndef __PYM_TICKEVWR
#define __PYM_TICKEVWR

#include <Python.h>
#include "loop_engine/engine.h"
#include "loop_engine/ev_iterator.h"

PyObject *build_evwr(track_ctx_t *);
PyObject *build_evwr_from_evit(ev_iterator_t *, track_ctx_t *);
PyObject *build_evrepr(uint_t tick, midicev_t *midicev);


typedef struct {
  PyObject_HEAD
  ev_iterator_t evit;
  track_ctx_t   *trackctx;
} midiseq_evwrObject;


#endif
