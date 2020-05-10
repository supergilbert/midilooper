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

#include "pbt_default_font.h"

#include "msq_gui_inc.h"
#include "pbt_font.h"
#include "pbt_draw.h"
#include "pbt_event_handler.h"
#include "pbt_window_gadget.h"
#include "pbt_gadget.h"
#include "wbe_glfw.h"

gradient_value_t default_gradient_value_list[] =
  {
   {8192,  {0x0,  0x80, 0x0,  0xFF}},
   {12288, {0x80, 0x80, 0x0,  0xFF}},
   {16384, {0xD0, 0x0,  0x0,  0xFF}}
  };

#define default_gradient_len 3

void msq_init_play_button_img(pbt_pixbuf_t *img,
                              unsigned int size,
                              unsigned char *fg_color,
                              unsigned char *bg_color)
{
  unsigned int height = (2 * size) / 3,
    ypos = size / 6,
    xpos = size / 3;

  pbt_pixbuf_init(img, size, size);
  pbt_pixbuf_fill(img, bg_color);
  pbt_pixbuf_draw_triangle_right(img,
                                 xpos, ypos,
                                 height,
                                 fg_color);
}

void msq_init_pause_button_img(pbt_pixbuf_t *img,
                               unsigned int size,
                               unsigned char *fg_color,
                               unsigned char *bg_color)
{
  unsigned int square_size = (2 * size) / 3,
    width = square_size / 3,
    xpos1 = size / 6,
    xpos2 = xpos1 + (2 * width);

  pbt_pixbuf_init(img, size, size);
  pbt_pixbuf_fill(img, bg_color);
  pbt_pixbuf_fillrect(img,
                      xpos1, xpos1,
                      width, square_size,
                      fg_color);
  pbt_pixbuf_fillrect(img,
                      xpos2, xpos1,
                      width, square_size,
                      fg_color);
}

void msq_init_stop_button_img(pbt_pixbuf_t *img,
                              unsigned int size,
                              unsigned char *fg_color,
                              unsigned char *bg_color)
{
  unsigned int square_size = (2 * size) / 3,
    square_pos = size / 6;
  pbt_pixbuf_init(img, size, size);
  pbt_pixbuf_fill(img, bg_color);
  pbt_pixbuf_fillrect(img,
                      square_pos, square_pos,
                      square_size, square_size,
                      fg_color);
}

void msq_init_rec_button_img(pbt_pixbuf_t *img,
                             unsigned int size,
                             unsigned char *fg_color,
                             unsigned char *bg_color)
{
  unsigned int square_size = (2 * size) / 3,
    square_pos = size / 6;

  pbt_pixbuf_init(img, size, size);
  pbt_pixbuf_fill(img, bg_color);
  pbt_pixbuf_draw_disc(img,
                       square_pos, square_pos,
                       square_size,
                       fg_color);
}

void msq_init_plus_button_img(pbt_pixbuf_t *img,
                              unsigned int size,
                              unsigned char *fg_color,
                              unsigned char *bg_color)
{
  unsigned int plus_size = (2 * size) / 3,
    plus_pos = size / 6;

  pbt_pixbuf_init(img, size, size);
  pbt_pixbuf_fill(img, bg_color);
  pbt_pixbuf_draw_plus(img,
                       plus_pos, plus_pos,
                       plus_size,
                       fg_color);
}

void msq_init_minus_button_img(pbt_pixbuf_t *img,
                               unsigned int size,
                               unsigned char *fg_color,
                               unsigned char *bg_color)
{
  unsigned int minus_size = (2 * size) / 3,
    minus_pos = size / 6;

  pbt_pixbuf_init(img, size, size);
  pbt_pixbuf_fill(img, bg_color);
  pbt_pixbuf_draw_minus(img,
                        minus_pos, minus_pos,
                        minus_size,
                        fg_color);
}

void msq_init_track_hp_img(pbt_pixbuf_t *pixbuf,
                           unsigned int size,
                           unsigned char *border_color,
                           unsigned char *bg_color,
                           unsigned char *fg_color)
{
  unsigned int tmp;

  pbt_pixbuf_init(pixbuf, size, size);
  pbt_pixbuf_fill(pixbuf, border_color);
  tmp = pixbuf->width >> 3;
  pbt_pixbuf_fillrect(pixbuf,
                      tmp, tmp,
                      pixbuf->width - (tmp << 1),
                      pixbuf->width - (tmp << 1),
                      bg_color);
  pbt_pixbuf_draw_hp(pixbuf,
                     (pixbuf->width >> 2) + 1,
                     (pixbuf->width >> 2) + 1,
                     pixbuf->width >> 1,
                     fg_color);
}

