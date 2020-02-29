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

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#include "./engine.h"
#include "debug_tool/debug_tool.h"
#include "midi/midifile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

buf_node_t *_append_sysex_port(buf_node_t *tail, output_t *output, uint_t idx)
{
  buf_node_t *node = NULL;
  char       *port_name = (char *) output_get_name(output);
  size_t     name_len = strlen(port_name);
  byte_t     buf[6];

  set_be32b_uint(buf, idx);
  set_be16b_uint(&(buf[4]), (uint_t) name_len);
  node = add_buf_node(buf, 6);

  node->next = sysex_buf_node_end((byte_t *) port_name, name_len);
  tail = _append_sysex_header(tail,
                              get_buf_list_size(node),
                              MSQ_SYSEX_PORTNAME);
  tail->next = node;
  return node->next;
}

void write_midifile_track_engine_ctx(int fd, engine_ctx_t *ctx)
{
  list_iterator_t iter;
  output_t        *output;
  buf_node_t      head = {NULL, 0, NULL};
  buf_node_t      *node;
  uint_t          idx = 0;

  for (iter_init(&iter, &(ctx->output_list)),
         node = &head,
         idx = 0;
       iter_node(&iter);
       iter_next(&iter),
         idx++)
    {
      output = iter_node_ptr(&iter);
      node = _append_sysex_port(node, output, idx);
    }

  node = _append_metaev_set_tempo(node, ctx->tempo);
  /* node = _append_metaev_eot(node); */
  _append_metaev_eot(node);

  node = create_midifile_trackhdr(get_buf_list_size(head.next));
  node->next = head.next;

  write_buf_list(fd, node);
  free_buf_list(node);
}

int_t get_outputid(list_t *output_list, output_t *output)
{
   list_iterator_t iter;
   output_t        *tmp;
   int_t           idx;

   for (iter_init(&iter, output_list),
          idx = 0;
        iter_node(&iter);
        iter_next(&iter),
          idx++)
     {
       tmp = iter_node_ptr(&iter);
       if (tmp == output)
         return idx;
     }
   return -1;
}

void _set_track_val_list(track_ctx_t *trackctx,
                         list_t *bindings,
                         byte_t *val_list,
                         size_t *list_sz)
{
  list_iterator_t iter_binding, iter_track;
  uint_t          idx;
  track_ctx_t     *trackctx_ptr = NULL;
  binding_t       *binding = NULL;

  memset(val_list, 0, 256);
  for (iter_init(&iter_binding, bindings), idx = 0;
       iter_node(&iter_binding);
       iter_next(&iter_binding))
    {
      binding = iter_node_ptr(&iter_binding);
      for (iter_init(&iter_track, &(binding->tracks));
           iter_node(&iter_track);
           iter_next(&iter_track))
        {
          trackctx_ptr = iter_node_ptr(&iter_track);
          if (trackctx == trackctx_ptr)
            {
              val_list[idx] = binding->val;
              idx++;
              break;
            }
        }
    }
  *list_sz = idx;
}

void set_midifile_track(track_ctx_t *trackctx,
                        midifile_track_t *mtrack)
{
  engine_ctx_t *ctx = trackctx->engine;

  bzero(mtrack, sizeof (midifile_track_t));

  COPY_LIST_NODE(&(trackctx->track->tickev_list), &(mtrack->track.tickev_list));
  mtrack->track.name = trackctx->track->name;

  mtrack->sysex_loop_start = trackctx->loop_start;
  mtrack->sysex_loop_len = trackctx->loop_len;
  mtrack->sysex_portid = trackctx->output != NULL ?
    get_outputid(&(ctx->output_list),
                 trackctx->output) :
    -1;

  _set_track_val_list(trackctx,
                      &(ctx->bindings.notepress),
                      mtrack->bindings.notes,
                      &(mtrack->bindings.notes_sz));
  _set_track_val_list(trackctx,
                      &(ctx->bindings.keypress),
                      mtrack->bindings.keys,
                      &(mtrack->bindings.keys_sz));
}

void gen_midinote_bindings_str(char *mnb_str,
                               byte_t *notes,
                               size_t notes_sz)
{
  const char *notes_name[] =
    {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  const char *name;
  uint_t     idx, note_idx;
  int_t      note_oct;

  for (idx = 0; idx < notes_sz; idx++)
    {
      note_idx = notes[idx] % 12;
      name = notes_name[note_idx];
      while (*name)
        {
          *mnb_str = *name;
          mnb_str++;
          name++;
        }
      note_oct = (notes[idx] / 12) - 1;
      if (note_oct < 0)
        {
          *mnb_str = '-';
          mnb_str++;
          *mnb_str = '1';
          mnb_str++;
        }
      else
        {
          *mnb_str = '0' + note_oct;
          mnb_str++;
        }
    }
  *mnb_str = '\0';
}

void gen_miditrack_info(char *ret_str,
                        engine_ctx_t *ctx,
                        track_ctx_t *trackctx)
{
  midifile_track_t mtrack;
  char             mnb_str[256];

  set_midifile_track(trackctx, &mtrack);
  /* Marking "end of string" */
  mtrack.bindings.keys[mtrack.bindings.keys_sz] = '\0';
  if (mtrack.bindings.notes_sz == 0)
    strcpy(mnb_str, "none");
  else
    gen_midinote_bindings_str(mnb_str,
                              mtrack.bindings.notes,
                              mtrack.bindings.notes_sz);
#define _TRACK_INFO_FORMAT                              \
  "%s loop[%d-%d] out[%s]\nbindings[%s|%s]"
  sprintf(ret_str,
          _TRACK_INFO_FORMAT,
          trackctx->track->name,
          mtrack.sysex_loop_start / ctx->ppq,
          mtrack.sysex_loop_len / ctx->ppq,
          trackctx->output != NULL ?
          output_get_name(trackctx->output) :
          "none",
          mtrack.bindings.keys_sz != 0 ?
          (char *) mtrack.bindings.keys :
          "none",
          mnb_str);
}

#include <errno.h>
void engine_save_project(engine_ctx_t *ctx, const char *file_path, msq_bool_t template)
{
  int              fd;
  list_iterator_t  iter;
  track_ctx_t      *trackctx = NULL;
  midifile_track_t mtrack;

  if (0 == access(file_path, F_OK))
    unlink(file_path);
  debug("start of write");
  fd = open(file_path, O_WRONLY|O_CREAT, (S_IRUSR|S_IWUSR
                                          | S_IRGRP|S_IWGRP
                                          | S_IROTH|S_IWOTH));
  if (fd == -1)
    {
      output_error("problem while open file to save (%s)\n",
                   strerror(errno));
      return;
    }

  write_midifile_header(fd, ctx->track_list.len + 1, ctx->ppq);

  write_midifile_track_engine_ctx(fd, ctx);

  if (ctx->track_list.len > 0)
    {
      for (iter_init(&iter, &(ctx->track_list));
           iter_node(&iter);
           iter_next(&iter))
        {
          trackctx = iter_node_ptr(&iter);
          set_midifile_track(trackctx, &mtrack);
          write_midifile_track(fd, &mtrack, template);
        }
      close(fd);
    }
  debug("End of write\n\n\n");
}
