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

#include "pbt_event_handler_inc.h"
#include "pbt_gadget.h"
#include "pbt_tools.h"
#include "wbe_glfw_inc.h"

#include <string.h>

void pbt_evh_clear(pbt_evh_t *evh)
{
  pbt_evh_node_t *node, *tmp;

  node = evh->head;
  while (node != NULL)
    {
      tmp = node;
      node = node->next;
      free(tmp);
    }
}

void pbt_evh_del_node(pbt_evh_t *evh, pbt_ggt_t *ggt)
{
  pbt_evh_node_t *node, *next;

  if (evh->input_wgt != NULL
      && evh->input_wgt->ggt == ggt)
    evh->input_wgt = NULL;
  if (evh->entered_wgt != NULL
      && evh->entered_wgt->ggt == ggt)
    evh->entered_wgt = NULL;

  node = evh->head;
  if (node->ggt == ggt)
    {
      evh->head = node->next;
      free(node);
      return;
    }
  while (node->next != NULL)
    {
      next = node->next;
      if (next->ggt == ggt)
        {
          node->next = next->next;
          free(next);
          return;
        }
      node = next;
    }
  pbt_abort("Unable to delete specified ggt node in event handler.");
}

pbt_evh_node_t *pbt_evh_get_or_create(pbt_evh_t *evh, pbt_ggt_t *ggt)
{
  pbt_evh_node_t *node;

  if (evh->head == NULL)
    {
      evh->head = malloc(sizeof (pbt_evh_node_t));
      memset(evh->head, 0, sizeof (pbt_evh_node_t));
      evh->head->ggt = ggt;
      return evh->head;
    }
  else
    {
      for (node = evh->head;
           node->next != NULL;
           node = node->next)
        if (node->ggt == ggt)
          return node;
      if (node->ggt == ggt)
        return node;
      node->next = malloc(sizeof (pbt_evh_node_t));
      memset(node->next, 0, sizeof (pbt_evh_node_t));
      node->next->ggt = ggt;
      return node->next;
    }
}

void pbt_evh_add_set_focus_cb(pbt_evh_t *evh,
                              pbt_ggt_t *ggt,
                              pbt_evh_input_cb_t set_focus_cb,
                              void *set_focus_arg)
{
  pbt_evh_node_t *ggt_evh = pbt_evh_get_or_create(evh, ggt);

  ggt_evh->set_focus_cb = set_focus_cb;
  ggt_evh->set_focus_arg = set_focus_arg;
}

void pbt_evh_add_unset_focus_cb(pbt_evh_t *evh,
                                pbt_ggt_t *ggt,
                                pbt_evh_input_cb_t unset_focus_cb,
                                void *unset_focus_arg)
{
  pbt_evh_node_t *ggt_evh = pbt_evh_get_or_create(evh, ggt);

  ggt_evh->unset_focus_cb = unset_focus_cb;
  ggt_evh->unset_focus_arg = unset_focus_arg;
}

void pbt_evh_add_enter_cb(pbt_evh_t *evh,
                          pbt_ggt_t *ggt,
                          pbt_evh_notify_cb_t enter_cb,
                          void *enter_arg)
{
  pbt_evh_node_t *ggt_evh = pbt_evh_get_or_create(evh, ggt);

  ggt_evh->enter_cb = enter_cb;
  ggt_evh->enter_arg = enter_arg;
}

void pbt_evh_add_leave_cb(pbt_evh_t *evh,
                          pbt_ggt_t *ggt,
                          pbt_evh_notify_cb_t leave_cb,
                          void *leave_arg)
{
  pbt_evh_node_t *ggt_evh = pbt_evh_get_or_create(evh, ggt);

  ggt_evh->leave_cb = leave_cb;
  ggt_evh->leave_arg = leave_arg;
}

pbt_bool_t pbt_evh_handle_input_in(pbt_evh_t *evh,
                                   wbe_window_input_t *winev)
{
  pbt_evh_node_t *node;

  for (node = evh->head;
       node != NULL;
       node = node->next)
    if (node->set_focus_cb != NULL)
      if (node->set_focus_cb(node->ggt,
                             winev,
                             node->set_focus_arg) == PBT_TRUE)
        {
          if (node->unset_focus_cb != NULL)
            evh->input_wgt = node;
          return PBT_TRUE;
        }

  return PBT_FALSE;
}

pbt_bool_t pbt_evh_handle_input_out(pbt_evh_t *evh,
                                    wbe_window_input_t *winev)
{
  /* TODO debug check */
  if (evh->input_wgt == NULL)
    return PBT_TRUE;

  /* TODO debug check */
  if (evh->input_wgt->unset_focus_cb == NULL)
    return PBT_TRUE;

  return evh->input_wgt->unset_focus_cb(evh->input_wgt->ggt,
                                        winev,
                                        evh->input_wgt->unset_focus_arg);
}

void pbt_evh_handle_notify(pbt_evh_t *evh, int xpos, int ypos)
{
  pbt_evh_node_t *node;

  if (evh->entered_wgt != NULL)
    {
      if (_PBT_IS_IN_GGT(evh->entered_wgt->ggt, xpos, ypos) == PBT_FALSE)
        {
          if (evh->entered_wgt->leave_cb != NULL)
            evh->entered_wgt->leave_cb(evh->entered_wgt->leave_arg);
          evh->entered_wgt = NULL;
        }
    }
  else
    {
      for (node = evh->head;
           node != NULL;
           node = node->next)
        if (_PBT_IS_IN_GGT(node->ggt, xpos, ypos) == PBT_TRUE)
          {
            if (node->enter_cb != NULL)
              node->enter_cb(node->enter_arg);
            evh->entered_wgt = node;
          }
    }
}

void pbt_evh_handle(pbt_evh_t *evh, wbe_window_input_t *winev)
{
  if (evh->input_wgt == NULL)
    {
      pbt_evh_handle_notify(evh, winev->xpos, winev->ypos);
      pbt_evh_handle_input_in(evh, winev);
    }
  else if (pbt_evh_handle_input_out(evh, winev) == PBT_TRUE)
    evh->input_wgt = NULL;
}