void msq_init_track_mute_img(pbt_pixbuf_t *pixbuf,
                             unsigned int size,
                             unsigned char *border_color,
                             unsigned char *bg_color,
                             unsigned char *fg_color)
{
  unsigned int tmp;

  pbt_pixbuf_init(pixbuf, size, size);
  pbt_pixbuf_fill(pixbuf, border_color);
  tmp = pixbuf->width >> 3;
  pbt_pixbuf_fillrect(pixbuf,
                      tmp, tmp,
                      pixbuf->width - (tmp << 1),
                      pixbuf->width - (tmp << 1),
                      bg_color);
  pbt_pixbuf_draw_M(pixbuf,
                    (pixbuf->width >> 2) + 1,
                    (pixbuf->width >> 2) + 1,
                    pixbuf->width >> 1,
                    fg_color);
}

static unsigned char _default_play_color[4] = {0x0,
                                                    0x90,
                                                    0x0,
                                                    0xFF};

static unsigned char _default_rec_color[4] = {0xFF,
                                              0x0,
                                              0x0,
                                              0xFF};

static unsigned char _default_highlight_color[4] = {0x0,
                                                    0xFF,
                                                    0x0,
                                                    0xFF};

void gui_default_theme_destroy(msq_gui_theme_t *global_theme)
{
  pbt_wgt_default_theme_cursor_destroy(&(global_theme->theme));

  pbt_unload_font(&(global_theme->theme.font));

  pbt_pixbuf_destroy(&(global_theme->play_button_imgs[0]));
  pbt_pixbuf_destroy(&(global_theme->play_button_imgs[1]));
  pbt_pixbuf_destroy(&(global_theme->play_button_imgs[2]));
  pbt_pixbuf_destroy(&(global_theme->play_button_imgs[3]));
  pbt_pixbuf_destroy(&(global_theme->play_button_imgs[4]));
  pbt_pixbuf_destroy(&(global_theme->play_button_imgs[5]));

  pbt_pixbuf_destroy(&(global_theme->stop_button_imgs[0]));
  pbt_pixbuf_destroy(&(global_theme->stop_button_imgs[1]));
  pbt_pixbuf_destroy(&(global_theme->stop_button_imgs[2]));

  pbt_pixbuf_destroy(&(global_theme->rec_button_imgs[0]));
  pbt_pixbuf_destroy(&(global_theme->rec_button_imgs[1]));
  pbt_pixbuf_destroy(&(global_theme->rec_button_imgs[2]));
  pbt_pixbuf_destroy(&(global_theme->rec_button_imgs[3]));
  pbt_pixbuf_destroy(&(global_theme->rec_button_imgs[4]));
  pbt_pixbuf_destroy(&(global_theme->rec_button_imgs[5]));

  pbt_pixbuf_destroy(&(global_theme->plus_button_imgs[0]));
  pbt_pixbuf_destroy(&(global_theme->plus_button_imgs[1]));
  pbt_pixbuf_destroy(&(global_theme->plus_button_imgs[2]));

  pbt_pixbuf_destroy(&(global_theme->minus_button_imgs[0]));
  pbt_pixbuf_destroy(&(global_theme->minus_button_imgs[1]));
  pbt_pixbuf_destroy(&(global_theme->minus_button_imgs[2]));

  pbt_pixbuf_destroy(&(global_theme->track_mute_imgs[0]));
  pbt_pixbuf_destroy(&(global_theme->track_mute_imgs[1]));
  pbt_pixbuf_destroy(&(global_theme->track_mute_imgs[2]));
  pbt_pixbuf_destroy(&(global_theme->track_mute_imgs[3]));
}

#include "pbt_tools.h"

