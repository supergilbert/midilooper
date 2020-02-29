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

#include <pbt_font.h>
#include <pbt_draw.h>
#include <pbt_gadget.h>
#include <pbt_gadget_window.h>
#include <wbe_glfw.h>
#include <wbe_gl.h>
#include <pbt_event_handler.h>
#include <pbt_window_gadget.h>
#include <pbt_gadget_window.h>
#include <pbt_tools.h>

#include <unistd.h>

#include <fontconfig/fontconfig.h>

pbt_bool_t pbt_load_default_font(pbt_font_t *hdl)
{
  FcConfig *config = FcInitLoadConfigAndFonts();
  FcPattern *pat = FcNameParse((const FcChar8*) "Mono");
  FcResult result;
  FcPattern *font;
  FcChar8 *file = NULL;
  pbt_bool_t ret = PBT_FALSE;

  FcConfigSubstitute(config, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);
  font = FcFontMatch(config, pat, &result);
  if (font)
    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
      {
        printf("Default font path: %s\n", file);
        ret = pbt_load_font(hdl, (const char *) file, 16, 16);
      }
  FcPatternDestroy(pat);
  FcPatternDestroy(font);
  FcConfigDestroy(config);
  return ret;
}

pbt_bool_t test_theme_init(pbt_wgt_theme_t *theme)
{
  pbt_wgt_set_default_theme_color(theme);
  pbt_wgt_default_theme_cursor_init(theme);
  return pbt_load_default_font(&(theme->font));
}

void test_theme_destroy(pbt_wgt_theme_t *theme)
{
  pbt_wgt_default_theme_cursor_destroy(theme);
  pbt_unload_font(&(theme->font));
}

void draw_area_font_cb(pbt_pbarea_t *pbarea, void *theme_addr)
{
  pbt_wgt_theme_t *theme = theme_addr;

  pbt_pbarea_fill(pbarea, theme->window_bg);
  pbt_pbarea_printf(pbarea,
                    &(theme->font),
                    theme->window_fg,
                    0, 0,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789"
                    "abcdefghijklmnopqrstuvwxyz");
}

