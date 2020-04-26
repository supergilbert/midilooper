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

#include "pbt_pixel_buffer.h"
#include "math.h"

void pbt_pixbuf_draw_triangle_left(pbt_pixbuf_t *pixbuf,
                                   unsigned int xpos,
                                   unsigned int ypos,
                                   int size,
                                   unsigned char *color)
{
  xpos += (size - 1) / 2;
  while (size >= 0)
    {
      pbt_pixbuf_put_vline(pixbuf, xpos, ypos, size, color);
      xpos -= 1;
      ypos += 1;
      size -= 2;
    }
}

void pbt_pixbuf_draw_triangle_right(pbt_pixbuf_t *pixbuf,
                                    unsigned int xpos,
                                    unsigned int ypos,
                                    int size,
                                    unsigned char *color)
{
  while (size >= 0)
    {
      pbt_pixbuf_put_vline(pixbuf, xpos, ypos, size, color);
      xpos += 1;
      ypos += 1;
      size -= 2;
    }
}

void pbt_pixbuf_draw_triangle_up(pbt_pixbuf_t *pixbuf,
                                   unsigned int xpos,
                                   unsigned int ypos,
                                   int size,
                                   unsigned char *color)
{
  ypos += (size - 1) / 2;
  while (size >= 0)
    {
      pbt_pixbuf_put_hline(pixbuf, xpos, ypos, size, color);
      xpos += 1;
      ypos -= 1;
      size -= 2;
    }
}

void pbt_pixbuf_draw_triangle_down(pbt_pixbuf_t *pixbuf,
                                   unsigned int xpos,
                                   unsigned int ypos,
                                   int size,
                                   unsigned char *color)
{
  while (size >= 0)
    {
      pbt_pixbuf_put_hline(pixbuf, xpos, ypos, size, color);
      xpos += 1;
      ypos += 1;
      size -= 2;
    }
}

void pbt_pixbuf_draw_disc(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          int size,
                          unsigned char *color)
{
  unsigned int line_xpos, line_ypos, line_size;
  unsigned int idx = 0;
  float rayon = size / 2;
  float pos;

  while (idx < size)
    {
      pos = idx;
      if (pos < rayon)
        line_size = sqrtf(powf(rayon, 2.0) - powf(rayon - pos, 2.0));
      else
        line_size = sqrtf(pow(rayon, 2.0) - powf(rayon - (size - idx), 2.0));
      line_size = line_size << 1;
      line_ypos = ypos + (size - line_size) / 2;
      line_xpos = xpos + idx;
      pbt_pixbuf_put_vline(pixbuf, line_xpos, line_ypos, line_size, color);
      idx++;
    }
}

void pbt_pixbuf_draw_plus(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          int size,
                          unsigned char *color)
{
  unsigned int pos_inc = size / 2, line_width;
  if (size % 2 == 0)
    {
      pos_inc--;
      line_width = 2;
    }
  else
    line_width = 3;

  pbt_pixbuf_fillrect(pixbuf,
                      xpos,
                      ypos + pos_inc,
                      size,
                      line_width,
                      color);
  pbt_pixbuf_fillrect(pixbuf,
                      xpos + pos_inc,
                      ypos,
                      line_width,
                      size,
                      color);
}

void pbt_pixbuf_draw_minus(pbt_pixbuf_t *pixbuf,
                           unsigned int xpos,
                           unsigned int ypos,
                           int size,
                           unsigned char *color)
{
  unsigned int pos_inc = size / 2, line_width;
  if (size % 2 == 0)
    {
      pos_inc--;
      line_width = 2;
    }
  else
    line_width = 3;

  pbt_pixbuf_fillrect(pixbuf,
                      xpos,
                      ypos + pos_inc,
                      size,
                      line_width,
                      color);
}

void pbt_pixbuf_draw_hp(pbt_pixbuf_t *pixbuf,
                        unsigned int xpos,
                        unsigned int ypos,
                        int size,
                        unsigned char *color)
{
  pbt_pixbuf_fillrect(pixbuf,
                      xpos + (size / 8),
                      ypos + (size / 4) + ((size % 8) == 0 ? 0 : 1),
                      size / 2,
                      size / 2,
                      color);
  pbt_pixbuf_draw_triangle_left(pixbuf,
                                xpos + (3 * size / 8),
                                ypos,
                                size,
                                color);
}

void pbt_pixbuf_draw_M(pbt_pixbuf_t *pixbuf,
                       unsigned int xpos,
                       unsigned int ypos,
                       int size,
                       unsigned char *color)
{
  pbt_pixbuf_draw_triangle_up(pixbuf,
                              xpos,
                              ypos,
                              (size / 2),
                              color);
  pbt_pixbuf_draw_triangle_up(pixbuf,
                              xpos + (size / 2),
                              ypos,
                              (size / 2),
                              color);

  pbt_pixbuf_draw_triangle_down(pixbuf,
                                xpos + (size / 4),
                                ypos + (size / 4),
                                (size / 2),
                                color);

  pbt_pixbuf_fillrect(pixbuf,
                      xpos,
                      ypos,
                      (size / 4),
                      size,
                      color);
  pbt_pixbuf_fillrect(pixbuf,
                      xpos + (3 * size / 4),
                      ypos,
                      (size / 4),
                      size,
                      color);
}

void pbt_pixbuf_reduce_2x(pbt_pixbuf_t *pixbuf_dst,
                          pbt_pixbuf_t *pixbuf_src)
{
  unsigned int xpos_src, xpos_dst, ypos_src, ypos_dst;
  unsigned char new_color[4];
  unsigned char color1[4];
  unsigned char color2[4];
  unsigned char color3[4];
  unsigned char color4[4];

  for (ypos_src = 0, ypos_dst = 0;
       ypos_src + 1 < pixbuf_src->height;
       ypos_src += 2, ypos_dst += 1)
    for (xpos_src = 0, xpos_dst = 0;
         xpos_src + 1 < pixbuf_src->width;
         xpos_src += 2, xpos_dst += 1)
    {
      pbt_pixbuf_get_pxl(pixbuf_src, xpos_src,     ypos_src,     color1);
      pbt_pixbuf_get_pxl(pixbuf_src, xpos_src + 1, ypos_src,     color2);
      pbt_pixbuf_get_pxl(pixbuf_src, xpos_src,     ypos_src + 1, color3);
      pbt_pixbuf_get_pxl(pixbuf_src, xpos_src + 1, ypos_src + 1, color4);
      new_color[0] = (color1[0] + color2[0] + color3[0] + color4[0]) / 4;
      new_color[1] = (color1[1] + color2[1] + color3[1] + color4[1]) / 4;
      new_color[2] = (color1[2] + color2[2] + color3[2] + color4[2]) / 4;
      new_color[3] = (color1[3] + color2[3] + color3[3] + color4[3]) / 4;
      pbt_pixbuf_put_pxl(pixbuf_dst, xpos_dst, ypos_dst, new_color);
    }
}
