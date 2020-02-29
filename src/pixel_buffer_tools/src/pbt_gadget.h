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

#pragma once

#include "pbt_gadget_inc.h"

void _pbt_ggt_nodes_destroy(pbt_ggt_node_t *node);

pbt_ggt_node_t *pbt_ggt_node_pop(pbt_ggt_t *parent_ggt, pbt_ggt_node_t *node);

void pbt_ggt_drawarea_init(pbt_ggt_drawarea_t *drawarea,
                           unsigned int min_width,
                           unsigned int max_width,
                           unsigned int min_height,
                           unsigned int max_height,
                           pbt_draw_cb_t drawarea_cb,
                           void *drawarea_arg,
                           pbt_draw_update_area_cb_t update_area_cb,
                           void *update_area_arg);

void pbt_ggt_destroy_child_del_node(pbt_ggt_ctnr_t *ctnr, pbt_ggt_node_t *node);

pbt_ggt_node_t *_pbt_ggt_add_child_ggt_type(pbt_ggt_t *parent,
                                            pbt_ggt_t *child,
                                            pbt_ggt_node_type_t type);

#define _pbt_ggt_add_child_ggt(_ctnr_ggt, _child_ggt)                   \
  _pbt_ggt_add_child_ggt_type(&((_ctnr_ggt)->ggt), (_child_ggt), GADGET)

#define pbt_ggt_add_child_ggt(_ctnr, _child)            \
  _pbt_ggt_add_child_ggt((_ctnr), &((_child)->ggt))

void pbt_ggt_ctnr_add_separator(pbt_ggt_ctnr_t *ctnr_ggt,
                                unsigned int min,
                                unsigned int max,
                                unsigned char *color);

#define pbt_ggt_ctnr_add_static_separator(_ctnr, _size, _color)         \
  pbt_ggt_ctnr_add_separator((_ctnr), (_size), (_size), (_color))

#define pbt_ggt_ctnr_add_line(_ctnr, _color)                    \
  pbt_ggt_ctnr_add_static_separator((_ctnr), 1, (_color))

#define pbt_ggt_ctnr_add_empty(_ctnr, _color)                   \
  pbt_ggt_ctnr_add_static_separator((_ctnr), 0, (_color))

void pbt_ggt_hctnr_init(pbt_ggt_ctnr_t *ctnr);

void pbt_ggt_vctnr_init(pbt_ggt_ctnr_t *ctnr);

void pbt_ggt_node_it_del(pbt_ggt_node_it_t *node_it);

void pbt_ggt_node_it_init_ggt_add_child(pbt_ggt_node_it_t *node_it,
                                        pbt_ggt_t *parent_ggt,
                                        pbt_ggt_t *ggt,
                                        pbt_ggt_node_type_t node_type);

void pbt_ggt_wrapper_init(pbt_ggt_t *ggt,
                          void *addr,
                          pbt_ggt_t *child,
                          pbt_ggt_node_type_t type);

#define pbt_ggt_min_width(_ggt_struct)                          \
  (_ggt_struct)->ggt.get_min_width(&((_ggt_struct)->ggt))

#define pbt_ggt_max_width(_ggt_struct)                          \
  (_ggt_struct)->ggt.get_max_width(&((_ggt_struct)->ggt))

#define pbt_ggt_min_height(_ggt_struct)                         \
  (_ggt_struct)->ggt.get_min_height(&((_ggt_struct)->ggt))

#define pbt_ggt_max_height(_ggt_struct)                         \
  (_ggt_struct)->ggt.get_max_height(&((_ggt_struct)->ggt))

#define pbt_ggt_xpos(_ggt_struct)               \
  (_ggt_struct)->ggt.pbarea.xpos

#define pbt_ggt_ypos(_ggt_struct)               \
  (_ggt_struct)->ggt.pbarea.ypos

#define pbt_ggt_width(_ggt_struct)              \
  (_ggt_struct)->ggt.pbarea.width

#define pbt_ggt_height(_ggt_struct)             \
  (_ggt_struct)->ggt.pbarea.height

#define pbt_ggt_draw(_ggt_struct)                       \
  (_ggt_struct)->ggt.draw_cb(&((_ggt_struct)->ggt))

#define pbt_ggt_update_area(_ggt_struct, _pbarea)                       \
  (_ggt_struct)->ggt.update_area_cb(&((_ggt_struct)->ggt), (_pbarea))

#define pbt_ggt_destroy(_ggt_struct)                    \
  (_ggt_struct)->ggt.destroy_cb(&((_ggt_struct)->ggt))

#define PBT_IS_IN_GGT(_ggt_struct, _xpos, _ypos)                        \
  PBT_IS_IN_IMG_AREA(&((_ggt_struct)->ggt.pbarea), _xpos, _ypos)

  unsigned int pbt_ggt_wrapper_get_min_width(pbt_ggt_t *ggt);


unsigned int pbt_ggt_wrapper_get_min_width(pbt_ggt_t *ggt);

unsigned int pbt_ggt_wrapper_get_max_width(pbt_ggt_t *ggt);

unsigned int pbt_ggt_wrapper_get_min_height(pbt_ggt_t *ggt);

unsigned int pbt_ggt_wrapper_get_max_height(pbt_ggt_t *ggt);

void pbt_ggt_wrapper_draw(pbt_ggt_t *ggt);

void pbt_ggt_wrapper_update_area(pbt_ggt_t *ggt, pbt_pbarea_t *pbarea);

void pbt_ggt_wrapper_destroy(pbt_ggt_t *ggt);
