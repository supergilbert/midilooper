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

#include "pbt_type.h"
#include "pbt_font_inc.h"
#include "pbt_tools.h"

static FT_Library pbt_ft_library;
static unsigned int pbt_ft_library_ref = 0;

pbt_bool_t pbt_font_request_size(pbt_font_t *hdl,
                                 unsigned int width,
                                 unsigned int height)
{
  if (FT_Set_Pixel_Sizes(hdl->face, width, height) != 0)
    {
      pbt_logerr("Unable to set pixel sizes.");
      return PBT_FALSE;
    }

  if ((FT_FACE_FLAG_SCALABLE & hdl->face->face_flags)
      == FT_FACE_FLAG_SCALABLE)
    {
      /* Prevent floating point exception */
      if (hdl->face->units_per_EM == 0)
        {
          pbt_logerr("Found units per EM == 0");
          return PBT_FALSE;
        }
      hdl->xmin =
        hdl->face->bbox.xMin * hdl->face->size->metrics.x_ppem
        / hdl->face->units_per_EM;
      hdl->xmax =
        hdl->face->bbox.xMax * hdl->face->size->metrics.x_ppem
        / hdl->face->units_per_EM;
      hdl->ymin =
        hdl->face->bbox.yMin * hdl->face->size->metrics.y_ppem
        / hdl->face->units_per_EM;
      hdl->ymax =
        hdl->face->bbox.yMax * hdl->face->size->metrics.y_ppem
        / hdl->face->units_per_EM;
      hdl->max_width =
        hdl->face->max_advance_width * hdl->face->size->metrics.y_ppem
        / hdl->face->units_per_EM;
      hdl->max_height =
        hdl->face->height * hdl->face->size->metrics.y_ppem
        / hdl->face->units_per_EM;
    }
  else
    {
      hdl->xmin = 0;
      hdl->xmax = width;
      hdl->ymin = 0;
      hdl->ymax = height;
      hdl->max_width = width;
      hdl->max_height = height;
    }

  if (hdl->max_width == 0 || hdl->max_height == 0)
    {
      pbt_logerr("Font width or heigh == 0");
      return PBT_FALSE;
    }

  return PBT_TRUE;
}

pbt_bool_t pbt_unload_font(pbt_font_t *hdl)
{
  FT_Error err;

  err = FT_Done_Face(hdl->face);
  if (err != 0)
    {
      pbt_logerr("Unable to unload font (font family %s).",
                 hdl->face->family_name);
      return PBT_FALSE;
    }
  pbt_ft_library_ref--;

  if (pbt_ft_library_ref == 0)
    {
      err = FT_Done_FreeType(pbt_ft_library);
      if (err != 0)
        {
          pbt_logerr("Unable to unload freetype.");
          return PBT_FALSE;
        }
    }

  return PBT_TRUE;
}

pbt_bool_t _pbt_font_request_size(pbt_font_t *hdl,
                                  const unsigned int width,
                                  const unsigned int height)
{
  if (hdl->face->num_fixed_sizes)
    {
      pbt_logmsg("This font have %i fixed size\n"
                 "loading the first value:"
                 "width=%hi height=%hi size=%li xppem=%li yppem=%li unit_em=%hi",
                 hdl->face->num_fixed_sizes,
                 hdl->face->available_sizes->width,
                 hdl->face->available_sizes->height,
                 hdl->face->available_sizes->size,
                 hdl->face->available_sizes->x_ppem,
                 hdl->face->available_sizes->y_ppem,
                 hdl->face->units_per_EM);
      if (pbt_font_request_size(hdl,
                                hdl->face->available_sizes->width,
                                hdl->face->available_sizes->height)
          == PBT_FALSE)
        {
          pbt_logerr("Problem while requesting size.");
          return PBT_FALSE;
        }
    }
  else
    {
      if (pbt_font_request_size(hdl, width, height) == PBT_FALSE)
        {
          pbt_logerr("Problem while requesting size.");
          return PBT_FALSE;
        }
    }
  return PBT_TRUE;
}

pbt_bool_t pbt_load_font(pbt_font_t *hdl,
                         const char *path,
                         const unsigned int width,
                         const unsigned int height)
{
  FT_Error err;

  if (pbt_ft_library_ref == 0)
    {
      if (FT_Init_FreeType(&pbt_ft_library) != 0)
        {
          pbt_logerr("Unable to initialise freetype.");
          return PBT_FALSE;
        }
      pbt_ft_library_ref += 1;
    }

  err =  FT_New_Face(pbt_ft_library, path, 0, &(hdl->face));
  if (err == FT_Err_Unknown_File_Format)
    {
      pbt_logerr("Unable to load the font file %s."
                 " (font format is unsupported)",
                 path);
      return PBT_FALSE;
    }
  else if (err != 0)
    {
      pbt_logerr("Unable to read or open the font file %s.", path);
      return PBT_FALSE;
    }
  return _pbt_font_request_size(hdl, width, height);
}

pbt_bool_t pbt_load_memory_font(pbt_font_t *hdl,
                                const unsigned char *font_ptr,
                                unsigned int size,
                                const unsigned int width,
                                const unsigned int height)
{
  FT_Error err;

  if (pbt_ft_library_ref == 0)
    {
      if (FT_Init_FreeType(&pbt_ft_library) != 0)
        {
          pbt_logerr("Unable to initialise freetype.");
          return PBT_FALSE;
        }
      pbt_ft_library_ref += 1;
    }

  err =  FT_New_Memory_Face(pbt_ft_library, font_ptr, size, 0, &(hdl->face));
  if (err == FT_Err_Unknown_File_Format)
    {
      pbt_logerr("Unable to load the memory font."
                 " (font format is unsupported)");
      return PBT_FALSE;
    }
  else if (err != 0)
    {
      pbt_logerr("Unable to read or open the memory font.");
      return PBT_FALSE;
    }
  return _pbt_font_request_size(hdl, width, height);
}