void gui_default_theme_init(msq_gui_theme_t *global_theme)
{
  pbt_wgt_set_default_theme_color(&(global_theme->theme));
  pbt_wgt_default_theme_cursor_init(&(global_theme->theme));
  if (pbt_load_memory_font(&(global_theme->theme.font),
                           _get_pbt_default_font_ptr(),
                           _get_pbt_default_font_len(),
                           DEFAULT_FONT_SIZE,
                           DEFAULT_FONT_SIZE) == PBT_FALSE)
    pbt_abort("Unable to load memory font");

  global_theme->play_color = _default_play_color;
  global_theme->rec_color = _default_rec_color;
  global_theme->highlight_color = _default_highlight_color;

  global_theme->default_margin = global_theme->theme.font.max_height / 2;
  global_theme->default_separator = global_theme->default_margin / 2;

  msq_init_play_button_img(&(global_theme->play_button_imgs[0]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_normal_fg(global_theme),
                           msq_theme_wgt_normal_bg(global_theme));
  msq_init_play_button_img(&(global_theme->play_button_imgs[1]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_activated_fg(global_theme),
                           msq_theme_wgt_activated_bg(global_theme));
  msq_init_play_button_img(&(global_theme->play_button_imgs[2]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_hovered_fg(global_theme),
                           msq_theme_wgt_hovered_bg(global_theme));
  msq_init_pause_button_img(&(global_theme->play_button_imgs[3]),
                            global_theme->theme.font.max_height,
                            msq_theme_wgt_normal_fg(global_theme),
                            msq_theme_wgt_normal_bg(global_theme));
  msq_init_pause_button_img(&(global_theme->play_button_imgs[4]),
                            global_theme->theme.font.max_height,
                            msq_theme_wgt_activated_fg(global_theme),
                            msq_theme_wgt_activated_bg(global_theme));
  msq_init_pause_button_img(&(global_theme->play_button_imgs[5]),
                            global_theme->theme.font.max_height,
                            msq_theme_wgt_hovered_fg(global_theme),
                            msq_theme_wgt_hovered_bg(global_theme));

  msq_init_stop_button_img(&(global_theme->stop_button_imgs[0]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_normal_fg(global_theme),
                           msq_theme_wgt_normal_bg(global_theme));
  msq_init_stop_button_img(&(global_theme->stop_button_imgs[1]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_activated_fg(global_theme),
                           msq_theme_wgt_activated_bg(global_theme));
  msq_init_stop_button_img(&(global_theme->stop_button_imgs[2]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_hovered_fg(global_theme),
                           msq_theme_wgt_hovered_bg(global_theme));

  msq_init_rec_button_img(&(global_theme->rec_button_imgs[0]),
                          global_theme->theme.font.max_height,
                          msq_theme_wgt_normal_fg(global_theme),
                          msq_theme_wgt_normal_bg(global_theme));
  msq_init_rec_button_img(&(global_theme->rec_button_imgs[1]),
                          global_theme->theme.font.max_height,
                          msq_theme_wgt_activated_fg(global_theme),
                          msq_theme_wgt_activated_bg(global_theme));
  msq_init_rec_button_img(&(global_theme->rec_button_imgs[2]),
                          global_theme->theme.font.max_height,
                          msq_theme_wgt_hovered_fg(global_theme),
                          msq_theme_wgt_hovered_bg(global_theme));
  msq_init_rec_button_img(&(global_theme->rec_button_imgs[3]),
                          global_theme->theme.font.max_height,
                          global_theme->rec_color,
                          msq_theme_wgt_normal_bg(global_theme));
  msq_init_rec_button_img(&(global_theme->rec_button_imgs[4]),
                          global_theme->theme.font.max_height,
                          global_theme->rec_color,
                          msq_theme_wgt_activated_bg(global_theme));
  msq_init_rec_button_img(&(global_theme->rec_button_imgs[5]),
                          global_theme->theme.font.max_height,
                          global_theme->rec_color,
                          msq_theme_wgt_hovered_bg(global_theme));

  msq_init_plus_button_img(&(global_theme->plus_button_imgs[0]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_normal_fg(global_theme),
                           msq_theme_wgt_normal_bg(global_theme));
  msq_init_plus_button_img(&(global_theme->plus_button_imgs[1]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_activated_fg(global_theme),
                           msq_theme_wgt_activated_bg(global_theme));
  msq_init_plus_button_img(&(global_theme->plus_button_imgs[2]),
                           global_theme->theme.font.max_height,
                           msq_theme_wgt_hovered_fg(global_theme),
                           msq_theme_wgt_hovered_bg(global_theme));

  msq_init_minus_button_img(&(global_theme->minus_button_imgs[0]),
                            global_theme->theme.font.max_height,
                            msq_theme_wgt_normal_fg(global_theme),
                            msq_theme_wgt_normal_bg(global_theme));
  msq_init_minus_button_img(&(global_theme->minus_button_imgs[1]),
                            global_theme->theme.font.max_height,
                            msq_theme_wgt_activated_fg(global_theme),
                            msq_theme_wgt_activated_bg(global_theme));
  msq_init_minus_button_img(&(global_theme->minus_button_imgs[2]),
                            global_theme->theme.font.max_height,
                            msq_theme_wgt_hovered_fg(global_theme),
                            msq_theme_wgt_hovered_bg(global_theme));

  msq_init_track_hp_img(&(global_theme->track_mute_imgs[0]),
                        global_theme->theme.font.max_height * 4,
                        global_theme->play_color,
                        msq_theme_wgt_activated_bg(global_theme),
                        global_theme->play_color);
  msq_init_track_mute_img(&(global_theme->track_mute_imgs[1]),
                          global_theme->theme.font.max_height * 4,
                          msq_theme_wgt_activated_fg(global_theme),
                          msq_theme_wgt_activated_bg(global_theme),
                          msq_theme_wgt_activated_fg(global_theme));
  msq_init_track_hp_img(&(global_theme->track_mute_imgs[2]),
                        global_theme->theme.font.max_height * 4,
                        msq_theme_wgt_activated_fg(global_theme),
                        msq_theme_wgt_activated_bg(global_theme),
                        global_theme->play_color);
  msq_init_track_mute_img(&(global_theme->track_mute_imgs[3]),
                          global_theme->theme.font.max_height * 4,
                          global_theme->play_color,
                          msq_theme_wgt_activated_bg(global_theme),
                          msq_theme_wgt_activated_fg(global_theme));
}

void transport_gen_imgs_cb(pbt_pixbuf_t **imgs,
                           void *imgs_addr,
                           unsigned int width,
                           unsigned int height)
{
  *imgs = imgs_addr;
}

void handle_play_cb(void *transport_iface_addr)
{
  msq_transport_iface_t *transport_iface = transport_iface_addr;

  engine_start(transport_iface->engine_ctx);
}

void handle_stop_cb(void *transport_iface_addr)
{
  msq_transport_iface_t *transport_iface = transport_iface_addr;

  engine_stop(transport_iface->engine_ctx);
}

void msq_transport_update_button(msq_transport_iface_t *transport_iface)
{
  list_iterator_t iter;
  msq_transport_child_t *transport_child;

  for (iter_init(&iter, &(transport_iface->transport_childs));
       iter_node(&iter);
       iter_next(&iter))
    {
      transport_child = iter_node_ptr(&iter);
      pbt_wgt_draw(&(transport_child->rec));
      pbt_wgt_win_put_buffer(&(transport_child->rec.wgt));
    }
}

void handle_rec_cb(void *transport_child_addr)
{
  msq_transport_child_t *transport_child = transport_child_addr;
  track_ctx_t *track_ctx = transport_child->track_ctx;

  if (track_ctx->engine->rec == MSQ_TRUE)
    {
      if (track_ctx->engine->track_rec == track_ctx)
        track_ctx->engine->rec = MSQ_FALSE;
      else
        track_ctx->engine->track_rec = track_ctx;
    }
  else
    {
      track_ctx->engine->track_rec = track_ctx;
      track_ctx->engine->rec = MSQ_TRUE;
    }
  msq_transport_update_button(transport_child->parent);
}

void msq_destroy_transport_button(pbt_ggt_t *ggt)
{
  msq_transport_iface_t *transport_iface = ggt->priv;

  pbt_ggt_destroy(&(transport_iface->root_ctnr));
}

unsigned int msq_transport_tempo_get_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_transport_tempo_t *transport_tempo = wgt->priv;

  return transport_tempo->width;
}

unsigned int msq_transport_tempo_get_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_transport_tempo_t *transport_tempo = wgt->priv;

  return transport_tempo->global_theme->theme.font.max_height;
}

void msq_transport_tempo_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  msq_transport_tempo_t *transport_tempo = wgt->priv;

  pbt_pbarea_fill(&(ggt->pbarea),
                  transport_tempo->global_theme->theme.frame_bg);
  pbt_pbarea_printf(&(ggt->pbarea),
                    &(transport_tempo->global_theme->theme.font),
                    transport_tempo->global_theme->theme.frame_fg,
                    0, 0,
                    "BPM % 3u",
                    60000000 / transport_tempo->engine_ctx->tempo);
}

pbt_bool_t msq_transport_tempo_set_focus_cb(pbt_ggt_t *ggt,
                                            wbe_window_input_t *winev,
                                            void *track_node_addr)
{
  return PBT_FALSE;
}

void msq_transport_tempo_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           msq_transport_tempo_set_focus_cb,
                           NULL);
}

void msq_transport_tempo_init(msq_transport_tempo_t *transport_tempo,
                              engine_ctx_t *engine_ctx,
                              msq_gui_theme_t *global_theme)
{
  pbt_ggt_t *ggt = &(transport_tempo->wgt.ggt);

  transport_tempo->engine_ctx = engine_ctx;
  transport_tempo->global_theme = global_theme;
  pbt_font_get_string_width(&(global_theme->theme.font),
                            "BPM XXX",
                            &(transport_tempo->width));
  transport_tempo->wgt.priv = transport_tempo;

  ggt->priv = &(transport_tempo->wgt);
  ggt->get_min_width = msq_transport_tempo_get_width;
  ggt->get_max_width = msq_transport_tempo_get_width;
  ggt->get_min_height = msq_transport_tempo_get_height;
  ggt->get_max_height = msq_transport_tempo_get_height;
  ggt->update_area_cb = pbt_ggt_memcpy_area;;
  ggt->draw_cb = msq_transport_tempo_draw_cb;
  ggt->destroy_cb = pbt_wgt_evnode_destroy;

  transport_tempo->wgt.init_ev_cb = msq_transport_tempo_init_ev;
}

void msq_transport_tempo_dec(void *transport_tempo_addr)
{
  msq_transport_tempo_t *transport_tempo = transport_tempo_addr;
  unsigned int tmp = 60000000 / transport_tempo->engine_ctx->tempo;

  tmp = 60000000 / (tmp - 1);

  if (tmp > 1500000)
    tmp = 1500000;
  engine_set_tempo(transport_tempo->engine_ctx, tmp);
  pbt_wgt_draw(transport_tempo);
}

void msq_transport_tempo_inc(void *transport_tempo_addr)
{
  msq_transport_tempo_t *transport_tempo = transport_tempo_addr;
  unsigned int tmp = 60000000 / transport_tempo->engine_ctx->tempo;

  tmp = 60000000 / (tmp + 1);

  if (tmp < 288461)
    tmp = 288461;
  engine_set_tempo(transport_tempo->engine_ctx, tmp);
  pbt_wgt_draw(transport_tempo);
}


pbt_bool_t msq_transport_iface_set_focus_cb(pbt_ggt_t *ggt,
                                            wbe_window_input_t *winev,
                                            void *transport_iface_addr)
{
  msq_transport_iface_t *transport_iface = transport_iface_addr;
  static msq_bool_t key_pressed = MSQ_FALSE;

  if ((key_pressed == MSQ_FALSE)
      && (wbe_key_pressed(winev->keys, WBE_KEY_SPACE) == 1))
    {
      if (engine_is_running(transport_iface->engine_ctx) == MSQ_TRUE)
        {
          transport_iface->engine_ctx->rec = MSQ_FALSE;
          msq_transport_update_button(transport_iface);
        }
      engine_start(transport_iface->engine_ctx);
      key_pressed = MSQ_TRUE;
    }
  else if (wbe_key_pressed(winev->keys, WBE_KEY_SPACE) == 0)
    key_pressed = MSQ_FALSE;

  return PBT_FALSE;
}

void msq_transport_iface_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           msq_transport_iface_set_focus_cb,
                           wgt->priv);
}

void msq_transport_destroy(pbt_ggt_t *ggt)
{
  pbt_ggt_t *ctnr_ggt = ggt->childs->priv.ggt_addr;

  pbt_wgt_evnode_destroy(ggt);
  _pbt_ggt_destroy(ctnr_ggt);
}

void msq_transport_init(msq_transport_iface_t *transport_iface,
                        msq_gui_theme_t *global_theme,
                        engine_ctx_t *engine_ctx)
{
  msq_button_init(&(transport_iface->play),
                  &(global_theme->play_button_imgs[0]),
                  &(global_theme->play_button_imgs[1]),
                  &(global_theme->play_button_imgs[2]),
                  global_theme,
                  handle_play_cb,
                  transport_iface);
  msq_button_init(&(transport_iface->stop),
                  &(global_theme->stop_button_imgs[0]),
                  &(global_theme->stop_button_imgs[1]),
                  &(global_theme->stop_button_imgs[2]),
                  global_theme,
                  handle_stop_cb,
                  transport_iface);
  msq_transport_tempo_init(&(transport_iface->tempo_wgt),
                           engine_ctx,
                           global_theme);
  msq_button_minus_init(&(transport_iface->tempo_dec),
                        global_theme,
                        msq_transport_tempo_dec,
                        &(transport_iface->tempo_wgt));
  msq_button_plus_init(&(transport_iface->tempo_inc),
                       global_theme,
                       msq_transport_tempo_inc,
                       &(transport_iface->tempo_wgt));
  pbt_ggt_hctnr_init(&(transport_iface->root_ctnr));
  pbt_ggt_add_child_wgt(&(transport_iface->root_ctnr),
                        &(transport_iface->play));
  pbt_ggt_ctnr_add_static_separator(&(transport_iface->root_ctnr),
                                    global_theme->default_separator,
                                    msq_theme_window_bg(global_theme));
  pbt_ggt_add_child_wgt(&(transport_iface->root_ctnr),
                        &(transport_iface->stop));
  pbt_ggt_ctnr_add_static_separator(&(transport_iface->root_ctnr),
                                    global_theme->default_separator,
                                    msq_theme_window_bg(global_theme));
  pbt_ggt_ctnr_add_separator(&(transport_iface->root_ctnr),
                             global_theme->default_separator,
                             0,
                             msq_theme_window_bg(global_theme));
  pbt_ggt_add_child_wgt(&(transport_iface->root_ctnr),
                        &(transport_iface->tempo_dec));
  pbt_ggt_ctnr_add_static_separator(&(transport_iface->root_ctnr),
                                    global_theme->default_separator,
                                    msq_theme_window_bg(global_theme));
  pbt_ggt_add_child_wgt(&(transport_iface->root_ctnr),
                        &(transport_iface->tempo_wgt));
  pbt_ggt_ctnr_add_static_separator(&(transport_iface->root_ctnr),
                                    global_theme->default_separator,
                                    msq_theme_window_bg(global_theme));
  pbt_ggt_add_child_wgt(&(transport_iface->root_ctnr),
                        &(transport_iface->tempo_inc));
  transport_iface->engine_ctx = engine_ctx;

  _pbt_ggt_setup_ggt_child_wrapper(&(transport_iface->wgt.ggt),
                                   &(transport_iface->child),
                                   GADGET,
                                   &(transport_iface->root_ctnr.ggt));
  transport_iface->wgt.ggt.childs = &(transport_iface->child);
  transport_iface->wgt.ggt.priv = &(transport_iface->wgt);
  transport_iface->wgt.priv = transport_iface;
  transport_iface->wgt.ggt.destroy_cb = msq_transport_destroy;

  transport_iface->wgt.init_ev_cb = msq_transport_iface_init_ev;
}

void msq_transport_child_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  msq_transport_child_t *transport_child = wgt->priv;

  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           msq_transport_iface_set_focus_cb,
                           transport_child->parent);
}

