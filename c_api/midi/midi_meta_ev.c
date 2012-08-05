#include "midi/midiev.h"
#include "debug_tool/debug_tool.h"

#include <strings.h>

void		get_midifile_asciitext(byte_t *buffer, midimev_t *metaev, uint_t *offset)
{
  metaev->val = get_varlen_from_idx(buffer, offset);
  bcopy(&(buffer[*offset]), metaev->data, metaev->val);
  metaev->data[metaev->val] = 0;
  metaev->size = metaev->val;
}

uint_t		get_midi_meta_event(midimev_t *metaev, byte_t *buffer)
{
  uint_t offset = 0;
  //byte_t type;

  switch (buffer[offset])
    {
    case ME_SEQUENCENUMBER:
      /* debug_midifile("*** Sequence number event ***\n"); */
      offset++;
      if (buffer[offset] != 2)
	{
	  output_error("Error: unexpected size\n");
	  return FALSE;
	}
      offset++;
      metaev->type = ME_SEQUENCENUMBER;

      metaev->val = (buffer[offset] << 8) + buffer[offset + 1];
      /* debug_midifile("\tSequence number: %i\n", metaev->val); */
      offset += 2;
      break;

    case ME_TEXTEVENT:
      offset++;
      get_midifile_asciitext(buffer, metaev, &offset);
      metaev->type = ME_TEXTEVENT;
      offset += metaev->val;
      break;

    case ME_COPYRIGHTNOTICE:
      offset++;
      get_midifile_asciitext(buffer, metaev, &offset);
      metaev->type = ME_COPYRIGHTNOTICE;
      offset += metaev->val;
      break;

    case ME_NAME:
      /* debug_midifile("*** Sequence name event ***\n"); */
      offset++;
      get_midifile_asciitext(buffer, metaev, &offset);
      metaev->type = ME_NAME;
      /* debug_midifile("\tSequence name: %s (size=%d)\n", (char *) metaev->data, metaev->val); */

      offset += metaev->val;
      break;

    case ME_CUEPOINT:
      offset++;
      get_midifile_asciitext(buffer, metaev, &offset);
      metaev->type = ME_CUEPOINT;

      offset += metaev->val;
      break;

    case ME_ENDOFTRACK:
      /* debug_midifile("*** End of track event %p ***\n", &(buffer[offset]) + 2); */
      metaev->type = ME_ENDOFTRACK;
      offset++;
      break;

    case ME_SETTEMPO:
      /* debug_midifile("*** Set tempo event ***\n"); */
      offset++;
      if (buffer[offset] != 3)
	{
	  output_error("Error: unexpected size\n");
	  return 0;
	}
      offset++;
      metaev->type = ME_SETTEMPO;

      metaev->val = (buffer[offset] << 16) + (buffer[offset + 1] << 8) + buffer[offset + 2];
      /* metaev->val = (buffer[offset]) + (buffer[offset + 1] << 8) + (buffer[offset + 2] << 16); */
      debug_midi("\ttempo: %i (micro second / quarter note)\n", metaev->val);
      offset += 3;
      break;

    case ME_SMPTE_OFFSET:
      /* debug_midi("*** SMPTE offset event ***\n"); */
      offset++;
      if (buffer[offset] != 5)
	{
	  output_error("Error: unexpected size\n");
	  return 0;
	}
      offset++;
      metaev->type = ME_SMPTE_OFFSET;
      /* debug_midi("\tTime: %i:%02i:%02i\n" */
      /* 	  "\tFr: %i\n" */
      /* 	  "\tSubFr: %i\n", */
      /* 	 y a d truc; a faire ;ici c pas ;bon du tout  buffer[0] & 127 >> 7 ? , buffer[1], buffer[2], buffer[3], buffer[4]); */
      offset += 5;
      break;

    case ME_TIMESIGNATURE:
      /* debug_midi("*** Time Signature event ***\n"); */
      offset++;
      if (buffer[offset] != 4)
	{
	  output_error("Error: unexpected size\n");
	  return 0;
	}
      offset++;
      metaev->type = ME_TIMESIGNATURE;

      /* debug_midi("\tNumerator of time sig: %i\n" */
      /*       "\tDenominator of time sig: %i\n" */
      /*       "\tNumber of ticks in metronome click: %i\n" */
      /*       "\tNumber of 32nd notes to the quarter note: %i\n", */
      /*       buffer[offset], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3]); */
      offset += 4;
      break;

    case ME_KEYSIGNATURE:
      /* debug_midi("*** Key Signature event ***\n"); */
      offset++;
      if (buffer[offset] != 2)
	{
	  output_error("Error: unexpected size\n");
	  return 0;
	}
      offset++;
      metaev->type = ME_KEYSIGNATURE;

      /* debug_midi("\tsharps/flats %i\n" */
      /*       "\tmajor/minor %i\n", */
      /*       buffer[offset], buffer[offset + 1]); */
      offset += 2;
      break;

    case ME_SEQUENCERSPECIFIC:
      /* debug_midi("*** Sequencer Specific event ***\n"); */
      offset++;
      metaev->val = get_varlen_from_idx(buffer, &offset);
      metaev->type = ME_SEQUENCERSPECIFIC;

      /* debug_midi("\t Manufacturer's ID %i\n", */
      /*       buffer[offset]); */
      offset += metaev->val;
      break;

    default:
      output_error("!!! UNKNOWN meta event hex:0x%02X dec:%i\n",
		  buffer[offset],
		  buffer[offset]);
      metaev->type = buffer[offset];
      offset++;
      output_error("1 offset=0x%X\n", offset);
      offset += get_varlen_from_idx(buffer, &offset);
      output_error("2 offset=0x%X\n", offset);
      output_error("");
      break;
    }
  return offset;
}
