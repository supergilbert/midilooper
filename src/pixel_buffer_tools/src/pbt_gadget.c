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

/* Need factorisations */

#include <stdlib.h>
#include <string.h>

#include "pbt_gadget_inc.h"
#include "pbt_tools.h"

unsigned int pbt_ggt_drawarea_get_min_width(pbt_ggt_t *ggt)
{
  pbt_ggt_drawarea_t *drawarea = ggt->priv;

  return drawarea->min_width;
}

unsigned int pbt_ggt_drawarea_get_max_width(pbt_ggt_t *ggt)
{
  pbt_ggt_drawarea_t *drawarea = ggt->priv;

  return drawarea->max_width;
}

unsigned int pbt_ggt_drawarea_get_min_height(pbt_ggt_t *ggt)
{
  pbt_ggt_drawarea_t *drawarea = ggt->priv;

  return drawarea->min_height;
}

unsigned int pbt_ggt_drawarea_get_max_height(pbt_ggt_t *ggt)
{
  pbt_ggt_drawarea_t *drawarea = ggt->priv;

  return drawarea->max_height;
}

void pbt_ggt_drawarea_draw(pbt_ggt_t *ggt)
{
  pbt_ggt_drawarea_t *drawarea = ggt->priv;

  drawarea->draw_cb(&(ggt->pbarea),
                   drawarea->draw_arg);
}

void pbt_ggt_drawarea_update_area(pbt_ggt_t *ggt,
                                  pbt_pbarea_t *pbarea)
{
  pbt_ggt_drawarea_t *drawarea = ggt->priv;
  pbt_bool_t area_updated = PBT_FALSE;

  if ((drawarea->update_area_cb != NULL)
      && ((pbarea->width != ggt->pbarea.width)
          || (pbarea->height != ggt->pbarea.height)))
    area_updated = PBT_TRUE;

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));

  if (area_updated == PBT_TRUE)
    drawarea->update_area_cb(pbarea,
                            drawarea->update_area_arg);
}

void pbt_ggt_drawarea_init(pbt_ggt_drawarea_t *drawarea,
                           unsigned int min_width,
                           unsigned int max_width,
                           unsigned int min_height,
                           unsigned int max_height,
                           pbt_draw_cb_t draw_cb,
                           void *draw_arg,
                           pbt_draw_update_area_cb_t update_area_cb,
                           void *update_area_arg)
{
  drawarea->min_width = min_width;
  drawarea->max_width = max_width;
  drawarea->min_height = min_height;
  drawarea->max_height = max_height;
  drawarea->draw_cb = draw_cb;
  drawarea->draw_arg = draw_arg;
  drawarea->update_area_cb = update_area_cb;
  drawarea->update_area_arg = update_area_arg;

  drawarea->ggt.priv = drawarea;
  drawarea->ggt.get_min_width = pbt_ggt_drawarea_get_min_width;
  drawarea->ggt.get_max_width = pbt_ggt_drawarea_get_max_width;
  drawarea->ggt.get_min_height = pbt_ggt_drawarea_get_min_height;
  drawarea->ggt.get_max_height = pbt_ggt_drawarea_get_max_height;
  drawarea->ggt.update_area_cb = pbt_ggt_drawarea_update_area;
  drawarea->ggt.draw_cb = pbt_ggt_drawarea_draw;
}

pbt_ggt_node_t *pbt_ggt_node_pop(pbt_ggt_t *parent_ggt, pbt_ggt_node_t *node)
{
  pbt_ggt_node_t *tmp;

  if (parent_ggt->childs == NULL)
    return NULL;

  tmp = parent_ggt->childs;

  if (tmp == node)
    {
      parent_ggt->childs = tmp->next;
      return node;
    }

  if (tmp->next == NULL)
    return NULL;

  do {
    if (tmp->next == node)
      {
        tmp->next = tmp->next->next;
        return node;
      }
    tmp = tmp->next;
  } while (tmp != NULL);

  return NULL;
}

