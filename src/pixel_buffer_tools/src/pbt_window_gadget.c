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

#include "pbt_gadget_window.h"
#include "pbt_gadget.h"
#include "pbt_tools.h"
#include "pbt_event_handler.h"
#include "wbe_glfw.h"
#include "externals/pbt_wgt_gdk_cursor.h"

#include "pbt_font.h"
#include "pbt_draw.h"

#include <string.h>

static unsigned char _default_window_fg[4] = {0x0,
                                              0x0,
                                              0x0,
                                              0xFF};

static unsigned char _default_window_bg[4] = {0xE9,
                                              0xE9,
                                              0xE9,
                                              0xFF};

static unsigned char _default_frame_fg[4] = {0x30,
                                             0x30,
                                             0x30,
                                             0xFF};

static unsigned char _default_frame_bg[4] = {0xD0,
                                             0xD0,
                                             0xD0,
                                             0xFF};

static unsigned char _default_wgt_normal_fg[4] = {0x2C,
                                                  0x2C,
                                                  0x2C,
                                                  0xFF};

static unsigned char _default_wgt_normal_bg[4] = {0x80,
                                                  0x80,
                                                  0x80,
                                                  0xFF};

static unsigned char *_default_wgt_activated_fg = _default_wgt_normal_bg;

static unsigned char *_default_wgt_activated_bg = _default_wgt_normal_fg;

static unsigned char _default_wgt_hovered_fg[4] = {0x2C,
                                                   0x2C,
                                                   0x2C,
                                                   0xFF};

static unsigned char _default_wgt_hovered_bg[4] = {0x60,
                                                   0x60,
                                                   0x60,
                                                   0xFF};

void pbt_wgt_set_default_theme_color(pbt_wgt_theme_t *theme)
{
  theme->window_fg = _default_window_fg;
  theme->window_bg = _default_window_bg;
  theme->frame_fg = _default_frame_fg;
  theme->frame_bg = _default_frame_bg;
  theme->wgt_normal_fg = _default_wgt_normal_fg;
  theme->wgt_normal_bg = _default_wgt_normal_bg;
  theme->wgt_activated_fg = _default_wgt_activated_fg;
  theme->wgt_activated_bg = _default_wgt_activated_bg;
  theme->wgt_hovered_fg = _default_wgt_hovered_fg;
  theme->wgt_hovered_bg = _default_wgt_hovered_bg;
}

void pbt_wgt_default_theme_cursor_init(pbt_wgt_theme_t *theme)
{
  theme->cursor_arrow = wbe_cursor_init(&pbt_gdk_cursor_arrow);
  theme->cursor_pencil = wbe_cursor_init(&pbt_gdk_cursor_pencil);
  theme->cursor_ibeam = wbe_cursor_init(&pbt_gdk_cursor_ibeam);
  theme->cursor_finger = wbe_cursor_init(&pbt_gdk_cursor_finger);
  theme->cursor_grab = wbe_cursor_init(&pbt_gdk_cursor_grab);
  theme->cursor_grabbing = wbe_cursor_init(&pbt_gdk_cursor_grabbing);
  theme->cursor_hresize = wbe_cursor_init(&pbt_gdk_cursor_hresize);
  theme->cursor_vresize = wbe_cursor_init(&pbt_gdk_cursor_vresize);
}

void pbt_wgt_default_theme_cursor_destroy(pbt_wgt_theme_t *theme)
{
  wbe_cursor_destroy(theme->cursor_arrow);
  wbe_cursor_destroy(theme->cursor_pencil);
  wbe_cursor_destroy(theme->cursor_ibeam);
  wbe_cursor_destroy(theme->cursor_finger);
  wbe_cursor_destroy(theme->cursor_grab);
  wbe_cursor_destroy(theme->cursor_grabbing);
  wbe_cursor_destroy(theme->cursor_hresize);
  wbe_cursor_destroy(theme->cursor_vresize);
}

#include "wbe_pbw.h"

void pbt_wgt_gl_fillrect(pbt_wgt_t *wgt,
                         unsigned int xpos,
                         unsigned int ypos,
                         unsigned int width,
                         unsigned int height,
                         unsigned char *color)
{
  wbe_pbw_t *pb_win = &(wgt->ggt_win->pb_win);
  unsigned int tex_width = pb_win->buffer.width;
  unsigned int tex_height = pb_win->buffer.height;
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;
  GLfloat color_float[] = {(float) color[0] / 0xFF,
                           (float) color[1] / 0xFF,
                           (float) color[2] / 0xFF,
                           (float) color[3] / 0xFF};

  xmin_float = ((pbt_ggt_xpos(wgt) + xpos) * 2.0 / tex_width) - 1.0;
  ymin_float = (-(pbt_ggt_ypos(wgt) + (float) ypos) * 2.0 / tex_height) + 1.0;
  xmax_float = ((pbt_ggt_xpos(wgt) + xpos + width) * 2.0 / tex_width) - 1.0;
  ymax_float = (-(pbt_ggt_ypos(wgt) + (float) (ypos + height)) * 2.0 / tex_height) + 1.0;

  wbe_gl_color_put_rect(xmin_float,
                        ymin_float,
                        xmax_float,
                        ymax_float,
                        color_float);
}

