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
#include "pbt_font.h"

void pbt_pixbuf_putstr(pbt_pixbuf_t *pixbuf,
                    pbt_font_t *font,
                    unsigned char *color,
                    int xpos,
                    int ypos,
                    const char *str)
{
  int x_font, y_font;
  int x_pixbuf, y_pixbuf = ypos;
  int x_current = xpos;
  unsigned char buffer_color[4];
  unsigned char value;
  pbt_font_char_t font_char;

  while (*str != '\0')
    {
      pbt_font_load_char(font, &font_char, *str);
      for (y_font = 0, y_pixbuf = ypos + font_char.hoffset_y;
           y_font < (int) font_char.height &&
             y_pixbuf < (int) pixbuf->height;
           y_font++, y_pixbuf++)
        {
          for (x_font = 0, x_pixbuf = x_current + font_char.hoffset_x;
               x_font < (int) font_char.width &&
                 x_pixbuf < (int) pixbuf->width;
               x_font++, x_pixbuf++)
            {
              if (x_pixbuf < 0 || y_pixbuf < 0)
                continue;
              value = pbt_font_char_get_pxl(&font_char, x_font, y_font);
              pbt_pixbuf_get_pxl(pixbuf,
                              x_pixbuf,
                              y_pixbuf,
                              buffer_color);
              buffer_color[0] =
                (buffer_color[0] * (255 - value) + color[0] * value) / 255;
              buffer_color[1] =
                (buffer_color[1] * (255 - value) + color[1] * value) / 255;
              buffer_color[2] =
                (buffer_color[2] * (255 - value) + color[2] * value) / 255;
              buffer_color[3] =
                (buffer_color[3] * (255 - value) + color[3] * value) / 255;
              pbt_pixbuf_put_pxl(pixbuf,
                              x_pixbuf,
                              y_pixbuf,
                              buffer_color);
            }
        }
      x_current += font_char.hadvance;
      str++;
    }
}

#define MAX_STRL_LEN 512
void pbt_pixbuf_printf(pbt_pixbuf_t *pixbuf,
                    pbt_font_t *font,
                    unsigned char *color,
                    int xpos,
                    int ypos,
                    const char *format,
                    ...)
{
  char str[MAX_STRL_LEN];
  va_list ap;

  va_start(ap, format);
  vsnprintf(str, MAX_STRL_LEN, format, ap);
  pbt_pixbuf_putstr(pixbuf, font, color, xpos, ypos, str);
  va_end(ap);
}

void pbt_pbarea_putstr(pbt_pbarea_t *pbarea,
                         pbt_font_t *font,
                         unsigned char *color,
                         int xpos,
                         int ypos,
                         const char *str)
{
  int x_font, y_font;
  int x_pixbuf, y_pixbuf = ypos;
  int x_current = xpos;
  unsigned char buffer_color[4];
  unsigned char value;
  pbt_font_char_t font_char;

  while (*str != '\0')
    {
      pbt_font_load_char(font, &font_char, *str);
      for (y_font = 0, y_pixbuf = ypos + font_char.hoffset_y;
           y_font < (int) font_char.height &&
             y_pixbuf < (int) pbarea->height;
           y_font++, y_pixbuf++)
        {
          for (x_font = 0, x_pixbuf = x_current + font_char.hoffset_x;
               x_font < (int) font_char.width &&
                 x_pixbuf < (int) pbarea->width;
               x_font++, x_pixbuf++)
            {
              if (x_pixbuf < 0 || y_pixbuf < 0)
                continue;
              value = pbt_font_char_get_pxl(&font_char, x_font, y_font);
              pbt_pbarea_get_pxl(pbarea,
                                   x_pixbuf,
                                   y_pixbuf,
                                   buffer_color);
              buffer_color[0] =
                (buffer_color[0] * (255 - value) + color[0] * value) / 255;
              buffer_color[1] =
                (buffer_color[1] * (255 - value) + color[1] * value) / 255;
              buffer_color[2] =
                (buffer_color[2] * (255 - value) + color[2] * value) / 255;
              buffer_color[3] =
                (buffer_color[3] * (255 - value) + color[3] * value) / 255;
              pbt_pbarea_put_pxl(pbarea,
                                   x_pixbuf,
                                   y_pixbuf,
                                   buffer_color);
            }
        }
      x_current += font_char.hadvance;
      str++;
    }
}

#define MAX_STRL_LEN 512
void pbt_pbarea_printf(pbt_pbarea_t *pbarea,
                         pbt_font_t *font,
                         unsigned char *color,
                         int xpos,
                         int ypos,
                         const char *format,
                         ...)
{
  char str[MAX_STRL_LEN];
  va_list ap;

  va_start(ap, format);
  vsnprintf(str, MAX_STRL_LEN, format, ap);
  pbt_pbarea_putstr(pbarea, font, color, xpos, ypos, str);
  va_end(ap);
}
