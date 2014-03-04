#ifndef __EV_ITERATOR
#define __EV_ITERATOR

#include "midi/midiev_inc.h"

typedef struct {
  list_iterator_t tickit;
  list_iterator_t seqevit;
  uint_t          tick;
} ev_iterator_t;



void    dump_evit(ev_iterator_t *evit);
seqev_t *evit_init(ev_iterator_t *, list_t *tickev_list);
seqev_t *evit_tick_head(ev_iterator_t *);
bool_t  evit_searchev(ev_iterator_t *evit, uint_t tick, midicev_t *mcev);
seqev_t *evit_next_tick(ev_iterator_t *);
seqev_t *evit_get_seqev(ev_iterator_t *);
seqev_t *evit_next_seqev(ev_iterator_t *);
/* seqev_t *_evit_next_seqev(ev_iterator_t *ev_iterator); */
void    evit_copy(ev_iterator_t *evit_src, ev_iterator_t *evit_dst);
midicev_t *evit_next_midicev_type(ev_iterator_t *ev_iterator, byte_t channel, byte_t type);
midicev_t *evit_next_midiallchannel(ev_iterator_t *ev_iterator);
void evit_del_event(ev_iterator_t *ev_iterator);

bool_t _evit_check(ev_iterator_t *evit, list_t *tickev_list);
#ifdef __ROUGH
#define evit_check(evit, tickev_list)    (TRUE)
#else
#define evit_check(evit, tickev_list)    _evit_check(evit, (tickev_list))
#endif

#define evit_next_noteon(evit, chan)   (evit_next_midicev_type((evit), (chan), NOTEON))
#define evit_next_noteoff(evit, chan)  (evit_next_midicev_type((evit), (chan), NOTEOFF))
#define evit_next_ctrl(evit, chan)     (evit_next_midicev_type((evit), (chan), CONTROLCHANGE))

midicev_t *evit_next_noteoff_num(ev_iterator_t *ev_iterator, byte_t channel, byte_t num);

#endif
