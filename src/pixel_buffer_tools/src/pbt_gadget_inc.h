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

#include "pbt_pixel_buffer.h"

typedef enum
  {
   GADGET,
   WIDGET,
   SEPARATOR,
  } pbt_ggt_node_type_t;

typedef struct
{
  unsigned int min, max, alt_min, alt_max;
  unsigned char *color;
} pbt_ggt_ctnr_separator_t;

typedef struct pbt_node_s
{
  pbt_ggt_node_type_t type;
  unsigned int pos;
  unsigned int size;
  union
  {
    void *ggt_addr;
    pbt_ggt_ctnr_separator_t separator;
  } priv;
  struct pbt_node_s *next;
} pbt_ggt_node_t;

typedef struct pbt_ggt_st
{
  pbt_pbarea_t pbarea;
  void *priv;

  unsigned int (*get_min_width)(struct pbt_ggt_st *ggt);  /* mandatory */
  unsigned int (*get_max_width)(struct pbt_ggt_st *ggt);  /* mandatory */
  unsigned int (*get_min_height)(struct pbt_ggt_st *ggt); /* mandatory */
  unsigned int (*get_max_height)(struct pbt_ggt_st *ggt); /* mandatory */
  void (*update_area_cb)(struct pbt_ggt_st *ggt,
                         pbt_pbarea_t *pbarea); /* mandatory */
  void (*draw_cb)(struct pbt_ggt_st *ggt);        /* mandatory */
  void (*destroy_cb)(struct pbt_ggt_st *ggt);     /* Optional */

  pbt_ggt_node_t        *childs;        /* Optional */
} pbt_ggt_t;

typedef void (*pbt_draw_cb_t)(pbt_pbarea_t *pbarea, void *arg);

typedef void (*pbt_draw_update_area_cb_t)(pbt_pbarea_t *pbarea,
                                          void *arg);

typedef struct
{
  unsigned int              min_width, max_width, min_height, max_height;
  pbt_draw_cb_t             draw_cb;
  void                      *draw_arg;
  pbt_draw_update_area_cb_t update_area_cb;
  void                      *update_area_arg;
  pbt_ggt_t                 ggt;
} pbt_ggt_drawarea_t;

typedef void (*pbt_ggt_ctnr_draw_separator_cb_t)(pbt_pbarea_t *pbarea,
                                                 unsigned int pos,
                                                 unsigned int size,
                                                 unsigned char *color);

typedef struct
{
  pbt_ggt_ctnr_draw_separator_cb_t draw_separator;
  pbt_ggt_t                        ggt;
} pbt_ggt_ctnr_t;

typedef struct
{
  pbt_ggt_t *parent_ggt;
  pbt_ggt_node_t *node;
} pbt_ggt_node_it_t;

#define _pbt_ggt_min_width(_ggt) (_ggt)->get_min_width((_ggt))

#define _pbt_ggt_max_width(_ggt) (_ggt)->get_max_width((_ggt))

#define _pbt_ggt_min_height(_ggt) (_ggt)->get_min_height((_ggt))

#define _pbt_ggt_max_height(_ggt) (_ggt)->get_max_height((_ggt))

#define _pbt_ggt_xpos(_ggt) (_ggt)->pbarea.xpos

#define _pbt_ggt_ypos(_ggt) (_ggt)->pbarea.ypos

#define _pbt_ggt_width(_ggt) (_ggt)->pbarea.width

#define _pbt_ggt_height(_ggt) (_ggt)->pbarea.height

#define _pbt_ggt_draw(_ggt)                     \
  (_ggt)->draw_cb((_ggt))

#define _pbt_ggt_update_area(_ggt, _pbarea)     \
  (_ggt)->update_area_cb((_ggt), (_pbarea))

#define _pbt_ggt_destroy(_ggt) (_ggt)->destroy_cb((_ggt))

#define _PBT_IS_IN_GGT(_ggt, _xpos, _ypos)              \
  PBT_IS_IN_IMG_AREA(&((_ggt)->pbarea), _xpos, _ypos)
