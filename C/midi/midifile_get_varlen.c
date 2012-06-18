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
