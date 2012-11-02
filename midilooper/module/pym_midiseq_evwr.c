#include <Python.h>
#include "seqtool/seqtool.h"
#include "midi/midiev_inc.h"

#include "./pym_midiseq_evwr.h"

static void midiseq_evwr_dealloc(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *build_event_repr(uint_t tick, seqev_t *ev)
{
  midicev_t *midicev = NULL;

  if (ev->type == MIDICEV)
    {
      midicev = (midicev_t *) ev->addr;
      if (midicev->type == NOTEOFF || midicev->type == NOTEON)
        return Py_BuildValue("(iiiii)",
                             tick,
                             midicev->chan,
                             midicev->type,
                             midicev->event.note.num,
                             midicev->event.note.val);
      else
        {
          return Py_BuildValue("(iii)",
                               tick,
                               midicev->chan,
                               midicev->type);
          printf("Unsupported midi channel event type: %i\n", midicev->type);
        }
    }
  else
    Py_RETURN_NONE;
}

static PyObject *midiseq_evwr_getevent(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  tickev_t *tickev = NULL;
  seqev_t *ev = NULL;

  if (iter_node(&(self->evit)) != NULL)
    {
      ev = iter_node_ptr(&(self->evit));
      tickev = iter_node_ptr(&(self->tickit));
      return build_event_repr(tickev->tick, ev);
    }
  Py_RETURN_NONE;
}

static void evwr_goto_seqevlist_head(midiseq_evwrObject *self)
{
  tickev_t *tickev = NULL;

  if (self->tickit.node == NULL)
    return;
  tickev = (tickev_t *) iter_node_ptr(&(self->tickit));
  if (tickev != NULL)
    iter_init(&(self->evit), &(tickev->seqev_list));
}

static void evwr_goto_head(midiseq_evwrObject *self)
{
  iter_head(&(self->tickit));
  evwr_goto_seqevlist_head(self);
}

static void evwr_goto_next_tick(midiseq_evwrObject *self)
{
  if (iter_node(&(self->tickit)) != NULL)
    {
      iter_next(&(self->tickit));
      if (iter_node(&(self->tickit)) != NULL)
        evwr_goto_seqevlist_head(self);
    }
}

static void _evwr_goto_available_ev(midiseq_evwrObject *self)
{
  seqev_t *ev = NULL;

  while (iter_node(&(self->evit)) != NULL)
    {
      ev = iter_node_ptr(&(self->evit));
      if (ev->todel == TRUE)
        {
          iter_next(&(self->evit));
          if (iter_node(&(self->evit)) != NULL)
            continue;
          else
            {
              evwr_goto_next_tick(self);
              if (iter_node(&(self->evit)) != NULL)
                continue;
              else
                return;
            }
        }
      else
        return;
    }
}

static PyObject *midiseq_evwr_iter_next(PyObject *obj)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;

  if (iter_node(&(self->tickit)) != NULL)
    {
      if (self->evit_started == FALSE)
        {
          self->evit_started = TRUE;
          _evwr_goto_available_ev(self);
          if (iter_node(&(self->evit)) != NULL)
            {
              Py_INCREF(obj);
              return obj;
            }
        }
      else
        {
          if (iter_node(&(self->evit)) != NULL)
            {
              iter_next(&(self->evit));
              _evwr_goto_available_ev(self);
              if (iter_node(&(self->evit)) != NULL)
                {
                  Py_INCREF(obj);
                  return obj;
                }
              else
                {
                  evwr_goto_next_tick(self);
                  _evwr_goto_available_ev(self);
                  if (iter_node(&(self->evit)) != NULL)
                    {
                      Py_INCREF(obj);
                      return obj;
                    }
                }
            }
        }
      evwr_goto_head(self);
    }
  self->evit_started = FALSE;
  PyErr_SetNone(PyExc_StopIteration);
  return NULL;
}

static PyObject *midiseq_evwr_del_event(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *self = (midiseq_evwrObject *) obj;
  tickev_t           *tickev = NULL;

  iter_node_del(&(self->evit), free_seqev);
  if (self->evit.list->len <= 0)
    {
      iter_node_del(&(self->tickit), free_tickev);
      if (self->tickit.list->len > 0)
        {
          tickev = iter_node_ptr(&(self->tickit));
          iter_init(&(self->evit), &(tickev->seqev_list));
        }
    }
  Py_RETURN_NONE;
}

/* TODO */
/* static PyObject *midiseq_evwr_repr(midiseq_evwrObject *self) */
/* { */
/*   return PyString_FromFormat("midiseq evwr type"); */
/* } */

PyObject *midiseq_evwr_copy(PyObject *obj, PyObject *args);

static PyMethodDef midiseq_evwr_methods[] = {
  {"get_event", midiseq_evwr_getevent, METH_NOARGS,
   "Get the event representation as a tuple of integer"},
  {"_del_event", midiseq_evwr_del_event, METH_NOARGS,
   "Delete the current event of the track (/!\ Never use this function when engine is running it is really not thread safe and will surely make memory corruption)"},
  {"copy", midiseq_evwr_copy, METH_NOARGS,
   "Delete the currnent event of the track"},
  {NULL, NULL, 0, NULL}
};

static PyObject *midiseq_evwr_get_iter(PyObject *obj)
{
  Py_INCREF(obj);
  return obj;
}

static PyTypeObject midiseq_evwrType = {
    PyObject_HEAD_INIT(NULL)
    0,                          /* ob_size */
    "midiseq.evwr",             /* tp_name */
    sizeof(midiseq_evwrObject), /* tp_basicsize */
    0,                          /* tp_itemsize */
    midiseq_evwr_dealloc,       /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Tick event wrapper",       /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    midiseq_evwr_get_iter,      /* tp_iter */
    midiseq_evwr_iter_next,     /* tp_iternext */
    midiseq_evwr_methods,       /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
};

PyObject *midiseq_evwr_copy(PyObject *obj, PyObject *args)
{
  midiseq_evwrObject *evwr = (midiseq_evwrObject *) obj;
  midiseq_evwrObject *newevwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                                                 &midiseq_evwrType);

  memcpy(&(newevwr->tickit), &(evwr->tickit), sizeof(list_iterator_t));
  memcpy(&(newevwr->evit), &(evwr->evit), sizeof(list_iterator_t));
  return (PyObject *) newevwr;
}

PyObject *create_midiseq_evwr(track_t *track)
{
  tickev_t *tickev = NULL;
  midiseq_evwrObject *evwr = NULL;

  evwr = (midiseq_evwrObject *) PyObject_New(midiseq_evwrObject,
                                             &midiseq_evwrType);
  iter_init(&(evwr->tickit), &(track->tickev_list));
  if (iter_node(&(evwr->tickit)) != NULL)
    {
      tickev = (tickev_t *) iter_node_ptr(&(evwr->tickit));
      if (tickev != NULL)
        {
          iter_init(&(evwr->evit), &(tickev->seqev_list));
          evwr->evit_started = FALSE;
        }
    }
  return (PyObject *) evwr;
}

PyTypeObject *init_midiseq_evwrType(void)
{
  if (PyType_Ready(&midiseq_evwrType) < 0)
    return NULL;
  Py_INCREF(&midiseq_evwrType);
  return &midiseq_evwrType;
}