void pbt_wgt_gl_refresh_rect(pbt_wgt_t *wgt,
                             unsigned int xpos,
                             unsigned int ypos,
                             unsigned int width,
                             unsigned int height)
{
  wbe_pbw_t *pb_win = &(wgt->ggt_win->pb_win);
  unsigned int tex_width = pb_win->buffer.width;
  unsigned int tex_height = pb_win->buffer.height;
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;

  xmin_float = ((pbt_ggt_xpos(wgt) + xpos) * 2.0 / tex_width) - 1.0;
  ymin_float = (-(pbt_ggt_ypos(wgt) + (float) ypos) * 2.0 / tex_height) + 1.0;
  xmax_float = ((pbt_ggt_xpos(wgt) + xpos + width) * 2.0 / tex_width) - 1.0;
  ymax_float =
    (-(pbt_ggt_ypos(wgt) + (float) (ypos + height)) * 2.0 / tex_height) + 1.0;

  wbe_gl_texture_put_rect(xmin_float, ymin_float, xmax_float, ymax_float);
}

void pbt_wgt_gl_draw_line(pbt_wgt_t *wgt,
                          unsigned int xpos1,
                          unsigned int ypos1,
                          unsigned int xpos2,
                          unsigned int ypos2,
                          unsigned char *color)
{
  wbe_pbw_t *pb_win = &(wgt->ggt_win->pb_win);
  unsigned int tex_width = pb_win->buffer.width;
  unsigned int tex_height = pb_win->buffer.height;
  GLfloat xmin_float, ymin_float, xmax_float, ymax_float;
  GLfloat color_float[] = {(float) color[0] / 0xFF,
                           (float) color[1] / 0xFF,
                           (float) color[2] / 0xFF,
                           (float) color[3] / 0xFF};

  if (xpos1 >= pbt_ggt_width(wgt))
    xpos1 = pbt_ggt_width(wgt) - 1;
  if (xpos2 >= pbt_ggt_width(wgt))
    xpos2 = pbt_ggt_width(wgt) - 1;
  if (ypos1 >= pbt_ggt_height(wgt))
    ypos1 = pbt_ggt_height(wgt) - 1;
  if (ypos2 >= pbt_ggt_height(wgt))
    ypos2 = pbt_ggt_height(wgt) - 1;

  xmin_float = ((xpos1 + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
  ymin_float = (-((float) ypos1 + pbt_ggt_ypos(wgt)) * 2.0 / tex_height) + 1.0;
  xmax_float = ((xpos2 + pbt_ggt_xpos(wgt)) * 2.0 / tex_width) - 1.0;
  ymax_float = (-((float) ypos2 + pbt_ggt_ypos(wgt)) * 2.0 / tex_height) + 1.0;

  wbe_gl_color_put_line(xmin_float,
                        ymin_float,
                        xmax_float,
                        ymax_float,
                        color_float);
}

void pbt_draw_vsplit_separator_cb(pbt_pbarea_t *pbarea,
                                  void *color_addr)
{
  unsigned char *color = color_addr;

  pbt_pbarea_fill(pbarea, color);
}

void _pbt_wgt_vsplitted_area_update_area(pbt_ggt_t *splitted_area_ggt,
                                         pbt_wgt_splitted_area_t *splitted_area,
                                         pbt_pbarea_t *pbarea)
{
  pbt_pbarea_t area;
  unsigned int tmp_size;
  unsigned int maxh = _pbt_ggt_max_height(splitted_area_ggt),
    minh = _pbt_ggt_min_height(splitted_area_ggt);
  pbt_ggt_t *ggt1, *ggt2;

  if ((maxh != 0) && (pbarea->height > maxh))
    pbt_abort("Trying to update area to a size bigger than max.");

  if (pbarea->height < minh)
    pbt_abort("Trying to update area to a size smaller than minimum");

  ggt1 = splitted_area->node[0].priv.ggt_addr;
  ggt2 = splitted_area->node[2].priv.ggt_addr;
  if (pbarea->height < (_pbt_ggt_min_height(ggt1)
                        + PBT_SPLIT_AREA_SIZE
                        + splitted_area->node[2].size))
    {
      tmp_size = _pbt_ggt_min_height(ggt1);
      if (tmp_size == 0)
        tmp_size = 1;
      splitted_area->node[2].size =
        pbarea->height - (tmp_size + PBT_SPLIT_AREA_SIZE);
    }
  else
    {
      tmp_size = _pbt_ggt_max_height(ggt1);
      if ((tmp_size != 0)
          && (pbarea->height > (tmp_size
                                + PBT_SPLIT_AREA_SIZE
                                + splitted_area->node[2].size)))
        splitted_area->node[2].size =
          pbarea->height - (tmp_size + PBT_SPLIT_AREA_SIZE);
    }

  tmp_size =
    pbarea->height - (splitted_area->node[2].size + PBT_SPLIT_AREA_SIZE);

  pbt_pbarea_setup_from_area(&area, pbarea,
                             0, 0,
                             pbarea->width, tmp_size);
  _pbt_ggt_update_area(ggt1, &area);

  pbt_pbarea_setup_from_area(&area, pbarea,
                             0, tmp_size,
                             pbarea->width, PBT_SPLIT_AREA_SIZE);
  pbt_ggt_update_area(&(splitted_area->separator), &area);

  pbt_pbarea_setup_from_area(&area, pbarea,
                             0, tmp_size + PBT_SPLIT_AREA_SIZE,
                             pbarea->width, splitted_area->node[2].size);
  _pbt_ggt_update_area(ggt2, &area);
}

pbt_bool_t pbt_wgt_vsplitted_area_input_in(pbt_ggt_t *separator_ggt,
                                           wbe_window_input_t *winev,
                                           void *wgt_addr)
{
  pbt_wgt_t *wgt = wgt_addr;
  pbt_ggt_t *ggt = &(wgt->ggt);
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;

  if (_PBT_IS_IN_GGT(separator_ggt, winev->xpos, winev->ypos) == PBT_TRUE &&
      WBE_GET_BIT(winev->buttons, 0) == 1)
    {
      wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                            splitted_area->grab_cursor);
      splitted_area->last_ypos =
        winev->ypos - _pbt_ggt_ypos(ggt);
      return PBT_TRUE;
    }
  return PBT_FALSE;
}

pbt_bool_t pbt_wgt_vsplitted_area_released(pbt_ggt_t *separator_ggt,
                                           wbe_window_input_t *winev,
                                           void *wgt_addr)
{
  pbt_wgt_t *wgt = wgt_addr;
  pbt_ggt_t *ggt = &(wgt->ggt);
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  unsigned int tmp;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;
  int ypos = winev->ypos - _pbt_ggt_ypos(ggt);

  if (ypos < 0)
    ypos = 0;
  splitted_area->last_ypos = ypos;
  tmp = _pbt_ggt_min_height(ggt1);
  if (splitted_area->last_ypos < tmp)
    splitted_area->last_ypos = tmp;
  else
    {
      tmp = _pbt_ggt_height(ggt)
        - (_pbt_ggt_min_height(ggt2) + PBT_SPLIT_AREA_SIZE);
      if (splitted_area->last_ypos > tmp)
        splitted_area->last_ypos = tmp;
    }

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    {
      wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                            splitted_area->entered_cursor);
      splitted_area->node[2].size =
        _pbt_ggt_height(ggt) - splitted_area->last_ypos;
      _pbt_wgt_vsplitted_area_update_area(ggt,
                                          splitted_area,
                                          &(ggt->pbarea));
      _pbt_ggt_draw(ggt);
      pbt_wgt_win_put_buffer(wgt);
      return PBT_TRUE;
    }
  else
    {
      wbe_gl_texture_refresh();
      pbt_wgt_gl_draw_line(wgt,
                           0, splitted_area->last_ypos,
                           pbt_ggt_width(wgt), splitted_area->last_ypos,
                           splitted_area->color);
      wbe_gl_flush();
      /* pbt_wgt_swap_buffer(wgt); */
    }
  return PBT_FALSE;
}