void _pbt_ggt_nodes_destroy(pbt_ggt_node_t *node)
{
  pbt_ggt_t *ggt;
  pbt_ggt_node_t *node_to_free;

  while (node != NULL)
    {
      switch (node->type)
        {
        case GADGET:
        case WIDGET:
          ggt = node->priv.ggt_addr;
          if (ggt->destroy_cb != NULL)
            _pbt_ggt_destroy(ggt);
          else
            _pbt_ggt_nodes_destroy(ggt->childs);
          break;
        case SEPARATOR:
          break;
        default:
          pbt_abort("Unknown container node type (%d)", node->type);
        }
      node_to_free = node;
      node = node->next;
      free(node_to_free);
    }
}

void pbt_ggt_ctnr_destroy(pbt_ggt_t *ggt_ctnr)
{
  _pbt_ggt_nodes_destroy(ggt_ctnr->childs);
}

void pbt_ggt_destroy_child_node(pbt_ggt_t *parent_ggt, pbt_ggt_node_t *node)
{
  pbt_ggt_node_t *node_res = pbt_ggt_node_pop(parent_ggt, node);
  pbt_ggt_t *ggt;

  if (node_res == NULL)
    pbt_abort("Unable to locate the specified node.\n");

  if (node->type != SEPARATOR)
    {
      ggt = node->priv.ggt_addr;
      if (ggt->destroy_cb != NULL)
        _pbt_ggt_destroy(ggt);
      else
        _pbt_ggt_nodes_destroy(ggt->childs);
    }

  free(node);
}

void pbt_ggt_ctnr_add_node(pbt_ggt_t *ggt, pbt_ggt_node_t *new_node)
{
  pbt_ggt_node_t *tmp;

  if (ggt->childs == NULL)
    ggt->childs = new_node;
  else
    {
      tmp = ggt->childs;
      while (tmp != NULL)
        {
          if (tmp->next == NULL)
            {
              tmp->next = new_node;
              break;
            }
          tmp = tmp->next;
        }
    }
}

pbt_ggt_node_t *_pbt_ggt_add_child_ggt_type(pbt_ggt_t *parent,
                                            pbt_ggt_t *child,
                                            pbt_ggt_node_type_t type)
{
  pbt_ggt_node_t *new_node;

  new_node = malloc(sizeof (pbt_ggt_node_t));
  new_node->type = type;
  new_node->size = 0;
  new_node->priv.ggt_addr = child;
  new_node->next = NULL;

  pbt_ggt_ctnr_add_node(parent, new_node);
  return new_node;
}

void _pbt_ggt_ctnr_add_separator(pbt_ggt_ctnr_t *ctnr,
                                 unsigned int min,
                                 unsigned int max,
                                 unsigned int alt_min,
                                 unsigned int alt_max,
                                 unsigned char *color)
{
  pbt_ggt_node_t *new_node;

  new_node = malloc(sizeof (pbt_ggt_node_t));
  new_node->type = SEPARATOR;
  new_node->priv.separator.min = min;
  new_node->priv.separator.max = max;
  new_node->priv.separator.alt_min = alt_min;
  new_node->priv.separator.alt_max = alt_max;
  new_node->priv.separator.color = color;
  new_node->next = NULL;

  pbt_ggt_ctnr_add_node(&(ctnr->ggt), new_node);
}

void pbt_ggt_ctnr_add_separator(pbt_ggt_ctnr_t *ctnr,
                                unsigned int min,
                                unsigned int max,
                                unsigned char *color)
{
  _pbt_ggt_ctnr_add_separator(ctnr, min, max, 0, 0, color);
}

void pbt_ggt_ctnr_draw(pbt_ggt_t *ggt_ctnr)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp = ggt_ctnr->childs;
  pbt_ggt_ctnr_t *ctnr = ggt_ctnr->priv;

  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          _pbt_ggt_draw(tmp_ggt);
          break;
        case SEPARATOR:
          ctnr->draw_separator(&(ggt_ctnr->pbarea),
                               tmp->pos,
                               tmp->size,
                               tmp->priv.separator.color);
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      tmp = tmp->next;
    }
}

unsigned int pbt_ggt_hctnr_get_min_width(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_width, ret_min_width = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_width = _pbt_ggt_min_width(tmp_ggt);
          if (tmp_width == 0)
            ret_min_width += 1;
          else
            ret_min_width += tmp_width;
          break;
        case SEPARATOR:
          ret_min_width += tmp->priv.separator.min;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      tmp = tmp->next;
    }
  return ret_min_width;
}

