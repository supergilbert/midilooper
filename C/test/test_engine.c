#include "clock/clock.h"
#include "beta_engine/engine.h"
#include "midi/midifile.h"
#include "debug_tool/debug_tool.h"

track_t *get_track_number(midifile_t *midifile, uint_t number)
{
  list_iterator_t iter;
  iter_init(&iter, &(midifile->track_list));

  debug("Searching for track number %i\n",
        number);
  while (iter_node(&iter) != NULL && number)
    {
      number--;
      iter_next(&iter);
    }
  if (number == 0)
    return iter_node_ptr(&iter);
  else
    return NULL;
}

bool_t  test_engine(midifile_t *midifile)
{
  engine_ctx_t ctx;
  bool_t       eng_res = FALSE;

  bzero(&ctx, sizeof (engine_ctx_t));
  ctx.track_ctx.track = merge_all_track("???", &(midifile->track_list));
  iter_init(&(ctx.track_ctx.current_tickev),
            &(ctx.track_ctx.track->tickev_list));
  ctx.aseq_ctx = init_aseq("test_engine");
  set_clockloop_ms_ppq(&(ctx.looph),
                       midifile->info.tempo,
                       midifile->info.ppq,
                       engine_cb, &ctx);

  debug("Starting engine\n");
  eng_res = start_engine(&ctx);
  debug("End of engine\n");

  //  free_clockloop_struct(ctx.looph);
  free_aseq(ctx.aseq_ctx);
  free_track(ctx.track_ctx.track);
  return eng_res;
}

midifile_t  *init_test(int ac, char **av)
{
  int                   fd;
  midifile_t        *midifile;

  if (ac < 2)
    {
      output_error("Wrong number of argument\n");
      exit(1);
    }
  fd = open(av[1], O_RDONLY);
  if (fd == -1)
    {
      output_error("Problem with file \"%s\": %s\n", av[1], strerror(errno));
      exit(1);
    }
  debug("Reading file %s\n", av[1]);
  midifile = read_midifile_fd(fd);
  if (midifile == NULL)
    {
      output_error(ERROR_FMT"Error while reading file %s", ERROR_ARG, av[1]);
      exit(1);
    }
  if (midifile->info.tempo  == 0)
    {
      output_error("!!! tempo=0 changing it to \n");
      midifile->info.tempo = 500000;
    }
  debug("tempo=%i (bpm=%i)\nppq=%i\n",
	midifile->info.tempo,
	60000000 / midifile->info.tempo,
	midifile->info.ppq);
  return midifile;
}

void list_tracks(list_t *tracklist)
{
  list_iterator_t iter;
  track_t *track = NULL;
  uint_t num = 0;

  iter_init(&iter, tracklist);
  while (iter_node(&iter))
    {
      track = iter_node_ptr(&iter);
      output("track %d: %d events on \"%s\"\n",
             num,
             track->tickev_list.len,
             track->name);
      iter_next(&iter);
      num++;
    }
}

int			main(int ac, char **av)
{
  midifile_t    *midifile = NULL;
  int           ret = 1;

  midifile = init_test(ac, av);
  if (midifile == NULL)
    {
      output_error("Can't read midifile\n");
      exit(1);
    }

  if (ac == 3)
    {
      debug("av[2] %s\n", av[2]);
      if (strcmp(av[2], "info") != 0)
	{
          output("assuming expecting file info");
        }
      output("Format type     : %s (type=%d)\n"
             "Number of track : %d\n"
             "Tempo           : %d ms (= %d bpm)\n"
             "PPQ             : %d\n",
             get_midifile_type_str(midifile->info.type),
             midifile->info.type,
             midifile->number_of_track,
             midifile->info.tempo,
             midifile->info.tempo == 0 ? 0 : (60000000 / midifile->info.tempo),
             midifile->info.ppq);
      list_tracks(&(midifile->track_list));
      free_midifile(midifile);
      exit(0);
    }

  ret = test_engine(midifile);
  free_midifile(midifile);
  return ret;
}