unsigned int pbt_wgt_vsplitted_area_get_min_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;

  return _pbt_ggt_min_width(ggt1)
    + _pbt_ggt_min_width(ggt2);
}

unsigned int pbt_wgt_vsplitted_area_get_max_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;
  unsigned int max1 = _pbt_ggt_max_width(ggt1),
    max2 = _pbt_ggt_max_width(ggt2);

  return (max1 == 0 || max2 == 0) ? 0 : max1 + max2;
}

unsigned int pbt_wgt_vsplitted_area_get_min_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;
  unsigned int min_height_1 = _pbt_ggt_min_height(ggt1),
    min_height_2 = _pbt_ggt_min_height(ggt2);

  if (min_height_1 == 0)
    min_height_1 = 1;
  if (min_height_2 == 0)
    min_height_2 = 1;

  return min_height_1
    + PBT_SPLIT_AREA_SIZE
    + min_height_2;
}

unsigned int pbt_wgt_vsplitted_area_get_max_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;
  unsigned int max1 = _pbt_ggt_max_height(ggt1),
    max2 = _pbt_ggt_max_height(ggt2);

  return (max1 == 0 || max2 == 0) ? 0 : (max1 + PBT_SPLIT_AREA_SIZE + max2);
}

void pbt_wgt_vsplitted_area_update_area_cb(pbt_ggt_t *ggt,
                                           pbt_pbarea_t *pbarea)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
  _pbt_wgt_vsplitted_area_update_area(ggt, splitted_area, pbarea);
}

void pbt_wgt_vsplitted_area_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;

  _pbt_ggt_draw(ggt1);
  pbt_ggt_draw(&(splitted_area->separator));
  _pbt_ggt_draw(ggt2);
}

void pbt_wgt_vsplitted_area_destroy_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;
  pbt_ggt_t *ggt1 = splitted_area->node[0].priv.ggt_addr,
    *ggt2 = splitted_area->node[2].priv.ggt_addr;

  if (ggt1->destroy_cb)
    _pbt_ggt_destroy(ggt1);
  if (ggt2->destroy_cb)
    _pbt_ggt_destroy(ggt2);
  if (wgt->ggt_win != NULL)
    pbt_evh_del_node(&(wgt->ggt_win->evh), &(splitted_area->separator.ggt));
}

void pbt_wgt_vsplitted_area_enter_cb(void *wgt_addr)
{
  pbt_wgt_t *wgt = wgt_addr;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;

  wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                        splitted_area->entered_cursor);
}

void pbt_wgt_vsplitted_area_leave_cb(void *wgt_addr)
{
  pbt_wgt_t *wgt = wgt_addr;
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;

  wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                        splitted_area->leave_cursor);
}

void pbt_wgt_vsplitted_area_init_ev(pbt_wgt_t *wgt,
                                    pbt_ggt_win_t *ggt_win)
{
  pbt_wgt_splitted_area_t *splitted_area = wgt->priv;

  wgt->ggt_win = ggt_win;

  pbt_evh_add_set_focus_cb(&(wgt->ggt_win->evh),
                           &(splitted_area->separator.ggt),
                           pbt_wgt_vsplitted_area_input_in,
                           wgt);
  pbt_evh_add_unset_focus_cb(&(wgt->ggt_win->evh),
                             &(splitted_area->separator.ggt),
                             pbt_wgt_vsplitted_area_released,
                             wgt);

  pbt_evh_add_enter_cb(&(wgt->ggt_win->evh),
                       &(splitted_area->separator.ggt),
                       pbt_wgt_vsplitted_area_enter_cb,
                       wgt);
  pbt_evh_add_leave_cb(&(wgt->ggt_win->evh),
                       &(splitted_area->separator.ggt),
                       pbt_wgt_vsplitted_area_leave_cb,
                       wgt);
}

