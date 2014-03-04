#ifndef __PYM_MTOOLS
#define __PYM_MTOOLS
#include "loop_engine/engine.h"

PyObject *sel_noteonoff_repr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             char note_min,
                             char note_max);
PyObject *sel_noteonoff_evwr(track_ctx_t *trackctx,
                             byte_t channel,
                             uint_t tick_min,
                             uint_t tick_max,
                             char note_min,
                             char note_max);
PyObject *getall_noteonoff_repr(list_t *tickev_list, byte_t channel);
PyObject *getall_event_repr(list_t *tickev_list);
PyObject *add_evrepr_list(track_ctx_t *trackctx, PyObject *pylist);
void     delete_evwr_list(track_ctx_t *trackctx, PyObject *pylist);
PyObject *try_gen_evwr_list(track_ctx_t *trackctx, PyObject *pylist);

/* PyObject *get_event_list_repr(list_t *); */
/* PyObject *get_note_list_repr(list_t *tickev_list, byte_t channel); */
/* PyObject *get_note_list_selection(list_t *tickev_list, */
/*                                   byte_t channel, */
/*                                   uint_t tick_min, */
/*                                   uint_t tick_max, */
/*                                   char note_min, */
/*                                   char note_max); */
#endif
