#ifndef __DEBUG_TOOL
#define __DEBUG_TOOL

#include "tool/tool.h"
#include <stdio.h>


void print_bin(FILE *output, byte_t *buffer, uint_t size);
void output(char *fmt, ...);
void output_error(char *fmt, ...);
void debug(char *fmt, ...);
void debug_midi(char *fmt, ...);
void output_warning(char *fmt, ...);
void debug_wait(char *fmt, ...);


#define TRACE_FMT "in %s at %s:%d\n"
#define TRACE_ARG __FUNCTION__, __FILE__, __LINE__

#define ERROR_FMT "ERROR "TRACE_FMT
#define ERROR_ARG TRACE_ARG

#define trace_func debug(TRACE_FMT, TRACE_ARG)

#endif