unsigned int pbt_ggt_hctnr_get_max_width(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_width, ret_max_width = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_width = _pbt_ggt_max_width(tmp_ggt);
          break;
        case SEPARATOR:
          tmp_width = tmp->priv.separator.max;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      if (tmp_width == 0)
        return 0;
      ret_max_width += tmp_width;
      tmp = tmp->next;
    }
  return ret_max_width;
}

unsigned int pbt_ggt_hctnr_get_min_height(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_height, ret_min_height = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_height = _pbt_ggt_min_height(tmp_ggt);
          break;
        case SEPARATOR:
          tmp_height = tmp->priv.separator.alt_min;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      if (ret_min_height < tmp_height)
        ret_min_height = tmp_height;
      tmp = tmp->next;
    }
  if (ret_min_height == 0)
    return 1;
  return ret_min_height;
}

unsigned int pbt_ggt_hctnr_get_max_height(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_height, ret_max_height = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_height = _pbt_ggt_max_height(tmp_ggt);
          break;
        case SEPARATOR:
          tmp_height = tmp->priv.separator.alt_max;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      if (tmp_height != 0
          && (ret_max_height == 0 || tmp_height < ret_max_height))
        ret_max_height = tmp_height;
      tmp = tmp->next;
    }
  return ret_max_height;
}

unsigned int pbt_ggt_vctnr_get_min_width(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_width, ret_min_width = 0;

  for (tmp = ctnr_ggt->childs;
       tmp != NULL;
       tmp = tmp->next)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_width = _pbt_ggt_min_width(tmp_ggt);
          break;
        case SEPARATOR:
          tmp_width = tmp->priv.separator.alt_min;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      if (ret_min_width < tmp_width)
        ret_min_width = tmp_width;
    }
  if (ret_min_width == 0)
    return 1;
  return ret_min_width;
}

unsigned int pbt_ggt_vctnr_get_max_width(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_width, ret_max_width = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_width = _pbt_ggt_max_width(tmp_ggt);
          break;
        case SEPARATOR:
          tmp_width = tmp->priv.separator.alt_max;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      if (tmp_width != 0
          && (ret_max_width == 0 || tmp_width < ret_max_width))
        ret_max_width = tmp_width;
      tmp = tmp->next;
    }
  return ret_max_width;
}

unsigned int pbt_ggt_vctnr_get_min_height(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_height, ret_min_height = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_height = _pbt_ggt_min_height(tmp_ggt);
          if (tmp_height == 0)
            ret_min_height += 1;
          else
            ret_min_height += tmp_height;
          break;
        case SEPARATOR:
          ret_min_height += tmp->priv.separator.min;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      tmp = tmp->next;
    }
  return ret_min_height;
}

unsigned int pbt_ggt_vctnr_get_max_height(pbt_ggt_t *ctnr_ggt)
{
  pbt_ggt_t *tmp_ggt;
  pbt_ggt_node_t *tmp;
  unsigned int tmp_height, ret_max_height = 0;

  tmp = ctnr_ggt->childs;
  while (tmp != NULL)
    {
      switch (tmp->type)
        {
        case GADGET:
        case WIDGET:
          tmp_ggt = tmp->priv.ggt_addr;
          tmp_height = _pbt_ggt_max_height(tmp_ggt);
          break;
        case SEPARATOR:
          tmp_height = tmp->priv.separator.max;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", tmp->type);
        }
      if (tmp_height == 0)
        return 0;
      ret_max_height += tmp_height;
      tmp = tmp->next;
    }
  return ret_max_height;
}

typedef unsigned int (*_pbt_ggt_size_t)(pbt_ggt_t *ggt);

unsigned int pbt_ggt_hctnr_get_ggt_min(pbt_ggt_t *ggt)
{
  return _pbt_ggt_min_width(ggt);
}

unsigned int pbt_ggt_hctnr_get_ggt_max(pbt_ggt_t *ggt)
{
  return _pbt_ggt_max_width(ggt);
}

unsigned int pbt_ggt_vctnr_get_ggt_min(pbt_ggt_t *ggt)
{
  return _pbt_ggt_min_height(ggt);
}

