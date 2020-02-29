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

#include <stdlib.h>             /* malloc */

#include "pbt_pixel_buffer_inc.h"

#define DEFAULT_PXL_SIZE   4

void pbt_pixbuf_init(pbt_pixbuf_t *pixbuf,
                     unsigned int width,
                     unsigned int height)
{
  pixbuf->width = width;
  pixbuf->height = height;
  pixbuf->pxl_sz = DEFAULT_PXL_SIZE;
  pixbuf->line_sz = pixbuf->width * DEFAULT_PXL_SIZE;
  pixbuf->pxl_count = pixbuf->width * pixbuf->height;
  pixbuf->pixels = malloc(width * height * DEFAULT_PXL_SIZE);
}

void pbt_pixbuf_resize(pbt_pixbuf_t *pixbuf,
                       unsigned int width,
                       unsigned int height)
{
  if (pixbuf->pixels != NULL)
    free(pixbuf->pixels);
  pbt_pixbuf_init(pixbuf, width, height);
}

void pbt_pixbuf_fill(pbt_pixbuf_t *pixbuf, unsigned char *color)
{
  unsigned char *ptr, *ptr_end;

  for (ptr = pixbuf->pixels,
         ptr_end = ptr + (pixbuf->pxl_count * pixbuf->pxl_sz);
       ptr < ptr_end;
       ptr += pixbuf->pxl_sz)
    {
      ptr[0] = color[0];
      ptr[1] = color[1];
      ptr[2] = color[2];
      ptr[3] = color[3];
    }
}

#include "pbt_tools.h"

void pbt_pixbuf_fillrect(pbt_pixbuf_t *pixbuf,
                         unsigned int xpos_rect,
                         unsigned int ypos_rect,
                         unsigned int width,
                         unsigned int height,
                         unsigned char *color)
{
  unsigned char *offset, *offset_end;
  unsigned char *ptr, *ptr_end;

  if (xpos_rect > pixbuf->width
      || xpos_rect + width > pixbuf->width)
    pbt_abort("ERROR: buffer fillrect out of range in width"
              " (x:%d w:%d y:%d h:%d for w:%d h:%d)",
              xpos_rect, width,
              ypos_rect, height,
              pixbuf->width, pixbuf->height);
  else if (ypos_rect > pixbuf->height
           || ypos_rect + height > pixbuf->height)
    pbt_abort("ERROR: buffer fillrect out of range in height"
              " (x:%d w:%d y:%d h:%d for w:%d h:%d)",
              xpos_rect, width,
              ypos_rect, height,
              pixbuf->width, pixbuf->height);

  for (offset = pbt_pixbuf_get_addr(pixbuf, xpos_rect, ypos_rect),
         offset_end = offset + (height * pixbuf->line_sz);
       offset < offset_end;
       offset += pixbuf->line_sz)
    for (ptr = offset,
           ptr_end = ptr + (width * pixbuf->pxl_sz);
         ptr < ptr_end;
         ptr += pixbuf->pxl_sz)
      {
        ptr[0] = color[0];
        ptr[1] = color[1];
        ptr[2] = color[2];
        ptr[3] = color[3];
      }
}

void pbt_pixbuf_put_pxl(pbt_pixbuf_t *pixbuf,
                        unsigned int xpos,
                        unsigned int ypos,
                        unsigned char *color)
{
  unsigned char *offset = pbt_pixbuf_get_addr(pixbuf, xpos, ypos);

  if (xpos > pixbuf->width)
    pbt_abort("ERROR: buffer put pxl out of range (x:%d > pbw:%d)",
              xpos, pixbuf->width);
  if (ypos > pixbuf->height)
    pbt_abort("ERROR: buffer put pxl out of range (y:%d > pbh:%d)",
              ypos, pixbuf->height);

  offset[0] = color[0];
  offset[1] = color[1];
  offset[2] = color[2];
  offset[3] = color[3];
}

void pbt_pixbuf_get_pxl(pbt_pixbuf_t *pixbuf,
                        unsigned int xpos,
                        unsigned int ypos,
                        unsigned char *color)
{
  unsigned char *offset = pbt_pixbuf_get_addr(pixbuf, xpos, ypos);

  if (xpos > pixbuf->width)
    pbt_abort("ERROR: buffer get pxl out of range (x:%d > pbw:%d)",
              xpos, pixbuf->width);
  if (ypos > pixbuf->height)
    pbt_abort("ERROR: buffer get pxl out of range (y:%d > pbh:%d)",
              ypos, pixbuf->height);

  color[0] = offset[0];
  color[1] = offset[1];
  color[2] = offset[2];
  color[3] = offset[3];
}

