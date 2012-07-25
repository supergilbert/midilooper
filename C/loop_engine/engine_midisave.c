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
  mtrack.sysex_portid = (ctx->aseqport_ctx != NULL) ? ctx->aseqport_ctx->output_port : -1;
  write_midifile_track(fd, &mtrack);
}

buf_node_t *_append_sysex_port(buf_node_t *tail, aseqport_ctx_t *aport)
{
  buf_node_t *node = NULL;
  char       *port_name =  (char *)aseqport_get_name(aport);
  size_t     name_len = strlen(port_name);
  byte_t     buf[6];

  set_be32b_uint(buf, (uint_t) aport->output_port);
  set_be16b_uint(&(buf[4]), (uint_t) name_len);
  node = init_buf_node(buf, 6);

  node->next = sysex_buf_node_end((byte_t *) port_name, name_len);
  tail = _append_sysex_header(tail, get_buf_list_size(node), MSQ_PORTNAME_SYSEX);
  tail->next = node;
  return node->next;
}

void write_midifile_track_engine_ctx(int fd, engine_ctx_t *ctx)
{
  list_iterator_t iter;
  aseqport_ctx_t *aport;
  buf_node_t head = {NULL, 0, NULL};
  buf_node_t *node = &head;

  for (iter_init(&iter, &(ctx->aseqport_list)),
         node = &head;
       iter_node(&iter);
       iter_next(&iter))
    {
      aport = iter_node_ptr(&iter);
      node = _append_sysex_port(node, aport);
    }

  node = _append_metaev_eot(node);

  node = get_midifile_trackhdr(get_buf_list_size(head.next));
  node->next = head.next;

  write_buf_list(fd, node);
  free_buf_list(node);
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

      write_midifile_header(fd, ctx->track_list.len + 1, ctx->ppq);

      write_midifile_track_engine_ctx(fd, ctx);

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
