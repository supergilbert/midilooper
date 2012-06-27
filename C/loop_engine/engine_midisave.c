#include "./engine.h"
#include "debug_tool/debug_tool.h"
#include "midi/midifile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void write_trackctx2midifile(int fd, track_ctx_t *ctx)
{
  midifile_track_t mtrack;

  bzero(&mtrack, sizeof (midifile_track_t));

  COPY_LIST_NODE(&(ctx->track->tickev_list), &(mtrack.track.tickev_list));
  mtrack.track.name = ctx->track->name;

  mtrack.sysex_len = ctx->len;
  write_midifile_track(fd, &mtrack);
}

void engine_save_project(engine_ctx_t *ctx, char *file_path)
{
  int             fd;
  list_iterator_t iter;
  track_ctx_t     *trackctx = NULL;

  if (ctx->track_list.len > 0)
    {
      if (0 == access(file_path, F_OK))
        unlink(file_path);
      debug("start of write");
      fd = open(file_path, O_WRONLY|O_CREAT, (S_IRUSR|S_IWUSR
                                              | S_IRGRP|S_IWGRP
                                              | S_IROTH|S_IWOTH));
      if (fd == -1)
        {
          output_error("problem while open file to save (%s)\n", strerror(errno));
          return;
        }
      write_midifile_header(fd, ctx->track_list.len, ctx->ppq);
      for (iter_init(&iter, &(ctx->track_list));
           iter_node(&iter);
           iter_next(&iter))
        {
          trackctx = iter_node_ptr(&iter);
          write_trackctx2midifile(fd, trackctx);
        }
      close(fd);
    }
  debug("End of write\n\n\n");
}
