/* Copyright 2012-2016 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include "tool/tool.h"

#include "debug_tool/debug_tool.h" /* temporaire */

/* /!\ This function return an integer from "variable-length midi file value",
   and move the address pointed by buffer_ptr */
uint_t	get_varlen_from_ptr(byte_t **buffer_ptr)
{
  uint_t		value;
#define BUF_PTR	(*buffer_ptr)

#ifdef DEBUG_MIDI_MODE
  print_bin(stdout, BUF_PTR, 4);
#endif
  if (*BUF_PTR & 128)
    {
      value = (*BUF_PTR & 127);
      BUF_PTR++;
      value = value << 7;
      if (*BUF_PTR & 128)
	{
	  value += (*BUF_PTR & 127);
	  BUF_PTR++;
	  value = value << 7;
	  if (*BUF_PTR & 128)
	    {
	      value = (*BUF_PTR & 127);
	      BUF_PTR++;
	      value = (value << 7) + *BUF_PTR;
	    }
	  else
	    value += *BUF_PTR;
	}
      else
	value += *BUF_PTR;
    }
  else
    value = *BUF_PTR;

  BUF_PTR++;
  return value;
}

/* /!\ This function return an integer from "variable-length midi file value",
   and increase the int pointed by idx_ptr */
uint_t	get_varlen_from_idx(byte_t *buffer, uint_t *idx_ptr)
{
  uint_t		delta;
#define IDX_PTR		(*idx_ptr)

#ifdef DEBUG_MIDI_MODE
  print_bin(stdout, &(buffer[IDX_PTR]), 4);
#endif
  if (buffer[IDX_PTR] & 128)
    {
      delta = (buffer[IDX_PTR] & 127);
      IDX_PTR++;
      delta = delta << 7;
      if (buffer[IDX_PTR] & 128)
	{
	  delta += (buffer[IDX_PTR] & 127);
	  IDX_PTR++;
	  delta = delta << 7;
	  if (buffer[IDX_PTR] & 128)
	    {
	      delta = (buffer[IDX_PTR] & 127);
	      IDX_PTR++;
	      delta = (delta << 7) + buffer[IDX_PTR];
	    }
	  else
	    delta += buffer[IDX_PTR];
	}
      else
	delta += buffer[IDX_PTR];
    }
  else
    delta = buffer[IDX_PTR];

  IDX_PTR++;
  return delta;
}
