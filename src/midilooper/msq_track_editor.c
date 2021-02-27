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

/* You should have received a copy of the GNU Gneneral Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */

#include <string.h>
#include <unistd.h>

#include <pbt_draw.h>
#include <pbt_event_handler.h>
#include <pbt_font.h>
#include <pbt_gadget.h>
#include <pbt_gadget_window.h>
#include <pbt_pixel_buffer.h>
#include <pbt_tools.h>
#include <pbt_window_gadget.h>
#include <wbe_glfw.h>
#include <wbe_gl.h>
#include <wbe_pbw.h>

#include <loop_engine/engine.h>
#include <seqtool/ev_iterator.h>

#include "msq_gui.h"

#include "msq_track_editor_inc.h"

static const char *eng_note_strlist[] = {"C",
                                         "C#",
                                         "D",
                                         "D#",
                                         "E",
                                         "F",
                                         "F#",
                                         "G",
                                         "G#",
                                         "A",
                                         "A#",
                                         "B"};

static const msq_bool_t tonale_regular_note[] = {MSQ_TRUE,
                                                 MSQ_FALSE,
                                                 MSQ_TRUE,
                                                 MSQ_FALSE,
                                                 MSQ_TRUE,
                                                 MSQ_TRUE,
                                                 MSQ_FALSE,
                                                 MSQ_TRUE,
                                                 MSQ_FALSE,
                                                 MSQ_TRUE,
                                                 MSQ_FALSE,
                                                 MSQ_TRUE};

static const char *value_type_list[] = {"   Note Vel.",
                                        "   Pitch",
                                        "   Ctrl.   0",
                                        "   Ctrl.   1",
                                        "   Ctrl.   2",
                                        "   Ctrl.   3",
                                        "   Ctrl.   4",
                                        "   Ctrl.   5",
                                        "   Ctrl.   6",
                                        "   Ctrl.   7",
                                        "   Ctrl.   8",
                                        "   Ctrl.   9",
                                        "   Ctrl.  10",
                                        "   Ctrl.  11",
                                        "   Ctrl.  12",
                                        "   Ctrl.  13",
                                        "   Ctrl.  14",
                                        "   Ctrl.  15",
                                        "   Ctrl.  16",
                                        "   Ctrl.  17",
                                        "   Ctrl.  18",
                                        "   Ctrl.  19",
                                        "   Ctrl.  20",
                                        "   Ctrl.  21",
                                        "   Ctrl.  22",
                                        "   Ctrl.  23",
                                        "   Ctrl.  24",
                                        "   Ctrl.  25",
                                        "   Ctrl.  26",
                                        "   Ctrl.  27",
                                        "   Ctrl.  28",
                                        "   Ctrl.  29",
                                        "   Ctrl.  30",
                                        "   Ctrl.  31",
                                        "   Ctrl.  32",
                                        "   Ctrl.  33",
                                        "   Ctrl.  34",
                                        "   Ctrl.  35",
                                        "   Ctrl.  36",
                                        "   Ctrl.  37",
                                        "   Ctrl.  38",
                                        "   Ctrl.  39",
                                        "   Ctrl.  40",
                                        "   Ctrl.  41",
                                        "   Ctrl.  42",
                                        "   Ctrl.  43",
                                        "   Ctrl.  44",
                                        "   Ctrl.  45",
                                        "   Ctrl.  46",
                                        "   Ctrl.  47",
                                        "   Ctrl.  48",
                                        "   Ctrl.  49",
                                        "   Ctrl.  50",
                                        "   Ctrl.  51",
                                        "   Ctrl.  52",
                                        "   Ctrl.  53",
                                        "   Ctrl.  54",
                                        "   Ctrl.  55",
                                        "   Ctrl.  56",
                                        "   Ctrl.  57",
                                        "   Ctrl.  58",
                                        "   Ctrl.  59",
                                        "   Ctrl.  60",
                                        "   Ctrl.  61",
                                        "   Ctrl.  62",
                                        "   Ctrl.  63",
                                        "   Ctrl.  64",
                                        "   Ctrl.  65",
                                        "   Ctrl.  66",
                                        "   Ctrl.  67",
                                        "   Ctrl.  68",
                                        "   Ctrl.  69",
                                        "   Ctrl.  70",
                                        "   Ctrl.  71",
                                        "   Ctrl.  72",
                                        "   Ctrl.  73",
                                        "   Ctrl.  74",
                                        "   Ctrl.  75",
                                        "   Ctrl.  76",
                                        "   Ctrl.  77",
                                        "   Ctrl.  78",
                                        "   Ctrl.  79",
                                        "   Ctrl.  80",
                                        "   Ctrl.  81",
                                        "   Ctrl.  82",
                                        "   Ctrl.  83",
                                        "   Ctrl.  84",
                                        "   Ctrl.  85",
                                        "   Ctrl.  86",
                                        "   Ctrl.  87",
                                        "   Ctrl.  88",
                                        "   Ctrl.  89",
                                        "   Ctrl.  90",
                                        "   Ctrl.  91",
                                        "   Ctrl.  92",
                                        "   Ctrl.  93",
                                        "   Ctrl.  94",
                                        "   Ctrl.  95",
                                        "   Ctrl.  96",
                                        "   Ctrl.  97",
                                        "   Ctrl.  98",
                                        "   Ctrl.  99",
                                        "   Ctrl. 100",
                                        "   Ctrl. 101",
                                        "   Ctrl. 102",
                                        "   Ctrl. 103",
                                        "   Ctrl. 104",
                                        "   Ctrl. 105",
                                        "   Ctrl. 106",
                                        "   Ctrl. 107",
                                        "   Ctrl. 108",
                                        "   Ctrl. 109",
                                        "   Ctrl. 110",
                                        "   Ctrl. 111",
                                        "   Ctrl. 112",
                                        "   Ctrl. 113",
                                        "   Ctrl. 114",
                                        "   Ctrl. 115",
                                        "   Ctrl. 116",
                                        "   Ctrl. 117",
                                        "   Ctrl. 118",
                                        "   Ctrl. 119",
                                        "   Ctrl. 120",
                                        "   Ctrl. 121",
                                        "   Ctrl. 122",
                                        "   Ctrl. 123",
                                        "   Ctrl. 124",
                                        "   Ctrl. 125",
                                        "   Ctrl. 126",
                                        "   Ctrl. 127"};

static const char *channel_list[] = {"   Channel  0",
                                     "   Channel  1",
                                     "   Channel  2",
                                     "   Channel  3",
                                     "   Channel  4",
                                     "   Channel  5",
                                     "   Channel  6",
                                     "   Channel  7",
                                     "   Channel  8",
                                     "   Channel  9",
                                     "   Channel 10",
                                     "   Channel 11",
                                     "   Channel 12",
                                     "   Channel 13",
                                     "   Channel 14"};

static const char *resolution_list[] = {"4xqn",
                                        "2xqn",
                                        "qn (beat)",
                                        "qn/2",
                                        "qn/4",
                                        "qn/8",
                                        "qn/16",
                                        "qn/32",
                                        "qn/64",
                                        "qn/3",
                                        "qn/6",
                                        "qn/12",
                                        "qn/24"};

static list_t _msq_editor_clipboard = {};
static unsigned char _msq_editor_clipboard_note_max;

#define RESOLUTION_LIST_LEN 13

#define DIV_SIZE_MIN 20

#define DEFAULT_GGTBAR_WIDTH 10
#define MIN_GRID_WIDTH 320
#define MIN_GRID_HEIGHT 240

#define MAX_14b_VAL 16383
#define MAX_7b_VAL  127

#define MSQGETPPQ(_editor_ctx)                  \
  ((_editor_ctx)->track_ctx->engine->ppq)

#define XPOS2TICK(_editor_ctx, xpos)                                    \
  ((xpos) * ((int) MSQGETPPQ(_editor_ctx)) / ((int) (_editor_ctx)->qn_size))

#define TICK2XPOS(_editor_ctx, tick)                                    \
  ((tick) * ((int) (_editor_ctx)->qn_size) / ((int) MSQGETPPQ(_editor_ctx)))

#define NOTE2YPOS(_editor_ctx, note)                    \
  ((MAX_7b_VAL - (note)) * (_editor_ctx)->note_height)

#define YPOS2NOTE(_editor_ctx, ypos)                    \
  (MAX_7b_VAL - ((ypos) / (_editor_ctx)->note_height))

#define GRIDYMAX(_editor_ctx)                   \
  NOTE2YPOS((_editor_ctx), -1)

#define GRIDXPOS(xpos, grid)                                            \
  ((xpos) + (grid)->editor_ctx->hadj.pos - pbt_ggt_xpos(&((grid)->wgt)))

#define GRIDYPOS(ypos, grid)                                            \
  ((ypos) + (grid)->editor_ctx->vadj.pos - pbt_ggt_ypos(&((grid)->wgt)))

#define tctx_window_fg(_editor_ctx)                             \
  msq_theme_window_fg((_editor_ctx)->theme->global_theme)

#define tctx_window_bg(_editor_ctx)                             \
  msq_theme_window_bg((_editor_ctx)->theme->global_theme)

#define tctx_frame_fg(_editor_ctx)                              \
  msq_theme_frame_fg((_editor_ctx)->theme->global_theme)

#define tctx_frame_bg(_editor_ctx)                              \
  msq_theme_frame_bg((_editor_ctx)->theme->global_theme)

#define tctx_wgt_normal_fg(_editor_ctx)                         \
  msq_theme_wgt_normal_fg((_editor_ctx)->theme->global_theme)

#define tctx_wgt_normal_bg(_editor_ctx)                         \
  msq_theme_wgt_normal_bg((_editor_ctx)->theme->global_theme)

#define tctx_cursor_arrow(_editor_ctx)                          \
  msq_theme_cursor_arrow((_editor_ctx)->theme->global_theme)

#define tctx_cursor_pencil(_editor_ctx)                         \
  msq_theme_cursor_pencil((_editor_ctx)->theme->global_theme)

#define tctx_cursor_grab(_editor_ctx)                           \
  msq_theme_cursor_grab((_editor_ctx)->theme->global_theme)

#define tctx_cursor_grabbing(_editor_ctx)                       \
  msq_theme_cursor_grabbing((_editor_ctx)->theme->global_theme)

#define tctx_cursor_hresize(_editor_ctx)                        \
  msq_theme_cursor_hresize((_editor_ctx)->theme->global_theme)

#define tctx_cursor_vresize(_editor_ctx)                        \
  msq_theme_cursor_vresize((_editor_ctx)->theme->global_theme)

#define SCROLL_INC 100

void _msq_bytearray_set_bit(unsigned char *array,
                            size_t bit_idx,
                            wbe_bool_t bool)
{
  size_t octet_idx = bit_idx / 8;

  bit_idx = bit_idx % 8;
  WBE_SET_BIT(array[octet_idx], bit_idx, bool);
}

wbe_bool_t _msq_bytearray_get_bit(unsigned char *array,
                                  size_t bit_idx)
{
  size_t octet_idx = bit_idx / 8;

  bit_idx = bit_idx % 8;
  return WBE_GET_BIT(array[octet_idx], bit_idx);
}

void msq_track_info_init(msq_track_info_t *track_info, track_t *track)
{
  ev_iterator_t evit;
  midicev_t *mcev;
  msq_channel_info_t *channel_info;
  size_t idx;

  for (idx = 0; idx < CHANNEL_LIST_LEN; idx++)
    {
      channel_info = &(track_info->channels_info[idx]);
      memset(channel_info,
             0,
             sizeof (msq_channel_info_t));
      channel_info->note_min = 255;
    }
  track_info->ev_len = 0;
  track_info->min_tick = (unsigned int) -1; /* Max */
  track_info->max_tick = 0;
  for (mcev = evit_init_midiallchannel(&evit, &(track->tickev_list));
       mcev != NULL;
       mcev = evit_next_midiallchannel(&evit))
    {
      channel_info = &(track_info->channels_info[mcev->chan]);
      if (mcev->type == MSQ_MIDI_NOTEOFF || mcev->type == MSQ_MIDI_NOTEON)
        {
          _msq_bytearray_set_bit(channel_info->usage,
                                 0,
                                 1);
          if (mcev->event.note.num < channel_info->note_min)
            channel_info->note_min = mcev->event.note.num;
          if (mcev->event.note.num > channel_info->note_max)
            channel_info->note_max = mcev->event.note.num;
        }
      else if (mcev->type == MSQ_MIDI_PITCHWHEELCHANGE)
        _msq_bytearray_set_bit(channel_info->usage,
                               1,
                               1);
      else if (mcev->type == MSQ_MIDI_CONTROLCHANGE)
        _msq_bytearray_set_bit(channel_info->usage,
                               2 + mcev->event.ctrl.num,
                               1);
      if (track_info->min_tick > evit.tick)
        track_info->min_tick = evit.tick;
      if (track_info->max_tick < evit.tick)
        track_info->max_tick = evit.tick;
      channel_info->ev_len += 1;
      track_info->ev_len += 1;
    }
}

void msq_adj_inc(pbt_adj_t *adj, unsigned int inc)
{
  if ((adj->pos + inc) > (adj->max - adj->size))
    adj->pos = adj->max - adj->size;
  else
    adj->pos += inc;
}

void msq_adj_dec(pbt_adj_t *adj, unsigned int dec)
{
  if (adj->pos < dec)
    adj->pos = 0;
  else
    adj->pos -= dec;
}

void handle_vadj_change(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  pbt_wgt_draw(&(track_editor->piano_wgt));
  pbt_ggt_draw(&(track_editor->grid_wgt.wgt));
  pbt_ggt_win_put_buffer(&(track_editor->ggt_win));
}

void handle_hadj_change(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  pbt_ggt_draw(&(track_editor->timeline_wgt));
  pbt_ggt_draw(&(track_editor->grid_wgt.wgt));
  pbt_ggt_draw(&(track_editor->value_wgt.wgt));
  pbt_ggt_win_put_buffer(&(track_editor->ggt_win));
}

void draw_piano_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = piano_wgt->editor_ctx;
  unsigned int pxl_pos;
  unsigned int note_max_pos =
    GRIDYMAX(editor_ctx) - editor_ctx->vadj.pos;
  unsigned int note;
  unsigned int tmp;
  const char *note_str;

  if (note_max_pos > _pbt_ggt_height(ggt))
    note_max_pos = _pbt_ggt_height(ggt);

  pbt_pbarea_fillrect(&(ggt->pbarea),
                      0,
                      0,
                      _pbt_ggt_width(ggt),
                      note_max_pos,
                      editor_ctx->theme->piano_white_color);

  tmp = editor_ctx->vadj.pos % editor_ctx->note_height;
  if (tmp != 0)
    {
      pxl_pos = editor_ctx->note_height - tmp;

      note = YPOS2NOTE(editor_ctx,
                       editor_ctx->vadj.pos - tmp);
      note_str = eng_note_strlist[note % 12];
      if (tonale_regular_note[note % 12])
        pbt_pbarea_printf(&(ggt->pbarea),
                          &(editor_ctx->theme->global_theme->theme.font),
                          tctx_frame_fg(editor_ctx),
                          0,  - tmp + 1,
                          "% 4d %-2s % 2d",
                          note,
                          note_str,
                          (note / 12) - 1);
      else
        {
          pbt_pbarea_fillrect(&(ggt->pbarea),
                              0,
                              0,
                              _pbt_ggt_width(ggt),
                              pxl_pos,
                              editor_ctx->theme->piano_black_color);

          pbt_pbarea_printf(&(ggt->pbarea),
                            &(editor_ctx->theme->global_theme->theme.font),
                            tctx_frame_fg(editor_ctx),
                            0, - tmp + 1,
                            "% 4d %-2s % 2d",
                            note,
                            note_str,
                            (note / 12) - 1);
        }
    }
  else
    pxl_pos = 0;

  while ((pxl_pos + 1) < note_max_pos)
    {
      note = YPOS2NOTE(editor_ctx,
                       editor_ctx->vadj.pos + pxl_pos);
      note_str = eng_note_strlist[note % 12];
      pbt_pbarea_put_hline(&(ggt->pbarea),
                           0,
                           pxl_pos,
                           _pbt_ggt_width(ggt),
                           tctx_frame_fg(editor_ctx));
      if (tonale_regular_note[note % 12])
        pbt_pbarea_printf(&(ggt->pbarea),
                          &(editor_ctx->theme->global_theme->theme.font),
                          tctx_frame_fg(editor_ctx),
                          0, pxl_pos + 1,
                          "% 4d %-2s % 2d",
                          note,
                          note_str,
                          (note / 12) - 1);
      else
        {
          if ((pxl_pos + 1 + editor_ctx->note_height - 2) >= note_max_pos)
            tmp = note_max_pos - (pxl_pos + 1);
          else
            tmp = editor_ctx->note_height - 1;

          pbt_pbarea_fillrect(&(ggt->pbarea),
                              0,
                              pxl_pos + 1,
                              _pbt_ggt_width(ggt),
                              tmp,
                              editor_ctx->theme->piano_black_color);

          pbt_pbarea_printf(&(ggt->pbarea),
                            &(editor_ctx->theme->global_theme->theme.font),
                            tctx_frame_fg(editor_ctx),
                            0, pxl_pos + 1,
                            "% 4d %-2s % 2d",
                            note,
                            note_str,
                            (note / 12) - 1);
        }
      pxl_pos += editor_ctx->note_height;
    }
}

unsigned int piano_ggt_get_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  /* track_editor_ctx_t *editor_ctx = wgt->priv; */

  return piano_wgt->editor_ctx->theme->piano_width;
}

void piano_wgt_update_area_cb(pbt_ggt_t *ggt, pbt_pbarea_t *pbarea)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = piano_wgt->editor_ctx;

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
  editor_ctx->vadj.size = pbarea->height;
  editor_ctx->vadj.max = GRIDYMAX(editor_ctx);
  if (editor_ctx->vadj.size > editor_ctx->vadj.max)
    editor_ctx->vadj.max = editor_ctx->vadj.size;
  if ((editor_ctx->vadj.pos + editor_ctx->vadj.size) > editor_ctx->vadj.max)
    editor_ctx->vadj.pos = editor_ctx->vadj.max - editor_ctx->vadj.size;
}

void pbt_gl_texncol_put_rect(wbe_pbw_t *pb_win,
                             unsigned int xpos, unsigned int ypos,
                             unsigned int width, unsigned int height,
                             unsigned char *color)
{
  int tex_width, tex_height;
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;
  GLfloat color_float[] = {(float) color[0] / 0xFF,
                           (float) color[1] / 0xFF,
                           (float) color[2] / 0xFF,
                           (float) color[3] / 0xFF};

  tex_width = pb_win->buffer.width;
  tex_height = pb_win->buffer.height;
  xmin_float = (xpos * 2.0 / tex_width) - 1.0;
  ymax_float = (-((float) ypos) * 2.0 / tex_height) + 1.0;
  xmax_float = ((xpos + width) * 2.0 / tex_width) - 1.0;
  ymin_float = (-((float) (ypos + height)) * 2.0 / tex_height) + 1.0;

  wbe_pbw_make_context(pb_win);
  _wbe_gl_texture_n_color_put_rect(xmin_float,
                                   ymin_float,
                                   xmax_float,
                                   ymax_float,
                                   color_float);
  wbe_gl_flush();
}

void _pbt_gl_col_put_rect(wbe_pbw_t *pb_win,
                          unsigned int xpos, unsigned int ypos,
                          unsigned int width, unsigned int height,
                          unsigned char *color)
{
  int tex_width, tex_height;
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;
  GLfloat color_float[] = {(float) color[0] / 0xFF,
                           (float) color[1] / 0xFF,
                           (float) color[2] / 0xFF,
                           (float) color[3] / 0xFF};

  tex_width = pb_win->buffer.width;
  tex_height = pb_win->buffer.height;
  xmin_float = (xpos * 2.0 / tex_width) - 1.0;
  ymax_float = (-((float) ypos) * 2.0 / tex_height) + 1.0;
  xmax_float = ((xpos + width) * 2.0 / tex_width) - 1.0;
  ymin_float = (-((float) (ypos + height)) * 2.0 / tex_height) + 1.0;

  _wbe_gl_texture_n_color_put_rect(xmin_float,
                                   ymin_float,
                                   xmax_float,
                                   ymax_float,
                                   color_float);
  wbe_gl_flush();
}

void _piano_wgt_highlight_note(pbt_wgt_t *wgt, unsigned char note)
{
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = piano_wgt->editor_ctx;
  /* track_editor_ctx_t *editor_ctx = wgt->priv; */
  unsigned int ymin = pbt_ggt_ypos(wgt)
    + NOTE2YPOS(editor_ctx, note)
    - editor_ctx->vadj.pos;
  unsigned int height = editor_ctx->note_height;
  unsigned char color[] = {0x0, 0x0, 0x0, 0xA0};

  /* TODO refresh last pos */
  pbt_wgt_gl_refresh(wgt);

  if (ymin < pbt_ggt_ypos(wgt))
    {
      height = editor_ctx->note_height - (pbt_ggt_ypos(wgt) - ymin);
      ymin = pbt_ggt_ypos(wgt);
    }
  else if ((ymin + editor_ctx->note_height)
           > (pbt_ggt_ypos(wgt) + pbt_ggt_height(wgt)))
    {
      height = (pbt_ggt_ypos(wgt) + pbt_ggt_height(wgt)) - ymin;
    }

  pbt_gl_texncol_put_rect(&(wgt->ggt_win->pb_win),
                          pbt_ggt_xpos(wgt),
                          ymin,
                          editor_ctx->theme->piano_width,
                          height,
                          color);
}

void piano_wgt_update_highlight(pbt_wgt_t *wgt, unsigned int ypos)
{
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = piano_wgt->editor_ctx;
  /* track_editor_ctx_t *editor_ctx = wgt->priv; */
  static unsigned char last_note = 0;
  unsigned char new_note;

  pbt_ggt_win_make_context(wgt->ggt_win);

  if ((ypos >= pbt_ggt_ypos(wgt))
      && (ypos < (pbt_ggt_ypos(wgt) + pbt_ggt_height(wgt))))
    {
      new_note = YPOS2NOTE(editor_ctx,
                           (ypos - pbt_ggt_ypos(wgt)) + editor_ctx->vadj.pos);
      if (last_note == new_note)
        return;
      if (new_note > 127)
        return;

      _piano_wgt_highlight_note(wgt, new_note);
      last_note = new_note;
    }
  else
    pbt_wgt_gl_refresh(wgt);
}

void trackctx_play_noteoff(track_ctx_t *track_ctx,
                           unsigned char note,
                           unsigned char channel)
{
  midicev_t mcev = {.chan = channel,
                    .type = MSQ_MIDI_NOTEOFF,
                    .event.note.num = note,
                    .event.note.val = 0};

  if (track_ctx->output != NULL)
    output_write(track_ctx->output, &mcev);
}

void trackctx_play_noteon(track_ctx_t *track_ctx,
                          unsigned char note,
                          unsigned char channel,
                          unsigned short value)
{
  midicev_t mcev = {.chan = channel,
                    .type = MSQ_MIDI_NOTEON,
                    .event.note.num = note,
                    .event.note.val = value};

  if (track_ctx->output != NULL)
    output_write(track_ctx->output, &mcev);
}