void pbt_pixbuf_put_vline(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          unsigned int size,
                          unsigned char *color)
{
  unsigned char *ptr;
  unsigned char *ptr_end;

  if ((xpos > pixbuf->width)
      || (ypos + size > pixbuf->height))
    pbt_abort("ERROR: buffer put vline out of range (x:%d y:%d h:%d)",
              xpos, ypos, size);

  for (ptr = pbt_pixbuf_get_addr(pixbuf, xpos, ypos),
         ptr_end = ptr + (size * pixbuf->line_sz);
       ptr < ptr_end;
       ptr += pixbuf->line_sz)
    {
      ptr[0] = color[0];
      ptr[1] = color[1];
      ptr[2] = color[2];
      ptr[3] = color[3];
    }
}

void pbt_pixbuf_put_hline(pbt_pixbuf_t *pixbuf,
                          unsigned int xpos,
                          unsigned int ypos,
                          unsigned int size,
                          unsigned char *color)
{
  unsigned char *ptr;
  unsigned char *ptr_end;

  if (xpos + size > pixbuf->width)
    pbt_abort("ERROR: "
              "buffer put hline out of range (x:%d + w:%d > pbw:%d)",
              xpos, size, pixbuf->width);
  if (ypos > pixbuf->height)
    pbt_abort("ERROR: "
              "buffer put hline out of range (y:%d > pbh:%d)",
              ypos, pixbuf->height);

  for (ptr = pbt_pixbuf_get_addr(pixbuf, xpos, ypos),
         ptr_end = ptr + (size * pixbuf->pxl_sz);
       ptr < ptr_end;
       ptr += pixbuf->pxl_sz)
    {
      ptr[0] = color[0];
      ptr[1] = color[1];
      ptr[2] = color[2];
      ptr[3] = color[3];
    }
}

void pbt_pixbuf_put_rect(pbt_pixbuf_t *pixbuf,
                         unsigned int xpos,
                         unsigned int ypos,
                         unsigned int width,
                         unsigned int height,
                         unsigned char *color)
{
  pbt_pixbuf_put_hline(pixbuf,
                       xpos, ypos,
                       width,
                       color);
  pbt_pixbuf_put_hline(pixbuf,
                       xpos, ypos + (height - 1),
                       width,
                       color);
  pbt_pixbuf_put_vline(pixbuf,
                       xpos, ypos,
                       height,
                       color);
  pbt_pixbuf_put_vline(pixbuf,
                       xpos + (width - 1), ypos,
                       height,
                       color);
}

void pbt_pbarea_setup_from_buffer(pbt_pbarea_t *pbarea,
                                  pbt_pixbuf_t *pixbuf,
                                  unsigned int xpos,
                                  unsigned int ypos,
                                  unsigned int width,
                                  unsigned int height)
{
  if (xpos + width > pixbuf->width)
    pbt_abort("ERROR: "
              "pbarea setup from buffer out of range (x:%d w:%d > pbw:%d)",
              xpos, width, pixbuf->width);
  if (ypos + height > pixbuf->height)
    pbt_abort("ERROR: "
              "pbarea setup from buffer out of range (y:%d h:%d > pbh:%d)",
              ypos, height, pixbuf->height);

  pbarea->xpos = xpos;
  pbarea->ypos = ypos;
  pbarea->width = width;
  pbarea->height = height;
  pbarea->pixbuf = pixbuf;
}

void pbt_pbarea_resize(pbt_pbarea_t *pbarea,
                       unsigned int xpos,
                       unsigned int ypos,
                       unsigned int width,
                       unsigned int height)
{
  pbarea->xpos = xpos;
  pbarea->ypos = ypos;
  pbarea->width = width;
  pbarea->height = height;
}

#include <string.h>
void pbt_pixbuf_copy_pixbuf(pbt_pixbuf_t *pixbuf_dst,
                            unsigned int xpos_dst,
                            unsigned int ypos_dst,
                            pbt_pixbuf_t *pixbuf_src,
                            unsigned int xpos_src,
                            unsigned int ypos_src,
                            unsigned int width,
                            unsigned int height)
{
  void *ptr_dst, *ptr_dst_end,
    *ptr_src;
  size_t width_sz;

  for (width_sz = width * pixbuf_dst->pxl_sz,
         ptr_src = pbt_pixbuf_get_addr(pixbuf_src, xpos_src, ypos_src),
         ptr_dst = pbt_pixbuf_get_addr(pixbuf_dst, xpos_dst, ypos_dst),
         ptr_dst_end = (ptr_dst + (height * pixbuf_dst->line_sz));
       ptr_dst < ptr_dst_end;
       ptr_src += pixbuf_src->line_sz,
         ptr_dst += pixbuf_dst->line_sz)
    memcpy(ptr_dst, ptr_src, width_sz);
}
