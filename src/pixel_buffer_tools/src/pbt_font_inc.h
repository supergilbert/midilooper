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

#ifndef __PBT_FONT_INC_H_INCLUDED__
#define __PBT_FONT_INC_H_INCLUDED__

#include <ft2build.h>

#include FT_FREETYPE_H

#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, NULL } };

const static struct
{
  int          err_code;
  const char*  err_msg;
} ft_errors[] =
#include FT_ERRORS_H

#undef FTMODERR_H_
#define FT_MODERRDEF( e, v, s )  { FT_Mod_Err_ ## e, s },
#define FT_MODERR_START_LIST     {
#define FT_MODERR_END_LIST       { 0, 0 } };

const static struct
{
  int          mod_err_offset;
  const char*  mod_err_msg;
} ft_mod_errors[] =
#include FT_MODULE_ERRORS_H

typedef struct
{
  FT_Face face;
  int xmin, xmax, ymin, ymax;
  unsigned int max_width, max_height;
} pbt_font_t;

typedef struct
{
  FT_Bitmap *bitmap;
  unsigned int width;
  unsigned int height;
  unsigned int hadvance;
  unsigned int hoffset_x;
  unsigned int hoffset_y;
} pbt_font_char_t;

#endif  /* __PBT_FONT_INC_H_INCLUDED__ */
