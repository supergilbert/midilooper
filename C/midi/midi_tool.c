#include "midi/midifile.h"

typedef struct
{
  uint_t tick;
  size_t size;
} hdl_tsz_t;

void hdl_seqev_size(void *addr, void *size_ptr)
{
  seqev_t   *seqev = (seqev_t *) addr;
  size_t    *size = (size_t *) size_ptr;
  midicev_t *midicev = NULL;

    if (seqev->type == MIDICEV)
      {
        midicev = seqev->addr;
        switch (midicev->type)
          {
          case NOTEON:
          case NOTEOFF:
          case CONTROLCHANGE:
            *size += 4;          /* 3 byte of the midi channel event plus
                                    minimal variable length size */
            break;
          default:
            break;
          }
      }
}

#include "debug_tool/debug_tool.h"

void hdl_tickev_size(void *addr, void *hdlptr)
{
  tickev_t  *tickev = (tickev_t *) addr;
  hdl_tsz_t *hdl_tsz = (hdl_tsz_t *) hdlptr;
  uint_t    tickdiff = tickev->tick - hdl_tsz->tick;

  hdl_tsz->size += GETVLVSIZE(tickdiff); /* adding a possible part size of the
                                             new tick event */
  debug("tickdiff: %d ; var len tick size: %d ; buf size: %d", tickdiff, GETVLVSIZE(tickdiff), hdl_tsz->size);
  hdl_tsz->tick = tickev->tick;
  foreach_list_node(&(tickev->seqev_list), hdl_seqev_size, &(hdl_tsz->size));
}

size_t midifile_trackev_size(track_t *track)
{
  hdl_tsz_t hdl_tsz = {0, 0};

  foreach_list_node(&(track->tickev_list), hdl_tickev_size, &hdl_tsz);
  return hdl_tsz.size;
}
