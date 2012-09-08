#ifndef __DEBUG_TOOL
#define __DEBUG_TOOL

#include "tool/tool.h"
#include <stdio.h>


void print_bin(FILE *output, byte_t *buffer, uint_t size);
void output(char *fmt, ...);
void _output_error(char *fmt, ...);
void _debug(char *fmt, ...);
void _debug_midi(char *fmt, ...);
void _output_warning(char *fmt, ...);
//void debug_wait(char *fmt, ...);

#define TRACE_FMT "(%s %s:%d) "
#define TRACE_ARG __FUNCTION__, __FILE__, __LINE__

#define ERROR_FMT "ERROR "TRACE_FMT
#define ERROR_ARG TRACE_ARG

#define debug(format, ...)          _debug(TRACE_FMT format "\n", TRACE_ARG, ##__VA_ARGS__)
#define debug_midi(format, ...)     _debug_midi(TRACE_FMT format "\n", TRACE_ARG, ##__VA_ARGS__)
#define output_error(format, ...)   _output_error(TRACE_FMT format "\n", TRACE_ARG, ##__VA_ARGS__)
#define output_warning(format, ...) _output_warning(TRACE_FMT format "\n", TRACE_ARG, ##__VA_ARGS__)

#define trace_func debug(TRACE_FMT "\n", TRACE_ARG)

#endif
