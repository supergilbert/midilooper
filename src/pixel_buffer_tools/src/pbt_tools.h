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

#pragma once

#include <stdio.h>

EXTERN_C_BEGIN

const char *_pbt_get_relative_source_path(const char *original_path);

void _pbt_output(FILE *stream, const char *format, ...);

#define pbt_logmsg(format, ...) _pbt_output(stdout, format "\n", ##__VA_ARGS__)
#define pbt_logerr(format, ...) _pbt_output(stderr, format "\n", ##__VA_ARGS__)

void _pbt_abort(char *format, ...);

#define PBT_TRACE_FORMAT ">>> In %s at %s:%d\n"

#define pbt_logdbg(format, ...)                                         \
  pbt_logmsg(PBT_TRACE_FORMAT format,                                   \
             __FUNCTION__, __FILE__, __LINE__,                          \
             ##__VA_ARGS__)

#define pbt_abort(format, ...)                                          \
  _pbt_abort(PBT_TRACE_FORMAT format,                                   \
             __FUNCTION__,                                              \
             _pbt_get_relative_source_path(__FILE__),                   \
             __LINE__,                                                  \
             ##__VA_ARGS__)

EXTERN_C_END
