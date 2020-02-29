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

#ifndef __PBT_PIXEL_BUFFER_H_INCLUDED__
#define __PBT_PIXEL_BUFFER_H_INCLUDED__

#include "pbt_pixel_buffer_inc.h"

#include <stdlib.h>             /* free */

void pbt_pixbuf_init(pbt_pixbuf_t *pixbuf,
                     unsigned int width,
                     unsigned int height);

void pbt_pixbuf_resize(pbt_pixbuf_t *pixbuf,
                       unsigned int width,
                       unsigned int height);

#define pbt_pixbuf_destroy(pixbuf) free((pixbuf)->pixels)

void pbt_pixbuf_fill(pbt_pixbuf_t *pixbuf, unsigned char *color);

void pbt_pixbuf_fillrect(pbt_pixbuf_t *pixbuf,
                         unsigned int xpos,
                         unsigned int ypos,
                         unsigned int width,
                         unsigned int height,
                         unsigned char *color);

void pbt_pixbuf_put_pxl(pbt_pixbuf_t *pixbuf,
                        unsigned int xpos,
                        unsigned int ypos,
                        unsigned char *color);

void pbt_pixbuf_get_pxl(pbt_pixbuf_t *pixbuf,
                        unsigned int xpos,
                        unsigned int ypos,
                        unsigned char *color);

void pbt_pixbuf_put_hline(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          unsigned int size,
                          unsigned char *color);

void pbt_pixbuf_put_vline(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          unsigned int size,
                          unsigned char *color);

void pbt_pixbuf_put_rect(pbt_pixbuf_t *pixbuf,
                         unsigned int xpos,
                         unsigned int ypos,
                         unsigned int width,
                         unsigned int height,
                         unsigned char *color);

void pbt_pixbuf_copy_pixbuf(pbt_pixbuf_t *pixbuf_dst,
                            unsigned int xpos_dst,
                            unsigned int ypos_dst,
                            pbt_pixbuf_t *pixbuf_src,
                            unsigned int xpos_src,
                            unsigned int ypos_src,
                            unsigned int width_src,
                            unsigned int height_src);

/* rename setup -> init */
void pbt_pbarea_setup_from_buffer(pbt_pbarea_t *pbarea,
                                  pbt_pixbuf_t *pixbuf,
                                  unsigned int xpos,
                                  unsigned int ypos,
                                  unsigned int width,
                                  unsigned int height);

#define pbt_pbarea_setup_from_area(_pbarea,                     \
                                   _from_area,                  \
                                   _xpos,                       \
                                   _ypos,                       \
                                   _width,                      \
                                   _height)                     \
  pbt_pbarea_setup_from_buffer((_pbarea),                       \
                               (_from_area)->pixbuf,            \
                               (_xpos) + (_from_area)->xpos,    \
                               (_ypos) + (_from_area)->ypos,    \
                               (_width),                        \
                               (_height))

void pbt_pbarea_resize(pbt_pbarea_t *pbarea,
                       unsigned int xpos,
                       unsigned int ypos,
                       unsigned int width,
                       unsigned int height);

#define pbt_pbarea_fill(_pbarea, _color)        \
  pbt_pixbuf_fillrect((_pbarea)->pixbuf,        \
                      (_pbarea)->xpos,          \
                      (_pbarea)->ypos,          \
                      (_pbarea)->width,         \
                      (_pbarea)->height,        \
                      _color)

#define pbt_pbarea_fillrect(_pbarea, _xarg, _yarg, _width, _height, _color) \
  pbt_pixbuf_fillrect((_pbarea)->pixbuf,                                \
                      (_pbarea)->xpos + (_xarg),                        \
                      (_pbarea)->ypos + (_yarg),                        \
                      (_width),                                         \
                      (_height),                                        \
                      (_color))

#define pbt_pbarea_put_pxl(_pbarea, _xarg, _yarg, _color)       \
  pbt_pixbuf_put_pxl((_pbarea)->pixbuf,                         \
                     (_pbarea)->xpos + (_xarg),                 \
                     (_pbarea)->ypos + (_yarg),                 \
                     (_color))

#define pbt_pbarea_get_pxl(_pbarea, _xarg, _yarg, _color)       \
  pbt_pixbuf_get_pxl((_pbarea)->pixbuf,                         \
                     (_pbarea)->xpos + (_xarg),                 \
                     (_pbarea)->ypos + (_yarg),                 \
                     (_color))

#define pbt_pbarea_put_hline(_pbarea, _xarg, _yarg, _size, _color)      \
  pbt_pixbuf_put_hline((_pbarea)->pixbuf,                               \
                       (_pbarea)->xpos + (_xarg),                       \
                       (_pbarea)->ypos + (_yarg),                       \
                       (_size),                                         \
                       (_color))

#define pbt_pbarea_put_vline(_pbarea, _xarg, _yarg, _size, _color)      \
  pbt_pixbuf_put_vline((_pbarea)->pixbuf,                               \
                       (_pbarea)->xpos + (_xarg),                       \
                       (_pbarea)->ypos + (_yarg),                       \
                       (_size),                                         \
                       (_color))

#define pbt_pbarea_put_rect(_pbarea, _xarg, _yarg, _width, _height, _color) \
  pbt_pixbuf_put_rect((_pbarea)->pixbuf,                                \
                      (_pbarea)->xpos + (_xarg),                        \
                      (_pbarea)->ypos + (_yarg),                        \
                      (_width),                                         \
                      (_height),                                        \
                      (_color))

#define pbt_pixbuf_copy_pbarea(_pixbuf,                         \
                               _xpixbuf, _ypixbuf,              \
                               _area,                           \
                               _xarea, _yarea,                  \
                               _width_area, _height_area)       \
  pbt_pixbuf_copy_pixbuf((_pixbuf),                             \
                         (_xpixbuf),                            \
                         (_ypixbuf),                            \
                         (_area)->pixbuf,                       \
                         (_area)->xpos + (_xarea),              \
                         (_area)->ypos + (_yarea),              \
                         (_width_area),                         \
                         (_height_area))

#define pbt_pbarea_copy_pixbuf(_area,                           \
                               _xarea, _yarea,                  \
                               _pixbuf,                         \
                               _xpixbuf, _ypixbuf,              \
                               _width_pixbuf, _height_pixbuf)   \
  pbt_pixbuf_copy_pixbuf((_area)->pixbuf,                       \
                         (_area)->xpos + (_xarea),              \
                         (_area)->ypos + (_yarea),              \
                         (_pixbuf),                             \
                         (_xpixbuf),                            \
                         (_ypixbuf),                            \
                         (_width_pixbuf),                       \
                         (_height_pixbuf))

#define pbt_pbarea_copy_pbarea(_dst,                    \
                               _xdst, _ydst,            \
                               _src,                    \
                               _xsrc, _ysrc,            \
                               _width_src, _height_src) \
  pbt_pixbuf_copy_pixbuf((_dst),                        \
                         (_dst)->xpos + (_xdst),        \
                         (_dst)->ypos + (_ydst),        \
                         (_src)->pixbuf,                \
                         (_src)->xpos + (_xsrc),        \
                         (_src)->ypos + (_ysrc),        \
                         (_width_src),                  \
                         (_height_src))

#endif /* __PBT_PIXEL_BUFFER_H_INCLUDED__ */