void _pbt_wgt_vsplitted_area_init(pbt_wgt_splitted_area_t *splitted_area,
                                  unsigned char *separator_color,
                                  wbe_cursor_t *entered_cursor,
                                  wbe_cursor_t *grab_cursor,
                                  wbe_cursor_t *leave_cursor)
{
  pbt_ggt_node_t *node;

  pbt_ggt_drawarea_init(&(splitted_area->separator),
                        1, 0,
                        PBT_SPLIT_AREA_SIZE, PBT_SPLIT_AREA_SIZE,
                        pbt_draw_vsplit_separator_cb,
                        separator_color,
                        NULL, NULL);
  splitted_area->color = separator_color;

  node = &(splitted_area->node[0]);
  node->next = &(splitted_area->node[1]);

  node = &(splitted_area->node[1]);
  node->type = GADGET;
  node->priv.ggt_addr = &(splitted_area->separator.ggt);
  node->next = &(splitted_area->node[2]);

  node = &(splitted_area->node[2]);
  node->next = NULL;

  splitted_area->entered_cursor = entered_cursor;
  splitted_area->grab_cursor = grab_cursor;
  splitted_area->leave_cursor = leave_cursor;
  splitted_area->wgt.priv = splitted_area;

  splitted_area->wgt.ggt.priv = &(splitted_area->wgt);
  splitted_area->wgt.ggt.childs = &(splitted_area->node[0]);
  splitted_area->wgt.ggt.get_min_width = pbt_wgt_vsplitted_area_get_min_width;
  splitted_area->wgt.ggt.get_max_width = pbt_wgt_vsplitted_area_get_max_width;
  splitted_area->wgt.ggt.get_min_height = pbt_wgt_vsplitted_area_get_min_height;
  splitted_area->wgt.ggt.get_max_height = pbt_wgt_vsplitted_area_get_max_height;
  splitted_area->wgt.ggt.update_area_cb = pbt_wgt_vsplitted_area_update_area_cb;
  splitted_area->wgt.ggt.draw_cb = pbt_wgt_vsplitted_area_draw_cb;
  splitted_area->wgt.ggt.destroy_cb = pbt_wgt_vsplitted_area_destroy_cb;

  splitted_area->wgt.init_ev_cb = pbt_wgt_vsplitted_area_init_ev;
}

void _pbt_wgt_vsplitted_add_ggt1(pbt_wgt_splitted_area_t *splitted_area,
                                 pbt_ggt_t *ggt1)
{
  pbt_ggt_node_t *node = &(splitted_area->node[0]);

  node->type = GADGET;
  node->priv.ggt_addr = ggt1;
}

void _pbt_wgt_vsplitted_add_wgt1(pbt_wgt_splitted_area_t *splitted_area,
                                 pbt_wgt_t *wgt1)
{
  pbt_ggt_node_t *node = &(splitted_area->node[0]);

  node->type = WIDGET;
  node->priv.ggt_addr = &(wgt1->ggt);
}

void _pbt_wgt_vsplitted_add_ggt2(pbt_wgt_splitted_area_t *splitted_area,
                                 pbt_ggt_t *ggt2,
                                 unsigned int size)
{
  pbt_ggt_node_t *node = &(splitted_area->node[2]);

  if (size < _pbt_ggt_min_height(ggt2))
    pbt_abort("Gadget 2 start size is inferior to gadget minimum.");
  if ((_pbt_ggt_max_height(ggt2) != 0)
      && (size > _pbt_ggt_max_height(ggt2)))
    pbt_abort("Gadget 2 start size is superior to gadget maximum.");

  node->type = GADGET;
  node->priv.ggt_addr = ggt2;
  node->size = size;
}

void _pbt_wgt_vsplitted_add_wgt2(pbt_wgt_splitted_area_t *splitted_area,
                                 pbt_wgt_t *wgt2,
                                 unsigned int size)
{
  pbt_ggt_node_t *node = &(splitted_area->node[2]);

  if (size < pbt_ggt_min_height(wgt2))
    pbt_abort("Gadget 2 start size is inferior to gadget minimum.");
  if ((pbt_ggt_max_height(wgt2) != 0)
      && (size > pbt_ggt_max_height(wgt2)))
    pbt_abort("Gadget 2 start size is superior to gadget maximum.");

  node->type = WIDGET;
  node->priv.ggt_addr = &(wgt2->ggt);
  node->size = size;
}

void _pbt_wgt_vsplitted_area_init_gg(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_ggt_t *ggt1,
                                     pbt_ggt_t *ggt2,
                                     unsigned int ggt2_size,
                                     unsigned char *separator_color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor)
{
  _pbt_wgt_vsplitted_area_init(splitted_area,
                               separator_color,
                               entered_cursor,
                               grab_cursor,
                               leave_cursor);
  _pbt_wgt_vsplitted_add_ggt1(splitted_area, ggt1);
  _pbt_wgt_vsplitted_add_ggt2(splitted_area, ggt2, ggt2_size);
}

void _pbt_wgt_vsplitted_area_init_wg(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_wgt_t *wgt1,
                                     pbt_ggt_t *ggt2,
                                     unsigned int ggt2_size,
                                     unsigned char *separator_color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor)
{
  _pbt_wgt_vsplitted_area_init(splitted_area,
                               separator_color,
                               entered_cursor,
                               grab_cursor,
                               leave_cursor);
  _pbt_wgt_vsplitted_add_wgt1(splitted_area, wgt1);
  _pbt_wgt_vsplitted_add_ggt2(splitted_area, ggt2, ggt2_size);
}

