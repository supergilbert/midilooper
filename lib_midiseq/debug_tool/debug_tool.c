/* Copyright 2012-2014 Gilbert Romer */

/* This file is part of gmidilooper. */

/* gmidilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* gmidilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include <stdarg.h>

#include "debug_tool/debug_tool.h"

#define OUTPUT  stdout
#define ERROUT  stderr

/* TODO */

void            print_hex(FILE *output, byte_t *buffer, uint_t size)
{
  uint_t        idx = 0;

  while (idx < size)
    {
      fprintf(output, " %02X", buffer[idx]);
      idx++;
    }
}

void            print_ascii(FILE *output, byte_t *buffer, uint_t size)
{
#ifdef DEBUG_MODE
  uint_t        idx = 0;

  while (idx < size)
    {
      if (buffer[idx] > 31 && buffer[idx] < 127)
        fprintf(output, "%c", buffer[idx]);
      else
        fprintf(output, ".");
      idx++;
    }
#endif
}

void            print_bin(FILE *output, byte_t *buffer, uint_t size)
{
#ifdef DEBUG_MODE
  print_hex(output, buffer, size);
  fprintf(output, "\t");
  print_ascii(output, buffer, size);
  fprintf(output, "\n");
#endif
}

void		output(char *fmt, ...)
{
  va_list	ap;

  va_start(ap, fmt);
  vfprintf(OUTPUT, fmt, ap);
  va_end(ap);
}

void		_debug(char *fmt, ...)
{
#ifdef DEBUG_MODE
  va_list	ap;

  va_start(ap, fmt);
  vfprintf(OUTPUT, fmt, ap);
  va_end(ap);
#endif
}

void		_debug_midi(char *fmt, ...)
{
#ifdef DEBUG_MIDI_MODE
  va_list	ap;

  va_start(ap, fmt);
  vfprintf(OUTPUT, fmt, ap);
  va_end(ap);
#endif
}

void		_output_warning(char *fmt, ...)
{
  va_list	ap;

  va_start(ap, fmt);
  fprintf(OUTPUT, "\033[33m");
  vfprintf(OUTPUT, fmt, ap);
  fprintf(OUTPUT, "\033[0m");
  va_end(ap);
}

void vaoutput_error(char *fmt, va_list ap)
{
  fprintf(ERROUT, "\033[31m");
  vfprintf(ERROUT, fmt, ap);
  fprintf(ERROUT, "\033[0m");
}

void		_output_error(char *fmt, ...)
{
  va_list	ap;

  va_start(ap, fmt);
  vaoutput_error(fmt, ap);
  va_end(ap);
}

void _msq_assert(bool_t bool, char *format, ...)
{
  va_list	ap;

  if (bool == FALSE)
    {
      va_start(ap, format);
      vaoutput_error(format, ap);
      va_end(ap);
      exit(1);
    }
}