pbt_bool_t piano_wgt_unset_focus_cb(pbt_ggt_t *ggt,
                                    wbe_window_input_t *winev,
                                    void *editor_ctx_addr)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = piano_wgt->editor_ctx;
  /* track_editor_ctx_t *editor_ctx = wgt->priv; */
  unsigned char new_note;

  piano_wgt_update_highlight(wgt, winev->ypos);

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    {
      trackctx_play_noteoff(editor_ctx->track_ctx,
                            editor_ctx->tmp_coo[0],
                            editor_ctx->channel);
      return PBT_TRUE;
    }

  new_note = YPOS2NOTE(editor_ctx,
                       winev->ypos - pbt_ggt_ypos(wgt) + editor_ctx->vadj.pos);

  if (editor_ctx->tmp_coo[0] == new_note)
    return PBT_FALSE;
  if (new_note > 127)
    return PBT_FALSE;

  _piano_wgt_highlight_note(wgt, new_note);
  trackctx_play_noteoff(editor_ctx->track_ctx,
                        editor_ctx->tmp_coo[0],
                        editor_ctx->channel);
  trackctx_play_noteon(editor_ctx->track_ctx,
                       new_note,
                       editor_ctx->channel,
                       editor_ctx->default_velocity);
  editor_ctx->tmp_coo[0] = new_note;

  return PBT_FALSE;
}

void msq_draw_hggts(msq_hggts_t *hggts)
{
  pbt_wgt_t *wgt;

  _pbt_ggt_draw(hggts->piano);
  _pbt_ggt_draw(hggts->grid);
  _pbt_ggt_draw(hggts->vscroll);
  wgt = hggts->grid->priv;
  pbt_ggt_win_put_buffer(wgt->ggt_win);
}

pbt_bool_t piano_wgt_set_focus_cb(pbt_ggt_t *ggt,
                                  wbe_window_input_t *winev,
                                  void *editor_ctx_addr)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_piano_wgt_t *piano_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = piano_wgt->editor_ctx;
  unsigned char new_note;

  piano_wgt_update_highlight(wgt, winev->ypos);

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_TRUE)
    {
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          new_note = YPOS2NOTE(editor_ctx,
                               winev->ypos
                               - pbt_ggt_ypos(wgt)
                               + editor_ctx->vadj.pos);
          if (new_note <= 127)
            {
              trackctx_play_noteon(editor_ctx->track_ctx,
                                   new_note,
                                   editor_ctx->channel,
                                   editor_ctx->default_velocity);
              editor_ctx->tmp_coo[0] = new_note;
            }
          return PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 3) == 1)
        {
          msq_adj_inc(&(editor_ctx->vadj), SCROLL_INC);
          msq_draw_hggts(piano_wgt->hggts);
        }
      else if (WBE_GET_BIT(winev->buttons, 4) == 1)
        {
          msq_adj_dec(&(editor_ctx->vadj), SCROLL_INC);
          msq_draw_hggts(piano_wgt->hggts);
        }
    }

  return PBT_FALSE;
}

void piano_wgt_init_ev_cb(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           piano_wgt_set_focus_cb,
                           wgt->priv);
  pbt_evh_add_unset_focus_cb(&(ggt_win->evh),
                             &(wgt->ggt),
                             piano_wgt_unset_focus_cb,
                             wgt->priv);
}

void piano_wgt_init(msq_piano_wgt_t *piano_wgt,
                    track_editor_ctx_t *editor_ctx,
                    msq_hggts_t *hggts)
{
  piano_wgt->editor_ctx = editor_ctx;
  piano_wgt->hggts = hggts;
  piano_wgt->wgt.priv = piano_wgt;

  piano_wgt->wgt.ggt.priv = &(piano_wgt->wgt);
  piano_wgt->wgt.ggt.get_min_width = piano_ggt_get_width;
  piano_wgt->wgt.ggt.get_max_width = piano_ggt_get_width;
  piano_wgt->wgt.ggt.get_min_height = pbt_ggt_return_zero;
  piano_wgt->wgt.ggt.get_max_height = pbt_ggt_return_zero;
  piano_wgt->wgt.ggt.update_area_cb = piano_wgt_update_area_cb;
  piano_wgt->wgt.ggt.draw_cb = draw_piano_cb;
  piano_wgt->wgt.ggt.destroy_cb = pbt_wgt_evnode_destroy;

  piano_wgt->wgt.init_ev_cb = piano_wgt_init_ev_cb;
}

void draw_timeline_loop(pbt_pbarea_t *pbarea,
                        track_editor_ctx_t *editor_ctx,
                        unsigned int loop_start,
                        unsigned int loop_len)
{
  unsigned int size = editor_ctx->theme->global_theme->theme.font.max_height;
  unsigned int ypos = pbarea->height - size;
  unsigned int pxl_pos = TICK2XPOS(editor_ctx, loop_start);

  pbt_pbarea_fillrect(pbarea,
                      0, ypos,
                      pbarea->width, size,
                      tctx_frame_bg(editor_ctx));

  if ((pxl_pos >= editor_ctx->hadj.pos)
      && (pxl_pos + size) < (editor_ctx->hadj.pos + editor_ctx->hadj.size))
    pbt_pbarea_draw_triangle_right(pbarea,
                                   pxl_pos - editor_ctx->hadj.pos,
                                   ypos,
                                   size,
                                   tctx_frame_fg(editor_ctx));

  pxl_pos =  TICK2XPOS(editor_ctx, loop_start + loop_len);
  if ((pxl_pos >= editor_ctx->hadj.pos)
      && (pxl_pos + size) < (editor_ctx->hadj.pos + editor_ctx->hadj.size))
    pbt_pbarea_draw_triangle_left(pbarea,
                                  pxl_pos - editor_ctx->hadj.pos - (size >> 1),
                                  ypos,
                                  size,
                                  tctx_frame_fg(editor_ctx));

}

void draw_timeline_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;
  track_editor_ctx_t *editor_ctx = &(track_editor->editor_ctx);
  unsigned int pxl_pos;
  unsigned int tick_pos;
  unsigned int qn_num;
  unsigned int tmp;
  unsigned int stride_4x4;
  unsigned int font_max_height
    = editor_ctx->theme->global_theme->theme.font.max_height;
  unsigned int size;

  pbt_pbarea_fill(&(ggt->pbarea), tctx_frame_bg(editor_ctx));

  if (editor_ctx->vline_stride != 0)
    {
      tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
      tick_pos +=
        editor_ctx->vline_stride - (tick_pos % editor_ctx->vline_stride);
      for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos,
             size = font_max_height / 2;
           pxl_pos < _pbt_ggt_width(ggt);
           tick_pos += editor_ctx->vline_stride,
             pxl_pos = TICK2XPOS(editor_ctx, tick_pos)
             - editor_ctx->hadj.pos)
        pbt_pbarea_put_vline(&(ggt->pbarea),
                             pxl_pos, 0, 5,
                             editor_ctx->theme->smooth_line_color);
    }

  tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
  tick_pos +=  editor_ctx->quantize - (tick_pos % editor_ctx->quantize);
  for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos;
       pxl_pos < _pbt_ggt_width(ggt);
       tick_pos += editor_ctx->quantize,
         pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos)
    pbt_pbarea_put_vline(&(ggt->pbarea),
                         pxl_pos, 0, font_max_height,
                         tctx_wgt_normal_fg(editor_ctx));

  stride_4x4 = 4 * MSQGETPPQ(editor_ctx);
  tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
  tick_pos +=  stride_4x4 - (tick_pos % stride_4x4);
  for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos,
         size = font_max_height * 3 / 2;
       pxl_pos < _pbt_ggt_width(ggt);
       tick_pos += stride_4x4,
         pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos)
    pbt_pbarea_put_vline(&(ggt->pbarea),
                         pxl_pos, 0, size,
                         tctx_frame_fg(editor_ctx));

  tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
  tmp = tick_pos % MSQGETPPQ(editor_ctx);
  if (tmp != 0)
    {
      pxl_pos = TICK2XPOS(editor_ctx, tick_pos - tmp)
        - editor_ctx->hadj.pos;
      qn_num = tick_pos / MSQGETPPQ(editor_ctx);
      pbt_pbarea_printf(&(ggt->pbarea),
                        &(editor_ctx->theme->global_theme->theme.font),
                        tctx_frame_fg(editor_ctx),
                        pxl_pos + 2, font_max_height,
                        "%d",
                        qn_num);
      tick_pos +=  MSQGETPPQ(editor_ctx) - (tmp);
    }
  for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos;
       pxl_pos < _pbt_ggt_width(ggt);
       tick_pos += MSQGETPPQ(editor_ctx),
         pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos)
    {
      qn_num = tick_pos / MSQGETPPQ(editor_ctx); /* get qn number */
      pbt_pbarea_printf(&(ggt->pbarea),
                        &(editor_ctx->theme->global_theme->theme.font),
                        tctx_frame_fg(editor_ctx),
                        pxl_pos + 2, font_max_height,
                        "%d",
                        qn_num);
    }

  draw_timeline_loop(&(ggt->pbarea),
                     editor_ctx,
                     editor_ctx->track_ctx->loop_start,
                     editor_ctx->track_ctx->loop_len);
}

unsigned int timeline_ggt_get_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;

  return track_editor->editor_ctx.theme->timeline_height;
}

void msq_update_hadj_startnlen(track_editor_ctx_t *editor_ctx)
{
  editor_ctx->hadj.max = TICK2XPOS(editor_ctx,
                                   editor_ctx->track_ctx->loop_start
                                   + editor_ctx->track_ctx->loop_len
                                   + editor_ctx->track_ctx->engine->ppq);
  if (editor_ctx->hadj.max < (editor_ctx->hadj.size + editor_ctx->qn_size))
    editor_ctx->hadj.max = editor_ctx->hadj.size + editor_ctx->qn_size;
  if ((editor_ctx->hadj.pos + editor_ctx->hadj.size + editor_ctx->qn_size)
      > editor_ctx->hadj.max)
    editor_ctx->hadj.pos =
      editor_ctx->hadj.max - (editor_ctx->hadj.size + editor_ctx->qn_size);
}

void timeline_wgt_update_area_cb(pbt_ggt_t *ggt, pbt_pbarea_t *pbarea)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;
  track_editor_ctx_t *editor_ctx = &(track_editor->editor_ctx);

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
  editor_ctx->hadj.size = pbarea->width;
  msq_update_hadj_startnlen(editor_ctx);
}

/* TODO timeline mode need enum */
#define TIMELINE_MODE_START 0
#define TIMELINE_MODE_END   1
#define TIMELINE_MODE_POS   3

void _timeline_wgt_set_loop_startnlen(int xpos,
                                      track_editor_ctx_t *editor_ctx,
                                      unsigned int *start,
                                      unsigned int *len)
{
  unsigned int tick;

  if (xpos < 0)
    xpos = 0;
  tick = XPOS2TICK(editor_ctx, xpos);
  /* pad to quarter note */
  tick = ((tick + (editor_ctx->track_ctx->engine->ppq / 2))
          / editor_ctx->track_ctx->engine->ppq)
    * editor_ctx->track_ctx->engine->ppq;
  if (editor_ctx->tmp_coo[0] == TIMELINE_MODE_START)
    {
      *start = tick;
      *len = editor_ctx->track_ctx->loop_len;
    }
  else /* if (editor_ctx->tmp_coo[0] == TIMELINE_MODE_END) */
    {
      *start = editor_ctx->track_ctx->loop_start;
      if (tick <= editor_ctx->track_ctx->loop_start)
        *len = editor_ctx->track_ctx->engine->ppq;
      else
        *len = (tick - editor_ctx->track_ctx->loop_start);
    }
}

void draw_combobox_button(pbt_pixbuf_t *pixbuf, char *label,
                          pbt_font_t *font,
                          unsigned char *fg, unsigned char *bg)
{
  int size = font->max_height - 6;;
  unsigned int xpos = pixbuf->width - 4 - size,
    ypos = 2 + (size / 2);

  pbt_pixbuf_fill(pixbuf, bg);
  pbt_pixbuf_printf(pixbuf, font,
                    fg,
                    1, 0,
                    label);
  pbt_pixbuf_draw_triangle_down(pixbuf, xpos, ypos, size, fg);
}

void msq_combobox_draw_pixbufs(msq_combobox_t *combobox)
{
  draw_combobox_button(&(combobox->pb_released),
                       combobox->combobox_get_default_str(combobox->cbb_addr),
                       &(combobox->gui_theme->theme.font),
                       combobox->gui_theme->theme.wgt_normal_fg,
                       combobox->gui_theme->theme.wgt_normal_bg);
  draw_combobox_button(&(combobox->pb_pressed),
                       combobox->combobox_get_default_str(combobox->cbb_addr),
                       &(combobox->gui_theme->theme.font),
                       combobox->gui_theme->theme.wgt_activated_fg,
                       combobox->gui_theme->theme.wgt_activated_bg);
  draw_combobox_button(&(combobox->pb_hovered),
                       combobox->combobox_get_default_str(combobox->cbb_addr),
                       &(combobox->gui_theme->theme.font),
                       combobox->gui_theme->theme.wgt_hovered_fg,
                       combobox->gui_theme->theme.wgt_hovered_bg);
}

void msq_combobox_refresh(msq_combobox_t *combobox)
{
  msq_combobox_draw_pixbufs(combobox);
  pbt_wgt_draw(&(combobox->button));
  pbt_wgt_win_put_buffer(&(combobox->button.wgt));
}

void msq_draw_vggts(msq_vggts_t *vggts)
{
  _pbt_ggt_draw(vggts->timeline);
  _pbt_ggt_draw(vggts->grid);
  _pbt_ggt_draw(vggts->value);
  _pbt_ggt_draw(vggts->hscroll);
  _pbt_ggt_draw(vggts->zoom);
  msq_combobox_refresh(vggts->channel_cbbox);
}

void msq_track_draw_tick_pos(track_editor_t *track_editor, uint_t tick)
{
  track_editor_ctx_t *editor_ctx = &(track_editor->editor_ctx);
  int xpos = TICK2XPOS(editor_ctx, tick) - editor_ctx->hadj.pos;
  pbt_wgt_t *wgt;

  /* TODO timeline mode need enum */
  if (xpos < 0
      || xpos > pbt_ggt_width(&(track_editor->grid_wgt.wgt))
      || (track_editor->grid_wgt.state != GRID_NO_MODE
          && track_editor->grid_wgt.state != GRID_WRITE_MODE)
      || (track_editor->value_wgt.state != VALUE_NO_MODE
          && track_editor->value_wgt.state != VALUE_WRITE_MODE))
    return;

  wgt = &(track_editor->grid_wgt.wgt);

  pbt_ggt_win_make_context(wgt->ggt_win);
  if (track_editor->last_xpos != 0)
    track_editor->last_xpos--;
  pbt_wgt_gl_refresh_rect(wgt,
                          track_editor->last_xpos, 0,
                          2, pbt_ggt_height(wgt));
  pbt_wgt_gl_draw_line(wgt,
                       xpos, 0,
                       xpos, pbt_ggt_height(&(track_editor->grid_wgt.wgt)),
                       tctx_wgt_normal_fg(&(track_editor->editor_ctx)));
  wbe_gl_flush();
  track_editor->last_xpos = xpos;
}

pbt_bool_t timeline_wgt_unset_focus_cb(pbt_ggt_t *ggt,
                                       wbe_window_input_t *winev,
                                       void *track_editor_addr)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;
  track_editor_ctx_t *editor_ctx = &(track_editor->editor_ctx);
  int xpos;
  unsigned int start, len;

  if (editor_ctx->tmp_coo[0] == TIMELINE_MODE_POS)
    {
      if (WBE_GET_BIT(winev->buttons, 2) == 0)
        {
          xpos = winev->xpos - _pbt_ggt_xpos(ggt) + editor_ctx->hadj.pos;
          engine_set_tick(editor_ctx->track_ctx->engine,
                          XPOS2TICK(editor_ctx, xpos));
          return PBT_TRUE;
        }
      else
        {
          xpos = winev->xpos - _pbt_ggt_xpos(ggt) + editor_ctx->hadj.pos;
          msq_track_draw_tick_pos(track_editor, XPOS2TICK(editor_ctx, xpos));
        }
    }
  else
    {
      if (WBE_GET_BIT(winev->buttons, 0) == 0)
        {
          xpos = winev->xpos - _pbt_ggt_xpos(ggt) + editor_ctx->hadj.pos;
          _timeline_wgt_set_loop_startnlen(xpos,
                                           editor_ctx,
                                           &(editor_ctx->track_ctx->loop_start),
                                           &(editor_ctx->track_ctx->loop_len));
          editor_ctx->track_ctx->need_sync = MSQ_TRUE;
          msq_update_hadj_startnlen(editor_ctx);
          msq_draw_vggts(&(track_editor->vggts));
          wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          return PBT_TRUE;
        }
      else
        {
          xpos = winev->xpos - _pbt_ggt_xpos(ggt) + editor_ctx->hadj.pos;
          _timeline_wgt_set_loop_startnlen(xpos, editor_ctx, &start, &len);
          draw_timeline_loop(&(ggt->pbarea), editor_ctx, start, len);
          wbe_pbw_put_buffer(&(track_editor->ggt_win.pb_win));
        }
    }
  return PBT_FALSE;
}

#define _is_in_loop_start_marker(_xpos, _editor_ctx)                    \
  (((_xpos) >= TICK2XPOS((_editor_ctx),                                 \
                         (_editor_ctx)->track_ctx->loop_start))         \
   && ((_xpos) < (TICK2XPOS((_editor_ctx),                              \
                            (_editor_ctx)->track_ctx->loop_start)       \
                  + ((_editor_ctx)->theme->global_theme->theme.font.max_height \
                     /2))))

#define _is_in_loop_end_marker(_xpos, _editor_ctx)                      \
  (((_xpos) >= (TICK2XPOS((_editor_ctx),                                \
                          ((_editor_ctx)->track_ctx->loop_start         \
                           + (_editor_ctx)->track_ctx->loop_len))       \
                - ((_editor_ctx)->theme->global_theme->theme.font.max_height \
                   / 2)))                                               \
   && ((_xpos) < (TICK2XPOS((_editor_ctx),                              \
                            ((_editor_ctx)->track_ctx->loop_start       \
                             + (_editor_ctx)->track_ctx->loop_len))     \
                  + (_editor_ctx)->theme->global_theme->theme.font.max_height)))

#define _is_in_loop_marker(_xpos, _editor_ctx)          \
  (_is_in_loop_start_marker((_xpos), (_editor_ctx))     \
   || _is_in_loop_end_marker((_xpos), (_editor_ctx)))

#define TIMELINE_MODE_CURSOR_ARROW 0
#define TIMELINE_MODE_CURSOR_HRSZ  1

pbt_bool_t timeline_wgt_set_focus_cb(pbt_ggt_t *ggt,
                                     wbe_window_input_t *winev,
                                     void *track_editor_addr)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;
  track_editor_ctx_t *editor_ctx = &(track_editor->editor_ctx);
  unsigned int xpos;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_TRUE)
    {
      xpos = winev->xpos - _pbt_ggt_xpos(ggt) + editor_ctx->hadj.pos;
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          if (_is_in_loop_start_marker(xpos, editor_ctx))
            {
              editor_ctx->tmp_coo[0] = TIMELINE_MODE_START;
              return PBT_TRUE;
            }
          else if (_is_in_loop_end_marker(xpos, editor_ctx))
            {
              editor_ctx->tmp_coo[0] = TIMELINE_MODE_END;
              return PBT_TRUE;
            }
        }
      if (WBE_GET_BIT(winev->buttons, 2) == 1)
        {
          xpos = winev->xpos - _pbt_ggt_xpos(ggt) + editor_ctx->hadj.pos;
          msq_track_draw_tick_pos(track_editor, XPOS2TICK(editor_ctx, xpos));
          editor_ctx->tmp_coo[0] = TIMELINE_MODE_POS;
          return PBT_TRUE;
        }
      else
        {
          if ((editor_ctx->tmp_coo[0] == TIMELINE_MODE_CURSOR_ARROW)
              && _is_in_loop_marker(xpos, editor_ctx))
            {
              wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                    tctx_cursor_hresize(editor_ctx));
              editor_ctx->tmp_coo[0] = TIMELINE_MODE_CURSOR_HRSZ;
            }
          else if ((editor_ctx->tmp_coo[0] == TIMELINE_MODE_CURSOR_HRSZ)
                   && (!_is_in_loop_marker(xpos, editor_ctx)))
            {
              wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                    tctx_cursor_arrow(editor_ctx));
              editor_ctx->tmp_coo[0] = TIMELINE_MODE_CURSOR_ARROW;
            }
        }
    }
  return PBT_FALSE;
}

void timeline_wgt_leave_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;
  track_editor_ctx_t *editor_ctx = &(track_editor->editor_ctx);

  wbe_window_set_cursor(track_editor->ggt_win.pb_win.win_be,
                        tctx_cursor_arrow(editor_ctx));
  editor_ctx->tmp_coo[0] = 0;
}

void timeline_wgt_init_ev_cb(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  track_editor_t *track_editor = wgt->priv;

  pbt_evh_add_set_focus_cb(&(wgt->ggt_win->evh),
                           &(wgt->ggt),
                           timeline_wgt_set_focus_cb,
                           track_editor);
  pbt_evh_add_unset_focus_cb(&(wgt->ggt_win->evh),
                             &(wgt->ggt),
                             timeline_wgt_unset_focus_cb,
                             track_editor);
  pbt_evh_add_leave_cb(&(wgt->ggt_win->evh),
                       &(wgt->ggt),
                       timeline_wgt_leave_cb,
                       track_editor);
}

void timeline_wgt_init(pbt_wgt_t *wgt, track_editor_t *track_editor)
{
  wgt->priv = track_editor;

  wgt->ggt.priv = wgt;
  wgt->ggt.get_min_width = pbt_ggt_return_zero;
  wgt->ggt.get_max_width = pbt_ggt_return_zero;
  wgt->ggt.get_min_height = timeline_ggt_get_height;
  wgt->ggt.get_max_height = timeline_ggt_get_height;
  wgt->ggt.update_area_cb = timeline_wgt_update_area_cb;
  wgt->ggt.draw_cb = draw_timeline_cb;
  wgt->ggt.destroy_cb = pbt_wgt_evnode_destroy;
  wgt->init_ev_cb = timeline_wgt_init_ev_cb;
}

void trackctx_del_event(track_ctx_t *track_ctx,
                        ev_iterator_t *ev_iterator)
{
  evit_del_event(ev_iterator);
}

void _track_editor_ctx_set_channel(track_editor_ctx_t *track_editor_ctx, byte_t channel)
{
  track_ctx_t *track_ctx = track_editor_ctx->track_ctx;
  ev_iterator_t ev_iterator = {};
  midicev_t *midicev = NULL;

  track_editor_ctx->channel = channel;
  for (midicev = evit_init_midiallchannel(&ev_iterator,
                                          &(track_ctx->track->tickev_list));
       midicev != NULL;
       midicev = evit_next_midiallchannel(&ev_iterator))
    midicev->chan = channel;
}

void _history_evit_add_midicev(list_t *history,
                               ev_iterator_t *evit,
                               unsigned int tick,
                               midicev_t *mcev)
{
  msq_history_elt_t *history_elt;

  history_elt = malloc(sizeof (msq_history_elt_t));
  history_elt->type = MSQ_ADD;
  history_elt->tick[0] = tick;
  memcpy(&(history_elt->ev[0]), mcev, sizeof (midicev_t));
  push_to_list(history, history_elt);

  evit_add_midicev(evit, tick, mcev);
}

typedef struct
{
  ev_iterator_t evit_noteon;
  ev_iterator_t evit_noteoff;
} noteonoff_t;