void _pbt_wgt_vsplitted_area_init_gw(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_ggt_t *ggt1,
                                     pbt_wgt_t *wgt2,
                                     unsigned int ggt2_size,
                                     unsigned char *separator_color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor)
{
  _pbt_wgt_vsplitted_area_init(splitted_area,
                               separator_color,
                               entered_cursor,
                               grab_cursor,
                               leave_cursor);
  _pbt_wgt_vsplitted_add_ggt1(splitted_area, ggt1);
  _pbt_wgt_vsplitted_add_wgt2(splitted_area, wgt2, ggt2_size);
}

void _pbt_wgt_vsplitted_area_init_ww(pbt_wgt_splitted_area_t *splitted_area,
                                     pbt_wgt_t *wgt1,
                                     pbt_wgt_t *wgt2,
                                     unsigned int ggt2_size,
                                     unsigned char *separator_color,
                                     wbe_cursor_t *entered_cursor,
                                     wbe_cursor_t *grab_cursor,
                                     wbe_cursor_t *leave_cursor)
{
  _pbt_wgt_vsplitted_area_init(splitted_area,
                               separator_color,
                               entered_cursor,
                               grab_cursor,
                               leave_cursor);
  _pbt_wgt_vsplitted_add_wgt1(splitted_area, wgt1);
  _pbt_wgt_vsplitted_add_wgt2(splitted_area, wgt2, ggt2_size);
}

unsigned int pbt_wgt_button_get_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_button_t *button = wgt->priv;

  return button->width;
}

unsigned int pbt_wgt_button_get_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_button_t *button = wgt->priv;

  return button->height;
}

void pbt_wgt_button_update_area_cb(pbt_ggt_t *ggt,
                                   pbt_pbarea_t *pbarea)
{

  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
}

void pbt_wgt_button_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_button_t *button = wgt->priv;
  unsigned int xpos, ypos;
  pbt_pixbuf_t *pixbuf;
  unsigned char *bg_color;

  if (button->state == PRESSED)
    {
      pixbuf = button->pb_pressed;
      bg_color = button->bg_pressed;
    }
  else if (button->state == RELEASED)
    {
      pixbuf = button->pb_released;
      bg_color = button->bg_released;
    }
  else if (button->state == HOVERED)
    {
      pixbuf = button->pb_hovered;
      bg_color = button->bg_hovered;
    }
  xpos = (ggt->pbarea.width - pixbuf->width) / 2;
  ypos = (ggt->pbarea.height - pixbuf->height) / 2;

  pbt_pbarea_fill(&(ggt->pbarea), bg_color);
  pbt_pbarea_copy_pixbuf(&(ggt->pbarea),
                         xpos, ypos,
                         pixbuf,
                         0, 0,
                         pixbuf->width, pixbuf->height);
}

pbt_bool_t pbt_wgt_button_pressed(pbt_ggt_t *ggt,
                                  wbe_window_input_t *winev,
                                  void *unused)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_button_t *button = wgt->priv;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_TRUE
      && WBE_GET_BIT(winev->buttons, 0) == 1)
    {
      button->state = PRESSED;
      pbt_ggt_draw(wgt);
      pbt_ggt_win_put_buffer(wgt->ggt_win);
      return PBT_TRUE;
    }
  return PBT_FALSE;
}

pbt_bool_t pbt_wgt_button_released(pbt_ggt_t *ggt,
                                   wbe_window_input_t *winev,
                                   void *unused)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_button_t *button = wgt->priv;

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    {
      button->state = RELEASED;
      if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_TRUE)
        button->cb(button->cb_arg);
      pbt_ggt_draw(wgt);
      pbt_ggt_win_put_buffer(wgt->ggt_win);
      return PBT_TRUE;
    }
  return PBT_FALSE;
}

void pbt_wgt_evnode_destroy(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;

  if (wgt->ggt_win != NULL)
    pbt_evh_del_node(&(wgt->ggt_win->evh), ggt);
}

void pbt_wgt_button_enter_cb(void *wgt_addr)
{
  pbt_wgt_t *wgt = wgt_addr;
  pbt_wgt_button_t *button = wgt->priv;

  wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                        button->entered_cursor);
  button->state = HOVERED;
  pbt_ggt_draw(wgt);
  pbt_ggt_win_put_buffer(wgt->ggt_win);
}

void pbt_wgt_button_leave_cb(void *wgt_addr)
{
  pbt_wgt_t *wgt = wgt_addr;
  pbt_wgt_button_t *button = wgt->priv;

  wbe_window_set_cursor(wgt->ggt_win->pb_win.win_be,
                        button->leave_cursor);
  button->state = RELEASED;
  pbt_ggt_draw(wgt);
  pbt_ggt_win_put_buffer(wgt->ggt_win);
}

void pbt_wgt_button_init_ev(pbt_wgt_t *wgt,
                            pbt_ggt_win_t *ggt_win)
{
  wgt->ggt_win = ggt_win;

  pbt_evh_add_set_focus_cb(&(wgt->ggt_win->evh),
                           &(wgt->ggt),
                           pbt_wgt_button_pressed,
                           NULL);
  pbt_evh_add_unset_focus_cb(&(wgt->ggt_win->evh),
                             &(wgt->ggt),
                             pbt_wgt_button_released,
                             NULL);
  pbt_evh_add_enter_cb(&(wgt->ggt_win->evh),
                       &(wgt->ggt),
                       pbt_wgt_button_enter_cb,
                       wgt);
  pbt_evh_add_leave_cb(&(wgt->ggt_win->evh),
                       &(wgt->ggt),
                       pbt_wgt_button_leave_cb,
                       wgt);
}