void msq_transport_child_destroy(pbt_ggt_t *ggt)
{
  list_iterator_t iter = {};
  pbt_wgt_t *wgt = ggt->priv;
  msq_transport_child_t *transport_child = wgt->priv;
  pbt_ggt_t *ctnr_ggt = ggt->childs->priv.ggt_addr;
  void *addr;

  pbt_wgt_evnode_destroy(ggt);
  for (iter_init(&iter, &(transport_child->parent->transport_childs));
       iter_node(&iter) != NULL;
       iter_next(&iter))
    {
      addr = iter_node_ptr(&iter);
      if (addr == transport_child)
        {
          iter_node_del(&iter, NULL);
          break;
        }
    }
  _pbt_ggt_destroy(ctnr_ggt);
}

void _msq_child_button_rec_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  pbt_wgt_button_t *rec_button = (pbt_wgt_button_t *) wgt->priv;
  msq_transport_child_t *transport_child
    = (msq_transport_child_t *) rec_button->cb_arg;
  track_ctx_t *track_ctx = transport_child->track_ctx;
  pbt_pixbuf_t *rec_button_imgs
    = transport_child->parent->tempo_wgt.global_theme->rec_button_imgs;

  if (track_ctx->engine->rec == MSQ_TRUE
      && track_ctx->engine->track_rec == track_ctx)
    {
      if (rec_button->pb_released != &(rec_button_imgs[3]))
        {
          rec_button->pb_released = &(rec_button_imgs[3]);
          rec_button->pb_pressed = &(rec_button_imgs[4]);
          rec_button->pb_hovered = &(rec_button_imgs[5]);
        }
    }
  else
    {
      if (rec_button->pb_released != &(rec_button_imgs[0]))
        {
          rec_button->pb_released = &(rec_button_imgs[0]);
          rec_button->pb_pressed = &(rec_button_imgs[1]);
          rec_button->pb_hovered = &(rec_button_imgs[2]);
        }
    }

  pbt_wgt_button_draw_cb(ggt);
}