void _history_add_note(track_editor_ctx_t *editor_ctx, note_t *note)
{
  noteonoff_t       *noteonoff;
  midicev_t         mcev;
  msq_history_elt_t *history_elt;

  history_elt = malloc(sizeof (msq_history_elt_t));
  history_elt->type = MSQ_ADD_NOTE;

  noteonoff = malloc(sizeof (noteonoff_t));
  evit_init(&(noteonoff->evit_noteon),
            &(editor_ctx->track_ctx->track->tickev_list));
  evit_init(&(noteonoff->evit_noteoff),
            &(editor_ctx->track_ctx->track->tickev_list));

  mcev.chan = note->channel;
  mcev.event.note.num = note->num;
  mcev.type = MSQ_MIDI_NOTEON;
  mcev.event.note.val = note->val;
  memcpy(&(history_elt->ev[0]), &mcev, sizeof (midicev_t));
  history_elt->tick[0] = note->tick;
  evit_add_midicev(&(noteonoff->evit_noteon), note->tick, &mcev);

  mcev.type = MSQ_MIDI_NOTEOFF;
  mcev.event.note.val = 0;
  memcpy(&(history_elt->ev[1]), &mcev, sizeof (midicev_t));
  history_elt->tick[1] = note->tick + note->len;
  evit_add_midicev(&(noteonoff->evit_noteoff), note->tick + note->len, &mcev);

  push_to_list_tail(&(editor_ctx->selected_notes), noteonoff);

  push_to_list(&(editor_ctx->history), history_elt);
}

void _history_del_noteonoff(track_editor_ctx_t *editor_ctx,
                            noteonoff_t *noteonoff,
                            evit_del_func_t del_func)
{
  msq_history_elt_t *history_elt;
  seqev_t *seqev_on = evit_get_seqev(&(noteonoff->evit_noteon));
  seqev_t *seqev_off = evit_get_seqev(&(noteonoff->evit_noteoff));

  if (seqev_on == NULL || seqev_off == NULL)
    {
      pbt_logmsg("/!\\ Problem while trying to del note\n"
                 "(Unable to retrieve events)");
      return;
    }
  history_elt = malloc(sizeof (msq_history_elt_t));
  history_elt->type = MSQ_DEL_NOTE;
  history_elt->tick[0] = noteonoff->evit_noteon.tick;
  memcpy(&(history_elt->ev[0]), seqev_on->addr, sizeof (midicev_t));
  history_elt->tick[1] = noteonoff->evit_noteoff.tick;
  memcpy(&(history_elt->ev[1]), seqev_off->addr, sizeof (midicev_t));

  push_to_list(&(editor_ctx->history), history_elt);

  del_func(editor_ctx->track_ctx, &(noteonoff->evit_noteon));
  del_func(editor_ctx->track_ctx, &(noteonoff->evit_noteoff));
}

#include "debug_tool/debug_tool.h"

void _history_evit_del_midicev(track_editor_ctx_t *editor_ctx,
                               ev_iterator_t *evit,
                               evit_del_func_t del_func)
{
  msq_history_elt_t *history_elt;
  seqev_t *seqev = evit_get_seqev(evit);

  if (seqev->type != MIDICEV)
    msq_assert(seqev->type == MIDICEV ? MSQ_TRUE : MSQ_FALSE,
               "Wrong event (not a midicev)");

  history_elt = malloc(sizeof (msq_history_elt_t));
  history_elt->type = MSQ_DEL;
  history_elt->tick[0] = evit->tick;
  memcpy(&(history_elt->ev[0]), seqev->addr, sizeof (midicev_t));
  push_to_list(&(editor_ctx->history), history_elt);

  del_func(editor_ctx->track_ctx, evit);
}

void _history_track_editor_set_channel(track_editor_ctx_t *editor_ctx,
                                       byte_t channel)
{
  msq_history_elt_t *history_elt;

  history_elt = malloc(sizeof (msq_history_elt_t));
  history_elt->type = MSQ_CHANGE_CHANNEL;
  history_elt->ev[0].chan = editor_ctx->channel;
  history_elt->_track_editor_ctx_priv = editor_ctx;
  _track_editor_ctx_set_channel(editor_ctx, channel);
  push_to_list(&(editor_ctx->history), history_elt);
}

void _history_add_mark(list_t *history)
{
  msq_history_elt_t *history_elt;

  history_elt = malloc(sizeof (msq_history_elt_t));
  history_elt->type = MSQ_MARK;
  push_to_list(history, history_elt);
}

void _history_undo(track_editor_ctx_t *editor_ctx)
{
  list_iterator_t it = {};
  msq_history_elt_t *history_elt;
  ev_iterator_t evit = {};
  ev_iterator_t evitoff = {};

  free_list_node(&(editor_ctx->selected_notes), free);

  evit_init(&evit, &(editor_ctx->track_ctx->track->tickev_list));

  iter_init(&it, &(editor_ctx->history));
  while (iter_node(&it) != NULL)
    {
      history_elt = iter_node_ptr(&it);
      if (history_elt->type == MSQ_MARK)
        {
          iter_node_del(&it, free);
          return;
        }
      else if (history_elt->type == MSQ_ADD)
        {
          if (evit_searchev(&evit,
                            history_elt->tick[0],
                            &(history_elt->ev[0])) == NULL)
            {
              pbt_logmsg("History event mismatch.");
              do {
                iter_node_del(&it, free);
              } while (iter_node(&it) != NULL);
              return;
            }
          evit_del_event(&evit);
        }
      else if (history_elt->type == MSQ_DEL)
        evit_add_midicev(&evit, history_elt->tick[0], &(history_elt->ev[0]));
      else if (history_elt->type == MSQ_ADD_NOTE)
        {
          evit_init(&evitoff, &(editor_ctx->track_ctx->track->tickev_list));
          if (evit_searchev(&evit,
                            history_elt->tick[0],
                            &(history_elt->ev[0])) == NULL
              || evit_searchev(&evitoff,
                               history_elt->tick[1],
                               &(history_elt->ev[1])) == NULL)
            {
              pbt_logmsg("History note mismatch.");
              do {
                iter_node_del(&it, free);
              } while (iter_node(&it) != NULL);
              return;
            }
          if (engine_is_running(editor_ctx->track_ctx->engine) == MSQ_TRUE)
            {
              trackctx_event2trash(editor_ctx->track_ctx, &evit);
              trackctx_event2trash(editor_ctx->track_ctx, &evitoff);
            }
          else
            {
              evit_del_event(&evit);
              evit_del_event(&evitoff);
            }
        }
      else if (history_elt->type == MSQ_DEL_NOTE)
        {
          evit_add_midicev(&evit, history_elt->tick[0], &(history_elt->ev[0]));
          evit_add_midicev(&evit, history_elt->tick[1], &(history_elt->ev[1]));
        }
      else if (history_elt->type == MSQ_CHANGE_CHANNEL)
        {
          _track_editor_ctx_set_channel(history_elt->_track_editor_ctx_priv,
                                        history_elt->ev[0].chan);
        }
      iter_node_del(&it, free);
    }
}

void delete_note_selection(msq_grid_wgt_t *grid)
{
  list_iterator_t iter;
  noteonoff_t *noteonoff;
  void (*del_func)(track_ctx_t *, ev_iterator_t *);

  /* /!\ TODO
     trackctx_del_event must be locked until all deletion is passed
     (SEGFAULT race condition) */
  if (grid->editor_ctx->track_ctx->engine
      && engine_is_running(grid->editor_ctx->track_ctx->engine) == MSQ_TRUE)
    del_func = trackctx_event2trash;
  else
    del_func = trackctx_del_event;

  for (iter_init(&iter, &(grid->editor_ctx->selected_notes));
       iter_node(&iter) != NULL;
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      _history_del_noteonoff(grid->editor_ctx, noteonoff, del_func);
    }

  free_list_node(&(grid->editor_ctx->selected_notes), free);

  msq_draw_vggts(grid->vggts);
}

