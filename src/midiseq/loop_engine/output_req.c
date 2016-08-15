/* Copyright 2012-2016 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include "engine.h"

/* @brief Search backward for the first unused request or create new one */
midireq_t *output_getunused_req(output_t *output)
{
  node_t     *node = NULL;
  midireq_t *midireq = NULL;
  midireq_t *ret = NULL;

  if (output->req_list.head)
    {
      node = output->req_list.tail;
      do
        {
          midireq = (midireq_t *) node->addr;
          if (midireq->used == TRUE)
            break;
          else
            ret = midireq;
          node = node->prev;
        } while (node != NULL);
      if (ret != NULL)
        return ret;
    }
  ret = myalloc(sizeof (midireq_t));
  ret->used = FALSE;
  push_to_list_tail(&(output->req_list), ret);
  return ret;
}

#include <strings.h>

void output_add_req(output_t *output, midicev_t *midicev)
{
  midireq_t *midireq = NULL;

  pthread_mutex_lock(&(output->req_lock));
  midireq = output_getunused_req(output);
  bcopy(midicev, &(midireq->midicev), sizeof(midicev_t));
  midireq->used = TRUE;
  pthread_mutex_unlock(&(output->req_lock));
}

/* @brief Search forward the first request used */
midireq_t *output_getnext_req(output_t *output)
{
  node_t     *node = NULL;
  midireq_t *midireq = NULL;

  for (node = output->req_list.head;
       node;
       node = node->next)
    {
      midireq = (midireq_t *) node->addr;
      if (midireq->used == TRUE)
        return midireq;
    }
  return NULL;
}

void output_play_reqlist(output_t *output)
{
  midireq_t *req = NULL;

  for (req = output_getnext_req(output);
       req;
       req = output_getnext_req(output))
    {
      _output_write(output, &(req->midicev));
      req->used = FALSE;
    }
}
