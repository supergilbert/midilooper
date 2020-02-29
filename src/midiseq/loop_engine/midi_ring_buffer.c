/* Copyright 2012-2020 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU Gneneral Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */

#include "loop_engine/engine.h"

void free_midiringbuff(midiringbuffer_t *mrb)
{
  free(mrb->buff);
  free(mrb);
}

midiringbuffer_t *init_midiringbuff(uint_t size)
{
  midiringbuffer_t *ringbuff = NULL;

  ringbuff = myalloc(sizeof (midiringbuffer_t));
  ringbuff->buff = myalloc(sizeof (midirec_t) * size);
  ringbuff->last  = &(ringbuff->buff[size - 1]);
  ringbuff->wptr  = ringbuff->buff;
  ringbuff->rptr  = ringbuff->buff;
  ringbuff->max   = MSQ_FALSE;
  return ringbuff;
}

static inline void _mrb_next_ptr(midiringbuffer_t *rbuff, midirec_t **ptr)
{
  /* Atomic assigment */
  ++(*ptr);
  if ((*ptr) > rbuff->last)
    (*ptr) = rbuff->buff;
}

#include <strings.h>
msq_bool_t mrb_write(midiringbuffer_t *rbuff, uint tick, midicev_t *mcev)
{
  if (rbuff->max == MSQ_TRUE)
    return MSQ_FALSE;

  rbuff->wptr->tick = tick;
  bcopy(mcev, &(rbuff->wptr->ev), sizeof (midicev_t));
  _mrb_next_ptr(rbuff, &(rbuff->wptr));
  if (rbuff->wptr == rbuff->rptr)
    rbuff->max = MSQ_TRUE;
  return MSQ_TRUE;
}

msq_bool_t mrb_read(midiringbuffer_t *rbuff, uint *tick, midicev_t *mcev)
{
  if (rbuff->rptr == rbuff->wptr) /* buffer empty */
    return MSQ_FALSE;

  *tick = rbuff->rptr->tick;
  bcopy(&(rbuff->rptr->ev), mcev, sizeof (midicev_t));
  _mrb_next_ptr(rbuff, &(rbuff->rptr));
  if (rbuff->max == MSQ_TRUE)
    rbuff->max = MSQ_FALSE;
  return MSQ_TRUE;
}