pbt_bool_t pbt_font_load_char(const pbt_font_t *hdl,
                              pbt_font_char_t *font_char,
                              const char c)
{
  FT_Error err;

  err = FT_Load_Char(hdl->face, c, FT_LOAD_RENDER);
  if (err != 0)
    {
      pbt_logerr("Error while loading char value returned = %i (c=0x%hhX)\n%s",
                 err, c, ft_errors[err]);
      return PBT_FALSE;
    }
  font_char->bitmap = &(hdl->face->glyph->bitmap);
  font_char->width = hdl->face->glyph->bitmap.width;
  font_char->height = hdl->face->glyph->bitmap.rows;
  font_char->hadvance = hdl->face->glyph->metrics.horiAdvance >> 6;
  font_char->hoffset_x = hdl->face->glyph->metrics.horiBearingX >> 6;
  font_char->hoffset_y =
    hdl->ymax - (hdl->face->glyph->metrics.horiBearingY >> 6);
  return PBT_TRUE;
}

pbt_bool_t pbt_font_get_string_width(const pbt_font_t *hdl,
                                     const char *str_var,
                                     unsigned int *width)
{
  pbt_font_char_t font_char;

  *width = 0;
  while (*str_var != '\0')
    {
      if (pbt_font_load_char(hdl, &font_char, *str_var) == PBT_FALSE)
        return PBT_FALSE;
      *width += font_char.hadvance;
      str_var++;
    }

  return PBT_TRUE;
}

void pbt_font_dump_info(const pbt_font_t *hdl)
{
  FT_Bitmap_Size *sizes_ptr;
  unsigned int idx;

  pbt_logmsg("Font Information:");
  pbt_logmsg("Family name: %s\nStyle name: %s",
             hdl->face->family_name,
             hdl->face->style_name);
  pbt_logmsg("Number of faces = %li using index %li",
             hdl->face->num_faces,
             hdl->face->face_index);
  pbt_logmsg("Number of fixed sizes = %i",
             hdl->face->num_fixed_sizes);
  for (sizes_ptr = hdl->face->available_sizes,
         idx = 0;
       idx < hdl->face->num_fixed_sizes;
       idx++)
    pbt_logmsg("width=%i heigth=%i size=%li",
               sizes_ptr[idx].height,
               sizes_ptr[idx].width,
               sizes_ptr[idx].size);
  pbt_logmsg("face boundingbox: box.xMin=%li box.yMin=%li box.xMax=%li"
             " box.yMax=%li\nunitperEM: %i",
             hdl->face->bbox.xMin, hdl->face->bbox.yMin,
             hdl->face->bbox.xMax, hdl->face->bbox.yMax,
             hdl->face->units_per_EM);
  pbt_logmsg("face metrics gives: xsz=%i ysz=%i in ppem xscale=%li yscale=%li",
             hdl->face->size->metrics.x_ppem,
             hdl->face->size->metrics.y_ppem,
             hdl->face->size->metrics.x_scale,
             hdl->face->size->metrics.y_scale);
  pbt_logmsg("face metrics gives: ascender=%li descender=%li height=%li"
             " maxadvance=%li",
             hdl->face->size->metrics.ascender,
             hdl->face->size->metrics.descender,
             hdl->face->size->metrics.height,
             hdl->face->size->metrics.max_advance);
  if (FT_FACE_FLAG_SCALABLE == hdl->face->face_flags)
    pbt_logmsg("SCALABLE FLAG is set");
  pbt_logmsg("Value used:\nxmin=%d xmax=%d ymin=%d ymax=%d\nwidth=%d height=%d",
             hdl->xmin, hdl->xmax, hdl->ymin, hdl->ymax,
             hdl->max_width, hdl->max_height);
}

unsigned char pbt_font_bitmap_get_pxl_gray(FT_Bitmap *bitmap,
                                           unsigned int xpos,
                                           unsigned int ypos)
{
  unsigned int idx = xpos + (ypos * bitmap->width);

  return (unsigned char) bitmap->buffer[idx];
}

pbt_bool_t pbt_font_bitmap_get_pxl_mono(FT_Bitmap *bitmap,
                                        unsigned int xpos,
                                        unsigned int ypos)
{
  unsigned int idx = xpos + (ypos * bitmap->width);
  unsigned char c;

  c = bitmap->buffer[(idx / 8)];
  if (((c >> (idx % 8)) & 1) == 1)
    return PBT_TRUE;
  else
    return PBT_FALSE;
}

pbt_bool_t pbt_font_char_get_pxl(const pbt_font_char_t *hdl,
                                 unsigned int xpos,
                                 unsigned int ypos)
{
  unsigned char ret = PBT_TRUE;

  if (xpos >= hdl->bitmap->width || ypos >= hdl->bitmap->rows)
    return 0;

  switch (hdl->bitmap->pixel_mode)
    {
    case FT_PIXEL_MODE_MONO:
      if (pbt_font_bitmap_get_pxl_mono(hdl->bitmap, xpos, ypos) == PBT_TRUE)
        ret = 0xff;
      break;
    case FT_PIXEL_MODE_GRAY:
      ret = pbt_font_bitmap_get_pxl_gray(hdl->bitmap, xpos, ypos);
      break;
    default:
      ret = 0;
    }

  return ret;                   /* for the fun */
}
