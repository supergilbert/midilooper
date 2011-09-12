#include "tool/tool.h"

#include "debug_tool/debug_tool.h" /* temporaire */

/* /!\ This function get deltatime from variable-length midi file value move,
   and move the address pointed by buffer_ptr */
uint_t	get_varlen_from_ptr(byte_t **buffer_ptr)
{
  uint_t		delta;
#define BUF_PTR	(*buffer_ptr)

  debug_midi("%s: buffer addr : %p\n", __FUNCTION__, BUF_PTR);
#ifdef DEBUG_MIDI_MODE
  print_bin(stdout, BUF_PTR, 4);
#endif
  if (*BUF_PTR & 128)
    {
      delta = (*BUF_PTR & 127);
      BUF_PTR++;
      delta = delta << 7;
      if (*BUF_PTR & 128)
	{
	  delta += (*BUF_PTR & 127);
	  BUF_PTR++;
	  delta = delta << 7;
	  if (*BUF_PTR & 128)
	    {
	      delta = (*BUF_PTR & 127);
	      BUF_PTR++;
	      delta = (delta << 7) + *BUF_PTR;
	    }
	  else
	    delta += *BUF_PTR;
	}
      else
	delta += *BUF_PTR;
    }
  else
    delta = *BUF_PTR;

  BUF_PTR++;
  debug_midi("%s: buffer addr : %p\n", __FUNCTION__, BUF_PTR);
  debug_midi("%s: delta = %d\n", __FUNCTION__, delta);
  return delta;
}

/* uint_t get_deltatime_from_ptr(byte_t **buffer_ptr) { return _get_varlen_from_ptr(buffer_ptr); } */
/* uint_t get_midibuf_deltatime(byte_t *buffer) { return get_deltatime_from_ptr(&buffer); } */

/* /!\ This function get deltatime from variable-length midi file value move,
   and change the value of the int pointed by idx_ptr */
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
  debug_midi("%s: idx: %d\n", __FUNCTION__, IDX_PTR);
  debug_midi("%s: len=%d\n", __FUNCTION__, delta);
  return delta;
}
