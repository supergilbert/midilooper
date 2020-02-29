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

#include "wbe_pbw_inc.h"
#include "wbe_gl.h"
#include "wbe_glfw.h"
#include "pbt_pixel_buffer.h"

void wbe_pbw_update_size(wbe_pbw_t *win,
                         unsigned int width,
                         unsigned int height)
{
  pbt_pixbuf_resize(&(win->buffer), width, height);
  wbe_pbw_make_context(win);
  _wbe_gl_resize(width, height);
}

void wbe_pbw_set_size(wbe_pbw_t *win,
                      unsigned int width,
                      unsigned int height)
{
  /* wbe_pbw_make_context(win); */
  _wbe_window_set_size(win->win_be, width, height);
  wbe_pbw_update_size(win, width, height);
}

void _wbe_pbw_texture_load(wbe_pbw_t *win)
{
  /* wbe_gl_bind_texture(win->texture_id); */
  _wbe_gl_texture_load(win->buffer.pixels,
                       win->buffer.width,
                       win->buffer.height);
}

void wbe_pbw_put_buffer(wbe_pbw_t *win)
{
  wbe_pbw_make_context(win);
  _wbe_gl_texture_load(win->buffer.pixels,
                       win->buffer.width,
                       win->buffer.height);
  wbe_gl_texture_refresh();
  wbe_gl_flush();
  /* wbe_window_swap_buffer(win->win_be); */
}

void wbe_pbw_refresh(wbe_pbw_t *win)
{
  wbe_pbw_make_context(win);
  /* wbe_gl_bind_texture(win->texture_id); */
  wbe_gl_texture_refresh();
  wbe_gl_flush();
  /* wbe_window_swap_buffer(win->win_be); */
}

void wbe_pbw_resize_cb(unsigned int width,
                       unsigned int height,
                       void *win_addr)
{
  wbe_pbw_t *win = win_addr;

  wbe_pbw_update_size(win, width, height);
}

void wbe_pbw_refresh_cb(void *win_addr)
{
  wbe_pbw_t *win = win_addr;

  wbe_pbw_refresh(win);
}

void wbe_pbw_destroy(wbe_pbw_t *win)
{
  wbe_window_destroy(win->win_be);
  pbt_pixbuf_destroy(&(win->buffer));
}

wbe_bool_t wbe_pbw_init(wbe_pbw_t *win,
                        const char *name,
                        unsigned int width,
                        unsigned int height,
                        wbe_bool_t resizeable)
{
  wbe_next_windows_single_buffer();
  wbe_window_init(&(win->win_be), name, width, height, resizeable);
  wbe_pbw_make_context(win);
  wbe_gl_texture_init();
  pbt_pixbuf_init(&(win->buffer), width, height);
  wbe_gl_gen_texture(&(win->texture_id));
  if (resizeable == WBE_TRUE)
    wbe_window_add_resize_cb(win->win_be, wbe_pbw_resize_cb, win);
  wbe_window_add_refresh_cb(win->win_be, wbe_pbw_refresh_cb, win);
  return WBE_TRUE;
}

void wbe_pbw_backend_destroy(void)
{
  wbe_gl_texture_destroy();
  wbe_window_backend_destroy();
}
