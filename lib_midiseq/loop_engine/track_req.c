/* Copyright 2012-2014 Gilbert Romer */

/* This file is part of gmidilooper. */

/* gmidilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* gmidilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include "engine.h"

/* @brief Search backward for the first unused request or create new one */
trackreq_t *trackreq_getunused_req(list_t *req_list)
{
  node_t     *node = NULL;
  trackreq_t *trackreq = NULL;
  trackreq_t *ret = NULL;

  if (req_list->head)
    {
      node = req_list->tail;
      do
        {
          trackreq = (trackreq_t *) node->addr;
          if (trackreq->used == TRUE)
            break;
          else
            ret = trackreq;
          node = node->prev;
        } while (node != NULL);
      if (ret != NULL)
        return ret;
    }
  ret = myalloc(sizeof (trackreq_t));
  ret->used = FALSE;
  push_to_list_tail(req_list, ret);
  return ret;
}

void       trackreq_play_midicev(track_ctx_t *trackctx, midicev_t *midicev)
{
  trackreq_t *trackreq = NULL;

  pthread_mutex_lock(&(trackctx->req_lock));
  trackreq = trackreq_getunused_req(&(trackctx->req_list));
  bcopy(midicev, &(trackreq->midicev), sizeof(midicev_t));
  trackreq->req = req_play_midicev;
  trackreq->used = TRUE;
  pthread_mutex_unlock(&(trackctx->req_lock));
}

void       trackreq_play_pendings(track_ctx_t *trackctx)
{
  trackreq_t *trackreq = NULL;

  pthread_mutex_lock(&(trackctx->req_lock));
  trackreq = trackreq_getunused_req(&(trackctx->req_list));
  trackreq->req = req_pending_notes;
  trackreq->used = TRUE;
  pthread_mutex_unlock(&(trackctx->req_lock));
}

/* @brief Search forward the first request used */
trackreq_t *trackreq_getnext_req(list_t *req_list)
{
  node_t     *node = NULL;
  trackreq_t *trackreq = NULL;

  for (node = req_list->head;
       node;
       node = node->next)
    {
      trackreq = (trackreq_t *) node->addr;
      if (trackreq->used == TRUE)
        return trackreq;
    }
  return NULL;
}
