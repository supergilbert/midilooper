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
/* void debug_wait(char *fmt, ...); */

#define output_warning(format, ...) _output_warning(format "\n",        \
                                                    ##__VA_ARGS__)

#define TRACE_FMT "(%s %s:%d) "
#define TRACE_ARG __FUNCTION__, __FILE__, __LINE__

#define ERROR_FMT "ERROR "TRACE_FMT
#define ERROR_ARG TRACE_ARG

#define debug(format, ...)          _debug(TRACE_FMT format "\n",       \
                                           TRACE_ARG,                   \
                                           ##__VA_ARGS__)
#define debug_midi(format, ...)     _debug_midi(TRACE_FMT format "\n",  \
                                                TRACE_ARG,              \
                                                ##__VA_ARGS__)
#define output_error(format, ...)   _output_error(TRACE_FMT format "\n", \
                                                  TRACE_ARG,            \
                                                  ##__VA_ARGS__)

#define trace_func debug(TRACE_FMT "\n", TRACE_ARG)

void _msq_assert(bool_t bool, char *format, ...);
#ifdef __ROUGH
#define msq_assert(bool, format, ...)(TRUE)
#else
#define msq_assert(bool, format, ...) _msq_assert(bool,                 \
                                                  TRACE_FMT format "\n", \
                                                  TRACE_ARG,            \
                                                  ##__VA_ARGS__)
#endif

#endif