void msq_transport_child_init(msq_transport_child_t *transport_child,
                              msq_transport_iface_t *transport_iface,
                              track_ctx_t *track_ctx)
{
  msq_gui_theme_t *global_theme = transport_iface->tempo_wgt.global_theme;

  transport_child->parent = transport_iface;
  transport_child->track_ctx = track_ctx;
  push_to_list(&(transport_iface->transport_childs), transport_child);

  msq_button_init(&(transport_child->play),
                  &(global_theme->play_button_imgs[0]),
                  &(global_theme->play_button_imgs[1]),
                  &(global_theme->play_button_imgs[2]),
                  global_theme,
                  handle_play_cb,
                  transport_iface);
  msq_button_init(&(transport_child->stop),
                  &(global_theme->stop_button_imgs[0]),
                  &(global_theme->stop_button_imgs[1]),
                  &(global_theme->stop_button_imgs[2]),
                  global_theme,
                  handle_stop_cb,
                  transport_iface);
  msq_button_init(&(transport_child->rec),
                  &(global_theme->rec_button_imgs[0]),
                  &(global_theme->rec_button_imgs[1]),
                  &(global_theme->rec_button_imgs[2]),
                  global_theme,
                  handle_rec_cb,
                  transport_child);
  transport_child->rec.wgt.ggt.draw_cb = _msq_child_button_rec_draw_cb;
  pbt_ggt_hctnr_init(&(transport_child->root_ctnr));
  pbt_ggt_add_child_wgt(&(transport_child->root_ctnr),
                        &(transport_child->play));
  pbt_ggt_ctnr_add_static_separator(&(transport_child->root_ctnr),
                                    global_theme->default_separator,
                                    msq_theme_window_bg(global_theme));
  pbt_ggt_add_child_wgt(&(transport_child->root_ctnr),
                        &(transport_child->stop));
  pbt_ggt_ctnr_add_static_separator(&(transport_child->root_ctnr),
                                    global_theme->default_separator,
                                    msq_theme_window_bg(global_theme));
  pbt_ggt_add_child_wgt(&(transport_child->root_ctnr),
                        &(transport_child->rec));

  _pbt_ggt_setup_ggt_child_wrapper(&(transport_child->wgt.ggt),
                                   &(transport_child->child),
                                   GADGET,
                                   &(transport_child->root_ctnr.ggt));
  transport_child->wgt.ggt.childs = &(transport_child->child);
  transport_child->wgt.ggt.priv = &(transport_child->wgt);
  transport_child->wgt.priv = transport_child;
  transport_child->wgt.ggt.destroy_cb = msq_transport_child_destroy;

  transport_child->wgt.init_ev_cb = msq_transport_child_init_ev;
}