void pbt_wgt_button_init(pbt_wgt_button_t *button,
                         pbt_pixbuf_t *pb_released,
                         pbt_pixbuf_t *pb_pressed,
                         pbt_pixbuf_t *pb_hovered,
                         unsigned char *bg_released,
                         unsigned char *bg_pressed,
                         unsigned char *bg_hovered,
                         wbe_cursor_t *entered_cursor,
                         wbe_cursor_t *leave_cursor,
                         pbt_wgt_cb_t cb,
                         void *cb_arg)
{
  button->pb_released = pb_released;
  button->pb_pressed = pb_pressed;
  button->pb_hovered = pb_hovered;
  button->bg_released = bg_released;
  button->bg_pressed = bg_pressed;
  button->bg_hovered = bg_hovered;
  button->entered_cursor = entered_cursor;
  button->leave_cursor = leave_cursor;

  button->wgt.priv = button;

  button->height = pb_released->height;
  if (button->height < pb_pressed->height)
    button->height = pb_pressed->height;
  if (button->height < pb_hovered->height)
    button->height = pb_hovered->height;

  button->width = pb_released->width;
  if (button->width < pb_pressed->width)
    button->width = pb_pressed->width;
  if (button->width < pb_hovered->width)
    button->width = pb_hovered->width;

  button->state = RELEASED;
  button->cb = cb;
  button->cb_arg = cb_arg;

  button->wgt.ggt.priv = &(button->wgt);
  button->wgt.ggt.get_min_width = pbt_wgt_button_get_width;
  button->wgt.ggt.get_max_width = pbt_wgt_button_get_width;
  button->wgt.ggt.get_min_height = pbt_wgt_button_get_height;
  button->wgt.ggt.get_max_height = pbt_wgt_button_get_height;
  button->wgt.ggt.update_area_cb = pbt_wgt_button_update_area_cb;
  button->wgt.ggt.draw_cb = pbt_wgt_button_draw_cb;
  button->wgt.ggt.destroy_cb = pbt_wgt_evnode_destroy;

  button->wgt.init_ev_cb = pbt_wgt_button_init_ev;
}

void pbt_wgt_pb_button_destroy(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_button_t *button = wgt->priv;

  pbt_pixbuf_destroy(button->pb_released);
  free(button->pb_released);
  pbt_pixbuf_destroy(button->pb_pressed);
  free(button->pb_pressed);
  pbt_pixbuf_destroy(button->pb_hovered);
  free(button->pb_hovered);
  pbt_wgt_evnode_destroy(ggt);
}

void pbt_wgt_pb_button_init(pbt_wgt_button_t *button,
                            pbt_render_pixbuf_cb_t render_pixbuf,
                            pbt_wgt_theme_t *theme,
                            pbt_wgt_cb_t cb,
                            void *cb_arg)
{
  unsigned wgt_height = theme->font.max_height;
  pbt_pixbuf_t *pb_normal;
  pbt_pixbuf_t *pb_activated;
  pbt_pixbuf_t *pb_hovered;
  pbt_pixbuf_t pb_tmp = {};

  pb_normal = malloc(sizeof (pbt_pixbuf_t));
  memset(pb_normal, 0, sizeof (pbt_pixbuf_t));
  pbt_pixbuf_init(pb_normal, wgt_height, wgt_height);

  pb_activated = malloc(sizeof (pbt_pixbuf_t));
  memset(pb_activated, 0, sizeof (pbt_pixbuf_t));
  pbt_pixbuf_init(pb_activated, wgt_height, wgt_height);

  pb_hovered = malloc(sizeof (pbt_pixbuf_t));
  memset(pb_hovered, 0, sizeof (pbt_pixbuf_t));
  pbt_pixbuf_init(pb_hovered, wgt_height, wgt_height);

  pbt_pixbuf_init(&pb_tmp, wgt_height * 2, wgt_height * 2);

  render_pixbuf(&pb_tmp, theme->wgt_normal_fg, theme->wgt_normal_bg);
  pbt_pixbuf_reduce_2x(pb_normal, &pb_tmp);

  render_pixbuf(&pb_tmp, theme->wgt_activated_fg, theme->wgt_activated_bg);
  pbt_pixbuf_reduce_2x(pb_activated, &pb_tmp);

  render_pixbuf(&pb_tmp, theme->wgt_hovered_fg, theme->wgt_hovered_bg);
  pbt_pixbuf_reduce_2x(pb_hovered, &pb_tmp);

  pbt_pixbuf_destroy(&pb_tmp);

  pbt_wgt_button_init(button,
                      pb_normal,
                      pb_activated,
                      pb_hovered,
                      theme->wgt_normal_bg,
                      theme->wgt_activated_bg,
                      theme->wgt_hovered_bg,
                      theme->cursor_finger,
                      theme->cursor_arrow,
                      cb,
                      cb_arg);
  button->wgt.ggt.destroy_cb = pbt_wgt_pb_button_destroy;
}