void draw_grid_selection(pbt_wgt_t *wgt, int *tmp_coo)
{
  int tex_width, tex_height;
  wbe_pbw_t *pb_win;
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;
  GLfloat color[] = {0, 0, 0, 0.5};

  if (tmp_coo[0] == tmp_coo[2])
    return;
  if (tmp_coo[1] == tmp_coo[3])
    return;

  pb_win = &(wgt->ggt_win->pb_win);
  tex_width = pb_win->buffer.width;
  tex_height = pb_win->buffer.height;

  if (tmp_coo[0] < tmp_coo[2])
    {
      xmin_float =
        ((float) (tmp_coo[0] + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
      xmax_float =
        ((float) (tmp_coo[2] + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
    }
  else
    {
      xmin_float =
        ((float) (tmp_coo[2] + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
      xmax_float =
        ((float) (tmp_coo[0] + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
    }

  if (tmp_coo[1] < tmp_coo[3])
    {
      ymin_float =
        (-((float) tmp_coo[3] + pbt_ggt_ypos(wgt)) * 2.0 / tex_height) + 1.0;
      ymax_float =
        (-((float) tmp_coo[1] + pbt_ggt_ypos(wgt)) * 2.0 / tex_height) + 1.0;
    }
  else
    {
      ymin_float =
        (-((float) tmp_coo[1] + pbt_ggt_ypos(wgt)) * 2.0 / tex_height) + 1.0;
      ymax_float =
        (-((float) tmp_coo[3] + pbt_ggt_ypos(wgt)) * 2.0 / tex_height) + 1.0;
    }

  wbe_pbw_make_context(pb_win);

  /* TODO refresh last pos */
  pbt_wgt_gl_refresh(wgt);

  _wbe_gl_texture_n_color_put_rect(xmin_float,
                                   ymin_float,
                                   xmax_float,
                                   ymax_float,
                                   color);
  wbe_gl_flush();
}

void set_gradient_color(track_editor_theme_t *theme,
                        unsigned char *color,
                        unsigned short value)
{
  size_t idx;
  unsigned int pondA, pondB, pondC;

  if (value <= theme->gradient_value_list[0].value)
    {
      pondA = value + 100;
      pondB = theme->gradient_value_list[0].value - value;
      pondC = theme->gradient_value_list[0].value + 100;
      color[0] = ((theme->piano_black_color[0]
                   * pondB)
                  + (theme->gradient_value_list[0].color[0]
                     * pondA))
        / pondC;
      color[1] = ((theme->piano_black_color[1] * pondB)
                  + (theme->gradient_value_list[0].color[1] * pondA))
        / pondC;
      color[2] = ((theme->piano_black_color[2] * pondB)
                  + (theme->gradient_value_list[0].color[2] * pondA))
        / pondC;
      color[3] = 0xFF;
    }

  for (idx = 0; idx < (theme->gradient_len - 1); idx++)
    {
      if (theme->gradient_value_list[idx].value <= value
          && value <= theme->gradient_value_list[idx + 1].value)
        {
          pondA = value - theme->gradient_value_list[idx].value;
          pondB = theme->gradient_value_list[idx + 1].value - value;
          pondC = theme->gradient_value_list[idx + 1].value
            - theme->gradient_value_list[idx].value;
          color[0] = ((theme->gradient_value_list[idx].color[0] * pondB)
                      + (theme->gradient_value_list[idx + 1].color[0] * pondA))
            / pondC;
          color[1] = ((theme->gradient_value_list[idx].color[1] * pondB)
                      + (theme->gradient_value_list[idx + 1].color[1] * pondA))
            / pondC;
          color[2] = ((theme->gradient_value_list[idx].color[2] * pondB)
                      + (theme->gradient_value_list[idx + 1].color[2] * pondA))
            / pondC;
          color[3] = 0xFF;
        }
    }
}

void draw_note(pbt_pbarea_t *pbarea,
               track_editor_ctx_t *editor_ctx,
               note_t *note,
               msq_bool_t selected)
{
  /* todo factorize wth draw_tmp_note */
  int rect_xpos, rect_ypos;
  int pxl_width, pxl_height;
  unsigned char color[4];

  rect_xpos = TICK2XPOS(editor_ctx, note->tick) - editor_ctx->hadj.pos;
  rect_ypos = NOTE2YPOS(editor_ctx, note->num) - editor_ctx->vadj.pos;
  pxl_width = TICK2XPOS(editor_ctx, note->len);

  if ((rect_xpos + pxl_width) < 0)
    return;

  if ((rect_ypos + ((int) editor_ctx->note_height)) < 0)
    return;

  if (rect_xpos > ((int) pbarea->width))
    return;

  if (rect_ypos > ((int) pbarea->height))
    return;

  if ((rect_xpos + pxl_width) > pbarea->width)
    pxl_width = pbarea->width - rect_xpos;
  else if (rect_xpos < 0)
    {
      pxl_width += rect_xpos;
      rect_xpos = 0;
    }

  if ((rect_ypos + editor_ctx->note_height)
      > pbarea->height)
    pxl_height = pbarea->height - rect_ypos;
  else if (rect_ypos < 0)
    {
      pxl_height = editor_ctx->note_height + rect_ypos;
      rect_ypos = 0;
    }
  else
    pxl_height = editor_ctx->note_height;

  set_gradient_color(editor_ctx->theme, color, note->val << 7);
  pbt_pbarea_fillrect(pbarea,
                      rect_xpos,
                      rect_ypos,
                      pxl_width,
                      pxl_height,
                      color);

  pbt_pbarea_put_rect(pbarea,
                      rect_xpos,
                      rect_ypos,
                      pxl_width,
                      pxl_height,
                      selected == MSQ_TRUE
                      ? editor_ctx->theme->selection_color
                      : tctx_frame_fg(editor_ctx));
}

msq_bool_t _is_in_selected_noteon(list_iterator_t *iter,
                                  list_t *selected_notes,
                                  midicev_t *noteon)
{
  noteonoff_t *noteonoff;
  seqev_t *seqev;

  for (iter_init(iter, selected_notes);
       iter_node(iter);
       iter_next(iter))
    {
      noteonoff = iter_node_ptr(iter);
      seqev = evit_get_seqev(&(noteonoff->evit_noteon));
      if (seqev->type == MIDICEV)
        if (seqev->addr == noteon)
          return MSQ_TRUE;
    }
  return MSQ_FALSE;
}

msq_bool_t is_in_selected_noteon(list_t *selected_notes,
                                 midicev_t *noteon)
{
  list_iterator_t iter = {};

  return _is_in_selected_noteon(&iter, selected_notes, noteon);
}

void draw_unselected_notes(pbt_pbarea_t *pbarea, track_editor_ctx_t *editor_ctx)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  midicev_t *noteon =
    evit_init_noteon(&evit_noteon,
                     &(editor_ctx->track_ctx->track->tickev_list),
                     editor_ctx->channel);
  midicev_t *noteoff;
  note_t note;

  while (noteon != NULL)
    {
      if (is_in_selected_noteon(&(editor_ctx->selected_notes),
                                noteon) == MSQ_FALSE)
        {
          evit_copy(&evit_noteon, &evit_noteoff);
          noteoff = evit_next_noteoff_num(&evit_noteoff,
                                          editor_ctx->channel,
                                          noteon->event.note.num);
          if (noteoff != NULL)
            {
              note.num = noteon->event.note.num;
              note.val = noteon->event.note.val;
              note.tick = evit_noteon.tick;
              note.len = evit_noteoff.tick - evit_noteon.tick;
              draw_note(pbarea, editor_ctx, &note, MSQ_FALSE);
            }
        }
      noteon = evit_next_noteon(&evit_noteon, editor_ctx->channel);
    }
}

void draw_notes_filter_selection(pbt_pbarea_t *pbarea,
                                 track_editor_ctx_t *editor_ctx)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  midicev_t *noteon =
    evit_init_noteon(&evit_noteon,
                     &(editor_ctx->track_ctx->track->tickev_list),
                     editor_ctx->channel);
  midicev_t *noteoff;
  note_t note;

  while (noteon != NULL)
    {
      evit_copy(&evit_noteon, &evit_noteoff);
      noteoff = evit_next_noteoff_num(&evit_noteoff,
                                      editor_ctx->channel,
                                      noteon->event.note.num);
      if (noteoff != NULL)
        {
          note.num = noteon->event.note.num;
          note.val = noteon->event.note.val;
          note.tick = evit_noteon.tick;
          note.len = evit_noteoff.tick - evit_noteon.tick;
          if (is_in_selected_noteon(&(editor_ctx->selected_notes),
                                    noteon) == MSQ_FALSE)
            draw_note(pbarea, editor_ctx, &note, MSQ_FALSE);
        }
      noteon = evit_next_noteon(&evit_noteon, editor_ctx->channel);
    }
}

void draw_notes(pbt_pbarea_t *pbarea, track_editor_ctx_t *editor_ctx)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  midicev_t *noteon =
    evit_init_noteon(&evit_noteon,
                     &(editor_ctx->track_ctx->track->tickev_list),
                     editor_ctx->channel);
  midicev_t *noteoff;
  note_t note;

  while (noteon != NULL)
    {
      evit_copy(&evit_noteon, &evit_noteoff);
      noteoff = evit_next_noteoff_num(&evit_noteoff,
                                      editor_ctx->channel,
                                      noteon->event.note.num);
      if (noteoff != NULL)
        {
          note.num = noteon->event.note.num;
          note.val = noteon->event.note.val;
          note.tick = evit_noteon.tick;
          note.len = evit_noteoff.tick - evit_noteon.tick;
          draw_note(pbarea,
                    editor_ctx,
                    &note,
                    is_in_selected_noteon(&(editor_ctx->selected_notes),
                                          noteon));
        }
      noteon = evit_next_noteon(&evit_noteon, editor_ctx->channel);
    }
}

void draw_loop_veil(pbt_pbarea_t *pbarea, track_editor_ctx_t *editor_ctx)
{
  unsigned int tmp;

  if (editor_ctx->hadj.pos < TICK2XPOS(editor_ctx,
                                       editor_ctx->track_ctx->loop_start))
    {
      tmp = TICK2XPOS(editor_ctx,
                      editor_ctx->track_ctx->loop_start)
        - editor_ctx->hadj.pos;
      if (tmp > pbarea->width)
        tmp = pbarea->width;
      msq_draw_veil(pbarea, 0, tmp, 0, pbarea->height);
    }

  if ((editor_ctx->hadj.pos + editor_ctx->hadj.size)
      > TICK2XPOS(editor_ctx,
                  editor_ctx->track_ctx->loop_start
                  + editor_ctx->track_ctx->loop_len))
    {
      tmp = TICK2XPOS(editor_ctx,
                      editor_ctx->track_ctx->loop_start
                      + editor_ctx->track_ctx->loop_len)
        - editor_ctx->hadj.pos;
      msq_draw_veil(pbarea, tmp, pbarea->width, 0, pbarea->height);
    }
}

uint_t msq_get_track_loop_tick(track_ctx_t *track_ctx, uint_t tick)
{
  if (tick < track_ctx->loop_start)
    {
      tick += track_ctx->loop_len
        * (1 + (track_ctx->loop_start / track_ctx->loop_len));
      if (tick > (track_ctx->loop_start + track_ctx->loop_len))
        return tick - track_ctx->loop_len;
      return tick;
    }
  else
    return ((tick - track_ctx->loop_start) % track_ctx->loop_len)
      + track_ctx->loop_start;
}

uint_t msq_get_current_track_tick(track_ctx_t *track_ctx)
{
  return msq_get_track_loop_tick(track_ctx, engine_get_tick(track_ctx->engine));
}

void pixbuf_safe_destroy(pbt_pixbuf_t *pixbuf)
{
  if (pixbuf->pixels != NULL)
    pbt_pixbuf_destroy(pixbuf);
}

void msq_track_draw_current_tick_pos(track_editor_t *track_editor)
{
  uint_t tick = msq_get_current_track_tick(track_editor->editor_ctx.track_ctx);

  msq_track_draw_tick_pos(track_editor, tick);
}

void _draw_grid(pbt_pbarea_t *pbarea, track_editor_ctx_t *editor_ctx)
{
  unsigned int pxl_pos;
  unsigned int tick_pos;
  unsigned int stride_4x4;
  unsigned int note_max_pos = GRIDYMAX(editor_ctx) - editor_ctx->vadj.pos;
  unsigned int note;
  unsigned int tmp;

  pbt_pbarea_fill(pbarea, tctx_frame_bg(editor_ctx));

  if (note_max_pos > pbarea->height)
    note_max_pos = pbarea->height;

  tmp = editor_ctx->vadj.pos % editor_ctx->note_height;
  if (tmp != 0)
    {
      pxl_pos = editor_ctx->note_height
        - (editor_ctx->vadj.pos % editor_ctx->note_height);
      note = YPOS2NOTE(editor_ctx, editor_ctx->vadj.pos);
      if (tonale_regular_note[note % 12] == MSQ_FALSE)
        pbt_pbarea_fillrect(pbarea,
                            0,
                            0,
                            pbarea->width,
                            pxl_pos,
                            editor_ctx->theme->piano_black_color);
    }
  else
    pxl_pos = 0;
  while (pxl_pos <= note_max_pos)
    {
      note = YPOS2NOTE(editor_ctx, editor_ctx->vadj.pos + pxl_pos);
      if (tonale_regular_note[note % 12] == MSQ_FALSE)
        {
          if (pxl_pos + editor_ctx->note_height
              > pbarea->height)
            tmp = pbarea->height - pxl_pos;
          else
            tmp = editor_ctx->note_height;
          pbt_pbarea_fillrect(pbarea,
                              0,
                              pxl_pos,
                              pbarea->width,
                              tmp,
                              editor_ctx->theme->piano_black_color);
        }
      pxl_pos += editor_ctx->note_height;
    }

  if (editor_ctx->vline_stride != 0)
    {
      tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
      tick_pos += editor_ctx->vline_stride
        - (tick_pos % editor_ctx->vline_stride);
      for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos;
           pxl_pos < pbarea->width;
           tick_pos += editor_ctx->vline_stride,
             pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos)
        pbt_pbarea_put_vline(pbarea,
                             pxl_pos, 0, note_max_pos,
                             editor_ctx->theme->smooth_line_color);
    }

  tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
  tick_pos +=  editor_ctx->quantize - (tick_pos % editor_ctx->quantize);
  for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos;
       pxl_pos < pbarea->width;
       tick_pos += editor_ctx->quantize,
         pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos)
    if ((tick_pos % MSQGETPPQ(editor_ctx)) == 0)
      {
        pbt_pbarea_put_vline(pbarea,
                             pxl_pos, 0, note_max_pos,
                             tctx_wgt_normal_fg(editor_ctx));
        if (pxl_pos + 1 < pbarea->width)
          pbt_pbarea_put_vline(pbarea,
                               pxl_pos + 1, 0, note_max_pos,
                               tctx_wgt_normal_fg(editor_ctx));
      }
    else
      pbt_pbarea_put_vline(pbarea,
                           pxl_pos, 0, note_max_pos,
                           tctx_wgt_normal_fg(editor_ctx));

  stride_4x4 = 4 * MSQGETPPQ(editor_ctx);
  tick_pos = XPOS2TICK(editor_ctx, editor_ctx->hadj.pos);
  tick_pos +=  stride_4x4 - (tick_pos % stride_4x4);
  for (pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos;
       pxl_pos < pbarea->width;
       tick_pos += stride_4x4,
         pxl_pos = TICK2XPOS(editor_ctx, tick_pos) - editor_ctx->hadj.pos)
    pbt_pbarea_fillrect(pbarea,
                        pxl_pos, 0,
                        2, note_max_pos,
                        tctx_frame_fg(editor_ctx));

  for (pxl_pos = editor_ctx->note_height
         - (editor_ctx->vadj.pos % editor_ctx->note_height);
       pxl_pos < note_max_pos;
       pxl_pos += editor_ctx->note_height)
    pbt_pbarea_put_hline(pbarea,
                         0,
                         pxl_pos,
                         pbarea->width,
                         tctx_frame_fg(editor_ctx));
}


void draw_grid_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_grid_wgt_t *grid_wgt = wgt->priv;

  _draw_grid(&(ggt->pbarea), grid_wgt->editor_ctx);
  draw_notes(&(ggt->pbarea), grid_wgt->editor_ctx);
  draw_loop_veil(&(ggt->pbarea), grid_wgt->editor_ctx);
}

noteonoff_t *is_in_note_selection(uint_t tick,
                                  byte_t channel,
                                  byte_t note,
                                  list_t *selected_notes)
{
  list_iterator_t iter;
  seqev_t   *seqev;
  midicev_t *midicev;
  noteonoff_t *noteonoff;

  for (iter_init(&iter, selected_notes);
       iter_node(&iter) != NULL;
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      if (noteonoff->evit_noteon.tick > tick)
        continue;
      seqev = evit_get_seqev(&(noteonoff->evit_noteon));
      midicev = seqev->addr;
      if (midicev->chan == channel
          && midicev->event.note.num == note
          && noteonoff->evit_noteoff.tick > tick)
        return noteonoff;
    }
  return NULL;
}

msq_bool_t mcev_is_in_list(list_t *mcev_list, midicev_t *mcev)
{
  node_t    *node = mcev_list->head;

  while (node != NULL)
    {
      if (node->addr == mcev)
        return MSQ_TRUE;
      node = node->next;
    }
  return MSQ_FALSE;
}

midicev_t *_evit_next_noteoff_num_excl(ev_iterator_t *evit_noteoff,
                                       unsigned char channel,
                                       unsigned char num,
                                       list_t *exclude_list)
{
  midicev_t *note_off = NULL;

  for (note_off = evit_next_noteoff_num(evit_noteoff,
                                        channel,
                                        num);
       note_off != NULL
         && mcev_is_in_list(exclude_list, note_off);
       note_off = evit_next_noteoff_num(evit_noteoff,
                                        channel,
                                        num));
  return note_off;
}

noteonoff_t *gen_noteonoff(ev_iterator_t *evit_noteon,
                           ev_iterator_t *evit_noteoff)
{
  noteonoff_t *noteonoff;

  noteonoff = malloc(sizeof (noteonoff_t));
  memcpy(&(noteonoff->evit_noteon),  evit_noteon,  sizeof (ev_iterator_t));
  memcpy(&(noteonoff->evit_noteoff), evit_noteoff, sizeof (ev_iterator_t));
  return noteonoff;
}

uint_t _get_selected_min_tick(list_t *selected)
{
  list_iterator_t it_selected = {};
  noteonoff_t     *noteonoff;
  uint_t          min_tick = (uint_t) -1; /* Set max value */

  for (iter_init(&it_selected, selected);
       iter_node(&it_selected);
       iter_next(&it_selected))
    {
      noteonoff = iter_node_ptr(&it_selected);
      if (noteonoff->evit_noteon.tick < min_tick)
        min_tick = noteonoff->evit_noteon.tick;
    }
  return min_tick;
}

void _select_noteonoff(list_t *selected,
                       list_t *tickev_list,
                       unsigned char channel,
                       uint_t tick_min,
                       uint_t tick_max,
                       unsigned char note_min,
                       unsigned char note_max)
{
  ev_iterator_t evit_noteon = {};
  ev_iterator_t evit_noteoff = {};
  list_iterator_t it_selected = {};
  midicev_t     *midicev_noteon = NULL;
  midicev_t     *midicev_noteoff = NULL;
  list_t        added_noteoff = {};
  noteonoff_t   *noteonoff;

  for (midicev_noteon = evit_init_noteon(&evit_noteon, tickev_list, channel);
       (midicev_noteon != NULL) && (evit_noteon.tick < tick_max);
       midicev_noteon = evit_next_noteon(&evit_noteon, channel))
    {
      if ((note_min <= midicev_noteon->event.note.num) &&
          (midicev_noteon->event.note.num <= note_max))
        {
          evit_copy(&evit_noteon, &evit_noteoff);
          midicev_noteoff =
            _evit_next_noteoff_num_excl(&evit_noteoff,
                                        midicev_noteon->chan,
                                        midicev_noteon->event.note.num,
                                        &added_noteoff);
          if ((midicev_noteoff != NULL) && (tick_min < evit_noteoff.tick))
            {
              if (_is_in_selected_noteon(&it_selected,
                                         selected,
                                         midicev_noteon) == MSQ_TRUE)
                iter_node_del(&it_selected, free);
              else
                {
                  noteonoff = gen_noteonoff(&evit_noteon, &evit_noteoff);
                  push_to_list_tail(selected, noteonoff);
                  push_to_list_tail(&added_noteoff, midicev_noteoff);
                }
            }
        }
    }
  free_list_node(&added_noteoff, NULL);
}

void handle_selection(track_editor_ctx_t *editor_ctx)
{
  unsigned int xmin, ymin, xmax, ymax;
  unsigned int tick_min, tick_max;
  unsigned char note_min, note_max;

  if (editor_ctx->tmp_coo[0] < editor_ctx->tmp_coo[2])
    {
      xmin = editor_ctx->tmp_coo[0] + editor_ctx->hadj.pos;
      xmax = editor_ctx->tmp_coo[2] + editor_ctx->hadj.pos;
    }
  else
    {
      xmin = editor_ctx->tmp_coo[2] + editor_ctx->hadj.pos;
      xmax = editor_ctx->tmp_coo[0] + editor_ctx->hadj.pos;
    }

  if (editor_ctx->tmp_coo[1] < editor_ctx->tmp_coo[3])
    {
      ymin = editor_ctx->tmp_coo[1] + editor_ctx->vadj.pos;
      ymax = editor_ctx->tmp_coo[3] + editor_ctx->vadj.pos;
    }
  else
    {
      ymin = editor_ctx->tmp_coo[3] + editor_ctx->vadj.pos;
      ymax = editor_ctx->tmp_coo[1] + editor_ctx->vadj.pos;
    }

  tick_min = XPOS2TICK(editor_ctx, xmin);
  tick_max = XPOS2TICK(editor_ctx, xmax);
  note_min = YPOS2NOTE(editor_ctx, ymax);
  note_max = YPOS2NOTE(editor_ctx, ymin);
  _select_noteonoff(&(editor_ctx->selected_notes),
                    &(editor_ctx->track_ctx->track->tickev_list),
                    editor_ctx->channel,
                    tick_min,
                    tick_max,
                    note_min,
                    note_max);
}

msq_bool_t note_collision(note_t *note,
                          track_editor_ctx_t *editor_ctx,
                          msq_bool_t filter_selection)
{
  ev_iterator_t evit_noteon, evit_noteoff;
  midicev_t *noteon =
    evit_init_noteon(&evit_noteon,
                     &(editor_ctx->track_ctx->track->tickev_list),
                     note->channel),
    *noteoff;

  while (noteon != NULL && evit_noteon.tick <= (note->tick + note->len))
    {
      if (filter_selection == MSQ_FALSE
          || is_in_selected_noteon(&(editor_ctx->selected_notes),
                                   noteon) == MSQ_FALSE)
        {
          if (noteon->event.note.num == note->num)
            {
              if (evit_noteon.tick >= note->tick)
                return MSQ_TRUE;
              evit_copy(&evit_noteon, &evit_noteoff);
              noteoff = evit_next_noteoff_num(&evit_noteoff,
                                              note->channel,
                                              note->num);
              if (noteoff == NULL)
                return MSQ_FALSE;
              if (evit_noteoff.tick >= note->tick)
                return MSQ_TRUE;
            }
        }
      noteon = evit_next_noteon(&evit_noteon, note->channel);
    }

  return MSQ_FALSE;
}

void handle_writting_note_mode_gen_note(note_t *note,
                                        track_editor_ctx_t *editor_ctx)
{
  note->num = editor_ctx->tmp_coo[1];
  note->val = editor_ctx->default_velocity;
  note->channel = editor_ctx->channel;
  if (editor_ctx->tmp_coo[0] < editor_ctx->tmp_coo[2])
    {
      note->tick = editor_ctx->tmp_coo[0];
      note->len = editor_ctx->tmp_coo[2] - editor_ctx->tmp_coo[0] - 1;
    }
  else if (editor_ctx->tmp_coo[0] == editor_ctx->tmp_coo[2])
    {
      note->tick = editor_ctx->tmp_coo[0];
      note->len = editor_ctx->quantize - 1;
    }
  else
    {
      note->tick = editor_ctx->tmp_coo[2];
      note->len = editor_ctx->tmp_coo[0] - editor_ctx->tmp_coo[2] - 1;
    }
}

void msq_screenshot_pbarea(pbt_pixbuf_t *screenshot, pbt_pbarea_t *pbarea)
{
  unsigned int width = pbarea->width,
    height = pbarea->height;

  if (screenshot->width != width
      || screenshot->height != height)
    {
      pixbuf_safe_destroy(screenshot);
      pbt_pixbuf_init(screenshot, width, height);
    }
  pbt_pixbuf_copy_pbarea(screenshot,
                         0, 0,
                         pbarea,
                         0, 0,
                         width, height);
}

void draw_tmp_note(pbt_wgt_t *wgt,
                   track_editor_ctx_t *editor_ctx,
                   note_t *note)
{
  /* todo factorize wth draw_note */
  int rect_xpos, rect_ypos;
  int pxl_width, pxl_height;
  unsigned char color[4];
  unsigned int wgt_width = pbt_ggt_width(wgt);
  unsigned int wgt_height = pbt_ggt_height(wgt);

  rect_xpos = TICK2XPOS(editor_ctx, note->tick) - editor_ctx->hadj.pos;
  rect_ypos = NOTE2YPOS(editor_ctx, note->num) - editor_ctx->vadj.pos;
  pxl_width = TICK2XPOS(editor_ctx, note->len);

  if ((rect_xpos + pxl_width) < 0)
    return;

  if ((rect_ypos + ((int) editor_ctx->note_height)) < 0)
    return;

  if (rect_xpos > ((int) wgt_width))
    return;

  if (rect_ypos > ((int) wgt_height))
    return;

  if ((rect_xpos + pxl_width) > wgt_width)
    pxl_width = wgt_width - rect_xpos;
  else if (rect_xpos < 0)
    {
      pxl_width += rect_xpos;
      rect_xpos = 0;
    }

  if ((rect_ypos + editor_ctx->note_height)
      > wgt_height)
    pxl_height = wgt_height - rect_ypos;
  else if (rect_ypos < 0)
    {
      pxl_height = editor_ctx->note_height + rect_ypos;
      rect_ypos = 0;
    }
  else
    pxl_height = editor_ctx->note_height;

  set_gradient_color(editor_ctx->theme, color, note->val << 7);
  pbt_wgt_gl_fillrect(wgt,
                      rect_xpos,
                      rect_ypos,
                      pxl_width,
                      pxl_height,
                      color);
}

int msq_quantify_tick(track_editor_ctx_t *editor_ctx, int tick)
{
  return ((tick + (editor_ctx->quantize >> 1)) / editor_ctx->quantize
          * editor_ctx->quantize);
}

void handle_writting_note_mode(wbe_window_input_t *winev,
                               msq_grid_wgt_t *grid_wgt)
{
  track_editor_ctx_t *editor_ctx = grid_wgt->editor_ctx;
  note_t note;

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    {
      if (winev->xpos >= (int) pbt_ggt_xpos(&(grid_wgt->wgt)))
        {
          /* editor_ctx previously set in else */
          handle_writting_note_mode_gen_note(&note, editor_ctx);
          if (note_collision(&note, editor_ctx, MSQ_FALSE) == MSQ_FALSE)
            {
              free_list_node(&(editor_ctx->selected_notes), free);
              _history_add_mark(&(editor_ctx->history));
              _history_add_note(editor_ctx, &note);
            }
        }
      msq_draw_vggts(grid_wgt->vggts);
      grid_wgt->state = GRID_WRITE_MODE;
    }
  else
    {
      if (winev->xpos < (int) pbt_ggt_xpos(&(grid_wgt->wgt)))
        return;
      editor_ctx->tmp_coo[2] =
        XPOS2TICK(editor_ctx,
                  winev->xpos
                  - pbt_ggt_xpos(&(grid_wgt->wgt))
                  + editor_ctx->hadj.pos);
      if (editor_ctx->tmp_coo[2] % editor_ctx->quantize != 0)
        editor_ctx->tmp_coo[2] = msq_quantify_tick(editor_ctx,
                                                   editor_ctx->tmp_coo[2]);
      handle_writting_note_mode_gen_note(&note, editor_ctx);
      if (note_collision(&note, editor_ctx, MSQ_FALSE) == MSQ_TRUE)
        return;

      wbe_pbw_make_context(&(grid_wgt->wgt.ggt_win->pb_win));
      /* TODO refresh last pos */
      pbt_wgt_gl_refresh(&(grid_wgt->wgt));
      draw_tmp_note(&(grid_wgt->wgt),
                    editor_ctx,
                    &note);
      wbe_gl_flush();
    }
}

msq_bool_t msq_check_note_list(list_t *note_list)
{
  list_iterator_t it1, it2;
  note_t *note1, *note2;

  for (iter_init(&it1, note_list);
       iter_node(&it1);
       iter_next(&it1))
    {
      note1 = iter_node_ptr(&it1);
      for (iter_copy(&it1, &it2),
             iter_next(&it2);
           iter_node(&it2);
           iter_next(&it2))
        {
          note2 = iter_node_ptr(&it2);
          if ((note1->num == note2->num)
              && (note1->channel == note2->channel)
              && (note1->tick <= note2->tick + note2->len)
              && (note1->tick + note1->len >= note2->tick))
            return MSQ_FALSE;
        }
    }
  return MSQ_TRUE;
}

msq_bool_t msq_move_note_start_list_init(list_t *new_note_list,
                                         track_editor_ctx_t *editor_ctx,
                                         int tick_offset)
{
  list_iterator_t iter = {};
  noteonoff_t *noteonoff;
  seqev_t *seqev;
  midicev_t *noteev;
  note_t note, *note_ptr;
  int tmp_tick_start;

  for (iter_init(&iter, &(editor_ctx->selected_notes));
       iter_node(&iter);
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      seqev = evit_get_seqev(&(noteonoff->evit_noteon));
      noteev = seqev->addr;
      if ((tick_offset + (int) noteonoff->evit_noteon.tick) < 0)
        tmp_tick_start = 0;
      else
        {
          tmp_tick_start = noteonoff->evit_noteon.tick + tick_offset;
          tmp_tick_start = msq_quantify_tick(editor_ctx, tmp_tick_start);
        }
      if ((int) noteonoff->evit_noteoff.tick <= tmp_tick_start)
        {
          tmp_tick_start = msq_quantify_tick(editor_ctx,
                                             noteonoff->evit_noteoff.tick);
          if ((int) noteonoff->evit_noteoff.tick <= tmp_tick_start)
            tmp_tick_start -= editor_ctx->quantize;
        }
      note.num =  noteev->event.note.num;
      note.tick = tmp_tick_start;
      note.channel = noteev->chan;
      note.val = noteev->event.note.val;
      note.len = noteonoff->evit_noteoff.tick -  tmp_tick_start;
      if (note_collision(&note, editor_ctx, MSQ_TRUE) == MSQ_TRUE)
        {
          free_list_node(new_note_list, free);
          return MSQ_FALSE;
        }
      note_ptr = malloc(sizeof (note_t));
      memcpy(note_ptr, &note, sizeof (note_t));
      push_to_list(new_note_list, note_ptr);
    }
  if (msq_check_note_list(new_note_list) == MSQ_FALSE)
    {
      free_list_node(new_note_list, free);
      return MSQ_FALSE;
    }
  return MSQ_TRUE;
}

#define msq_quantify_tick_end(_editor_ctx, _tick)       \
  (msq_quantify_tick((_editor_ctx), (_tick)) - 1)

msq_bool_t msq_move_note_end_list_init(list_t *new_note_list,
                                       track_editor_ctx_t *editor_ctx,
                                       int tick_offset)
{
  list_iterator_t iter = {};
  noteonoff_t *noteonoff;
  seqev_t *seqev;
  midicev_t *noteev;
  note_t note, *note_ptr;
  int tmp_tick_end;

  for (iter_init(&iter, &(editor_ctx->selected_notes));
       iter_node(&iter);
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      seqev = evit_get_seqev(&(noteonoff->evit_noteon));
      noteev = seqev->addr;
      tmp_tick_end = noteonoff->evit_noteoff.tick + tick_offset;
      tmp_tick_end = msq_quantify_tick_end(editor_ctx, tmp_tick_end);
      if ((int) noteonoff->evit_noteon.tick >= tmp_tick_end)
        {
          tmp_tick_end = msq_quantify_tick_end(editor_ctx,
                                               noteonoff->evit_noteon.tick);
          if ((int) noteonoff->evit_noteon.tick >= tmp_tick_end)
            tmp_tick_end += editor_ctx->quantize;
        }
      note.num =  noteev->event.note.num;
      note.tick = noteonoff->evit_noteon.tick;
      note.channel = noteev->chan;
      note.val = noteev->event.note.val;
      note.len = tmp_tick_end - noteonoff->evit_noteon.tick;
      if (note_collision(&note, editor_ctx, MSQ_TRUE) == MSQ_TRUE)
        {
          free_list_node(new_note_list, free);
          return MSQ_FALSE;
        }
      note_ptr = malloc(sizeof (note_t));
      memcpy(note_ptr, &note, sizeof (note_t));
      push_to_list(new_note_list, note_ptr);
    }
  if (msq_check_note_list(new_note_list) == MSQ_FALSE)
    {
      free_list_node(new_note_list, free);
      return MSQ_FALSE;
    }
  return MSQ_TRUE;
}


typedef msq_bool_t (*msq_move_note_list_init_t)(list_t *new_note_list,
                                                track_editor_ctx_t *editor_ctx,
                                                int tick_offset);

void msq_draw_move_note(msq_grid_wgt_t *grid,
                        wbe_window_input_t *winev,
                        msq_move_note_list_init_t new_note_list_init)
{
  list_iterator_t iter = {};
  track_editor_ctx_t *editor_ctx = grid->editor_ctx;
  int tick_offset = XPOS2TICK(editor_ctx,
                              winev->xpos - editor_ctx->tmp_coo[0]);
  list_t new_note_list = {};
  note_t note, *note_ptr;
  noteonoff_t *noteonoff;
  seqev_t *seqev;
  midicev_t *noteev;

  pbt_ggt_win_make_context(grid->wgt.ggt_win);
  pbt_wgt_gl_refresh(&(grid->wgt));
  if (new_note_list_init(&new_note_list, editor_ctx, tick_offset) == MSQ_TRUE)
    {
      for (iter_init(&iter, &(new_note_list));
           iter_node(&iter);
           iter_next(&iter))
        {
          note_ptr = iter_node_ptr(&iter);
          draw_tmp_note(&(grid->wgt), editor_ctx, note_ptr);
        }
      free_list_node(&new_note_list, free);
    }
  else
    {
      for (iter_init(&iter, &(editor_ctx->selected_notes));
           iter_node(&iter);
           iter_next(&iter))
        {
          noteonoff = iter_node_ptr(&iter);
          seqev = evit_get_seqev(&(noteonoff->evit_noteon));
          noteev = seqev->addr;
          note.tick = noteonoff->evit_noteon.tick;
          note.channel = noteev->chan;
          note.num = noteev->event.note.num;
          note.val = noteev->event.note.val;
          note.len = noteonoff->evit_noteoff.tick - note.tick;
          draw_tmp_note(&(grid->wgt), editor_ctx, &note);
        }
    }
  wbe_gl_flush();
}

void _msq_add_list_note(track_editor_ctx_t *editor_ctx, list_t *note_list)
{
  list_iterator_t iter = {};
  note_t *note_ptr;

  for (iter_init(&iter, note_list); iter_node(&iter); iter_next(&iter))
    {
      note_ptr = iter_node_ptr(&iter);
      _history_add_note(editor_ctx, note_ptr);
    }
}

void msq_write_move_note(msq_grid_wgt_t *grid,
                         wbe_window_input_t *winev,
                         msq_move_note_list_init_t new_note_list_init)
{
  track_editor_ctx_t *editor_ctx = grid->editor_ctx;
  int tick_offset = XPOS2TICK(editor_ctx,
                              winev->xpos - editor_ctx->tmp_coo[0]);
  list_t new_note_list = {};

  if (new_note_list_init(&new_note_list, editor_ctx, tick_offset) == MSQ_TRUE)
    {
      _history_add_mark(&(editor_ctx->history));
      delete_note_selection(grid);
      _msq_add_list_note(editor_ctx, &new_note_list);
      free_list_node(&new_note_list, free);
    }
}

#define msq_draw_move_note_start(grid, winev)                           \
  msq_draw_move_note((grid), (winev), msq_move_note_start_list_init)
#define msq_draw_move_note_end(grid, winev)                             \
  msq_draw_move_note((grid), (winev), msq_move_note_end_list_init)

#define msq_write_move_note_start(grid, winev)                          \
  msq_write_move_note((grid), (winev), msq_move_note_start_list_init)
#define msq_write_move_note_end(grid, winev)                            \
  msq_write_move_note((grid), (winev), msq_move_note_end_list_init)

msq_bool_t check_move_note(track_editor_ctx_t *editor_ctx,
                           int tick_offset,
                           int note_offset)
{
  list_iterator_t iter = {};
  noteonoff_t *noteonoff;
  seqev_t *seqev;
  midicev_t *noteev;
  note_t note;
  int tmp;

  for (iter_init(&iter, &(editor_ctx->selected_notes));
       iter_node(&iter);
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      /* Check move under 0 */
      if ((int) noteonoff->evit_noteon.tick < (- tick_offset))
        return MSQ_FALSE;
      seqev = evit_get_seqev(&(noteonoff->evit_noteon));
      noteev = seqev->addr;
      tmp = note_offset + (int) noteev->event.note.num;
      if (tmp < 0 || tmp > 127)
        return MSQ_FALSE;
      note.num = tmp;
      note.tick = noteonoff->evit_noteon.tick + tick_offset;
      note.channel = noteev->chan;
      note.val = noteev->event.note.val;
      note.len = noteonoff->evit_noteoff.tick
        - noteonoff->evit_noteon.tick;
      if (note_collision(&note, editor_ctx, MSQ_TRUE) == MSQ_TRUE)
        return MSQ_FALSE;
    }
  return MSQ_TRUE;
}

pbt_bool_t handle_move_note_mode(wbe_window_input_t *winev,
                                 msq_grid_wgt_t *grid)
{
  int tick_offset;
  int note_offset;
  list_iterator_t iter = {};
  noteonoff_t *noteonoff;
  seqev_t *seqev;
  midicev_t *noteev;
  note_t note;
  int xpos, ypos;
  note_t *tmp_note;
  list_t tmp_list = {};

  xpos = GRIDXPOS(winev->xpos, grid);
  ypos = GRIDYPOS(winev->ypos, grid);
  tick_offset = XPOS2TICK(grid->editor_ctx,
                          xpos - grid->editor_ctx->tmp_coo[0]);
  tick_offset = (tick_offset / (int) grid->editor_ctx->quantize)
    * (int) grid->editor_ctx->quantize;
  note_offset = ((int) grid->editor_ctx->tmp_coo[1] - ypos)
    / (int) grid->editor_ctx->note_height;

  /* Handle negative ticks */
  if (((int) grid->editor_ctx->selected_notes_min_tick) + tick_offset < 0)
    {
      tick_offset =
        grid->editor_ctx->selected_notes_min_tick % grid->editor_ctx->quantize;
      tick_offset = tick_offset - grid->editor_ctx->selected_notes_min_tick;
    }

  if (WBE_GET_BIT(winev->buttons, 0) == 1)
    {
      if (check_move_note(grid->editor_ctx,
                          tick_offset, note_offset) == MSQ_TRUE)
        {
          wbe_pbw_make_context(&(grid->wgt.ggt_win->pb_win));
          /* TODO refresh last pos */
          pbt_wgt_gl_refresh(&(grid->wgt));
          for (iter_init(&iter, &(grid->editor_ctx->selected_notes));
               iter_node(&iter);
               iter_next(&iter))
            {
              noteonoff = iter_node_ptr(&iter);
              seqev = evit_get_seqev(&(noteonoff->evit_noteon));
              noteev = seqev->addr;
              note.tick = noteonoff->evit_noteon.tick + tick_offset;
              note.channel = noteev->chan;
              note.num = noteev->event.note.num + note_offset;
              note.val = noteev->event.note.val;
              note.len = (noteonoff->evit_noteoff.tick
                          - noteonoff->evit_noteon.tick);
              draw_tmp_note(&(grid->wgt),
                            grid->editor_ctx,
                            &note);
            }
          wbe_gl_flush();
        }
    }
  else
    {
      if (check_move_note(grid->editor_ctx,
                          tick_offset, note_offset) == MSQ_TRUE)
        {
          for (iter_init(&iter, &(grid->editor_ctx->selected_notes));
               iter_node(&iter);
               iter_next(&iter))
            {
              noteonoff = iter_node_ptr(&iter);
              seqev = evit_get_seqev(&(noteonoff->evit_noteon));
              noteev = seqev->addr;
              tmp_note = malloc(sizeof (note_t));
              tmp_note->tick = noteonoff->evit_noteon.tick + tick_offset;
              tmp_note->channel = noteev->chan;
              tmp_note->num = noteev->event.note.num + note_offset;
              tmp_note->val = noteev->event.note.val;
              tmp_note->len = (noteonoff->evit_noteoff.tick
                               - noteonoff->evit_noteon.tick);
              push_to_list_tail(&tmp_list, tmp_note);
            }
          _history_add_mark(&(grid->editor_ctx->history));
          delete_note_selection(grid);
          _msq_add_list_note(grid->editor_ctx, &tmp_list);
          free_list_node(&tmp_list, free);
          msq_draw_vggts(grid->vggts);
        }
      return PBT_TRUE;
    }
  return PBT_FALSE;
}

midicev_t *_evit_get_midicev(ev_iterator_t *evit)
{
  seqev_t   *seqev   = NULL;

  if (iter_node(&(evit->seqevit)) == NULL)
    return NULL;

  seqev = iter_node_ptr(&(evit->seqevit));
  if (seqev->type != MIDICEV)
    return NULL;

  return seqev->addr;
}

void grid_copy_selection(msq_grid_wgt_t *grid)
{
  list_iterator_t iter;
  noteonoff_t *noteonoff;
  note_t *tmp_note;
  unsigned int tmp_tick = -1;
  unsigned char tmp_num = -1;
  midicev_t *noteev;

  free_list_node(&_msq_editor_clipboard, free);

  for (iter_init(&iter, &(grid->editor_ctx->selected_notes));
       iter_node(&iter);
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      noteev = _evit_get_midicev(&(noteonoff->evit_noteon));
      tmp_note = malloc(sizeof (note_t));
      tmp_note->tick = noteonoff->evit_noteon.tick;
      if (tmp_note->tick < tmp_tick)
        tmp_tick = tmp_note->tick;
      tmp_note->channel = noteev->chan;
      tmp_note->num = noteev->event.note.num;
      if (tmp_note->num < tmp_num)
        tmp_num = tmp_note->num;
      tmp_note->val = noteev->event.note.val;
      tmp_note->len = (noteonoff->evit_noteoff.tick
                       - noteonoff->evit_noteon.tick);
      push_to_list_tail(&_msq_editor_clipboard, tmp_note);
    }

  /* Set to minimum quantized tick when saving to clipboard (to change ??) */
  /* and set the max num */
  tmp_tick = tmp_tick - (tmp_tick % grid->editor_ctx->quantize);
  _msq_editor_clipboard_note_max = 0;
  for (iter_init(&iter, &_msq_editor_clipboard);
       iter_node(&iter);
       iter_next(&iter))
    {
      tmp_note = iter_node_ptr(&iter);
      tmp_note->tick -= tmp_tick;
      tmp_note->num -= tmp_num;
      if (_msq_editor_clipboard_note_max < tmp_note->num)
        _msq_editor_clipboard_note_max = tmp_note->num;
    }
}

void _select_all_noteonoff(list_t *selected,
                           list_t *tickev_list,
                           unsigned char channel)
{
  ev_iterator_t evit_noteon = {};
  ev_iterator_t evit_noteoff = {};
  midicev_t     *midicev_noteon = NULL;
  midicev_t     *midicev_noteoff = NULL;
  list_t        added_noteoff = {};
  noteonoff_t   *noteonoff;

  free_list_node(selected, free);
  for (midicev_noteon = evit_init_noteon(&evit_noteon, tickev_list, channel);
       midicev_noteon != NULL;
       midicev_noteon = evit_next_noteon(&evit_noteon, channel))
    {
      evit_copy(&evit_noteon, &evit_noteoff);
      midicev_noteoff =
        _evit_next_noteoff_num_excl(&evit_noteoff,
                                    midicev_noteon->chan,
                                    midicev_noteon->event.note.num,
                                    &added_noteoff);
      noteonoff = gen_noteonoff(&evit_noteon, &evit_noteoff);
      push_to_list_tail(selected, noteonoff);
      push_to_list_tail(&added_noteoff, midicev_noteoff);
    }
  free_list_node(&added_noteoff, NULL);
}

void grid_select_all(msq_grid_wgt_t *grid)
{
  _select_all_noteonoff(&(grid->editor_ctx->selected_notes),
                        &(grid->editor_ctx->track_ctx->track->tickev_list),
                        grid->editor_ctx->channel);
  pbt_ggt_draw(&(grid->wgt));
  pbt_wgt_win_put_buffer(&(grid->wgt));
}

pbt_bool_t handle_grid_paste_mode(wbe_window_input_t *winev,
                                  msq_grid_wgt_t *grid)
{
  list_iterator_t iter = {};
  note_t *note, tmp_note;
  unsigned int tick_offset;
  unsigned char num_offset;
  pbt_bool_t ret_bool = PBT_FALSE;
  midicev_t mcev;
  noteonoff_t *noteonoff;

  if (WBE_GET_BIT(winev->buttons, 0) == 1)
    {
      grid->state = GRID_NO_MODE;
      if (WBE_GET_BIT(winev->buttons, 1) == 1
          || WBE_GET_BIT(winev->buttons, 2) == 1)
        return PBT_TRUE;
      else
        ret_bool = PBT_TRUE;
    }

  tick_offset = XPOS2TICK(grid->editor_ctx,
                          winev->xpos + grid->editor_ctx->hadj.pos
                          - pbt_ggt_xpos(&(grid->wgt)));
  tick_offset = msq_quantify_tick(grid->editor_ctx, tick_offset);
  num_offset = YPOS2NOTE(grid->editor_ctx,
                         winev->ypos + grid->editor_ctx->vadj.pos
                         - pbt_ggt_ypos(&(grid->wgt)));
  if (num_offset < _msq_editor_clipboard_note_max)
    return PBT_FALSE;
  num_offset -= _msq_editor_clipboard_note_max;
  for (iter_init(&iter, &_msq_editor_clipboard);
       iter_node(&iter);
       iter_next(&iter))
    {
      note = iter_node_ptr(&iter);
      memcpy(&tmp_note, note, sizeof (note_t));
      tmp_note.tick += tick_offset;
      tmp_note.num += num_offset;
      if (tmp_note.num > 127
          || note_collision(&tmp_note,
                            grid->editor_ctx,
                            MSQ_FALSE) == MSQ_TRUE)
        return PBT_FALSE;
    }

  if (ret_bool == PBT_TRUE)
    {
      free_list_node(&(grid->editor_ctx->selected_notes), free);
      for (iter_init(&iter, &_msq_editor_clipboard);
           iter_node(&iter);
           iter_next(&iter))
        {
          note = iter_node_ptr(&iter);
          noteonoff = malloc(sizeof (noteonoff_t));
          evit_init(&(noteonoff->evit_noteon),
                    &(grid->editor_ctx->track_ctx->track->tickev_list));
          evit_init(&(noteonoff->evit_noteoff),
                    &(grid->editor_ctx->track_ctx->track->tickev_list));

          mcev.chan = grid->editor_ctx->channel;
          mcev.event.note.num = note->num + num_offset;

          mcev.type = MSQ_MIDI_NOTEON;
          mcev.event.note.val = note->val;
          _history_evit_add_midicev(&(grid->editor_ctx->history),
                                    &(noteonoff->evit_noteon),
                                    note->tick + tick_offset, &mcev);

          mcev.type = MSQ_MIDI_NOTEOFF;
          mcev.event.note.val = 0;

          _history_evit_add_midicev(&(grid->editor_ctx->history),
                                    &(noteonoff->evit_noteoff),
                                    note->tick + note->len + tick_offset, &mcev);

          push_to_list_tail(&(grid->editor_ctx->selected_notes),
                            noteonoff);
        }
      _history_add_mark(&(grid->editor_ctx->history));
      msq_draw_vggts(grid->vggts);
      pbt_wgt_win_put_buffer(&(grid->wgt));
    }
  else
    {
      wbe_pbw_make_context(&(grid->wgt.ggt_win->pb_win));
      /* TODO refresh last pos */
      pbt_wgt_gl_refresh(&(grid->wgt));
      for (iter_init(&iter, &_msq_editor_clipboard);
           iter_node(&iter);
           iter_next(&iter))
        {
          note = iter_node_ptr(&iter);
          memcpy(&tmp_note, note, sizeof (note_t));
          tmp_note.tick += tick_offset;
          tmp_note.num += num_offset;
          draw_tmp_note(&(grid->wgt),
                        grid->editor_ctx,
                        &tmp_note);
        }
      wbe_gl_flush();
    }

  return PBT_FALSE;
}

pbt_bool_t grid_wgt_unset_focus_cb(pbt_ggt_t *ggt,
                                   wbe_window_input_t *winev,
                                   void *grid_addr)
{
  msq_grid_wgt_t *grid = grid_addr;
  track_editor_ctx_t *editor_ctx = grid->editor_ctx;
  unsigned int xpos, ypos;
  pbt_bool_t ret_bool = PBT_FALSE;

  switch (grid->state)
    {
    case GRID_CTRL_A_MODE:
      if (wbe_key_pressedA(winev->keys, 'A') == WBE_FALSE)
        {
          grid_select_all(grid);
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      break;
    case GRID_CTRL_C_MODE:
      if (wbe_key_pressedA(winev->keys, 'C') == WBE_FALSE)
        {
          grid_copy_selection(grid);
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      break;
    case GRID_CTRL_V_MODE:
      if (wbe_key_pressedA(winev->keys, 'V') == WBE_FALSE)
        {
          grid->state = GRID_PASTE_MODE;
          handle_grid_paste_mode(winev, grid);
        }
      break;
    case GRID_CTRL_X_MODE:
      if (wbe_key_pressedA(winev->keys, 'X') == WBE_FALSE)
        {
          grid_copy_selection(grid);
          _history_add_mark(&(editor_ctx->history));
          delete_note_selection(grid);
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      break;
    case GRID_CTRL_Z_MODE:
      if (wbe_key_pressedA(winev->keys, 'Z') == WBE_FALSE)
        {
          _history_undo(grid->editor_ctx);
          grid->state = GRID_NO_MODE;
          msq_draw_vggts(grid->vggts);
          ret_bool = PBT_TRUE;
        }
      break;
    case GRID_SELECT_NOTE_MODE:
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          if (winev->xpos < (int) pbt_ggt_xpos(&(grid->wgt)))
            xpos = 0;
          else if (winev->xpos
                   > (int) (pbt_ggt_xpos(&(grid->wgt))
                            + pbt_ggt_width(&(grid->wgt))
                            - 1))
            xpos = pbt_ggt_width(&(grid->wgt)) - 1;
          else
            xpos = winev->xpos - pbt_ggt_xpos(&(grid->wgt));
          if (winev->ypos < (int) pbt_ggt_ypos(&(grid->wgt)))
            ypos = 0;
          else if (winev->ypos
                   > (int) (pbt_ggt_ypos(&(grid->wgt))
                            + pbt_ggt_height(&(grid->wgt))
                            - 1))
            ypos = pbt_ggt_height(&(grid->wgt)) - 1;
          else
            ypos = winev->ypos - pbt_ggt_ypos(&(grid->wgt));
          if (xpos < 0)
            xpos = 0;
          if (ypos < 0)
            ypos = 0;
          editor_ctx->tmp_coo[2] = xpos;
          editor_ctx->tmp_coo[3] = ypos;
          draw_grid_selection(&(grid->wgt),
                              editor_ctx->tmp_coo);
        }
      else if (WBE_GET_BIT(winev->buttons, 0) == 0)
        {
          if (wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_FALSE)
            free_list_node(&(editor_ctx->selected_notes), free);
          handle_selection(editor_ctx);
          pbt_ggt_draw(&(grid->wgt));
          pbt_ggt_win_put_buffer(grid->wgt.ggt_win);
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      break;

    case GRID_WRITE_MODE:
      if (WBE_GET_BIT(winev->buttons, 1) == 0)
        {
          wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          if (PBT_IS_IN_GGT(&(grid->wgt),
                            winev->xpos,
                            winev->ypos) == PBT_TRUE)
            {
              editor_ctx->tmp_coo[0] =
                XPOS2TICK(editor_ctx,
                          winev->xpos
                          - pbt_ggt_xpos(&(grid->wgt))
                          + editor_ctx->hadj.pos);
              editor_ctx->tmp_coo[0] =
                (editor_ctx->tmp_coo[0] / editor_ctx->quantize) * editor_ctx->quantize;
              editor_ctx->tmp_coo[1] =
                YPOS2NOTE(editor_ctx,
                          winev->ypos
                          - pbt_ggt_ypos(&(grid->wgt))
                          + editor_ctx->vadj.pos);
              grid->state = GRID_WRITTING_MODE;
              handle_writting_note_mode(winev, grid);
            }
        }
      break;

    case GRID_WRITTING_MODE:
      handle_writting_note_mode(winev, grid);
      break;

    case GRID_MOVE_NOTE_MODE:
      if (handle_move_note_mode(winev, grid) == PBT_TRUE)
        {
          wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      break;

    case GRID_PASTE_MODE:
      ret_bool = handle_grid_paste_mode(winev, grid);
      break;

    case GRID_MOVE_NOTE_START_MODE:
      if (WBE_GET_BIT(winev->buttons, 2) == 0)
        {
          msq_write_move_note_start(grid, winev);
          msq_draw_vggts(grid->vggts);
          wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 2) == 1)
        msq_draw_move_note_start(grid, winev);
      break;

    case GRID_MOVE_NOTE_END_MODE:
      if (WBE_GET_BIT(winev->buttons, 2) == 0)
        {
          msq_write_move_note_end(grid, winev);
          msq_draw_vggts(grid->vggts);
          wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          grid->state = GRID_NO_MODE;
          ret_bool = PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 2) == 1)
        msq_draw_move_note_end(grid, winev);
      break;

    case GRID_NO_MODE:
      /* todo ? */
      ret_bool = PBT_TRUE;
      break;
    }

  return ret_bool;
}

void _msq_update_zoom(track_editor_ctx_t *editor_ctx,
                      msq_vggts_t *vggts)
{
  editor_ctx->qn_size =
    10 + (editor_ctx->zoom_adj.pos * editor_ctx->zoom_adj.pos * 90 / 2500);
  msq_update_hadj_startnlen(editor_ctx);
  msq_draw_vggts(vggts);
}

void value_wgt_unselect(msq_value_wgt_t *value)
{
  value->in_selection_mode = MSQ_FALSE;
  pbt_ggt_draw(&(value->wgt));
  pbt_wgt_win_put_buffer(&(value->wgt));
}

void vggts_value_unselect(msq_vggts_t *vggts)
{
  pbt_wgt_t *value_wgt = vggts->value->priv;
  msq_value_wgt_t *value = value_wgt->priv;

  value_wgt_unselect(value);
}

void msq_quantify_note_selection(msq_grid_wgt_t *grid)
{
  track_editor_ctx_t *editor_ctx = grid->editor_ctx;
  list_iterator_t iter = {};
  noteonoff_t *noteonoff;
  seqev_t *seqev;
  midicev_t *noteev;
  note_t *note_ptr;
  int tmp_tick_end;
  list_t note_list = {};

  for (iter_init(&iter, &(editor_ctx->selected_notes));
       iter_node(&iter);
       iter_next(&iter))
    {
      noteonoff = iter_node_ptr(&iter);
      seqev = evit_get_seqev(&(noteonoff->evit_noteon));
      noteev = seqev->addr;
      note_ptr = malloc(sizeof (note_t));
      note_ptr->num =  noteev->event.note.num;
      note_ptr->tick = msq_quantify_tick(editor_ctx,
                                         noteonoff->evit_noteon.tick);
      note_ptr->channel = noteev->chan;
      note_ptr->val = noteev->event.note.val;
      tmp_tick_end = msq_quantify_tick_end(editor_ctx,
                                           noteonoff->evit_noteoff.tick);
      if (tmp_tick_end <= note_ptr->tick)
        tmp_tick_end = note_ptr->tick + editor_ctx->quantize - 1;
      note_ptr->len = tmp_tick_end - note_ptr->tick;
      push_to_list(&note_list, note_ptr);
    }
  if (msq_check_note_list(&note_list) == MSQ_FALSE)
    {
      free_list_node(&note_list, free);
      return;
    }
  _history_add_mark(&(editor_ctx->history));
  delete_note_selection(grid);
  _msq_add_list_note(editor_ctx, &note_list);
  msq_draw_vggts(grid->vggts);
}

pbt_bool_t grid_wgt_set_focus_cb(pbt_ggt_t *ggt,
                                 wbe_window_input_t *winev,
                                 void *grid_addr)
{
  msq_grid_wgt_t *grid = grid_addr;
  track_editor_ctx_t *editor_ctx = grid->editor_ctx;
  int tick;
  noteonoff_t *noteonoff;
  unsigned int xpos, ypos;

  if (editor_ctx->selected_notes.len != 0
      && ((wbe_key_pressed(winev->keys, WBE_KEY_SUPPR) == WBE_TRUE)
          || (wbe_key_pressed(winev->keys, WBE_KEY_BSPACE) == WBE_TRUE)))
    {
      _history_add_mark(&(editor_ctx->history));
      delete_note_selection(grid);
    }
  else if ((wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
           && (wbe_key_pressedA(winev->keys, 'A') == WBE_TRUE))
    {
      grid->state = GRID_CTRL_A_MODE;
      return PBT_TRUE;
    }
  else if ((wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
           && (wbe_key_pressedA(winev->keys, 'C') == WBE_TRUE))
    {
      grid->state = GRID_CTRL_C_MODE;
      return PBT_TRUE;
    }
  else if ((wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
           && (wbe_key_pressedA(winev->keys, 'V') == WBE_TRUE))
    {
      grid->state = GRID_CTRL_V_MODE;
      return PBT_TRUE;
    }
  else if ((wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
           && (wbe_key_pressedA(winev->keys, 'X') == WBE_TRUE))
    {
      grid->state = GRID_CTRL_X_MODE;
      return PBT_TRUE;
    }
  else if ((wbe_key_pressed(winev->keys, WBE_KEY_ALT) == WBE_TRUE)
           && (wbe_key_pressedA(winev->keys, 'Q') == WBE_TRUE))
    {
      msq_quantify_note_selection(grid);
      return PBT_TRUE;
    }
  else if ((wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
           && (wbe_key_pressedA(winev->keys, 'Z') == WBE_TRUE))
    {
      grid->state = GRID_CTRL_Z_MODE;
      return PBT_TRUE;
    }
  else if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_TRUE)
    {
      xpos = GRIDXPOS(winev->xpos, grid);
      ypos = GRIDYPOS(winev->ypos, grid);
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          if (is_in_note_selection(XPOS2TICK(editor_ctx, xpos),
                                   editor_ctx->channel,
                                   YPOS2NOTE(editor_ctx,
                                             ypos),
                                   &(editor_ctx->selected_notes)) != NULL)
            {
              editor_ctx->tmp_coo[0] = xpos;
              editor_ctx->tmp_coo[1] = ypos;
              wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                    tctx_cursor_grabbing(editor_ctx));
              _draw_grid(&(ggt->pbarea), editor_ctx);
              draw_notes_filter_selection(&(ggt->pbarea), editor_ctx);
              draw_loop_veil(&(ggt->pbarea), editor_ctx);
              _wbe_pbw_texture_load(&(grid->wgt.ggt_win->pb_win));
              grid->state = GRID_MOVE_NOTE_MODE;
              editor_ctx->selected_notes_min_tick =
                _get_selected_min_tick(&(editor_ctx->selected_notes));
              handle_move_note_mode(winev, grid);
            }
          else
            {
              vggts_value_unselect(grid->vggts);
              editor_ctx->tmp_coo[0] = winev->xpos - pbt_wgt_xpos(grid);
              editor_ctx->tmp_coo[1] = winev->ypos - pbt_wgt_ypos(grid);
              editor_ctx->tmp_coo[2] = editor_ctx->tmp_coo[0];
              editor_ctx->tmp_coo[3] = editor_ctx->tmp_coo[1];
              if (editor_ctx->selected_notes.len == 0)
                {
                  handle_selection(editor_ctx);
                  if (editor_ctx->selected_notes.len != 0)
                    {
                      editor_ctx->tmp_coo[0] = xpos;
                      editor_ctx->tmp_coo[1] = ypos;
                      wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                            tctx_cursor_grabbing(editor_ctx));
                      _draw_grid(&(ggt->pbarea), editor_ctx);
                      draw_notes_filter_selection(&(ggt->pbarea), editor_ctx);
                      draw_loop_veil(&(ggt->pbarea), editor_ctx);
                      _wbe_pbw_texture_load(&(grid->wgt.ggt_win->pb_win));
                      grid->state = GRID_MOVE_NOTE_MODE;
                      editor_ctx->selected_notes_min_tick =
                        _get_selected_min_tick(&(editor_ctx->selected_notes));
                      handle_move_note_mode(winev, grid);
                    }
                  else
                    grid->state = GRID_SELECT_NOTE_MODE;
                }
              else
                grid->state = GRID_SELECT_NOTE_MODE;
            }
          return PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 1) == 1)
        {
          wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                tctx_cursor_pencil(editor_ctx));
          grid->state = GRID_WRITE_MODE;
          return PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 2) == 1)
        {
          tick = XPOS2TICK(editor_ctx, xpos);
          noteonoff = is_in_note_selection(tick,
                                           editor_ctx->channel,
                                           YPOS2NOTE(editor_ctx, ypos),
                                           &(editor_ctx->selected_notes));
          if (noteonoff != NULL)
            {
              editor_ctx->tmp_coo[0] = winev->xpos;
              _draw_grid(&(ggt->pbarea), editor_ctx);
              draw_notes_filter_selection(&(ggt->pbarea), editor_ctx);
              draw_loop_veil(&(ggt->pbarea), editor_ctx);
              _wbe_pbw_texture_load(&(grid->wgt.ggt_win->pb_win));
              if (tick - noteonoff->evit_noteon.tick
                  < noteonoff->evit_noteoff.tick - tick)
                {
                  grid->state = GRID_MOVE_NOTE_START_MODE;
                  msq_draw_move_note_start(grid, winev);
                }
              else
                {
                  grid->state = GRID_MOVE_NOTE_END_MODE;
                  msq_draw_move_note_end(grid, winev);
                }
              wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                    tctx_cursor_hresize(editor_ctx));
              return PBT_TRUE;
            }
        }
      else if (WBE_GET_BIT(winev->buttons, 3) == 1)
        {
          if (wbe_key_pressed(winev->keys, WBE_KEY_SHIFT) == WBE_TRUE)
            {
              msq_adj_inc(&(editor_ctx->hadj), SCROLL_INC);
              msq_draw_vggts(grid->vggts);
            }
          else if (wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
            {
              if (editor_ctx->zoom_adj.pos < 10)
                editor_ctx->zoom_adj.pos = 0;
              else
                editor_ctx->zoom_adj.pos -= 10;
              _msq_update_zoom(editor_ctx, grid->vggts);
            }
          else
            {
              msq_adj_inc(&(editor_ctx->vadj), SCROLL_INC);
              msq_draw_hggts(grid->hggts);
            }
        }
      else if (WBE_GET_BIT(winev->buttons, 4) == 1)
        {
          if (wbe_key_pressed(winev->keys, WBE_KEY_SHIFT) == WBE_TRUE)
            {
              msq_adj_dec(&(editor_ctx->hadj), SCROLL_INC);
              msq_draw_vggts(grid->vggts);
            }
          else if (wbe_key_pressed(winev->keys, WBE_KEY_CONTROL) == WBE_TRUE)
            {
              editor_ctx->zoom_adj.pos += 10;
              if (editor_ctx->zoom_adj.pos > 100)
                editor_ctx->zoom_adj.pos = 100;
              _msq_update_zoom(editor_ctx, grid->vggts);
            }
          else
            {
              msq_adj_dec(&(editor_ctx->vadj), SCROLL_INC);
              msq_draw_hggts(grid->hggts);
            }
        }
      else if (editor_ctx->selected_notes.len > 0)
        {
          if (is_in_note_selection(XPOS2TICK(editor_ctx, xpos),
                                   editor_ctx->channel,
                                   YPOS2NOTE(editor_ctx, ypos),
                                   &(editor_ctx->selected_notes)) != NULL)
            wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                  tctx_cursor_grab(editor_ctx));
          else
            wbe_window_set_cursor(grid->wgt.ggt_win->pb_win.win_be,
                                  tctx_cursor_arrow(editor_ctx));
        }
    }
  return PBT_FALSE;
}

void grid_wgt_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  msq_grid_wgt_t *grid_wgt = wgt->priv;

  pbt_evh_add_set_focus_cb(&(wgt->ggt_win->evh),
                           &(wgt->ggt),
                           grid_wgt_set_focus_cb,
                           grid_wgt);
  pbt_evh_add_unset_focus_cb(&(wgt->ggt_win->evh),
                             &(wgt->ggt),
                             grid_wgt_unset_focus_cb,
                             grid_wgt);
}