unsigned int pbt_ggt_vctnr_get_ggt_max(pbt_ggt_t *ggt)
{
  return _pbt_ggt_max_height(ggt);
}

void pbt_ggt_ctnr_update_size(pbt_ggt_node_t *node_head,
                              _pbt_ggt_size_t get_min,
                              _pbt_ggt_size_t get_max,
                              const unsigned int total_size)
{
  pbt_ggt_t *ggt;
  pbt_ggt_node_t *ctnr_node, *end_node;
  unsigned int current_size, added_size, remaining_size, tmp_size;

  /* First check if size doesn't exceed */
  current_size = 0;
  for (ctnr_node = node_head;
       ctnr_node != NULL;
       ctnr_node = ctnr_node->next)
    {
      ctnr_node->pos = current_size;
      switch (ctnr_node->type)
        {
        case GADGET:
        case WIDGET:
          ggt = ctnr_node->priv.ggt_addr;
          ctnr_node->size = get_min(ggt);
          /* Size of ggt can not be 0 (minsize 0 <=> minsize 1) */
          if (ctnr_node->size == 0)
            ctnr_node->size = 1;
          break;
        case SEPARATOR:
          ctnr_node->size = ctnr_node->priv.separator.min;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", ctnr_node->type);
        }
      current_size += ctnr_node->size;
    }

  if (current_size > total_size)
    pbt_abort("Minimum size (%d) is superior to total size (%d)",
              current_size,
              total_size);

  /* ??? */
  if (current_size == total_size)
    return;

  remaining_size = total_size;

  /* Total size is superior to nodes minimum size */
  end_node = NULL;
  added_size = 0;
  while (remaining_size != 0)
    {
      if (end_node == node_head)
        pbt_abort("Unable to fill total size");

      /* Searching for the last node (that is not added) and recalculate the
         size to it. */
      for (current_size = 0,
             ctnr_node = node_head;
           ctnr_node->next != end_node;
           ctnr_node = ctnr_node->next)
        {
          ctnr_node->pos = current_size;
          switch (ctnr_node->type)
            {
            case GADGET:
            case WIDGET:
              ggt = ctnr_node->priv.ggt_addr;
              tmp_size = get_min(ggt);
              /* Bis (Size of ggt can not be 0) */
              if (tmp_size == 0)
                tmp_size = 1;
              break;
            case SEPARATOR:
              tmp_size = ctnr_node->priv.separator.min;
              break;
            default:
              pbt_abort("Unknown container node type (%d)", ctnr_node->type);
            }
          current_size += tmp_size;
        }

      /* Work on last node (that is not added) */
      switch (ctnr_node->type)
        {
        case GADGET:
        case WIDGET:
          ggt = ctnr_node->priv.ggt_addr;
          tmp_size = get_max(ggt);
          break;
        case SEPARATOR:
          tmp_size = ctnr_node->priv.separator.max;
          break;
        default:
          pbt_abort("Unknown container node type (%d)", ctnr_node->type);
        }
      if ((tmp_size == 0)
          || (current_size + tmp_size + added_size > total_size))
        {
          ctnr_node->pos = current_size;
          ctnr_node->size = total_size - (added_size + current_size);
          remaining_size = 0;
        }
      else if (current_size + tmp_size + added_size == total_size)
        {
          ctnr_node->pos = current_size;
          ctnr_node->size = tmp_size;
          remaining_size = 0;
        }
      else                      /* Size doesn't fill the area */
        {
          added_size += tmp_size;
          ctnr_node->size = tmp_size;
          ctnr_node->pos = total_size - added_size;
          remaining_size = total_size - (current_size + added_size);
          end_node = ctnr_node;
        }
    }
}

void pbt_ggt_hctnr_ggt_update_area(pbt_ggt_node_t *ctnr_node,
                                   pbt_pbarea_t *pbarea)
{
  pbt_pbarea_t tmp_area;
  unsigned int current_pos = 0;
  pbt_ggt_t *ggt;

  while (ctnr_node != NULL)
    {
      switch (ctnr_node->type)
        {
        case GADGET:
        case WIDGET:
          ggt = ctnr_node->priv.ggt_addr;
          pbt_pbarea_setup_from_area(&tmp_area, pbarea,
                                     current_pos, 0,
                                     ctnr_node->size, pbarea->height);
          _pbt_ggt_update_area(ggt, &tmp_area);
          break;
        case SEPARATOR:
          break;
        default:
          pbt_abort("Unknown container node type (%d)", ctnr_node->type);
        }
      current_pos += ctnr_node->size;
      ctnr_node = ctnr_node->next;
    }
}

