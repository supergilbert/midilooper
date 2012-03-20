#ifndef __MIDISEQ_PYM
#define __MIDISEQ_PYM

#include <Python.h>

PyTypeObject *init_midiseq_trackType(void);
PyTypeObject *init_midiseq_Type(void);
PyTypeObject *init_midiseq_fileType(void);
PyTypeObject *init_midiseq_evwrType(void);

#endif