void _pbt_wgt_label_button_init(pbt_wgt_button_t *button,
                                const char *label,
                                pbt_font_t *font,
                                unsigned char *normal_fg,
                                unsigned char *normal_bg,
                                unsigned char *activated_fg,
                                unsigned char *activated_bg,
                                unsigned char *hovered_fg,
                                unsigned char *hovered_bg,
                                wbe_cursor_t *entered_cursor,
                                wbe_cursor_t *leave_cursor,
                                pbt_wgt_cb_t cb,
                                void *cb_arg)
{
  unsigned int wgt_height = font->max_height, wgt_width;
  pbt_pixbuf_t *pb_normal;
  pbt_pixbuf_t *pb_activated;
  pbt_pixbuf_t *pb_hovered;

  pbt_font_get_string_width(font, label, &wgt_width);

  pb_normal = malloc(sizeof (pbt_pixbuf_t));
  memset(pb_normal, 0, sizeof (pbt_pixbuf_t));
  pbt_pixbuf_init(pb_normal, wgt_width, wgt_height);

  pb_activated = malloc(sizeof (pbt_pixbuf_t));
  memset(pb_activated, 0, sizeof (pbt_pixbuf_t));
  pbt_pixbuf_init(pb_activated, wgt_width, wgt_height);

  pb_hovered = malloc(sizeof (pbt_pixbuf_t));
  memset(pb_hovered, 0, sizeof (pbt_pixbuf_t));
  pbt_pixbuf_init(pb_hovered, wgt_width, wgt_height);

  pbt_pixbuf_fill(pb_normal, normal_bg);
  pbt_pixbuf_printf(pb_normal,
                    font,
                    normal_fg,
                    0, 0,
                    label);

  pbt_pixbuf_fill(pb_activated, activated_bg);
  pbt_pixbuf_printf(pb_activated,
                    font,
                    activated_fg,
                    0, 0,
                    label);

  pbt_pixbuf_fill(pb_hovered, hovered_bg);
  pbt_pixbuf_printf(pb_hovered,
                    font,
                    hovered_fg,
                    0, 0,
                    label);

  pbt_wgt_button_init(button,
                      pb_normal,
                      pb_activated,
                      pb_hovered,
                      normal_bg,
                      activated_bg,
                      hovered_bg,
                      entered_cursor,
                      leave_cursor,
                      cb,
                      cb_arg);
  button->wgt.ggt.destroy_cb = pbt_wgt_pb_button_destroy;
}

void pbt_wgt_label_button_init(pbt_wgt_button_t *button,
                               const char *label,
                               pbt_wgt_theme_t *theme,
                               pbt_wgt_cb_t cb,
                               void *cb_arg)
{
  _pbt_wgt_label_button_init(button, label,
                             &(theme->font),
                             theme->wgt_normal_fg, theme->wgt_normal_bg,
                             theme->wgt_activated_fg, theme->wgt_activated_bg,
                             theme->wgt_hovered_fg, theme->wgt_hovered_bg,
                             theme->cursor_finger,
                             theme->cursor_arrow,
                             cb,
                             cb_arg);
}

unsigned int pbt_wgt_scrollbar_get_size(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_scrollbar_t *scrollbar = wgt->priv;

  return scrollbar->size;
}

unsigned int pbt_ggt_return_zero(pbt_ggt_t *ggt) { return 0; }

void pbt_ggt_memcpy_area(pbt_ggt_t *ggt, pbt_pbarea_t *pbarea)
{
  memcpy(&(ggt->pbarea), pbarea, sizeof (pbt_pbarea_t));
}

void pbt_wgt_scrollbar_update_pos(pbt_wgt_scrollbar_t *wgt_scrollbar,
                                  int pos,
                                  int size)
{
  int offset;
  int max;

  offset = (pos -  wgt_scrollbar->last_pos)
    * ((int) wgt_scrollbar->adj->max)
    / size;

  if (offset > 0)
    {
      max = wgt_scrollbar->adj->max - wgt_scrollbar->adj->size;
      if ((wgt_scrollbar->adj->pos + offset) < max)
        wgt_scrollbar->adj->pos += offset;
      else
        wgt_scrollbar->adj->pos = max;
    }
  else if (offset < 0)
    {
      if ((((int) wgt_scrollbar->adj->pos) + offset) > 0)
        wgt_scrollbar->adj->pos += offset;
      else
        wgt_scrollbar->adj->pos = 0;
    }
  wgt_scrollbar->last_pos = pos;
}

void pbt_wgt_hscrollbar_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_scrollbar_t *scrollbar = wgt->priv;
  unsigned int pos, size;

  if (scrollbar->adj->max == 0)
    pbt_pbarea_fill(&(ggt->pbarea), scrollbar->fg_color);
  else
    {
      pos = scrollbar->adj->pos * ggt->pbarea.width
        / scrollbar->adj->max;
      size = scrollbar->adj->size * ggt->pbarea.width
        / scrollbar->adj->max;

      pbt_pbarea_fill(&(ggt->pbarea), scrollbar->bg_color);
      pbt_pbarea_fillrect(&(ggt->pbarea),
                          pos, 0,
                          size, ggt->pbarea.height,
                          scrollbar->fg_color);
    }
}

pbt_bool_t pbt_wgt_hscrollbar_set_focus_cb(pbt_ggt_t *ggt,
                                           wbe_window_input_t *winev,
                                           void *vscrollbar_addr)
{
  pbt_wgt_scrollbar_t *vscrollbar = vscrollbar_addr;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_FALSE
      || WBE_GET_BIT(winev->buttons, 0) == 0)
    return PBT_FALSE;
  vscrollbar->last_pos = winev->xpos;
  return PBT_TRUE;
}

pbt_bool_t pbt_wgt_hscrollbar_unset_focus_cb(pbt_ggt_t *ggt,
                                             wbe_window_input_t *winev,
                                             void *vscrollbar_addr)
{
  pbt_wgt_scrollbar_t *vscrollbar = vscrollbar_addr;

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    return PBT_TRUE;

  pbt_wgt_scrollbar_update_pos(vscrollbar, winev->xpos, _pbt_ggt_width(ggt));
  _pbt_ggt_draw(ggt);
  vscrollbar->cb(vscrollbar->cb_arg);
  return PBT_FALSE;
}

