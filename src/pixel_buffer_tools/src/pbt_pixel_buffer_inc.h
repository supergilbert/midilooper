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

#ifndef __PBT_PIXEL_BUFFER_INC_H_INCLUDED__
#define __PBT_PIXEL_BUFFER_INC_H_INCLUDED__

#include "pbt_type.h"

typedef struct
{
  unsigned int  width;
  unsigned int  height;
  unsigned int  pxl_sz;
  unsigned int  line_sz;
  unsigned int  pxl_count;
  unsigned char *pixels;
} pbt_pixbuf_t;

typedef struct
{
  unsigned int       xpos;
  unsigned int       ypos;
  unsigned int       width;
  unsigned int       height;
  pbt_pixbuf_t          *pixbuf;
} pbt_pbarea_t;

#define pbt_pixbuf_get_addr(pixbuf, _xpos, _ypos)                             \
  ((pixbuf)->pixels + ((_xpos) * (pixbuf)->pxl_sz) + ((_ypos) * (pixbuf)->line_sz))

#define pbt_pbarea_get_addr(pbarea, _xpos, _ypos)   \
  pbt_pixbuf_get_addr((pbarea)->pixbuf,                     \
                   (pbarea)->xpos + (_xpos),          \
                   (pbarea)->ypos + (_ypos))


#define PBT_IS_IN_IMG_AREA(pbarea, _xpos, _ypos)                      \
  ((_xpos) >= (int) (pbarea)->xpos                                    \
   && (_xpos) < (int) ((pbarea)->xpos + (pbarea)->width)            \
   && (_ypos) >= (int) (pbarea)->ypos                                 \
   && (_ypos) < (int) ((pbarea)->ypos + (pbarea)->height)           \
   ? PBT_TRUE : PBT_FALSE)

#endif /* __PBT_PIXEL_BUFFER_INC_H_INCLUDED__ */