void track_editor_default_theme_init(track_editor_theme_t *theme,
                                     msq_gui_theme_t *global_theme)
{
  theme->global_theme = global_theme;

  theme->smooth_line_color = msq_theme_frame_fg(global_theme);
  theme->piano_white_color = msq_theme_frame_bg(global_theme);
  theme->piano_black_color = msq_theme_wgt_normal_bg(global_theme);
  theme->selection_color = global_theme->highlight_color;

  theme->gradient_value_list = default_gradient_value_list;
  theme->gradient_len = default_gradient_len;

  if (pbt_font_get_string_width(&(theme->global_theme->theme.font),
                                "_000_#C_-0_______",
                                &(theme->piano_width)) == PBT_FALSE)
    theme->piano_width = theme->global_theme->theme.font.max_width * 17;

  theme->timeline_height = theme->global_theme->theme.font.max_height * 3;
}

char **_msq_str_list_copy(char **src_list, size_t list_len)
{
  size_t idx;
  char **dst_list = malloc(sizeof (char *) * list_len);

  for (idx = 0; idx < list_len; idx++)
    dst_list[idx] = strdup(src_list[idx]);
  return dst_list;
}

void msq_dialog_str_list_init(msq_dialog_iface_t *dialog_iface,
                              char **str_list,
                              size_t str_list_len)
{
  dialog_iface->str_list_len = str_list_len;
  dialog_iface->str_list = _msq_str_list_copy(str_list, str_list_len);
}

