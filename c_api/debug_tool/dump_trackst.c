#include "midi/midiev.h"
#include "debug_tool/debug_tool.h"

void dump_seqev(seqev_t *seqev)
{
  midicev_t *midicev = NULL;

  output("\tseqev: addr=%p deleted=%s",
         seqev,
         seqev->deleted == TRUE ? "\033[31mTRUE\033[0m" : "FALSE");
  if (seqev->type == MIDICEV)
    {
      output(" type=%s", "MIDICEV");
      midicev = (midicev_t *) seqev->addr;
      switch (midicev->type)
        {
        case NOTEON:
          output(" | NOTEON  num=%hhd val=%hhd\n", midicev->event.note.num, midicev->event.note.val);
          break;
        case NOTEOFF:
          output(" | NOTEOFF num=%hhd val=%hhd\n", midicev->event.note.num, midicev->event.note.val);
          break;
        default:
          output(" | Unsupported event\n");
        }
    }
  else
    output("type=UNKNOWN\n");
}

void dump_tickev(tickev_t *tickev)
{
  list_iterator_t seqevit;

  output("At tick %d %s got %d event(s)\n",
         tickev->tick,
         tickev->deleted == TRUE ? "(deleted)" : "(not deleted)",
         tickev->seqev_list.len);
  for (iter_init(&seqevit, &(tickev->seqev_list));
       iter_node(&seqevit) != NULL;
       iter_next(&seqevit))
    dump_seqev((seqev_t *) iter_node_ptr(&(seqevit)));
}

void dump_track(track_t *track)
{
  list_iterator_t tickit;

  output("track \"%s\" got %d tick event(s)\n", track->name, track->tickev_list.len);
  for (iter_init(&tickit, &(track->tickev_list));
       iter_node(&tickit) != NULL;
       iter_next(&tickit))
      dump_tickev((tickev_t *) iter_node_ptr(&tickit));
}
