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

#ifndef __PBT_FONT_H_INCLUDED__
#define __PBT_FONT_H_INCLUDED__

#include "pbt_type.h"
#include "pbt_font_inc.h"

pbt_bool_t pbt_unload_font(pbt_font_t *hdl);
pbt_bool_t pbt_load_font(pbt_font_t *hdl,
                         const char *path,
                         const unsigned int width,
                         const unsigned int height);
pbt_bool_t pbt_load_memory_font(pbt_font_t *hdl,
                                const unsigned char *font_ptr,
                                unsigned int size,
                                const unsigned int width,
                                const unsigned int height);
pbt_bool_t pbt_font_load_char(const pbt_font_t *hdl,
                              pbt_font_char_t *font_char,
                              const char c);
pbt_bool_t pbt_font_get_string_width(const pbt_font_t *hdl,
                                     const char *str_var,
                                     unsigned int *width);
void pbt_font_dump_info(const pbt_font_t *hdl);
unsigned char pbt_font_char_get_pxl(const pbt_font_char_t *font_char,
                                    unsigned int x,
                                    unsigned int y);

#endif  /* __PBT_FONT_H_INCLUDED__ */