void _msq_free_str_list(char **str_list, size_t str_list_len)
{
  char **str_list_ptr = str_list,
    **str_list_end = &(str_list[str_list_len]);

  while (str_list_ptr < str_list_end)
    {
      free(*str_list_ptr);
      str_list_ptr++;
    }
  free(str_list);
}

void _msq_dialog_str_list_free(msq_dialog_iface_t *dialog_iface)
{
  _msq_free_str_list(dialog_iface->str_list, dialog_iface->str_list_len);
  dialog_iface->str_list = NULL;
  dialog_iface->str_list_len = 0;
}

void msq_dialog_desactivate(msq_dialog_iface_t *dialog_iface)
{
  if (dialog_iface->str_list != NULL)
    _msq_dialog_str_list_free(dialog_iface);
  dialog_iface->activated = MSQ_FALSE;
  dialog_iface->need_update = MSQ_TRUE;
}

void msq_dialog_list(msq_dialog_iface_t *dialog_iface,
                     char **str_list,
                     size_t str_list_len,
                     msq_dialog_idx_result_cb_t result_idx_cb,
                     void *arg_addr)
{
  dialog_iface->type = LIST;
  dialog_iface->need_popup = MSQ_TRUE;
  dialog_iface->need_update = MSQ_TRUE;
  msq_dialog_str_list_init(dialog_iface, str_list, str_list_len);
  dialog_iface->result_idx_cb = result_idx_cb;
  dialog_iface->arg_addr = arg_addr;
}

void msq_dialog_filebrowser(msq_dialog_iface_t *dialog_iface,
                            msq_dialog_str_result_cb_t result_str_cb,
                            void *arg_addr)
{
  dialog_iface->type = FILE_BROWSER;
  dialog_iface->need_popup = MSQ_TRUE;
  dialog_iface->need_update = MSQ_TRUE;
  dialog_iface->result_str_cb = result_str_cb;
  dialog_iface->arg_addr = arg_addr;
}

void msq_dialog_text(msq_dialog_iface_t *dialog_iface,
                     const char *str)
{
  dialog_iface->type = STRING;
  dialog_iface->need_popup = MSQ_TRUE;
  dialog_iface->need_update = MSQ_TRUE;
  dialog_iface->str = (char *) str;
}

void msq_dialog_string_input(msq_dialog_iface_t *dialog_iface,
                             msq_dialog_str_result_cb_t result_str_cb,
                             void *arg_addr)
{
  dialog_iface->type = STRING_INPUT;
  dialog_iface->need_popup = MSQ_TRUE;
  dialog_iface->need_update = MSQ_TRUE;
  dialog_iface->result_str_cb = result_str_cb;
  dialog_iface->arg_addr = arg_addr;
}

void msq_dialog_result_idx(msq_dialog_iface_t *dialog_iface, size_t idx)
{
  msq_dialog_desactivate(dialog_iface);
  dialog_iface->result_idx_cb(idx, dialog_iface->arg_addr);
}

void msq_dialog_result_str(msq_dialog_iface_t *dialog_iface, char *str)
{
  msq_dialog_desactivate(dialog_iface);
  dialog_iface->result_str_cb(str, dialog_iface->arg_addr);
  *str = '\0';
}

unsigned int msq_margin_ggt_get_min_width(pbt_ggt_t *ggt)
{
  msq_margin_ggt_t *margin_ggt = ggt->priv;
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;
  unsigned int child_min_width = _pbt_ggt_min_width(child_ggt);

  if (child_min_width == 0)
    child_min_width = 1;

  return margin_ggt->left
    + child_min_width
    + margin_ggt->right;
}