void grid_wgt_destroy(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_grid_wgt_t *grid_wgt = wgt->priv;

  free_list_node(&(grid_wgt->editor_ctx->selected_notes), free);
  free_list_node(&(grid_wgt->editor_ctx->history), free);

  pbt_wgt_evnode_destroy(ggt);
}

void grid_wgt_init(msq_grid_wgt_t *grid,
                   track_editor_ctx_t *editor_ctx,
                   msq_hggts_t *hggts,
                   msq_vggts_t *vggts)
{
  grid->editor_ctx = editor_ctx;
  grid->hggts = hggts;
  grid->vggts = vggts;
  grid->wgt.priv = grid;

  grid->wgt.ggt.priv = &(grid->wgt);
  grid->wgt.ggt.get_min_width = pbt_ggt_return_zero;
  grid->wgt.ggt.get_max_width = pbt_ggt_return_zero;
  grid->wgt.ggt.get_min_height = pbt_ggt_return_zero;
  grid->wgt.ggt.get_max_height = pbt_ggt_return_zero;
  grid->wgt.ggt.update_area_cb = pbt_ggt_memcpy_area;
  grid->wgt.ggt.draw_cb = draw_grid_cb;
  grid->wgt.ggt.destroy_cb = grid_wgt_destroy;

  grid->wgt.init_ev_cb = grid_wgt_init_ev;
}

