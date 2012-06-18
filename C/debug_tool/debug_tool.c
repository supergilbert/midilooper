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

void		_output_error(char *fmt, ...)
{
  va_list	ap;

  va_start(ap, fmt);
  fprintf(ERROUT, "\033[31m");
  vfprintf(ERROUT, fmt, ap);
  fprintf(ERROUT, "\033[0m");
  va_end(ap);
}