unsigned int msq_margin_ggt_get_max_width(pbt_ggt_t *ggt)
{
  msq_margin_ggt_t *margin_ggt = ggt->priv;
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;
  unsigned int max_width = _pbt_ggt_max_width(child_ggt);

  if (max_width == 0)
    return 0;
  return margin_ggt->left + max_width + margin_ggt->right;
}

unsigned int msq_margin_ggt_get_min_height(pbt_ggt_t *ggt)
{
  msq_margin_ggt_t *margin_ggt = ggt->priv;
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;
  unsigned int child_min_height = _pbt_ggt_min_height(child_ggt);

  if (child_min_height == 0)
    child_min_height = 1;

  return margin_ggt->top + child_min_height + margin_ggt->bottom;
}

unsigned int msq_margin_ggt_get_max_height(pbt_ggt_t *ggt)
{
  msq_margin_ggt_t *margin_ggt = ggt->priv;
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;
  unsigned int child_max_height = _pbt_ggt_max_height(child_ggt);

  if (child_max_height == 0)
    return 0;
  return margin_ggt->top + child_max_height + margin_ggt->bottom;
}

void msq_margin_ggt_update_area_cb(pbt_ggt_t *ggt,
                                   pbt_pbarea_t *pbarea)
{
  msq_margin_ggt_t *margin_ggt = ggt->priv;
  unsigned int xpos = margin_ggt->left,
    ypos = margin_ggt->top,
    width = pbarea->width - (margin_ggt->left + margin_ggt->right),
    height = pbarea->height - (margin_ggt->top + margin_ggt->bottom);
  pbt_pbarea_t pbarea_child = {};
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
  pbt_pbarea_setup_from_area(&pbarea_child,
                             &(ggt->pbarea),
                             xpos, ypos,
                             width, height);
  _pbt_ggt_update_area(child_ggt,
                       &pbarea_child);
}

void msq_margin_ggt_draw_cb(pbt_ggt_t *ggt)
{
  msq_margin_ggt_t *margin_ggt = ggt->priv;
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;

  if (margin_ggt->left != 0)
      pbt_pbarea_fillrect(&(ggt->pbarea),
                          0, 0,
                          margin_ggt->left, ggt->pbarea.height,
                          margin_ggt->color);

  if (margin_ggt->right != 0)
    pbt_pbarea_fillrect(&(ggt->pbarea),
                        ggt->pbarea.width - margin_ggt->right, 0,
                        margin_ggt->right, ggt->pbarea.height,
                        margin_ggt->color);

  if (margin_ggt->top != 0)
    pbt_pbarea_fillrect(&(ggt->pbarea),
                        0, 0,
                        ggt->pbarea.width, margin_ggt->top,
                        margin_ggt->color);

  if (margin_ggt->bottom != 0)
    pbt_pbarea_fillrect(&(ggt->pbarea),
                        0, ggt->pbarea.height - margin_ggt->bottom,
                        ggt->pbarea.width, margin_ggt->bottom,
                        margin_ggt->color);

  _pbt_ggt_draw(child_ggt);
}

void msq_margin_ggt_destroy_cb(pbt_ggt_t *ggt)
{
  pbt_ggt_t *child_ggt = ggt->childs->priv.ggt_addr;

  if (child_ggt->destroy_cb != NULL)
    _pbt_ggt_destroy(child_ggt);
}

void msq_margin_ggt_init(msq_margin_ggt_t *margin,
                         pbt_ggt_node_type_t type,
                         pbt_ggt_t *child,
                         unsigned int left,
                         unsigned int right,
                         unsigned int top,
                         unsigned int bottom,
                         unsigned char *color)
{
  margin->left = left;
  margin->right = right;
  margin->top = top;
  margin->bottom = bottom;
  margin->color = color;
  margin->child.type = type;
  margin->child.priv.ggt_addr = child;
  margin->child.next = NULL;

  margin->ggt.childs = &(margin->child);
  margin->ggt.priv = margin;

  margin->ggt.get_min_width = msq_margin_ggt_get_min_width;
  margin->ggt.get_max_width = msq_margin_ggt_get_max_width;
  margin->ggt.get_min_height = msq_margin_ggt_get_min_height;
  margin->ggt.get_max_height = msq_margin_ggt_get_max_height;
  margin->ggt.update_area_cb = msq_margin_ggt_update_area_cb;
  margin->ggt.draw_cb = msq_margin_ggt_draw_cb;
  margin->ggt.destroy_cb = msq_margin_ggt_destroy_cb;
}

void msq_draw_veil(pbt_pbarea_t *pbarea,
                   unsigned int xmin,
                   unsigned int xmax,
                   unsigned int ymin,
                   unsigned int ymax)
{
  unsigned int xidx, yidx;
  unsigned char color[4];

  for (xidx = xmin; xidx < xmax; xidx++)
    for (yidx = ymin; yidx < ymax; yidx++)
      {
        pbt_pbarea_get_pxl(pbarea, xidx, yidx, color);
        color[0] >>= 1;
        color[1] >>= 1;
        color[2] >>= 1;
        pbt_pbarea_put_pxl(pbarea, xidx, yidx, color);
      }
}
