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

#include "pbt_gadget.h"
#include "pbt_window_gadget.h"
#include "pbt_gadget_window.h"
#include "wbe_pbw.h"
#include "pbt_tools.h"
#include "pbt_event_handler.h"

void pbt_ggt_win_update_area(pbt_ggt_win_t *ggt_win)
{
  pbt_pbarea_t buffer_area;

  pbt_pbarea_setup_from_buffer(&buffer_area,
                               &(ggt_win->pb_win.buffer),
                               0, 0,
                               ggt_win->pb_win.buffer.width,
                               ggt_win->pb_win.buffer.height);

  _pbt_ggt_update_area(ggt_win->ggt, &buffer_area);
}

void pbt_ggt_win_resize_cb(unsigned int width,
                           unsigned int height,
                           void *ggt_win_addr)
{
  pbt_ggt_win_t *ggt_win = ggt_win_addr;

  pbt_ggt_win_update_area(ggt_win);
  _pbt_ggt_draw(ggt_win->ggt);
  wbe_pbw_put_buffer(&(ggt_win->pb_win));
}

void pbt_ggt_win_destroy(pbt_ggt_win_t *ggt_win)
{
  /* pbt_evh_clear(&(ggt_win->evh)); */
  wbe_pbw_destroy(&(ggt_win->pb_win));
  if (ggt_win->ggt->destroy_cb != NULL)
    ggt_win->ggt->destroy_cb(ggt_win->ggt);
  else
    _pbt_ggt_nodes_destroy(ggt_win->ggt->childs);
}

#include <unistd.h>

void pbt_ggt_win_set_min_size(pbt_ggt_win_t *ggt_win)
{
  unsigned int width, height;

  width = _pbt_ggt_min_width(ggt_win->ggt);
  height = _pbt_ggt_min_height(ggt_win->ggt);
  wbe_pbw_set_size(&(ggt_win->pb_win), width, height);
  pbt_ggt_win_update_area(ggt_win);
  _pbt_ggt_draw(ggt_win->ggt);
  pbt_ggt_win_put_buffer(ggt_win); /* TODO RESOLVE THE ARTEFACT ON UPDATING SIZE */
}

void pbt_ggt_win_init_child_ev(pbt_ggt_win_t *ggt_win, pbt_ggt_node_t *child)
{
  pbt_ggt_t *ggt;
  pbt_wgt_t *wgt;

  while (child != NULL)
    {
      if (child->type == WIDGET || child->type == GADGET)
        {
          ggt = child->priv.ggt_addr;

          if (child->type == WIDGET)
            {
              wgt = ggt->priv;
              _pbt_wgt_init_ev(wgt, ggt_win);
            }
          pbt_ggt_win_init_child_ev(ggt_win, ggt->childs);
        }
      child = child->next;
    }
}

/* pbt_ggt_node_t *pbt_wgt_ctnr_add_ggt(pbt_ggt_win_t *ggt_win, */
/*                                      pbt_ggt_ctnr_t *ctnr, */
/*                                      pbt_ggt_t *ggt) */
/* { */
/*   pbt_ggt_node_t *new_node = _pbt_ggt_add_child_ggt(ctnr, ggt); */

/*   if (ggt_win != NULL) */
/*     pbt_ggt_win_init_child_ev(ggt_win, */
/*                               ggt->childs); */
/*   return new_node; */
/* } */

#define win_default_min_width  320
#define win_default_min_height 240

pbt_bool_t _pbt_ggt_win_init_w_ggt(pbt_ggt_win_t *ggt_win,
                                   const char *name,
                                   pbt_ggt_t *ggt,
                                   unsigned int width,
                                   unsigned int height,
                                   pbt_bool_t resizeable)
{
  unsigned int minw, minh, maxw, maxh;

  minw = _pbt_ggt_min_width(ggt);
  maxw = _pbt_ggt_max_width(ggt);
  if (width == 0)
    width = minw != 0 ? minw : win_default_min_width;
  else
    {
      if (width < minw || (maxw != 0 && width > maxw))
        pbt_abort("Gadget window start width does not correspond"
                  " to root gadget size\n"
                  " (width:%d min:%d max:%d)",
                  width, minw, maxw);
    }

  minh = _pbt_ggt_min_height(ggt);
  maxh = _pbt_ggt_max_height(ggt);
  if (height == 0)
    height = minh != 0 ? minh : win_default_min_height;
  else
    {
      if (height < minh || (maxh != 0 && height > maxh))
        pbt_abort("Gadget window start height does not correspond"
                  " to root gadget size\n"
                  " (height:%d min:%d max:%d)",
                  height, minh, maxh);
    }

  if (wbe_pbw_init(&(ggt_win->pb_win),
                   name,
                   width, height,
                   resizeable == PBT_TRUE ? WBE_TRUE : WBE_FALSE) == WBE_FALSE)
    return PBT_FALSE;

  ggt_win->ggt = ggt;

  wbe_pbw_size_limits(&(ggt_win->pb_win),
                      minw, minh, maxw, maxh);

  if (resizeable == PBT_TRUE)
    wbe_pbw_add_resize_cb(&(ggt_win->pb_win),
                          pbt_ggt_win_resize_cb,
                          ggt_win);

  pbt_ggt_win_update_area(ggt_win);


  /* ggt_win->evh.win = ggt_win->pb_win.win_be; */

  return PBT_TRUE;
}

pbt_bool_t _pbt_ggt_win_init(pbt_ggt_win_t *ggt_win,
                             const char *name,
                             pbt_ggt_t *ggt,
                             unsigned int width,
                             unsigned int height,
                             pbt_bool_t resizeable)
{
  if (_pbt_ggt_win_init_w_ggt(ggt_win,
                              name,
                              ggt,
                              width,
                              height,
                              resizeable) == PBT_FALSE)
    return PBT_FALSE;
  pbt_ggt_win_init_child_ev(ggt_win, ggt_win->ggt->childs);
  return PBT_TRUE;
}

pbt_bool_t _pbt_wgt_win_init(pbt_ggt_win_t *ggt_win,
                             const char *name,
                             pbt_wgt_t *root_wgt,
                             unsigned int width,
                             unsigned int height,
                             pbt_bool_t resizeable)
{
  if (_pbt_ggt_win_init_w_ggt(ggt_win,
                          name,
                          &(root_wgt->ggt),
                          width,
                          height,
                          resizeable) == PBT_FALSE)
    return PBT_FALSE;
  _pbt_wgt_init_ev(root_wgt, ggt_win);
  pbt_ggt_win_init_child_ev(ggt_win, ggt_win->ggt->childs);
  return PBT_TRUE;
}