void _draw_value_num(pbt_pbarea_t *pbarea,
                     track_editor_t *track_editor,
                     short value)
{
  char value_label[4];
  unsigned int str_width;
  unsigned int xpos;

  snprintf(value_label, 4, "%hd", value);

  pbt_pbarea_fill(pbarea, tctx_wgt_normal_bg(&(track_editor->editor_ctx)));

  pbt_font_get_string_width(&(track_editor->editor_ctx.theme->global_theme->theme.font),
                            value_label,
                            &str_width);
  xpos = (pbarea->width / 2) - (str_width / 2);
  pbt_pbarea_printf(pbarea,
                    &(track_editor->editor_ctx.theme->global_theme->theme.font),
                    tctx_wgt_normal_fg(&(track_editor->editor_ctx)),
                    xpos, 1,
                    value_label);
}

void draw_value_num(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;

  _draw_value_num(&(ggt->pbarea),
                  track_editor,
                  track_editor->editor_ctx.default_velocity);
}

void _draw_value_vbar(pbt_pbarea_t *pbarea,
                      track_editor_t *track_editor,
                      unsigned short value)
{
  unsigned int bar_ypos, bar_size;
  unsigned char bar_color[4];

  bar_size = ((pbarea->height - 1) * value / MAX_14b_VAL) + 1;
  bar_ypos = pbarea->height - bar_size;
  set_gradient_color(track_editor->editor_ctx.theme, bar_color, value);

  pbt_pbarea_fill(pbarea,
                  tctx_frame_bg(&(track_editor->editor_ctx)));
  pbt_pbarea_fillrect(pbarea,
                      1, bar_ypos,
                      pbarea->width - 2, bar_size,
                      bar_color);
}

void draw_value_vbar(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;

  _draw_value_vbar(&(ggt->pbarea),
                   track_editor,
                   track_editor->editor_ctx.default_velocity << 7);
}

#define VALUE_BAR_WIDTH 5

void draw_value(pbt_pbarea_t *pbarea,
                track_editor_ctx_t *editor_ctx,
                unsigned int tick,
                unsigned short velocity)
{
  int xpos = TICK2XPOS(editor_ctx, tick) - editor_ctx->hadj.pos;
  unsigned int len = ((pbarea->height - 1) * velocity / MAX_14b_VAL) + 1;
  unsigned int width = VALUE_BAR_WIDTH;
  unsigned char color[4];

  if ((xpos < 0) || (xpos >= pbarea->width))
    return;

  if ((xpos + width) >= pbarea->width)
    width = pbarea->width - xpos;

  set_gradient_color(editor_ctx->theme, color, velocity);

  pbt_pbarea_fillrect(pbarea,
                      xpos,
                      0,
                      width,
                      pbarea->height,
                      tctx_frame_bg(editor_ctx));

  pbt_pbarea_fillrect(pbarea,
                      xpos,
                      pbarea->height - len,
                      width,
                      len,
                      color);
}

msq_tmp_tick_bar_t *msq_tmp_tick_bar_add(msq_tmp_tick_bar_t *nodes_head,
                                         unsigned int tick,
                                         unsigned short velocity)
{
  msq_tmp_tick_bar_t *node, *new;

  if (nodes_head == NULL)
    {
      new = malloc(sizeof (msq_tmp_tick_bar_t));
      new->tick = tick;
      new->velocity = velocity;
      new->next = NULL;
      nodes_head = new;
    }
  else if (nodes_head->tick > tick)
    {
      new = malloc(sizeof (msq_tmp_tick_bar_t));
      new->tick = tick;
      new->velocity = velocity;
      new->next = nodes_head;
      nodes_head = new;
    }
  else if (nodes_head->tick == tick)
    nodes_head->velocity = velocity;
  else
    {
      for (node = nodes_head;
           node->next != NULL;
           node = node->next)
        {
          if (node->next->tick == tick)
            {
              node->next->velocity = velocity;
              break;
            }
          else if (node->next->tick > tick)
            {
              new = malloc(sizeof (msq_tmp_tick_bar_t));
              new->tick = tick;
              new->velocity = velocity;
              new->next = node->next;
              node->next = new;
              break;
            }
        }
      if (node->next == NULL)
        {
          new = malloc(sizeof (msq_tmp_tick_bar_t));
          new->tick = tick;
          new->velocity = velocity;
          new->next = NULL;
          node->next = new;
        }
    }
  return nodes_head;
}

msq_bool_t msq_tmp_tick_bar_get(msq_tmp_tick_bar_t *node,
                                unsigned int tick,
                                unsigned short *velocity)
{
  while (node)
    {
      if (node->tick == tick)
        {
          *velocity = node->velocity;
          return MSQ_TRUE;
        }
      node = node->next;
    }
  return MSQ_FALSE;
}

void msq_tmp_tick_bar_destroy(msq_tmp_tick_bar_t *node)
{
  msq_tmp_tick_bar_t *tmp;

  while (node)
    {
      tmp = node;
      node = node->next;
      free(tmp);
    }
}

short msq_tick_the3rule(unsigned int tick_k,
                        unsigned int tick_a,
                        unsigned short vel_a,
                        unsigned int tick_b,
                        unsigned short vel_b)
{
  short vel_diff = (short) vel_b - (short) vel_a;
  int tick_diff1 = (int) tick_k - (int) tick_a;
  int tick_diff2 = (int) tick_b - (int) tick_a;

  return (((tick_diff1) * vel_diff) / (tick_diff2)) + vel_a;
}

void draw_value_tmp_selection(pbt_wgt_t *wgt,
                              int xpos1,
                              int xpos2)
{
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;
  GLfloat color[] = {0, 0, 0, 0.5};
  wbe_pbw_t *pb_win = &(wgt->ggt_win->pb_win);
  int tex_width = pb_win->buffer.width,
    tex_height = pb_win->buffer.height;

  if (xpos1 < 0)
    xpos1 = 0;
  else if (xpos1 >= pbt_ggt_width(wgt))
    xpos1 = pbt_ggt_width(wgt) - 1;

  if (xpos2 < 0)
    xpos2 = 0;
  else if (xpos2 >= pbt_ggt_width(wgt))
    xpos2 = pbt_ggt_width(wgt) - 1;

  if (xpos1 == xpos2)
    return;

  if (xpos1 < xpos2)
    {
      xmin_float = (((float) xpos1 + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
      xmax_float = (((float) xpos2 + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
    }
  else
    {
      xmin_float = (((float) xpos2 + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
      xmax_float = (((float) xpos1 + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
    }
  ymin_float = (-((float) pbt_ggt_ypos(wgt) + pbt_ggt_height(wgt))
                * 2.0 / tex_height)
    + 1.0;
  ymax_float = (-((float) pbt_ggt_ypos(wgt))
                * 2.0 / tex_height)
    + 1.0;

  wbe_pbw_make_context(pb_win);
  /* TODO refresh last pos */
  pbt_wgt_gl_refresh(wgt);
  _wbe_gl_texture_n_color_put_rect(xmin_float,
                                   ymin_float,
                                   xmax_float,
                                   ymax_float,
                                   color);
  wbe_gl_flush();
}

void draw_values_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_value_wgt_t *value_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = value_wgt->editor_ctx;
  ev_iterator_t evit;
  midicev_t *mcev = NULL;
  unsigned int velocity;
  byte_t ctrl_num;

  pbt_pbarea_fill(&(ggt->pbarea), tctx_frame_bg(editor_ctx));

  if (value_wgt->type == VALUE_NOTELEVEL_TYPE)
    {
      mcev = evit_init_noteon(&evit,
                              &(editor_ctx->track_ctx->track->tickev_list),
                              editor_ctx->channel);
      while (mcev != NULL)
        {
          velocity = mcev->event.note.val << 7;
          draw_value(&(ggt->pbarea),
                     editor_ctx,
                     evit.tick,
                     velocity);
          mcev = evit_next_noteon(&evit, editor_ctx->channel);
        }
    }
  else
    {
      if (value_wgt->type == VALUE_PITCH_TYPE)
        {
          mcev = evit_init_pitch(&evit,
                                 &(editor_ctx->track_ctx->track->tickev_list),
                                 editor_ctx->channel);
          while (mcev != NULL)
            {
              velocity = ((mcev->event.pitchbend.Hval << 7)
                          + mcev->event.pitchbend.Lval);
              draw_value(&(ggt->pbarea),
                         editor_ctx,
                         evit.tick,
                         velocity);
              mcev = evit_next_pitch(&evit, editor_ctx->channel);
            }
        }
      else if (value_wgt->type >= VALUE_TYPE_OFFSET)
        {
          ctrl_num = value_wgt->type - VALUE_TYPE_OFFSET;
          mcev = evit_init_ctrl_num(&evit,
                                    &(editor_ctx->track_ctx->track->tickev_list),
                                    editor_ctx->channel,
                                    ctrl_num);
          while (mcev != NULL)
            {
              velocity = mcev->event.ctrl.val << 7;
              draw_value(&(ggt->pbarea),
                         editor_ctx,
                         evit.tick,
                         velocity);
              mcev = evit_next_ctrl_num(&evit, editor_ctx->channel, ctrl_num);
            }
        }

      if (value_wgt->in_selection_mode == MSQ_TRUE)
        {
          msq_draw_veil(&(ggt->pbarea),
                        value_wgt->selection_coo[0], value_wgt->selection_coo[1],
                        0, pbt_ggt_height(wgt));
        }
    }
}

unsigned int value_wgt_xpos2tick(pbt_wgt_t *wgt,
                                 track_editor_ctx_t *editor_ctx,
                                 int xpos)
{
  if ((editor_ctx->hadj.pos == 0)
      && (xpos <= (int) pbt_ggt_xpos(wgt)))
    return 0;
  return XPOS2TICK(editor_ctx,
                   xpos + editor_ctx->hadj.pos - pbt_ggt_xpos(wgt));
}

unsigned short value_wgt_ypos2vel(pbt_wgt_t *wgt, int ypos)
{
  unsigned int ggt_ypos, ggt_height;

  if (ypos <= pbt_ggt_ypos(wgt))
    return MAX_14b_VAL;
  else
    {
      ggt_ypos = ypos - pbt_ggt_ypos(wgt);
      ggt_height = pbt_ggt_height(wgt);
      if (ggt_ypos >= ggt_height)
        return 0;
      return (ggt_height - ggt_ypos) * MAX_14b_VAL / ggt_height;
    }
}

void value_wgt_draw_tmp_bar(msq_value_wgt_t *value_wgt)
{
  track_editor_ctx_t *editor_ctx = value_wgt->editor_ctx;
  msq_tmp_tick_bar_t *node = value_wgt->tmp_bar_head;
  unsigned int bar_size;
  unsigned int xpos, ypos;
  unsigned char color[4];

  wbe_pbw_make_context(&(value_wgt->wgt.ggt_win->pb_win));
  while (node)
    {
      xpos = TICK2XPOS(editor_ctx, node->tick) - editor_ctx->hadj.pos;
      bar_size = pbt_wgt_height(value_wgt) * node->velocity / MAX_14b_VAL;
      ypos = pbt_wgt_height(value_wgt) - bar_size;
      set_gradient_color(editor_ctx->theme, color, node->velocity);
      pbt_wgt_gl_fillrect(&(value_wgt->wgt),
                          xpos, 0,
                          VALUE_BAR_WIDTH, ypos,
                          editor_ctx->theme->global_theme->theme.frame_bg);
      pbt_wgt_gl_fillrect(&(value_wgt->wgt),
                          xpos, ypos,
                          VALUE_BAR_WIDTH, bar_size,
                          color);
      node = node->next;
    }
  wbe_gl_flush();
}

void value_wgt_update_tmp_bar(msq_value_wgt_t *value,
                              unsigned int tick,
                              unsigned short vel,
                              pbt_bool_t use_default)
{
  track_editor_ctx_t *editor_ctx = value->editor_ctx;
  unsigned int tick1, tick2, tmp_tick;
  unsigned short vel1, vel2;
  short new_vel;
  ev_iterator_t evit;
  midicev_t *midicev;

  tick2 = tick;

  if (tick2 == value->tmp_coo[0])
    {
      value->tmp_coo[1] = vel;
      return;
    }

  if (tick2 > value->tmp_coo[0])
    {
      tick1 = value->tmp_coo[0];
      vel1 = value->tmp_coo[1];
      vel2 = vel;
    }
  else
    {
      tick1 = tick2;
      vel1 = vel;
      tick2 = value->tmp_coo[0];
      vel2 = value->tmp_coo[1];
    }

  switch (value->type)
    {
    case VALUE_NOTELEVEL_TYPE:
      midicev = evit_init_noteon(&evit,
                                 &(editor_ctx->track_ctx->track->tickev_list),
                                 editor_ctx->channel);
      if (midicev != NULL)
        {
          while (midicev && evit.tick < tick1)
            midicev = evit_next_noteon(&evit, editor_ctx->channel);
          while (midicev && evit.tick <= tick2)
            {
              if (is_in_selected_noteon(&(editor_ctx->selected_notes),
                                        midicev) == MSQ_TRUE)
                {
                  if (use_default == PBT_TRUE)
                    new_vel = editor_ctx->default_velocity << 7;
                  else
                    {
                      new_vel = msq_tick_the3rule(evit.tick,
                                                  tick1, vel1,
                                                  tick2, vel2);
                      if (new_vel > MAX_14b_VAL)
                        new_vel = MAX_14b_VAL;
                      else if (new_vel < 0)
                        new_vel = 0;
                    }
                  value->tmp_bar_head =
                    msq_tmp_tick_bar_add(value->tmp_bar_head,
                                         evit.tick,
                                         new_vel);
                }
              midicev = evit_next_noteon(&evit, editor_ctx->channel);
            }
        }
      break;

    default:                    /* Controls and Pitch */
      for (tmp_tick = msq_quantify_tick(editor_ctx, tick1);
           tmp_tick < tick2;
           tmp_tick += editor_ctx->quantize)
        {
          if (use_default == PBT_TRUE)
            new_vel = editor_ctx->default_velocity << 7;
          else
            {
              new_vel = msq_tick_the3rule(tmp_tick,
                                          tick1, vel1,
                                          tick2, vel2);
              if (new_vel > MAX_14b_VAL)
                new_vel = MAX_14b_VAL;
              else if (new_vel < 0)
                new_vel = 0;
            }
          value->tmp_bar_head =
            msq_tmp_tick_bar_add(value->tmp_bar_head,
                                 tmp_tick,
                                 new_vel);
        }
    }
  value->tmp_coo[0] = tick;
  value->tmp_coo[1] = vel;
  value_wgt_draw_tmp_bar(value);
}

void value_wgt_write_tmp_bar(msq_value_wgt_t *value_wgt)
{
  track_editor_ctx_t *editor_ctx = value_wgt->editor_ctx;
  ev_iterator_t evit;
  midicev_t *midicev, new_midicev = {};
  msq_tmp_tick_bar_t *node = value_wgt->tmp_bar_head;
  unsigned char ctrl_num;

  switch (value_wgt->type)
    {
    case VALUE_NOTELEVEL_TYPE:
      midicev = evit_init_noteon(&evit,
                                 &(editor_ctx->track_ctx->track->tickev_list),
                                 editor_ctx->channel);
      while (node != NULL)
        {
          while (midicev != NULL && evit.tick < node->tick)
            midicev = evit_next_noteon(&evit, editor_ctx->channel);
          if (midicev == NULL || evit.tick != node->tick)
            pbt_logerr("Missing note events. (value widget)");
          else
            {
              do {
                if (is_in_selected_noteon(&(editor_ctx->selected_notes),
                                          midicev) == MSQ_TRUE)
                  {
                    midicev->event.note.val =
                      node->velocity * MAX_7b_VAL / MAX_14b_VAL;
                  }
                midicev = evit_next_noteon(&evit, editor_ctx->channel);
              } while (midicev != NULL && evit.tick == node->tick);
            }
          node = node->next;
        }
      break;
    case VALUE_PITCH_TYPE:
      evit_init(&evit,
                &(editor_ctx->track_ctx->track->tickev_list));
      while (node != NULL)
        {
          new_midicev.type = MSQ_MIDI_PITCHWHEELCHANGE;
          new_midicev.chan = editor_ctx->channel;
          new_midicev.event.pitchbend.Lval = node->velocity & 0x7F;
          new_midicev.event.pitchbend.Hval = (node->velocity >> 7) & 0x7F;
          _history_evit_add_midicev(&(editor_ctx->history),
                                    &evit,
                                    node->tick,
                                    &new_midicev);
          node = node->next;
        }
      break;
    default:                    /* Controls */
      evit_init(&evit,
                &(editor_ctx->track_ctx->track->tickev_list));
      ctrl_num = value_wgt->type - VALUE_TYPE_OFFSET;
      while (node != NULL)
        {
          new_midicev.type = MSQ_MIDI_CONTROLCHANGE;
          new_midicev.chan = editor_ctx->channel;
          new_midicev.event.ctrl.num = ctrl_num;
          new_midicev.event.ctrl.val = node->velocity * MAX_7b_VAL / MAX_14b_VAL;
          _history_evit_add_midicev(&(editor_ctx->history),
                                    &evit,
                                    node->tick,
                                    &new_midicev);
          node = node->next;
        }
    }
}

void value_wgt_select_note(pbt_wgt_t *grid_wgt,
                           unsigned char channel,
                           unsigned int tick_min,
                           unsigned int tick_max)
{
  msq_grid_wgt_t *grid = grid_wgt->priv;
  unsigned int tmp;

  if (tick_min == tick_max)
    return;
  if (tick_max < tick_min)
    {
      tmp = tick_min;
      tick_min = tick_max;
      tick_max = tmp;
    }

  free_list_node(&(grid->editor_ctx->selected_notes), free);
  _select_noteonoff(&(grid->editor_ctx->selected_notes),
                    &(grid->editor_ctx->track_ctx->track->tickev_list),
                    grid->editor_ctx->channel,
                    tick_min,
                    tick_max,
                    0,
                    127);
  pbt_ggt_draw(grid_wgt);
  pbt_wgt_win_put_buffer(grid_wgt);
}

pbt_bool_t value_wgt_unset_focus_cb(pbt_ggt_t *ggt,
                                    wbe_window_input_t *winev,
                                    void *addr_unused)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_value_wgt_t *value_wgt = wgt->priv;
  track_editor_ctx_t *editor_ctx = value_wgt->editor_ctx;
  int pos;

  switch (value_wgt->state)
    {
    case (VALUE_WRITE_MODE):
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          value_wgt->state = VALUE_WRITTING_MODE;
          value_wgt->tmp_coo[0] = value_wgt_xpos2tick(wgt,
                                                      editor_ctx,
                                                      winev->xpos);
          value_wgt->tmp_coo[1] = value_wgt_ypos2vel(wgt, winev->ypos);
        }
      else if (WBE_GET_BIT(winev->buttons, 2) == 1)
        {
          value_wgt->tmp_coo[0] = value_wgt_xpos2tick(wgt,
                                                      editor_ctx,
                                                      winev->xpos);
          value_wgt->state = VALUE_WRITTING_DEFAULT;
        }
      else if (WBE_GET_BIT(winev->buttons, 1) == 0)
        {
          msq_tmp_tick_bar_destroy(value_wgt->tmp_bar_head);
          value_wgt->tmp_bar_head = NULL;
          wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          value_wgt->state = VALUE_NO_MODE;
          return PBT_TRUE;
        }
      break;
    case (VALUE_WRITTING_MODE):
      if (WBE_GET_BIT(winev->buttons, 0) == 0)
        {
          value_wgt_write_tmp_bar(value_wgt);
          msq_draw_vggts(value_wgt->vggts);
          msq_tmp_tick_bar_destroy(value_wgt->tmp_bar_head);
          value_wgt->tmp_bar_head = NULL;
          value_wgt->state = VALUE_WRITE_MODE;
        }
      else
        value_wgt_update_tmp_bar(value_wgt,
                                 value_wgt_xpos2tick(wgt,
                                                     editor_ctx,
                                                     winev->xpos),
                                 value_wgt_ypos2vel(wgt, winev->ypos),
                                 PBT_FALSE);
      break;
    case (VALUE_WRITTING_DEFAULT):
      if (WBE_GET_BIT(winev->buttons, 2) == 0)
        {
          value_wgt_write_tmp_bar(value_wgt);
          msq_draw_vggts(value_wgt->vggts);
          msq_tmp_tick_bar_destroy(value_wgt->tmp_bar_head);
          value_wgt->tmp_bar_head = NULL;
          value_wgt->state = VALUE_WRITE_MODE;
        }
      else
        value_wgt_update_tmp_bar(value_wgt,
                                 value_wgt_xpos2tick(wgt,
                                                     editor_ctx,
                                                     winev->xpos),
                                 0,
                                 PBT_TRUE);
      break;
    case (VALUE_SELECT_MODE):
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          draw_value_tmp_selection(wgt,
                                   value_wgt->tmp_coo[0],
                                   winev->xpos
                                   - pbt_ggt_xpos(&(value_wgt->wgt)));
        }
      else
        {
          wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                tctx_cursor_arrow(editor_ctx));
          if (value_wgt->type == VALUE_NOTELEVEL_TYPE)
            value_wgt_select_note(value_wgt->vggts->grid->priv,
                                  editor_ctx->channel,
                                  XPOS2TICK(editor_ctx,
                                            value_wgt->tmp_coo[0]
                                            + editor_ctx->hadj.pos),
                                  value_wgt_xpos2tick(wgt,
                                                      editor_ctx,
                                                      winev->xpos));
          else
            {
              pos = winev->xpos - pbt_ggt_xpos(wgt);
              if (pos < 0)
                pos = 0;
              else if (pos >= pbt_ggt_width(wgt))
                pos = pbt_ggt_width(wgt) - 1;

              if (value_wgt->tmp_coo[0] <= pos)
                {
                  value_wgt->selection_coo[0] = value_wgt->tmp_coo[0];
                  value_wgt->selection_coo[1] = pos;
                }
              else
                {
                  value_wgt->selection_coo[0] = pos;
                  value_wgt->selection_coo[1] = value_wgt->tmp_coo[0];
                }
              value_wgt->in_selection_mode = MSQ_TRUE;
              pbt_ggt_draw(wgt);
              pbt_wgt_win_put_buffer(wgt);
            }
          value_wgt->state = VALUE_NO_MODE;
          return PBT_TRUE;
        }
      break;
    default:
      ;
    }
  return PBT_FALSE;
}

void value_wgt_delete_selection(msq_value_wgt_t *value)
{
  track_editor_ctx_t *editor_ctx = value->editor_ctx;
  unsigned int tick_min = XPOS2TICK(editor_ctx, value->selection_coo[0]),
    tick_max = XPOS2TICK(editor_ctx, value->selection_coo[1]);
  void (*del_func)(track_ctx_t *, ev_iterator_t *);
  ev_iterator_t evit;
  midicev_t *mcev = NULL;
  unsigned char ctrl_num;

  /* /!\ TODO
     trackctx_del_event must be locked until all deletion is passed
     (SEGFAULT race condition) */
  if (editor_ctx->track_ctx->engine
      && engine_is_running(editor_ctx->track_ctx->engine) == MSQ_TRUE)
    del_func = trackctx_event2trash;
  else
    del_func = trackctx_del_event;

  if (value->type == VALUE_PITCH_TYPE)
    {
      mcev = evit_init_pitch(&evit,
                             &(editor_ctx->track_ctx->track->tickev_list),
                             editor_ctx->channel);
      while (mcev != NULL && evit.tick < tick_min)
        mcev = evit_next_pitch(&evit, editor_ctx->channel);
      _history_add_mark(&(editor_ctx->history));
      while (mcev != NULL && evit.tick < tick_max)
        {
          _history_evit_del_midicev(editor_ctx, &evit, del_func);
          /* del_func(editor_ctx->track_ctx, &evit); */
          mcev = _evit_get_midicev(&evit);
          if (mcev
              && (mcev->chan != editor_ctx->channel
                  || mcev->type != MSQ_MIDI_PITCHWHEELCHANGE))
            mcev = evit_next_pitch(&evit, editor_ctx->channel);
        }
    }
  else if (value->type >= VALUE_TYPE_OFFSET)
    {
      ctrl_num = value->type - VALUE_TYPE_OFFSET;
      mcev = evit_init_ctrl_num(&evit,
                                &(editor_ctx->track_ctx->track->tickev_list),
                                editor_ctx->channel,
                                ctrl_num);
      while (mcev != NULL && evit.tick < tick_min)
        mcev = evit_next_ctrl_num(&evit, editor_ctx->channel, ctrl_num);
      _history_add_mark(&(editor_ctx->history));
      while (mcev != NULL && evit.tick < tick_max)
        {
          _history_evit_del_midicev(editor_ctx, &evit, del_func);
          /* del_func(editor_ctx->track_ctx, &evit); */
          mcev = _evit_get_midicev(&evit);
          if (mcev
              && (mcev->chan != editor_ctx->channel
                  || mcev->type != MSQ_MIDI_CONTROLCHANGE
                  || mcev->event.ctrl.num != ctrl_num))
            mcev = evit_next_ctrl_num(&evit,
                                      editor_ctx->channel,
                                      ctrl_num);
        }
    }
  value_wgt_unselect(value);
}

void vggts_grid_unselect(msq_vggts_t *vggts)
{
  pbt_wgt_t *grid_wgt = vggts->grid->priv;
  msq_grid_wgt_t *grid = grid_wgt->priv;

  free_list_node(&(grid->editor_ctx->selected_notes), free);
  pbt_ggt_draw(&(grid->wgt));
  pbt_ggt_win_put_buffer(grid->wgt.ggt_win);
  grid->state = GRID_NO_MODE;
}

pbt_bool_t value_wgt_set_focus_cb(pbt_ggt_t *ggt,
                                  wbe_window_input_t *winev,
                                  void *addr_unused)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_value_wgt_t *value = wgt->priv;
  track_editor_ctx_t *editor_ctx = value->editor_ctx;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_TRUE)
    {
      if (WBE_GET_BIT(winev->buttons, 0) == 1)
        {
          vggts_grid_unselect(value->vggts);
          value->in_selection_mode = MSQ_FALSE;
          pbt_ggt_draw(wgt);
          pbt_wgt_win_put_buffer(wgt);
          wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                tctx_cursor_hresize(editor_ctx));
          value->tmp_coo[0] = winev->xpos - pbt_ggt_xpos(&(value->wgt));
          value->state = VALUE_SELECT_MODE;
          return PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 1) == 1)
        {
          value->in_selection_mode = MSQ_FALSE;
          pbt_ggt_draw(wgt);
          pbt_wgt_win_put_buffer(wgt);
          wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                tctx_cursor_pencil(editor_ctx));
          value->state = VALUE_WRITE_MODE;
          return PBT_TRUE;
        }
      else if (WBE_GET_BIT(winev->buttons, 2) == 1)
        {
          value->in_selection_mode = MSQ_FALSE;
          pbt_ggt_draw(wgt);
          pbt_wgt_win_put_buffer(wgt);
          wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                                tctx_cursor_pencil(editor_ctx));
          value->tmp_coo[0] = value_wgt_xpos2tick(wgt,
                                                  editor_ctx,
                                                  winev->xpos);
          value->state = VALUE_WRITTING_DEFAULT;
          return PBT_TRUE;
        }
    }

  if ((value->in_selection_mode == MSQ_TRUE)
      && ((wbe_key_pressed(winev->keys, WBE_KEY_SUPPR) == WBE_TRUE)
          || (wbe_key_pressed(winev->keys, WBE_KEY_BSPACE) == WBE_TRUE)))
    value_wgt_delete_selection(value);
  return PBT_FALSE;
}

