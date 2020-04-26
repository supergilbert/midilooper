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

#ifndef __PBT_DRAW_H_
#define __PBT_DRAW_H_

#include "pbt_pixel_buffer_inc.h"
#include "pbt_font_inc.h"

void pbt_pixbuf_putstr(pbt_pixbuf_t *pixbuf,
                       pbt_font_t *font,
                       unsigned char *color,
                       int xpos,
                       int ypos,
                       const char *str);

void pbt_pixbuf_printf(pbt_pixbuf_t *pixbuf,
                       pbt_font_t *font,
                       unsigned char *color,
                       int xpos,
                       int ypos,
                       const char *format, ...);

void pbt_pbarea_putstr(pbt_pbarea_t *pbarea,
                       pbt_font_t *font,
                       unsigned char *color,
                       int xpos,
                       int ypos,
                       const char *str);

void pbt_pbarea_printf(pbt_pbarea_t *pbarea,
                       pbt_font_t *font,
                       unsigned char *color,
                       int xpos,
                       int ypos,
                       const char *format, ...);

void pbt_pixbuf_draw_triangle_left(pbt_pixbuf_t *pixbuf,
                                   unsigned int xpos,
                                   unsigned int ypos,
                                   int size,
                                   unsigned char *color);

#define pbt_pbarea_draw_triangle_left(_pbarea,                  \
                                      _xpos,                    \
                                      _ypos,                    \
                                      _size,                    \
                                      _color)                   \
  (pbt_pixbuf_draw_triangle_left((_pbarea)->pixbuf,             \
                                 (_pbarea)->xpos + (_xpos),     \
                                 (_pbarea)->ypos + (_ypos),     \
                                 (_size),                       \
                                 (_color)))

  void pbt_pixbuf_draw_triangle_right(pbt_pixbuf_t *pixbuf,
                                    unsigned int xpos,
                                    unsigned int ypos,
                                    unsigned int size,
                                    unsigned char *color);

#define pbt_pbarea_draw_triangle_right(_pbarea,                 \
                                       _xpos,                   \
                                       _ypos,                   \
                                       _size,                   \
                                       _color)                  \
  (pbt_pixbuf_draw_triangle_right((_pbarea)->pixbuf,            \
                                  (_pbarea)->xpos + (_xpos),    \
                                  (_pbarea)->ypos + (_ypos),    \
                                  (_size),                      \
                                  (_color)))

void pbt_pixbuf_draw_triangle_up(pbt_pixbuf_t *pixbuf,
                                 unsigned int xpos,
                                 unsigned int ypos,
                                 int size,
                                 unsigned char *color);

#define pbt_pbarea_draw_triangle_up(_pbarea,                    \
                                    _xpos,                      \
                                    _ypos,                      \
                                    _size,                      \
                                    _color)                     \
  (pbt_pixbuf_draw_triangle_up((_pbarea)->pixbuf,               \
                               (_pbarea)->xpos + (_xpos),       \
                               (_pbarea)->ypos + (_ypos),       \
                               (_size),                         \
                               (_color)))

void pbt_pixbuf_draw_triangle_down(pbt_pixbuf_t *pixbuf,
                                   unsigned int xpos,
                                   unsigned int ypos,
                                   int size,
                                   unsigned char *color);

#define pbt_pbarea_draw_triangle_down(_pbarea,                  \
                                      _xpos,                    \
                                      _ypos,                    \
                                      _size,                    \
                                      _color)                   \
  (pbt_pixbuf_draw_triangle_down((_pbarea)->pixbuf,             \
                                 (_pbarea)->xpos + (_xpos),     \
                                 (_pbarea)->ypos + (_ypos),     \
                                 (_size),                       \
                                 (_color)))

void pbt_pixbuf_draw_hp(pbt_pixbuf_t *pixbuf,
                        unsigned int xpos,
                        unsigned int ypos,
                        int size,
                        unsigned char *color);

#define pbt_pbarea_draw_hp(_pbarea, _xpos, _ypos, _size, _color)        \
  (pbt_pixbuf_draw_hp((_pbarea)->pixbuf,                                \
                      (_pbarea)->xpos + (_xpos),                        \
                      (_pbarea)->ypos + (_ypos),                        \
                      (_size),                                          \
                      (_color)))

void pbt_pixbuf_draw_M(pbt_pixbuf_t *pixbuf,
                       unsigned int xpos,
                       unsigned int ypos,
                       int size,
                       unsigned char *color);

#define pbt_pbarea_draw_M(_pbarea, _xpos, _ypos, _size, _color) \
  (pbt_pixbuf_draw_M((_pbarea)->pixbuf,                         \
                     (_pbarea)->xpos + (_xpos),                 \
                     (_pbarea)->ypos + (_ypos),                 \
                     (_size),                                   \
                     (_color)))

void pbt_pixbuf_draw_disc(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          int size,
                          unsigned char *color);

#define pbt_pbarea_draw_disc(_pbarea,                   \
                             _xpos,                     \
                             _ypos,                     \
                             _size,                     \
                             _color)                    \
  (pbt_pixbuf_draw_disc((_pbarea)->pixbuf,              \
                        (_pbarea)->xpos + (_xpos),      \
                        (_pbarea)->ypos + (_ypos),      \
                        (_size),                        \
                        (_color)))


void pbt_pixbuf_draw_plus(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          unsigned int size,
                          unsigned char *color);

#define pbt_pbarea_draw_plus(_pbarea,                   \
                             _xpos,                     \
                             _ypos,                     \
                             _size,                     \
                             _color)                    \
  (pbt_pixbuf_draw_plus((_pbarea)->pixbuf,              \
                        (_pbarea)->xpos + (_xpos),      \
                        (_pbarea)->ypos + (_ypos),      \
                        (_size),                        \
                        (_color)))

void pbt_pixbuf_draw_minus(pbt_pixbuf_t *pixbuf,
                           unsigned int xpos,
                           unsigned int ypos,
                           unsigned int size,
                           unsigned char *color);

#define pbt_pbarea_draw_minus(_pbarea,                   \
                              _xpos,                     \
                              _ypos,                     \
                              _size,                     \
                              _color)                    \
  (pbt_pixbuf_draw_minus((_pbarea)->pixbuf,              \
                         (_pbarea)->xpos + (_xpos),      \
                         (_pbarea)->ypos + (_ypos),      \
                         (_size),                        \
                         (_color)))

void pbt_pixbuf_reduce_2x(pbt_pixbuf_t *pixbuf_dst,
                          pbt_pixbuf_t *pixbuf_src);

#endif  /* __PBT_DRAW_H_ */