void pbt_ggt_vctnr_ggt_update_area(pbt_ggt_node_t *ctnr_node,
                                   pbt_pbarea_t *pbarea)
{
  pbt_pbarea_t tmp_area;
  unsigned int current_pos = 0;
  pbt_ggt_t *ggt;

  while (ctnr_node != NULL)
    {
      switch (ctnr_node->type)
        {
        case GADGET:
        case WIDGET:
          ggt = ctnr_node->priv.ggt_addr;
          pbt_pbarea_setup_from_area(&tmp_area, pbarea,
                                     0, current_pos,
                                     pbarea->width, ctnr_node->size);
          _pbt_ggt_update_area(ggt, &tmp_area);
        case SEPARATOR:
          break;
        default:
          pbt_abort("Unknown container node type (%d)", ctnr_node->type);
        }
      current_pos += ctnr_node->size;
      ctnr_node = ctnr_node->next;
    }
}

void pbt_ggt_hctnr_update_area(pbt_ggt_t *ggt_ctnr,
                               pbt_pbarea_t *pbarea)
{
  pbt_ggt_ctnr_update_size(ggt_ctnr->childs,
                           pbt_ggt_hctnr_get_ggt_min,
                           pbt_ggt_hctnr_get_ggt_max,
                           pbarea->width);

  memcpy(&(ggt_ctnr->pbarea), pbarea, sizeof (pbt_pbarea_t));

  pbt_ggt_hctnr_ggt_update_area(ggt_ctnr->childs, pbarea);
}

void pbt_ggt_vctnr_update_area(pbt_ggt_t *ggt_ctnr,
                               pbt_pbarea_t *pbarea)
{
  pbt_ggt_ctnr_update_size(ggt_ctnr->childs,
                           pbt_ggt_vctnr_get_ggt_min,
                           pbt_ggt_vctnr_get_ggt_max,
                           pbarea->height);

  memcpy(&(ggt_ctnr->pbarea), pbarea, sizeof (pbt_pbarea_t));

  pbt_ggt_vctnr_ggt_update_area(ggt_ctnr->childs, pbarea);
}

void pbt_ggt_hctnr_draw_separator(pbt_pbarea_t *pbarea,
                                  unsigned int pos,
                                  unsigned int size,
                                  unsigned char *color)
{
  pbt_pbarea_fillrect(pbarea, pos, 0, size, pbarea->height, color);
}

void pbt_ggt_hctnr_init(pbt_ggt_ctnr_t *ctnr)
{
  ctnr->draw_separator = pbt_ggt_hctnr_draw_separator;

  ctnr->ggt.priv = ctnr;
  ctnr->ggt.get_min_width = pbt_ggt_hctnr_get_min_width;
  ctnr->ggt.get_max_width = pbt_ggt_hctnr_get_max_width;
  ctnr->ggt.get_min_height = pbt_ggt_hctnr_get_min_height;
  ctnr->ggt.get_max_height = pbt_ggt_hctnr_get_max_height;
  ctnr->ggt.draw_cb = pbt_ggt_ctnr_draw;
  ctnr->ggt.update_area_cb = pbt_ggt_hctnr_update_area;
  ctnr->ggt.destroy_cb = pbt_ggt_ctnr_destroy;
}

void pbt_ggt_vctnr_draw_separator(pbt_pbarea_t *pbarea,
                                  unsigned int pos,
                                  unsigned int size,
                                  unsigned char *color)
{
  pbt_pbarea_fillrect(pbarea, 0, pos, pbarea->width, size, color);
}