void pbt_wgt_hscrollbar_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  wgt->ggt_win = ggt_win;
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           pbt_wgt_hscrollbar_set_focus_cb,
                           wgt->priv);
  pbt_evh_add_unset_focus_cb(&(ggt_win->evh),
                             &(wgt->ggt),
                             pbt_wgt_hscrollbar_unset_focus_cb,
                             wgt->priv);
}

void pbt_wgt_hscrollbar_init(pbt_wgt_scrollbar_t *scrollbar,
                             pbt_adj_t *adj,
                             unsigned int width,
                             unsigned char *fg_color,
                             unsigned char *bg_color,
                             pbt_wgt_cb_t cb,
                             void *cb_arg)
{
  scrollbar->adj = adj;
  scrollbar->size = width;
  scrollbar->fg_color = fg_color;
  scrollbar->bg_color = bg_color;
  scrollbar->cb = cb;
  scrollbar->cb_arg = cb_arg;
  scrollbar->wgt.priv = scrollbar;

  scrollbar->wgt.ggt.priv = &(scrollbar->wgt);
  scrollbar->wgt.ggt.get_min_width = pbt_ggt_return_zero;
  scrollbar->wgt.ggt.get_max_width = pbt_ggt_return_zero;
  scrollbar->wgt.ggt.get_min_height = pbt_wgt_scrollbar_get_size;
  scrollbar->wgt.ggt.get_max_height = pbt_wgt_scrollbar_get_size;
  scrollbar->wgt.ggt.update_area_cb = pbt_ggt_memcpy_area;
  scrollbar->wgt.ggt.draw_cb = pbt_wgt_hscrollbar_draw_cb;
  scrollbar->wgt.ggt.destroy_cb = pbt_wgt_evnode_destroy;

  scrollbar->wgt.init_ev_cb = pbt_wgt_hscrollbar_init_ev;
}

void pbt_wgt_vscrollbar_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = ggt->priv;
  pbt_wgt_scrollbar_t *scrollbar = wgt->priv;
  unsigned int pos, size;

  if (scrollbar->adj->max == 0)
    pbt_pbarea_fill(&(ggt->pbarea), scrollbar->fg_color);
  else
    {
      pos = scrollbar->adj->pos * ggt->pbarea.height
        / scrollbar->adj->max;
      size = scrollbar->adj->size * ggt->pbarea.height
        / scrollbar->adj->max;

      pbt_pbarea_fill(&(ggt->pbarea), scrollbar->bg_color);
      pbt_pbarea_fillrect(&(ggt->pbarea),
                          0, pos,
                          ggt->pbarea.width, size,
                          scrollbar->fg_color);
    }
}

pbt_bool_t pbt_wgt_vscrollbar_set_focus_cb(pbt_ggt_t *ggt,
                                           wbe_window_input_t *winev,
                                           void *vscrollbar_addr)
{
  pbt_wgt_scrollbar_t *scrollbar = vscrollbar_addr;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_FALSE
      || WBE_GET_BIT(winev->buttons, 0) == 0)
    return PBT_FALSE;
  scrollbar->last_pos = winev->ypos;
  return PBT_TRUE;
}

pbt_bool_t pbt_wgt_vscrollbar_unset_focus_cb(pbt_ggt_t *ggt,
                                             wbe_window_input_t *winev,
                                             void *wgt_vscrollbar_addr)
{
  pbt_wgt_scrollbar_t *wgt_scrollbar = wgt_vscrollbar_addr;

  if (WBE_GET_BIT(winev->buttons, 0) == 0)
    return PBT_TRUE;

  pbt_wgt_scrollbar_update_pos(wgt_scrollbar,
                               winev->ypos,
                               _pbt_ggt_height(ggt));
  _pbt_ggt_draw(ggt);
  wgt_scrollbar->cb(wgt_scrollbar->cb_arg);
  return PBT_FALSE;
}

void pbt_wgt_vscrollbar_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  wgt->ggt_win = ggt_win;
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           pbt_wgt_vscrollbar_set_focus_cb,
                           wgt->priv);
  pbt_evh_add_unset_focus_cb(&(ggt_win->evh),
                             &(wgt->ggt),
                             pbt_wgt_vscrollbar_unset_focus_cb,
                             wgt->priv);
}

void pbt_wgt_vscrollbar_init(pbt_wgt_scrollbar_t *scrollbar,
                             pbt_adj_t *adj,
                             unsigned int width,
                             unsigned char *fg_color,
                             unsigned char *bg_color,
                             pbt_wgt_cb_t cb,
                             void *cb_arg)
{
  scrollbar->adj = adj;
  scrollbar->size = width;
  scrollbar->fg_color = fg_color;
  scrollbar->bg_color = bg_color;
  scrollbar->cb = cb;
  scrollbar->cb_arg = cb_arg;
  scrollbar->wgt.priv = scrollbar;

  scrollbar->wgt.ggt.priv = &(scrollbar->wgt);
  scrollbar->wgt.ggt.get_min_width = pbt_wgt_scrollbar_get_size;
  scrollbar->wgt.ggt.get_max_width = pbt_wgt_scrollbar_get_size;
  scrollbar->wgt.ggt.get_min_height = pbt_ggt_return_zero;
  scrollbar->wgt.ggt.get_max_height = pbt_ggt_return_zero;
  scrollbar->wgt.ggt.update_area_cb = pbt_ggt_memcpy_area;
  scrollbar->wgt.ggt.draw_cb = pbt_wgt_vscrollbar_draw_cb;
  scrollbar->wgt.ggt.destroy_cb = pbt_wgt_evnode_destroy;

  scrollbar->wgt.init_ev_cb = pbt_wgt_vscrollbar_init_ev;
}