void draw_area_symbol_cb(pbt_pbarea_t *pbarea, void *theme_addr)
{
  pbt_wgt_theme_t *theme = theme_addr;
  unsigned int xpos = 0,
    size = theme->font.max_height;

  pbt_pbarea_fill(pbarea, theme->window_bg);
  pbt_pbarea_fillrect(pbarea,
                      xpos, 2,
                      size - 4,
                      size - 4,
                      theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_triangle_left(pbarea,
                                xpos, 2,
                                size - 4,
                                theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_triangle_right(pbarea,
                                 xpos, 2,
                                 size - 4,
                                 theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_triangle_up(pbarea,
                              xpos, 2,
                              size - 4,
                              theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_triangle_down(pbarea,
                                xpos, 2,
                                size - 4,
                                theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_M(pbarea,
                    xpos, 2,
                    size - 4,
                    theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_disc(pbarea,
                       xpos, 2,
                       size - 4,
                       theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_plus(pbarea,
                       xpos, 2,
                       size - 4,
                       theme->window_fg);
  xpos += size;
  pbt_pbarea_draw_minus(pbarea,
                        xpos, 2,
                        size - 4,
                        theme->window_fg);
}

void draw_area_empty_cb(pbt_pbarea_t *pbarea, void *color_addr)
{
  unsigned char *color = color_addr;

  pbt_pbarea_fill(pbarea, color);
}

void test_gadget_input_cb(wbe_window_input_t *winev, void *evh_addr)
{
  pbt_evh_t *evh = evh_addr;

  pbt_evh_handle(evh, winev);
}

void test_button_disc(void *unused)
{
  printf("Bouh\n");
}

void render_disc(pbt_pixbuf_t *pixbuf,
                 unsigned char *fg,
                 unsigned char *bg)
{
  pbt_pixbuf_fill(pixbuf, bg);
  pbt_pixbuf_draw_disc(pixbuf,
                       0, 0,
                       pixbuf->height,
                       fg);
}

void render_minus(pbt_pixbuf_t *pixbuf,
                  unsigned char *fg,
                  unsigned char *bg)
{
  unsigned int plus_size = (2 * pixbuf->width) / 3,
    plus_pos = pixbuf->width / 6;

  pbt_pixbuf_fill(pixbuf, bg);
  pbt_pixbuf_draw_minus(pixbuf,
                        plus_pos, plus_pos,
                        plus_size,
                        fg);
}

typedef struct
{
  pbt_ggt_drawarea_t draw_area;
  pbt_wgt_button_t button;
  pbt_ggt_ctnr_t hctnr;
  pbt_ggt_ctnr_t vctnr;
  pbt_ggt_node_it_t node_it;
} test_vlist_node_t;

typedef struct
{
  pbt_ggt_ctnr_t vlist;
  pbt_ggt_drawarea_t vlist_header;
  pbt_wgt_theme_t *theme;
  pbt_wgt_button_t add_button;
} test_vlist_t;

void test_vlist_del_node_cb(void *test_vlist_node_addr)
{
  test_vlist_node_t *test_vlist_node = test_vlist_node_addr;
  pbt_ggt_win_t *ggt_win = test_vlist_node->button.wgt.ggt_win;

  pbt_ggt_node_it_del(&(test_vlist_node->node_it));
  pbt_ggt_win_set_min_size(ggt_win);
}

#include "pbt_gadget_window.h"

void test_vlist_add(test_vlist_t *test_vlist)
{
  test_vlist_node_t *test_vlist_node = malloc(sizeof (test_vlist_node_t));
  pbt_ggt_t *ggt_wrapper = malloc(sizeof (pbt_ggt_t));

  memset(test_vlist_node, 0, sizeof (test_vlist_node_t));
  memset(ggt_wrapper, 0, sizeof (pbt_ggt_t));

  pbt_ggt_drawarea_init(&(test_vlist_node->draw_area),
                        0, 0,
                        test_vlist->theme->font.max_height, 0,
                        draw_area_empty_cb,
                        test_vlist->theme->frame_bg,
                        NULL, NULL);
  /* pbt_wgt_pb_button_init(&(test_vlist_node->button), */
  /*                        render_minus, */
  /*                        test_vlist->theme, */
  /*                        test_vlist_del_node_cb, */
  /*                        test_vlist_node); */

  pbt_ggt_hctnr_init(&(test_vlist_node->hctnr));
  pbt_ggt_add_child_ggt(&(test_vlist_node->hctnr),
                        &(test_vlist_node->draw_area));
  /* pbt_ggt_ctnr_add_line(&(test_vlist_node->hctnr), test_vlist->theme->window_bg); */
  /* pbt_ggt_add_child_wgt(&(test_vlist_node->hctnr), &(test_vlist_node->button)); */

  pbt_ggt_vctnr_init(&(test_vlist_node->vctnr));
  pbt_ggt_ctnr_add_line(&(test_vlist_node->vctnr), test_vlist->theme->window_bg);
  pbt_ggt_add_child_ggt(&(test_vlist_node->vctnr), &(test_vlist_node->hctnr));
  pbt_ggt_wrapper_init(ggt_wrapper,
                       test_vlist_node,
                       &(test_vlist_node->vctnr.ggt),
                       GADGET);

  pbt_ggt_node_it_init_ggt_add_child(&(test_vlist_node->node_it),
                                     &(test_vlist->vlist.ggt),
                                     ggt_wrapper,
                                     GADGET);
  pbt_ggt_win_init_child_ev(test_vlist->add_button.wgt.ggt_win,
                            test_vlist_node->node_it.node);
}


void test_vlist_add_button_cb(void *test_vlist_addr)
{
  test_vlist_t *test_vlist = test_vlist_addr;

  test_vlist_add(test_vlist);
  pbt_ggt_win_set_min_size(test_vlist->add_button.wgt.ggt_win);
}

void render_plus(pbt_pixbuf_t *pixbuf,
                 unsigned char *fg,
                 unsigned char *bg)
{
  unsigned int plus_size = (2 * pixbuf->width) / 3,
    plus_pos = pixbuf->width / 6;

  pbt_pixbuf_fill(pixbuf, bg);
  pbt_pixbuf_draw_plus(pixbuf,
                       plus_pos, plus_pos,
                       plus_size,
                       fg);
}

void test_vlist_init(test_vlist_t *test_vlist, pbt_wgt_theme_t *theme)
{
  test_vlist->theme = theme;

  pbt_ggt_drawarea_init(&(test_vlist->vlist_header),
                        test_vlist->theme->font.max_height * 6, 0,
                        test_vlist->theme->font.max_height, 0,
                        draw_area_empty_cb,
                        test_vlist->theme->frame_bg,
                        NULL, NULL);

  pbt_ggt_vctnr_init(&(test_vlist->vlist));
  pbt_ggt_add_child_ggt(&(test_vlist->vlist), &(test_vlist->vlist_header));

  pbt_wgt_pb_button_init(&(test_vlist->add_button),
                         render_plus,
                         test_vlist->theme,
                         test_vlist_add_button_cb,
                         test_vlist);
}

void test_gadget(void)
{
  pbt_ggt_win_t ggt_win = {};
  pbt_ggt_ctnr_t vctnr1 = {};
  pbt_ggt_ctnr_t vctnr2 = {};
  pbt_ggt_drawarea_t draw_area_font = {};
  pbt_ggt_drawarea_t draw_area_symbol = {};
  pbt_wgt_theme_t theme = {};
  pbt_wgt_splitted_area_t splitted_area = {};
  pbt_ggt_ctnr_t hctnr = {};
  pbt_pixbuf_t pixbuf_pressed = {}, pixbuf_released = {}, pixbuf_hovered = {};
  pbt_wgt_button_t button1 = {};
  pbt_wgt_button_t button2 = {};
  pbt_wgt_button_t buttonlabel = {};
  pbt_wgt_scrollbar_t hscrollbar = {};
  pbt_wgt_scrollbar_t vscrollbar = {};
  pbt_adj_t adj = {.pos = 0, .size = 200, .max = 1000};
  test_vlist_t test_vlist = {};

  wbe_window_backend_init();

  test_theme_init(&theme);

  test_vlist_init(&test_vlist, &theme);

  pbt_ggt_drawarea_init(&draw_area_font,
                        1, 0,
                        theme.font.max_height, 0,
                        draw_area_font_cb,
                        &theme,
                        NULL,
                        NULL);
  pbt_ggt_drawarea_init(&draw_area_symbol,
                        theme.font.max_height * 6, 0,
                        theme.font.max_height, 0,
                        draw_area_symbol_cb,
                        &theme,
                        NULL,
                        NULL);

  pbt_ggt_vctnr_init(&vctnr1);
  pbt_ggt_add_child_ggt(&vctnr1, &(test_vlist.vlist));
  pbt_ggt_add_child_wgt(&vctnr1, &(test_vlist.add_button));
  pbt_ggt_add_child_ggt(&vctnr1, &draw_area_font);
  pbt_ggt_ctnr_add_line(&vctnr1, theme.window_fg);
  pbt_ggt_add_child_ggt(&vctnr1, &draw_area_symbol);
  pbt_ggt_ctnr_add_line(&vctnr1, theme.window_fg);
  pbt_ggt_ctnr_add_empty(&vctnr1, theme.window_bg);

  pbt_pixbuf_init(&pixbuf_released,
                  theme.font.max_height,
                  theme.font.max_height);
  pbt_pixbuf_fill(&pixbuf_released, theme.wgt_normal_bg);
  pbt_pixbuf_draw_disc(&pixbuf_released,
                       0, 0,
                       theme.font.max_height,
                       theme.wgt_normal_fg);
  pbt_pixbuf_init(&pixbuf_pressed,
                  theme.font.max_height,
                  theme.font.max_height);
  pbt_pixbuf_fill(&pixbuf_pressed, theme.wgt_activated_bg);
  pbt_pixbuf_draw_disc(&pixbuf_pressed,
                       0, 0,
                       theme.font.max_height,
                       theme.wgt_activated_fg);
  pbt_pixbuf_init(&pixbuf_hovered,
                  theme.font.max_height,
                  theme.font.max_height);
  pbt_pixbuf_fill(&pixbuf_hovered, theme.wgt_hovered_bg);
  pbt_pixbuf_draw_disc(&pixbuf_hovered,
                       0, 0,
                       theme.font.max_height,
                       theme.wgt_hovered_fg);
  pbt_wgt_button_init(&button1,
                      &pixbuf_released,
                      &pixbuf_pressed,
                      &pixbuf_hovered,
                      theme.wgt_normal_bg,
                      theme.wgt_activated_bg,
                      theme.wgt_hovered_bg,
                      theme.cursor_finger,
                      theme.cursor_arrow,
                      test_button_disc,
                      NULL);

  pbt_wgt_pb_button_init(&button2,
                         render_disc,
                         &theme,
                         test_button_disc,
                         NULL);

  pbt_wgt_label_button_init(&buttonlabel,
                            "coucou",
                            &theme,
                            test_button_disc,
                            NULL);

  pbt_wgt_hscrollbar_init(&hscrollbar,
                          &adj,
                          theme.font.max_height,
                          theme.window_fg,
                          theme.window_bg,
                          test_button_disc,
                          NULL);

  pbt_wgt_vscrollbar_init(&vscrollbar,
                          &adj,
                          theme.font.max_height,
                          theme.window_fg,
                          theme.window_bg,
                          test_button_disc,
                          NULL);

  pbt_ggt_hctnr_init(&hctnr);
  pbt_ggt_add_child_wgt(&hctnr, &button1);
  pbt_ggt_ctnr_add_empty(&hctnr, theme.window_bg);
  pbt_ggt_add_child_wgt(&hctnr, &button2);
  pbt_ggt_ctnr_add_empty(&hctnr, theme.window_bg);
  pbt_ggt_add_child_wgt(&hctnr, &buttonlabel);
  pbt_ggt_ctnr_add_empty(&hctnr, theme.window_bg);
  pbt_ggt_vctnr_init(&vctnr2);
  pbt_ggt_add_child_ggt(&vctnr2, &hctnr);
  pbt_ggt_add_child_wgt(&vctnr2, &hscrollbar);
  pbt_ggt_ctnr_add_empty(&vctnr2, theme.window_bg);

  pbt_wgt_vsplitted_area_init_gg(&splitted_area,
                                 &vctnr1,
                                 &vctnr2,
                                 50,
                                 theme.window_fg,
                                 theme.cursor_grab,
                                 theme.cursor_grabbing,
                                 theme.cursor_arrow);

  pbt_wgt_win_init(&ggt_win,
                   "testpbt",
                   &splitted_area,
                   0, 0, PBT_TRUE);
  wbe_gl_texture_n_color_init();
  wbe_gl_color_init();

  wbe_window_map(ggt_win.pb_win.win_be);
  _pbt_ggt_draw(ggt_win.ggt);

  wbe_window_add_input_cb(ggt_win.pb_win.win_be,
                          test_gadget_input_cb,
                          &(ggt_win.evh));


  GLfloat put_rect_color[] = {1.0f, 0.0f, 0.0f, 0.5f};
  GLfloat put_line_color[] = {0.0f, 0.0f, 0.0f};

  while (wbe_window_closed(ggt_win.pb_win.win_be) == WBE_FALSE)
    {
      wbe_window_handle_events();
      pbt_ggt_win_put_buffer(&ggt_win);
      _wbe_gl_texture_n_color_put_rect(-0.7f, -0.8f, 0.5f, 0.5f,
                                       put_rect_color);
      wbe_gl_color_put_line(-1.0f, 1.0f, 1.0f, -1.0f, put_line_color);
      wbe_gl_flush();
      /* wbe_window_swap_buffer(ggt_win.pb_win.win_be); */
      usleep(100000);
    }

  pbt_ggt_win_destroy(&ggt_win);

  test_theme_destroy(&theme);

  pbt_pixbuf_destroy(&pixbuf_hovered);
  pbt_pixbuf_destroy(&pixbuf_pressed);
  pbt_pixbuf_destroy(&pixbuf_released);

  wbe_gl_color_destroy();
  wbe_gl_texture_n_color_destroy();
  wbe_pbw_backend_destroy();
}

int main(void)
{
  test_gadget();
  return 0;
}