void pbt_ggt_vctnr_init(pbt_ggt_ctnr_t *ctnr)
{
  ctnr->draw_separator = pbt_ggt_vctnr_draw_separator;

  ctnr->ggt.priv = ctnr;
  ctnr->ggt.get_min_width = pbt_ggt_vctnr_get_min_width;
  ctnr->ggt.get_max_width = pbt_ggt_vctnr_get_max_width;
  ctnr->ggt.get_min_height = pbt_ggt_vctnr_get_min_height;
  ctnr->ggt.get_max_height = pbt_ggt_vctnr_get_max_height;
  ctnr->ggt.draw_cb = pbt_ggt_ctnr_draw;
  ctnr->ggt.update_area_cb = pbt_ggt_vctnr_update_area;
  ctnr->ggt.destroy_cb = pbt_ggt_ctnr_destroy;
}


void pbt_ggt_node_it_del(pbt_ggt_node_it_t *node_it)
{
  pbt_ggt_destroy_child_node(node_it->parent_ggt, node_it->node);
}

void pbt_ggt_node_it_init_ggt_add_child(pbt_ggt_node_it_t *node_it,
                                        pbt_ggt_t *parent_ggt,
                                        pbt_ggt_t *ggt,
                                        pbt_ggt_node_type_t node_type)
{
  node_it->parent_ggt = parent_ggt;
  node_it->node = _pbt_ggt_add_child_ggt_type(parent_ggt, ggt, node_type);
}

unsigned int pbt_ggt_child_get_min_width(pbt_ggt_t *ggt)
{
  pbt_ggt_t *child = ggt->childs->priv.ggt_addr;

  return _pbt_ggt_min_width(child);
}

unsigned int pbt_ggt_child_get_max_width(pbt_ggt_t *ggt)
{
  pbt_ggt_t *child = ggt->childs->priv.ggt_addr;

  return _pbt_ggt_max_width(child);
}

unsigned int pbt_ggt_child_get_min_height(pbt_ggt_t *ggt)
{
  pbt_ggt_t *child = ggt->childs->priv.ggt_addr;

  return _pbt_ggt_min_height(child);
}

unsigned int pbt_ggt_child_get_max_height(pbt_ggt_t *ggt)
{
  pbt_ggt_t *child = ggt->childs->priv.ggt_addr;

  return _pbt_ggt_max_height(child);
}

void pbt_ggt_child_draw(pbt_ggt_t *ggt)
{
  pbt_ggt_t *child = ggt->childs->priv.ggt_addr;

  _pbt_ggt_draw(child);
}

void pbt_ggt_child_update_area(pbt_ggt_t *ggt, pbt_pbarea_t *pbarea)
{
  pbt_ggt_t *child = ggt->childs->priv.ggt_addr;

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
  _pbt_ggt_update_area(child, pbarea);
}

void pbt_ggt_child_destroy(pbt_ggt_t *ggt)
{
  _pbt_ggt_nodes_destroy(ggt->childs);
  if (ggt->priv != NULL)
    free(ggt->priv);
  free(ggt);
}

void _pbt_ggt_child_init(pbt_ggt_t *ggt,
                         void *addr,
                         pbt_ggt_t *child,
                         pbt_ggt_node_type_t type)
{
  ggt->get_min_width = pbt_ggt_child_get_min_width;
  ggt->get_max_width = pbt_ggt_child_get_max_width;
  ggt->get_min_height = pbt_ggt_child_get_min_height;
  ggt->get_max_height = pbt_ggt_child_get_max_height;
  ggt->draw_cb = pbt_ggt_child_draw;
  ggt->update_area_cb = pbt_ggt_child_update_area;
  ggt->priv = addr;
  ggt->destroy_cb = pbt_ggt_child_destroy;
  _pbt_ggt_add_child_ggt_type(ggt, child, type);
}

void _pbt_ggt_setup_ggt_child_wrapper(pbt_ggt_t *ggt,
                                      pbt_ggt_node_t *node,
                                      pbt_ggt_node_type_t type,
                                      pbt_ggt_t *child_ggt)
{
  ggt->get_min_width = pbt_ggt_child_get_min_width;
  ggt->get_max_width = pbt_ggt_child_get_max_width;
  ggt->get_min_height = pbt_ggt_child_get_min_height;
  ggt->get_max_height = pbt_ggt_child_get_max_height;
  ggt->draw_cb = pbt_ggt_child_draw;
  ggt->update_area_cb = pbt_ggt_child_update_area;
  node->type = type;
  node->priv.ggt_addr = child_ggt;
  node->next = NULL;
  ggt->childs = node;
}