void value_wgt_init_ev_cb(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  pbt_evh_add_set_focus_cb(&(wgt->ggt_win->evh),
                           &(wgt->ggt),
                           value_wgt_set_focus_cb,
                           NULL);
  pbt_evh_add_unset_focus_cb(&(wgt->ggt_win->evh),
                             &(wgt->ggt),
                             value_wgt_unset_focus_cb,
                             NULL);
}

void value_wgt_init(msq_value_wgt_t *value,
                    track_editor_ctx_t *editor_ctx,
                    msq_vggts_t *vggts)
{
  value->editor_ctx = editor_ctx;
  value->tmp_bar_head = NULL;
  value->type = VALUE_NOTELEVEL_TYPE;
  value->vggts = vggts;
  value->wgt.priv = value;

  value->wgt.ggt.priv = &(value->wgt);
  value->wgt.ggt.get_min_width = pbt_ggt_return_zero;
  value->wgt.ggt.get_max_width = pbt_ggt_return_zero;
  value->wgt.ggt.get_min_height = pbt_ggt_return_zero;
  value->wgt.ggt.get_max_height = pbt_ggt_return_zero;
  value->wgt.ggt.update_area_cb = pbt_ggt_memcpy_area;
  value->wgt.ggt.draw_cb = draw_values_cb;
  value->wgt.ggt.destroy_cb = pbt_wgt_evnode_destroy;

  value->wgt.init_ev_cb = value_wgt_init_ev_cb;
}

pbt_bool_t value_num_set_focus_cb(pbt_ggt_t *ggt,
                                  wbe_window_input_t *winev,
                                  void *addr_unused)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;

  if ((_PBT_IS_IN_GGT(ggt,
                      winev->xpos,
                      winev->ypos) == PBT_FALSE)
      || (WBE_GET_BIT(winev->buttons, 0) == 0))
    return PBT_FALSE;

  track_editor->editor_ctx.tmp_coo[0] = winev->ypos;
  return PBT_TRUE;
}

void refresh_default_value(track_editor_t *track_editor, int ypos_diff)
{
  /* GL ?? */
  int current_size;
  int new_7_val, new_14_val;

  current_size = track_editor->editor_ctx.default_velocity
    * pbt_ggt_height(&(track_editor->value_vbar_wgt))
    / MAX_7b_VAL;
  if (current_size + ypos_diff < 0)
    {
      _draw_value_vbar(&(track_editor->value_vbar_wgt.ggt.pbarea), track_editor, 0);
      _draw_value_num(&(track_editor->value_num_wgt.ggt.pbarea), track_editor, 0);
    }
  else if (current_size + ypos_diff
           > pbt_ggt_height(&(track_editor->value_vbar_wgt)))
    {
      _draw_value_vbar(&(track_editor->value_vbar_wgt.ggt.pbarea),
                       track_editor,
                       MAX_14b_VAL);
      _draw_value_num(&(track_editor->value_num_wgt.ggt.pbarea),
                      track_editor,
                      MAX_7b_VAL);
    }
  else
    {
      new_7_val = track_editor->editor_ctx.default_velocity
        + (ypos_diff * MAX_7b_VAL
           / ((int) pbt_ggt_height(&(track_editor->value_vbar_wgt))));
      new_14_val = new_7_val << 7;
      _draw_value_vbar(&(track_editor->value_vbar_wgt.ggt.pbarea), track_editor,
                       new_14_val);
      _draw_value_num(&(track_editor->value_num_wgt.ggt.pbarea),
                      track_editor,
                      (short) new_7_val);
    }
  pbt_wgt_win_put_buffer(&(track_editor->value_num_wgt));
}

pbt_bool_t value_num_unset_focus_cb(pbt_ggt_t *ggt,
                                    wbe_window_input_t *winev,
                                    void *addr_unused)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;
  int new_vel;

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    {
      new_vel = (track_editor->editor_ctx.tmp_coo[0] - winev->ypos) * MAX_7b_VAL
        / ((int) pbt_ggt_height(&(track_editor->value_vbar_wgt)));
      track_editor->editor_ctx.default_velocity =
        (((int) track_editor->editor_ctx.default_velocity) + new_vel < 0) ?
        0 :
        (track_editor->editor_ctx.default_velocity + new_vel > MAX_7b_VAL) ?
        MAX_7b_VAL :
        track_editor->editor_ctx.default_velocity + new_vel;
      pbt_ggt_draw(&(track_editor->value_vbar_wgt));
      pbt_ggt_draw(&(track_editor->value_num_wgt));
      return PBT_TRUE;
    }

  refresh_default_value(track_editor,
                        (track_editor->editor_ctx.tmp_coo[0] - winev->ypos));
  return PBT_FALSE;
}

void set_cursor_hdouble_arrow_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  wbe_window_set_cursor(track_editor->ggt_win.pb_win.win_be,
                        tctx_cursor_hresize(&(track_editor->editor_ctx)));
}

void set_cursor_vdouble_arrow_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  wbe_window_set_cursor(track_editor->ggt_win.pb_win.win_be,
                        tctx_cursor_vresize(&(track_editor->editor_ctx)));
}

void set_cursor_arrow_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  wbe_window_set_cursor(track_editor->ggt_win.pb_win.win_be,
                        tctx_cursor_arrow(&(track_editor->editor_ctx)));
}

void value_bar_wgt_init_ev_cb(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  track_editor_t *track_editor = wgt->priv;

  pbt_evh_add_enter_cb(&(wgt->ggt_win->evh),
                       &(wgt->ggt),
                       set_cursor_vdouble_arrow_cb,
                       track_editor);
  pbt_evh_add_leave_cb(&(wgt->ggt_win->evh),
                       &(wgt->ggt),
                       set_cursor_arrow_cb,
                       track_editor);
  pbt_evh_add_set_focus_cb(&(wgt->ggt_win->evh),
                           &(wgt->ggt),
                           value_num_set_focus_cb,
                           NULL);
  pbt_evh_add_unset_focus_cb(&(wgt->ggt_win->evh),
                             &(wgt->ggt),
                             value_num_unset_focus_cb,
                             NULL);
}

unsigned int value_wgt_return_note_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  track_editor_t *track_editor = wgt->priv;

  return track_editor->editor_ctx.note_height;
}

unsigned int value_wgt_return_bar_width(pbt_ggt_t *ggt)
{
  return DEFAULT_GGTBAR_WIDTH;
}

void value_num_wgt_init(pbt_wgt_t *wgt, track_editor_t *track_editor)
{
  wgt->ggt.priv = wgt;
  wgt->priv = track_editor;
  wgt->ggt.get_min_width = pbt_ggt_return_zero;
  wgt->ggt.get_max_width = pbt_ggt_return_zero;
  wgt->ggt.get_min_height = value_wgt_return_note_height;
  wgt->ggt.get_max_height = value_wgt_return_note_height;
  wgt->ggt.update_area_cb = pbt_ggt_memcpy_area;
  wgt->ggt.draw_cb = draw_value_num;
  wgt->ggt.destroy_cb = pbt_wgt_evnode_destroy;
  wgt->init_ev_cb = value_bar_wgt_init_ev_cb;
}

void value_vbar_wgt_init(pbt_wgt_t *wgt, track_editor_t *track_editor)
{
  wgt->ggt.priv = wgt;
  wgt->priv = track_editor;
  wgt->ggt.get_min_width = value_wgt_return_bar_width;
  wgt->ggt.get_max_width = value_wgt_return_bar_width;
  wgt->ggt.get_min_height = pbt_ggt_return_zero;
  wgt->ggt.get_max_height = pbt_ggt_return_zero;
  wgt->ggt.update_area_cb = pbt_ggt_memcpy_area;
  wgt->ggt.draw_cb = draw_value_vbar;
  wgt->ggt.destroy_cb = pbt_wgt_evnode_destroy;
  wgt->init_ev_cb = value_bar_wgt_init_ev_cb;
}

void window_input_cb(wbe_window_input_t *winev, void *track_editor_addr)
{
  track_editor_t *track_editor = (track_editor_t *) track_editor_addr;

  if (wbe_window_mapped(track_editor->ggt_win.pb_win.win_be) == WBE_FALSE)
    return;
  pbt_evh_handle(&(track_editor->ggt_win.evh), winev);
}

pbt_bool_t gadget_b1_pressed_cb(pbt_ggt_t *gadget,
                                wbe_window_input_t *winev,
                                void *unused)
{
  if (_PBT_IS_IN_GGT(gadget, winev->xpos, winev->ypos) &&
      WBE_GET_BIT(winev->buttons, 0) == 1)
    return PBT_TRUE;
  return PBT_FALSE;
}

void value_num_dec_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  if (track_editor->editor_ctx.default_velocity == 0)
    return;
  track_editor->editor_ctx.default_velocity -= 1;
  pbt_ggt_draw(&(track_editor->value_num_wgt));
  pbt_ggt_draw(&(track_editor->value_vbar_wgt));
}

void value_num_inc_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  if (track_editor->editor_ctx.default_velocity >= MAX_7b_VAL)
    {
      track_editor->editor_ctx.default_velocity = MAX_7b_VAL;
      return;
    }
  track_editor->editor_ctx.default_velocity += 1;
  pbt_ggt_draw(&(track_editor->value_num_wgt));
  pbt_ggt_draw(&(track_editor->value_vbar_wgt));
}

char *resolution_setting_get_default_str(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  return (char *) resolution_list[track_editor->editor_ctx.resolution_idx];
}

void resolution_setting_get_list(void *track_editor_addr,
                                 char ***list,
                                 size_t *list_len)
{
  *list = (char **) resolution_list;
  *list_len = RESOLUTION_LIST_LEN;
}

void resolution_setting_dialog_res_cb(size_t idx,
                                      void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;
  track_ctx_t *track_ctx = track_editor->editor_ctx.track_ctx;

  track_editor->editor_ctx.resolution_idx = idx;

  switch (idx)
    {
    case 0:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq * 4;
      break;
    case 1:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq * 2;
      break;
    case 2:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq;
      break;
    case 3:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 2;
      break;
    case 4:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 4;
      break;
    case 5:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 8;
      break;
    case 6:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 16;
      break;
    case 7:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 32;
      break;
    case 8:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 64;
      break;
    case 9:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 3;
      break;
    case 10:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 6;
      break;
    case 11:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 12;
      break;
    case 12:
      track_editor->editor_ctx.quantize = track_ctx->engine->ppq / 24;
      break;
    }
  msq_draw_vggts(&(track_editor->vggts));
}

char *output_cbb_get_default_str(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  if (track_editor->editor_ctx.track_ctx->output == NULL)
    return "No output";
  return (char *) output_get_name(track_editor->editor_ctx.track_ctx->output);
}

void output_cbb_get_list(void *track_editor_addr,
                         char ***list,
                         size_t *list_len)
{
  track_editor_t *track_editor = track_editor_addr;
  static char **static_list = NULL;
  static size_t len, idx;

  if (static_list != NULL)
    {
      for (idx = 0; idx < len; idx++)
        free(static_list[idx]);
      free(static_list);
    }

  static_list =
    engine_gen_output_str_list(track_editor->editor_ctx.track_ctx->engine,
                               &len);
  *list = static_list;
  *list_len = len;
}

void output_cbb_dialog_res_cb(size_t idx,
                              void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  if (idx == 0)
    track_editor->editor_ctx.track_ctx->output = NULL;
  else
    track_editor->editor_ctx.track_ctx->output =
      engine_get_output(track_editor->editor_ctx.track_ctx->engine,
                        idx - 1);
  _pbt_ggt_draw(track_editor->transport.track_node);
  pbt_wgt_win_put_buffer(&(track_editor->transport.parent->play.wgt));
}

char *channel_setting_get_default_str(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  return track_editor->editor_ctx.channel_list[track_editor->editor_ctx.channel];
}

msq_bool_t msq_channel_info_has_event(msq_channel_info_t *channel_info)
{
  size_t idx;

  for (idx = 0;
       idx < CHANNEL_USAGE_LEN;
       idx++)
    if (channel_info->usage[idx] != 0)
      return MSQ_TRUE;
  return MSQ_FALSE;
}

void channel_setting_get_list(void *track_editor_addr,
                              char ***list,
                              size_t *list_len)
{
  track_editor_t *track_editor = track_editor_addr;
  msq_channel_info_t *channel_info;
  size_t channel_num;

  msq_track_info_init(&(track_editor->editor_ctx.track_info),
                      track_editor->editor_ctx.track_ctx->track);

  *list = track_editor->editor_ctx.channel_list;
  *list_len = CHANNEL_LIST_LEN;

  for (channel_num = 0;
       channel_num < CHANNEL_LIST_LEN;
       channel_num++)
    {
      channel_info =
        &(track_editor->editor_ctx.track_info.channels_info[channel_num]);
      (*list)[channel_num][1] =
        msq_channel_info_has_event(channel_info) ==  MSQ_TRUE ? '*' : ' ';
    }
}

void channel_setting_dialog_res_cb(size_t idx,
                                   void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  _history_add_mark(&(track_editor->editor_ctx.history));
  _history_track_editor_set_channel(&(track_editor->editor_ctx), idx);
  msq_draw_vggts(&(track_editor->vggts));
}

char *value_type_get_default_str(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  return track_editor->editor_ctx.value_type_list[track_editor->value_wgt.type];
}

void value_type_get_list(void *track_editor_addr,
                         char ***list,
                         size_t *list_len)
{
  track_editor_t *track_editor = track_editor_addr;
  size_t value_type_num;

  msq_track_info_init(&(track_editor->editor_ctx.track_info),
                      track_editor->editor_ctx.track_ctx->track);

  *list = track_editor->editor_ctx.value_type_list;
  *list_len = VALUE_TYPE_LEN;

  for (value_type_num = 0;
       value_type_num < CHANNEL_LIST_LEN;
       value_type_num++)
    (*list)[value_type_num][1] =
      _msq_bytearray_get_bit(track_editor->editor_ctx.track_info.channels_info[track_editor->editor_ctx.channel].usage,
                             value_type_num) == WBE_TRUE
      ? '*' : ' ';
}

void value_type_dialog_res_cb(size_t idx,
                              void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  track_editor->value_wgt.type = idx;
  pbt_wgt_draw(&(track_editor->value_wgt));
  pbt_ggt_win_put_buffer(&(track_editor->ggt_win));
}

void msq_combobox_dialog_res_cb(size_t idx,
                                void *combobox_addr)
{
  msq_combobox_t *combobox = combobox_addr;

  combobox->cb(idx, combobox->cbb_addr);
  msq_combobox_draw_pixbufs(combobox);
}

