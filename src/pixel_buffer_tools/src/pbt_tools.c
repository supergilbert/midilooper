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

#include <stdio.h>
#include <stdarg.h>

#include <string.h>

#define RELATIVE_SOURCE_KEY "/src/"
#define RELATIVE_SOURCE_LEN 5

const char *_pbt_get_relative_source_path(const char *original_path)
{
  unsigned int idx = strlen(original_path);

  do {
    idx--;
    if (strncmp(&(original_path[idx]),
                RELATIVE_SOURCE_KEY,
                RELATIVE_SOURCE_LEN) == 0)
      return &(original_path[idx + 1]);
  } while (idx > 0);

  return original_path;
}

void _pbt_output(FILE *stream, char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);
}

#include <stdlib.h>

void _pbt_abort(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  fprintf(stderr, "\033[31m");
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
  fprintf(stderr, "\033[0m");
  va_end(ap);
  abort();
}