void msq_combo_button_cb(void *combobox_addr)
{
  msq_combobox_t *combobox = combobox_addr;
  char **list = NULL;
  size_t list_len;

  combobox->combobox_get_list(combobox->cbb_addr,
                              &list,
                              &list_len);
  msq_dialog_list(combobox->dialog_iface,
                  list,
                  list_len,
                  msq_combobox_dialog_res_cb,
                  combobox);
}

#define MSQ_MAX_STRLEN 120

#include <stdio.h>
void msq_combobox_destroy_cb(pbt_ggt_t *ggt)
{
  msq_combobox_t *combobox = ggt->priv;
  pbt_ggt_t *button_ggt = ggt->childs->priv.ggt_addr;

  _pbt_ggt_destroy(button_ggt);

  pbt_pixbuf_destroy(&(combobox->pb_released));
  pbt_pixbuf_destroy(&(combobox->pb_pressed));
  pbt_pixbuf_destroy(&(combobox->pb_hovered));
}

void msq_combobox_init(msq_combobox_t *combobox,
                       msq_dialog_iface_t *dialog_iface,
                       msq_gui_theme_t *gui_theme,
                       msq_combobox_get_default_str_t combobox_get_default_str,
                       msq_combobox_get_list_t combobox_get_list,
                       msq_combobox_cb_t cb,
                       void *cbb_addr)
{
  size_t idx;
  unsigned int str_width, tmp_str_width;
  char max_str[MSQ_MAX_STRLEN];
  char **list = NULL;
  size_t list_len;

  combobox->dialog_iface = dialog_iface;
  combobox->gui_theme = gui_theme;
  combobox->combobox_get_default_str = combobox_get_default_str;
  combobox->combobox_get_list = combobox_get_list;
  combobox->cb = cb;
  combobox->cbb_addr = cbb_addr;

  combobox->combobox_get_list(combobox->cbb_addr,
                              &list,
                              &list_len);

  for (idx = 0, str_width = 0; idx < list_len; idx++)
    {
      snprintf(max_str, MSQ_MAX_STRLEN, "%s___", list[idx]);
      pbt_font_get_string_width(&(combobox->gui_theme->theme.font),
                                max_str,
                                &tmp_str_width);
      if (tmp_str_width > str_width)
        str_width = tmp_str_width;
    }
  pbt_pixbuf_init(&(combobox->pb_released),
                  str_width,
                  combobox->gui_theme->theme.font.max_height);
  pbt_pixbuf_init(&(combobox->pb_pressed),
                  str_width,
                  combobox->gui_theme->theme.font.max_height);
  pbt_pixbuf_init(&(combobox->pb_hovered),
                  str_width,
                  combobox->gui_theme->theme.font.max_height);

  msq_combobox_draw_pixbufs(combobox);

  pbt_wgt_button_init(&(combobox->button),
                      &(combobox->pb_released),
                      &(combobox->pb_pressed),
                      &(combobox->pb_hovered),
                      gui_theme->theme.wgt_normal_bg,
                      gui_theme->theme.wgt_activated_bg,
                      gui_theme->theme.wgt_hovered_bg,
                      gui_theme->theme.cursor_finger,
                      gui_theme->theme.cursor_arrow,
                      msq_combo_button_cb,
                      combobox);

  _pbt_ggt_setup_ggt_child_wrapper(&(combobox->ggt),
                                   &(combobox->button_node),
                                   WIDGET,
                                   &(combobox->button.wgt.ggt));
  combobox->ggt.priv = combobox;
  combobox->ggt.destroy_cb = msq_combobox_destroy_cb;
}

void msq_draw_empty(pbt_pbarea_t *pbarea, void *bg_addr)
{
  unsigned char *bg_color = bg_addr;

  pbt_pbarea_fill(pbarea, bg_color);
}

unsigned int _msq_zoom_scrollbar_get_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_scrollbar_t *scrollbar = wgt->priv;
  track_editor_t *track_editor = scrollbar->cb_arg;

  return track_editor->editor_ctx.theme->piano_width;
}

void handle_hadj_zoom_change(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  _msq_update_zoom(&(track_editor->editor_ctx), &(track_editor->vggts));
}

void msq_quantify_note_selection_cb(void *track_editor_addr)
{
  track_editor_t *track_editor = track_editor_addr;

  msq_quantify_note_selection(&(track_editor->grid_wgt));
}

unsigned char _msq_editor_get_first_channel(msq_track_info_t *track_info)
{
  size_t channel_num;
  msq_channel_info_t *channel_info;

  for (channel_num = 0;
       channel_num < CHANNEL_LIST_LEN;
       channel_num++)
    {
      channel_info = &(track_info->channels_info[channel_num]);
      if (msq_channel_info_has_event(channel_info) ==  MSQ_TRUE)
        return channel_num;
    }
  return 0;
}

void _msq_track_editor_setup_adj(track_editor_t *track_editor)
{
  unsigned int note_min, note_max;
  unsigned int min_vpos, max_vpos, size;

  note_min = track_editor->editor_ctx.track_info.channels_info[track_editor->editor_ctx.channel].note_min;
  note_max = track_editor->editor_ctx.track_info.channels_info[track_editor->editor_ctx.channel].note_max;

  if (note_min == 255 && note_max == 0)
    {
      note_min = 60;
      note_max = 84;
    }
  min_vpos = NOTE2YPOS(&(track_editor->editor_ctx), note_max);
  max_vpos = NOTE2YPOS(&(track_editor->editor_ctx), note_min);
  size = max_vpos - min_vpos;
  if (size <= track_editor->editor_ctx.vadj.size)
    track_editor->editor_ctx.vadj.pos =
      min_vpos - ((track_editor->editor_ctx.vadj.size - size) / 2);
  else
    track_editor->editor_ctx.vadj.pos =
      min_vpos + ((size - track_editor->editor_ctx.vadj.size) / 2);

  msq_update_hadj_startnlen(&(track_editor->editor_ctx));
}

#define TRACK_EDITOR_START_WIDTH  800
#define TRACK_EDITOR_START_HEIGHT 600

void track_editor_destroy(track_editor_t *track_editor)
{
  pbt_ggt_win_destroy(&(track_editor->ggt_win));
  _msq_free_str_list(track_editor->editor_ctx.value_type_list, VALUE_TYPE_LEN);
  _msq_free_str_list(track_editor->editor_ctx.channel_list, CHANNEL_LIST_LEN);
}

void track_editor_init(track_editor_t *track_editor,
                       track_editor_theme_t *theme,
                       msq_dialog_iface_t *dialog_iface,
                       msq_transport_iface_t *transport_iface,
                       pbt_ggt_t *track_node_ggt,
                       track_ctx_t *track_ctx,
                       unsigned int qn_size,
                       unsigned int note_height,
                       unsigned int quantize,
                       unsigned char note_scale)
{
  unsigned int vline_divisor;
  unsigned int min_width, min_height, max_width, max_height;

  track_editor->editor_ctx.value_type_list =
    _msq_str_list_copy(value_type_list, VALUE_TYPE_LEN);
  track_editor->editor_ctx.channel_list =
    _msq_str_list_copy(channel_list, CHANNEL_LIST_LEN);

  track_editor->editor_ctx.theme = theme;
  track_editor->dialog_iface = dialog_iface;
  track_editor->editor_ctx.channel = 0;

  track_editor->editor_ctx.default_velocity = 64;
  track_editor->editor_ctx.default_pitch = 0;
  track_editor->editor_ctx.default_ctrl_val = 64;

  track_editor->editor_ctx.note_height =
    track_editor->editor_ctx.theme->global_theme->theme.font.max_height + 2;

  track_editor->editor_ctx.track_ctx = track_ctx;
  track_editor->editor_ctx.qn_size = qn_size;
  track_editor->editor_ctx.quantize = quantize;
  track_editor->editor_ctx.note_scale = note_scale;

  msq_track_info_init(&(track_editor->editor_ctx.track_info),
                      track_ctx->track);

  track_editor->editor_ctx.channel =
    _msq_editor_get_first_channel(&(track_editor->editor_ctx.track_info));

  /* value type + line + value num + line */
  track_editor->value_min_height = track_editor->editor_ctx.note_height * 2 + 2;

  /* TODO "triolet" */
  track_editor->editor_ctx.vline_stride = 0;
  vline_divisor = 1;
  if ((qn_size > DIV_SIZE_MIN) && ((MSQGETPPQ(&(track_editor->editor_ctx)) % 2) == 0))
    {
      /* "Croche" */
      vline_divisor = vline_divisor << 1;

      if (((qn_size / vline_divisor) > DIV_SIZE_MIN) &&
          ((MSQGETPPQ(&(track_editor->editor_ctx)) % (vline_divisor << 1)) == 0))
        {
          /* "Double croche" */
          vline_divisor = vline_divisor << 1;

          if (((qn_size / vline_divisor) > DIV_SIZE_MIN) &&
              ((MSQGETPPQ(&(track_editor->editor_ctx)) % (vline_divisor << 1)) == 0))
            {
              /* "Triple croche" */
              vline_divisor = vline_divisor << 1;

              if (((qn_size / vline_divisor) > DIV_SIZE_MIN) &&
                  ((MSQGETPPQ(&(track_editor->editor_ctx)) % (vline_divisor << 1)) == 0))
                /* "Quadruple croche" (maximum) */
                vline_divisor = vline_divisor << 1;
            }
        }
    }
  if (vline_divisor != 1)
    track_editor->editor_ctx.vline_stride =
      MSQGETPPQ(&(track_editor->editor_ctx)) / vline_divisor;

  track_editor->hggts.piano   = &(track_editor->piano_wgt.wgt.ggt);
  track_editor->hggts.grid    = &(track_editor->grid_wgt.wgt.ggt);
  track_editor->hggts.vscroll = &(track_editor->vscrollbar_wgt.wgt.ggt);

  track_editor->vggts.timeline = &(track_editor->timeline_wgt.ggt);
  track_editor->vggts.grid     = &(track_editor->grid_wgt.wgt.ggt);
  track_editor->vggts.value    = &(track_editor->value_wgt.wgt.ggt);
  track_editor->vggts.hscroll  = &(track_editor->hscrollbar_wgt.wgt.ggt);
  track_editor->vggts.zoom     = &(track_editor->hscrollbar_zoom_wgt.wgt.ggt);
  track_editor->vggts.channel_cbbox = &(track_editor->channel_combobox);

  msq_transport_child_init(&(track_editor->transport),
                           transport_iface,
                           track_ctx,
                           track_node_ggt);
  pbt_ggt_hctnr_init(&(track_editor->hctnr_transport));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_transport),
                        &(track_editor->transport));
  pbt_ggt_ctnr_add_empty(&(track_editor->hctnr_transport),
                         tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_vctnr_init(&(track_editor->vctnr_transport));
  pbt_ggt_add_child_ggt(&(track_editor->vctnr_transport),
                        &(track_editor->hctnr_transport));
  _pbt_ggt_ctnr_add_separator(&(track_editor->vctnr_transport),
                              0,
                              0,
                              track_editor->editor_ctx.theme->piano_width,
                              track_editor->editor_ctx.theme->piano_width,
                              tctx_window_bg(&(track_editor->editor_ctx)));

  track_editor->editor_ctx.resolution_idx = 4;
  msq_combobox_init(&(track_editor->resolution_combobox),
                    track_editor->dialog_iface,
                    track_editor->editor_ctx.theme->global_theme,
                    resolution_setting_get_default_str,
                    resolution_setting_get_list,
                    resolution_setting_dialog_res_cb,
                    track_editor);
  pbt_wgt_label_button_init(&(track_editor->quantify_button),
                            " Q ",
                            &(track_editor->editor_ctx.theme->global_theme->theme),
                            msq_quantify_note_selection_cb,
                            track_editor);
  /* msq_combobox_draw_pixbufs(&(track_editor->resolution_combobox)); */
  msq_combobox_init(&(track_editor->output_combobox),
                    track_editor->dialog_iface,
                    track_editor->editor_ctx.theme->global_theme,
                    output_cbb_get_default_str,
                    output_cbb_get_list,
                    output_cbb_dialog_res_cb,
                    track_editor);
  msq_combobox_init(&(track_editor->channel_combobox),
                    track_editor->dialog_iface,
                    track_editor->editor_ctx.theme->global_theme,
                    channel_setting_get_default_str,
                    channel_setting_get_list,
                    channel_setting_dialog_res_cb,
                    track_editor);

  pbt_ggt_hctnr_init(&(track_editor->hctnr_header));
  pbt_ggt_add_child_ggt(&(track_editor->hctnr_header),
                        &(track_editor->vctnr_transport));
  /* pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_header), */
  /*                                   track_editor->editor_ctx.theme->piano_width */
  /*                                   + theme->global_theme->default_separator, */
  /*                                   tctx_window_bg(&(track_editor->editor_ctx))); */
  /* /\* pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_header), *\/ */
  /* /\*                                   theme->global_theme->default_separator, *\/ */
  /* /\*                                   tctx_window_bg(&(track_editor->editor_ctx))); *\/ */
  /* pbt_ggt_add_child_wgt(&(track_editor->hctnr_header), */
  /*                       &(track_editor->transport)); */
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_header),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_ggt(&(track_editor->hctnr_header),
                        &(track_editor->output_combobox));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_header),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_ggt(&(track_editor->hctnr_header),
                        &(track_editor->channel_combobox));
  pbt_ggt_ctnr_add_empty(&(track_editor->hctnr_header),
                         msq_theme_window_bg(theme->global_theme));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_header),
                        &(track_editor->quantify_button));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_header),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_ggt(&(track_editor->hctnr_header),
                        &(track_editor->resolution_combobox));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_header),
                                    theme->global_theme->default_separator
                                    + DEFAULT_GGTBAR_WIDTH,
                                    tctx_window_bg(&(track_editor->editor_ctx)));

  timeline_wgt_init(&(track_editor->timeline_wgt),
                    track_editor);
  pbt_ggt_hctnr_init(&(track_editor->hctnr_up));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_up),
                                    track_editor->editor_ctx.theme->piano_width,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_up),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  _pbt_ggt_add_child_wgt(&(track_editor->hctnr_up),
                         &(track_editor->timeline_wgt));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_up),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_up),
                                    DEFAULT_GGTBAR_WIDTH,
                                    tctx_window_bg(&(track_editor->editor_ctx)));

  piano_wgt_init(&(track_editor->piano_wgt),
                 &(track_editor->editor_ctx),
                 &(track_editor->hggts));
  grid_wgt_init(&(track_editor->grid_wgt),
                &(track_editor->editor_ctx),
                &(track_editor->hggts),
                &(track_editor->vggts));
  pbt_wgt_vscrollbar_init(&(track_editor->vscrollbar_wgt),
                          &(track_editor->editor_ctx.vadj),
                          DEFAULT_GGTBAR_WIDTH,
                          tctx_wgt_normal_fg(&(track_editor->editor_ctx)),
                          tctx_frame_bg(&(track_editor->editor_ctx)),
                          handle_vadj_change,
                          track_editor);
  pbt_ggt_hctnr_init(&(track_editor->hctnr_middle));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_middle),
                        &(track_editor->piano_wgt));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_middle),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_middle),
                        &(track_editor->grid_wgt));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_middle),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_middle),
                        &(track_editor->vscrollbar_wgt));

  pbt_wgt_hscrollbar_init(&(track_editor->hscrollbar_wgt),
                          &(track_editor->editor_ctx.hadj),
                          DEFAULT_GGTBAR_WIDTH,
                          tctx_wgt_normal_fg(&(track_editor->editor_ctx)),
                          tctx_frame_bg(&(track_editor->editor_ctx)),
                          handle_hadj_change,
                          track_editor);
  track_editor->editor_ctx.zoom_adj.max = 110;
  track_editor->editor_ctx.zoom_adj.size = 10;
  track_editor->editor_ctx.zoom_adj.pos = 50;
  pbt_wgt_hscrollbar_init(&(track_editor->hscrollbar_zoom_wgt),
                          &(track_editor->editor_ctx.zoom_adj),
                          DEFAULT_GGTBAR_WIDTH,
                          tctx_wgt_normal_fg(&(track_editor->editor_ctx)),
                          tctx_frame_bg(&(track_editor->editor_ctx)),
                          handle_hadj_zoom_change,
                          track_editor);
  track_editor->hscrollbar_zoom_wgt.wgt.ggt.get_min_width = _msq_zoom_scrollbar_get_width;
  track_editor->hscrollbar_zoom_wgt.wgt.ggt.get_max_width = _msq_zoom_scrollbar_get_width;
  pbt_ggt_hctnr_init(&(track_editor->hctnr_hscrollbar));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_hscrollbar),
                        &(track_editor->hscrollbar_zoom_wgt));
  /* pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_hscrollbar), */
  /*                                   track_editor->editor_ctx.theme->piano_width, */
  /*                                   tctx_window_bg(&(track_editor->editor_ctx))); */
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_hscrollbar),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_hscrollbar),
                        &(track_editor->hscrollbar_wgt));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_hscrollbar),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_hscrollbar),
                                    DEFAULT_GGTBAR_WIDTH,
                                    tctx_window_bg(&(track_editor->editor_ctx)));

  msq_combobox_init(&(track_editor->value_type_combobox),
                    track_editor->dialog_iface,
                    track_editor->editor_ctx.theme->global_theme,
                    value_type_get_default_str,
                    value_type_get_list,
                    value_type_dialog_res_cb,
                    track_editor);
  value_num_wgt_init(&(track_editor->value_num_wgt), track_editor);
  msq_button_minus_init(&(track_editor->value_num_dec),
                        track_editor->editor_ctx.theme->global_theme,
                        value_num_dec_cb,
                        track_editor);
  msq_button_plus_init(&(track_editor->value_num_inc),
                       track_editor->editor_ctx.theme->global_theme,
                       value_num_inc_cb,
                       track_editor);
  pbt_ggt_hctnr_init(&(track_editor->value_num_ctnr));
  pbt_ggt_add_child_wgt(&(track_editor->value_num_ctnr),
                        &(track_editor->value_num_dec));
  pbt_ggt_ctnr_add_line(&(track_editor->value_num_ctnr),
                        msq_theme_frame_bg(theme->global_theme));
  _pbt_ggt_add_child_wgt(&(track_editor->value_num_ctnr),
                         &(track_editor->value_num_wgt));
  pbt_ggt_ctnr_add_line(&(track_editor->value_num_ctnr),
                        msq_theme_frame_bg(theme->global_theme));
  pbt_ggt_add_child_wgt(&(track_editor->value_num_ctnr),
                        &(track_editor->value_num_inc));
  pbt_ggt_vctnr_init(&(track_editor->value_vctnr_ggt));
  pbt_ggt_add_child_ggt(&(track_editor->value_vctnr_ggt),
                        &(track_editor->value_type_combobox));
  pbt_ggt_ctnr_add_line(&(track_editor->value_vctnr_ggt),
                        msq_theme_frame_bg(theme->global_theme));
  pbt_ggt_add_child_ggt(&(track_editor->value_vctnr_ggt),
                        &(track_editor->value_num_ctnr));
  pbt_ggt_drawarea_init(&(track_editor->value_empty_ggt),
                        track_editor->editor_ctx.theme->piano_width,
                        track_editor->editor_ctx.theme->piano_width,
                        0,
                        0,
                        msq_draw_empty,
                        tctx_frame_bg(&(track_editor->editor_ctx)),
                        NULL, NULL);
  pbt_ggt_add_child_ggt(&(track_editor->value_vctnr_ggt),
                        &(track_editor->value_empty_ggt));
  /* pbt_ggt_ctnr_add_empty(&(track_editor->value_vctnr_ggt), */
  /*                        tctx_frame_bg(&(track_editor->editor_ctx))); */

  value_vbar_wgt_init(&(track_editor->value_vbar_wgt), track_editor);
  value_wgt_init(&(track_editor->value_wgt),
                 &(track_editor->editor_ctx),
                 &(track_editor->vggts));
  /* msq_value_set_type(&(track_editor->value_wgt), VALUE_NOTELEVEL_TYPE); */
  pbt_ggt_hctnr_init(&(track_editor->hctnr_bottom));
  pbt_ggt_add_child_ggt(&(track_editor->hctnr_bottom),
                        &(track_editor->value_vctnr_ggt));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_bottom),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_wgt(&(track_editor->hctnr_bottom),
                        &(track_editor->value_wgt));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->hctnr_bottom),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  _pbt_ggt_add_child_wgt(&(track_editor->hctnr_bottom),
                         &(track_editor->value_vbar_wgt));

  pbt_ggt_vctnr_init(&(track_editor->vctnr1));
  pbt_ggt_add_child_ggt(&(track_editor->vctnr1), &(track_editor->hctnr_header));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->vctnr1),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_ggt(&(track_editor->vctnr1), &(track_editor->hctnr_up));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->vctnr1),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_ggt(&(track_editor->vctnr1),
                        &(track_editor->hctnr_middle));

  pbt_ggt_vctnr_init(&(track_editor->vctnr2));
  pbt_ggt_add_child_ggt(&(track_editor->vctnr2),
                        &(track_editor->hctnr_bottom));
  pbt_ggt_ctnr_add_static_separator(&(track_editor->vctnr2),
                                    theme->global_theme->default_separator,
                                    tctx_window_bg(&(track_editor->editor_ctx)));
  pbt_ggt_add_child_ggt(&(track_editor->vctnr2),
                        &(track_editor->hctnr_hscrollbar));

  pbt_wgt_vsplitted_area_init_gg(&(track_editor->splitted_area),
                                 &(track_editor->vctnr1),
                                 &(track_editor->vctnr2),
                                 100,
                                 tctx_window_bg(&(track_editor->editor_ctx)),
                                 tctx_cursor_grab(&(track_editor->editor_ctx)),
                                 tctx_cursor_grabbing(&(track_editor->editor_ctx)),
                                 tctx_cursor_arrow(&(track_editor->editor_ctx)));

  unsigned int win_margin_size = theme->global_theme->theme.font.max_height / 2;
  msq_margin_ggt_init_w(&(track_editor->margin),
                        &(track_editor->splitted_area.wgt),
                        win_margin_size,
                        win_margin_size,
                        win_margin_size,
                        win_margin_size,
                        tctx_window_bg(&(track_editor->editor_ctx)));

  pbt_ggt_win_init(&(track_editor->ggt_win),
                   track_editor->editor_ctx.track_ctx->track->name != NULL
                   ? track_editor->editor_ctx.track_ctx->track->name
                   : "no name",
                   &(track_editor->margin),
                   TRACK_EDITOR_START_WIDTH,
                   TRACK_EDITOR_START_HEIGHT,
                   PBT_TRUE);

  wbe_window_add_input_cb(track_editor->ggt_win.pb_win.win_be,
                          window_input_cb,
                          (void *) track_editor);

  min_width  = pbt_ggt_min_width(&(track_editor->margin));
  min_height = pbt_ggt_min_height(&(track_editor->margin));
  max_width  = pbt_ggt_max_width(&(track_editor->margin));
  max_height = pbt_ggt_max_height(&(track_editor->margin));

  wbe_window_size_limits(track_editor->ggt_win.pb_win.win_be,
                         min_width,
                         min_height,
                         max_width  == 0 ? WBE_SIZE_DONT_CARE : max_width,
                         max_height == 0 ? WBE_SIZE_DONT_CARE : max_height);

  wbe_window_set_cursor(track_editor->ggt_win.pb_win.win_be,
                        tctx_cursor_arrow(&(track_editor->editor_ctx)));

  _msq_track_editor_setup_adj(track_editor);

  _pbt_ggt_draw(track_editor->ggt_win.ggt);
}
