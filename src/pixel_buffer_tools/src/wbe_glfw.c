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

#include <stdlib.h>             /* free */
#include <string.h>             /* memset */

#include "wbe_glfw_inc.h"
#include "wbe_gl.h"
#include "pbt_cursor_inc.h"

static wbe_window_backend_t _wbe_window_backend = {.initialised = WBE_FALSE};

void wbe_backend_evhdl_node_free(wbe_window_evhdl_node_t *evhdl_node)
{
  wbe_window_input_cb_node_t *input_cb_node, *input_cb_next;
  wbe_window_resize_cb_node_t *resize_cb_node, *resize_cb_next;
  wbe_window_focus_cb_node_t *focus_cb_node, *focus_cb_next;
  wbe_window_refresh_cb_node_t *refresh_cb_node, *refresh_cb_next;

  for (input_cb_node = evhdl_node->input_head;
       input_cb_node != NULL;
       input_cb_node = input_cb_next)
    {
      input_cb_next = input_cb_node->next;
      free(input_cb_node);
    }
  for (resize_cb_node = evhdl_node->resize_head;
       resize_cb_node != NULL;
       resize_cb_node = resize_cb_next)
    {
      resize_cb_next = resize_cb_node->next;
      free(resize_cb_node);
    }
  for (focus_cb_node = evhdl_node->focus_head;
       focus_cb_node != NULL;
       focus_cb_node = focus_cb_next)
    {
      focus_cb_next = focus_cb_node->next;
      free(focus_cb_node);
    }
  for (refresh_cb_node = evhdl_node->refresh_head;
       refresh_cb_node != NULL;
       refresh_cb_node = refresh_cb_next)
    {
      refresh_cb_next = refresh_cb_node->next;
      free(refresh_cb_node);
    }
  free(evhdl_node);
}

void wbe_window_backend_destroy(void)
{
  wbe_window_evhdl_node_t *evhdl_node, *evhdl_next;

  if (_wbe_window_backend.initialised == WBE_FALSE)
    return;

  glfwTerminate();
  _wbe_window_backend.initialised = WBE_FALSE;
  evhdl_node = _wbe_window_backend.winevhdl_head;
  while (evhdl_node != NULL)
    {
      evhdl_next = evhdl_node->next;
      wbe_backend_evhdl_node_free(evhdl_node);
      evhdl_node = evhdl_next;
    }
}

wbe_window_evhdl_node_t *wbe_window_find_evhdl(GLFWwindow *glfw_win)
{
  wbe_window_evhdl_node_t *node;

  for (node = _wbe_window_backend.winevhdl_head;
       node != NULL;
       node = node->next)
    if (node->win == glfw_win)
      return node;
  return NULL;
}

void wbe_glfw_callback_cursor_pos(GLFWwindow *glfw_win,
                                  double xpos,
                                  double ypos)
{
  wbe_window_evhdl_node_t *node = wbe_window_find_evhdl(glfw_win);

  if (node == NULL)
    return;

  node->new_input = WBE_TRUE;

  node->input.xpos = (int) xpos;
  node->input.ypos = (int) ypos;
}

void wbe_glfw_callback_cursor_buttons(GLFWwindow *glfw_win,
                                      int buttons,
                                      int action,
                                      int mods)
{
  wbe_window_evhdl_node_t *node = wbe_window_find_evhdl(glfw_win);

  if (node == NULL)
    return;

  node->new_input = WBE_TRUE;

  WBE_SET_BIT(node->input.buttons, buttons, action == 1 ? 1 : 0);
}

void wbe_glfw_callback_cursor_scroll(GLFWwindow *glfw_win,
                                     double xoffset,
                                     double yoffset)
{
  wbe_window_evhdl_node_t *node = wbe_window_find_evhdl(glfw_win);

  if (node == NULL)
    return;

  node->new_input = WBE_TRUE;

  if (yoffset < (double) 0)
    WBE_SET_BIT(node->input.buttons, 3, 1);
  else if (yoffset > (double) 0)
    WBE_SET_BIT(node->input.buttons, 4, 1);
}

#define wbe_key_set(_keys, _idx, _bool)                 \
  WBE_SET_BIT((_keys)[(_idx) / 8], (_idx) % 8, (_bool))

void wbe_key_unset_all(wbe_byte_t *keys)
{
  wbe_byte_t *end = keys + WBE_KEYMAP_LEN;

  while (keys < end)
    {
      *keys = 0;
      keys++;
    }
}

void wbe_glfw_callback_key(GLFWwindow *glfw_win,
                           int key,
                           int scancode,
                           int action,
                           int mods)
{
  wbe_window_evhdl_node_t *node = wbe_window_find_evhdl(glfw_win);
  wbe_bool_t wbe_action;

  if (node == NULL)
    return;

  node->new_input = WBE_TRUE;

  if (action == GLFW_PRESS)
    wbe_action = WBE_TRUE;
  else
    wbe_action = WBE_FALSE;

  if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
    {
      if (_wbe_window_backend.key_layout == WBE_KEY_LAYOUT_FR)
        {
          /* binding glfw key input event for ctrl-z ctrl-a ... */
          if (key == GLFW_KEY_A)
            key = GLFW_KEY_Q;
          else if (key == GLFW_KEY_Q)
            key = GLFW_KEY_A;
          else if (key == GLFW_KEY_W)
            key = GLFW_KEY_Z;
          else if (key == GLFW_KEY_Z)
            key = GLFW_KEY_W;
          /* other keys are not needed for remappping */
        }
      wbe_key_set(node->input.keys,
                  WBE_KEY_A + (key - GLFW_KEY_A),
                  wbe_action);
    }
  else
    {
      switch (key)
        {
        case GLFW_KEY_ESCAPE:
          wbe_key_set(node->input.keys, WBE_KEY_ESCAPE, wbe_action);
          break;
        case GLFW_KEY_SPACE:
          wbe_key_set(node->input.keys, WBE_KEY_SPACE, wbe_action);
          break;
        case GLFW_KEY_BACKSPACE:
          wbe_key_set(node->input.keys, WBE_KEY_BSPACE, wbe_action);
          break;
        case GLFW_KEY_DELETE:
          wbe_key_set(node->input.keys, WBE_KEY_SUPPR, wbe_action);
          break;
        case GLFW_KEY_ENTER:
          wbe_key_set(node->input.keys, WBE_KEY_ENTER, wbe_action);
          break;
        case GLFW_KEY_LEFT:
          wbe_key_set(node->input.keys, WBE_KEY_LEFT, wbe_action);
          break;
        case GLFW_KEY_RIGHT:
          wbe_key_set(node->input.keys, WBE_KEY_RIGHT, wbe_action);
          break;
        case GLFW_KEY_UP:
          wbe_key_set(node->input.keys, WBE_KEY_UP, wbe_action);
          break;
        case GLFW_KEY_DOWN:
          wbe_key_set(node->input.keys, WBE_KEY_DOWN, wbe_action);
          break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
          wbe_key_set(node->input.keys, WBE_KEY_SHIFT, wbe_action);
          break;
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
          wbe_key_set(node->input.keys, WBE_KEY_CONTROL, wbe_action);
          break;
        case GLFW_KEY_LEFT_ALT:
        case GLFW_KEY_RIGHT_ALT:
          wbe_key_set(node->input.keys, WBE_KEY_ALT, wbe_action);
          break;
        case GLFW_KEY_LEFT_SUPER:
        case GLFW_KEY_RIGHT_SUPER:
          wbe_key_set(node->input.keys, WBE_KEY_SUPER, wbe_action);
          break;
        default:
          ;
        }
    }
}

void wbe_glfw_callback_resize(GLFWwindow *glfw_win, int width, int height)
{
  wbe_window_evhdl_node_t *node = wbe_window_find_evhdl(glfw_win);

  if (node == NULL)
    return;

  node->resized = WBE_TRUE;
  node->width = width;
  node->height = height;
}

void wbe_glfw_callback_focus(GLFWwindow *glfw_win, int focused)
{
  wbe_window_evhdl_node_t *node = wbe_window_find_evhdl(glfw_win);

  if (node == NULL)
    return;

  node->focus_changed = WBE_TRUE;
  if (focused == GLFW_TRUE)
    node->focus_state = WBE_TRUE;
  else
    {
      wbe_key_unset_all(node->input.keys);
      node->focus_state = WBE_FALSE;
    }
}

void wbe_glfw_callback_refresh(GLFWwindow *glfw_win)
{
  wbe_window_evhdl_node_t *win_node = wbe_window_find_evhdl(glfw_win);

  if (win_node == NULL)
    return;

  win_node->need_refresh = WBE_TRUE;
}

void wbe_window_backend_set_key_layout(wbe_key_layout_t key_layout)
{
  _wbe_window_backend.key_layout = key_layout;
}

wbe_bool_t wbe_window_backend_init(void)
{
  if (_wbe_window_backend.initialised == WBE_TRUE)
    return WBE_FALSE;
  if (glfwInit() == GLFW_FALSE)
    return WBE_FALSE;

  _wbe_window_backend.winevhdl_head = NULL;
  /* _wbe_window_backend.key_layout = WBE_KEY_LAYOUT_FR; */
  _wbe_window_backend.initialised = WBE_TRUE;
  return WBE_TRUE;
}

void wbe_backend_remove_win_event(wbe_window_t *win)
{
  wbe_window_evhdl_node_t *evhdl_node, *evhdl_next;

  evhdl_node = _wbe_window_backend.winevhdl_head;
  if (evhdl_node->win == win)
    {
      _wbe_window_backend.winevhdl_head = evhdl_node->next;
      wbe_backend_evhdl_node_free(evhdl_node);
      return;
    }
  while (evhdl_node->next != NULL)
    {
      evhdl_next = evhdl_node->next;
      if (evhdl_next->win == win)
        {
          evhdl_node->next = evhdl_next->next;
          wbe_backend_evhdl_node_free(evhdl_next);
          return;
        }
      evhdl_node = evhdl_next;
    }
}

void wbe_window_destroy(wbe_window_t *win)
{
  wbe_backend_remove_win_event(win);
  glfwDestroyWindow(win);
}

void wbe_window_init(wbe_window_t **win,
                     const char *name,
                     unsigned int width,
                     unsigned int height,
                     wbe_bool_t resizeable)
{
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE,
                 resizeable == WBE_TRUE ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  if (_wbe_window_backend.first_win == NULL)
    {
      *win = glfwCreateWindow(width, height, name, NULL, NULL);
      _wbe_window_backend.first_win = *win;
    }
  else
    *win = glfwCreateWindow(width,
                            height,
                            name,
                            NULL,
                            _wbe_window_backend.first_win);
}

void wbe_window_map(wbe_window_t *win)
{
  glfwSetWindowShouldClose(win, GLFW_FALSE);
  glfwShowWindow(win);
}

void wbe_window_add_input_cb(wbe_window_t *win,
                             wbe_window_input_cb_t callback,
                             void *arg)
{
  wbe_window_input_cb_node_t *cb_node =
    malloc(sizeof (wbe_window_input_cb_node_t));
  wbe_window_evhdl_node_t *winevhdl_node = wbe_window_find_evhdl(win);

  if (winevhdl_node == NULL)
    {
      winevhdl_node = malloc(sizeof (wbe_window_evhdl_node_t));
      memset(winevhdl_node, 0, sizeof (wbe_window_evhdl_node_t));
      winevhdl_node->win = win;
      winevhdl_node->next = _wbe_window_backend.winevhdl_head;
      _wbe_window_backend.winevhdl_head = winevhdl_node;
    }

  cb_node->callback = callback;
  cb_node->arg = arg;
  cb_node->next = winevhdl_node->input_head;
  if (winevhdl_node->input_head == NULL)
    {
      glfwSetCursorPosCallback(win,
                               wbe_glfw_callback_cursor_pos);
      glfwSetMouseButtonCallback(win,
                                 wbe_glfw_callback_cursor_buttons);
      glfwSetScrollCallback(win,
                            wbe_glfw_callback_cursor_scroll);
      glfwSetKeyCallback(win,
                         wbe_glfw_callback_key);
    }
  winevhdl_node->input_head = cb_node;
}

void wbe_window_add_resize_cb(wbe_window_t *win,
                              wbe_window_resize_cb_t callback,
                              void *arg)
{
  wbe_window_resize_cb_node_t *cb_node =
    malloc(sizeof (wbe_window_resize_cb_node_t)),
    *last_node;
  wbe_window_evhdl_node_t *winevhdl_node =
    wbe_window_find_evhdl(win);

  if (winevhdl_node == NULL)
    {
      winevhdl_node = malloc(sizeof (wbe_window_evhdl_node_t));
      memset(winevhdl_node, 0, sizeof (wbe_window_evhdl_node_t));
      winevhdl_node->win = win;
      winevhdl_node->next = _wbe_window_backend.winevhdl_head;
      _wbe_window_backend.winevhdl_head = winevhdl_node;
    }

  cb_node->callback = callback;
  cb_node->arg = arg;
  cb_node->next = NULL;
  if (winevhdl_node->resize_head == NULL)
    {
      glfwSetFramebufferSizeCallback(win, wbe_glfw_callback_resize);
      /* glfwSetWindowSizeCallback(win, wbe_glfw_callback_resize); */
      winevhdl_node->resize_head = cb_node;
    }
  else
    {
      last_node = winevhdl_node->resize_head;
      while (last_node->next != NULL)
        last_node = last_node->next;
      last_node->next = cb_node;
    }
}

void wbe_window_add_focus_cb(wbe_window_t *win,
                             wbe_window_focus_cb_t callback,
                             void *arg)
{
  wbe_window_focus_cb_node_t *cb_node =
    malloc(sizeof (wbe_window_focus_cb_node_t));
  wbe_window_evhdl_node_t *winevhdl_node =
    wbe_window_find_evhdl(win);

  if (winevhdl_node == NULL)
    {
      winevhdl_node = malloc(sizeof (wbe_window_evhdl_node_t));
      memset(winevhdl_node, 0, sizeof (wbe_window_evhdl_node_t));
      winevhdl_node->win = win;
      winevhdl_node->next = _wbe_window_backend.winevhdl_head;
      _wbe_window_backend.winevhdl_head = winevhdl_node;
    }

  cb_node->callback = callback;
  cb_node->arg = arg;
  cb_node->next = winevhdl_node->focus_head;
  if (winevhdl_node->focus_head == NULL)
    glfwSetWindowFocusCallback(win, wbe_glfw_callback_focus);
  winevhdl_node->focus_head = cb_node;
}

void wbe_window_add_refresh_cb(wbe_window_t *win,
                             wbe_window_refresh_cb_t callback,
                             void *arg)
{
  wbe_window_refresh_cb_node_t *cb_node =
    malloc(sizeof (wbe_window_refresh_cb_node_t));
  wbe_window_evhdl_node_t *winevhdl_node =
    wbe_window_find_evhdl(win);

  if (winevhdl_node == NULL)
    {
      winevhdl_node = malloc(sizeof (wbe_window_evhdl_node_t));
      memset(winevhdl_node, 0, sizeof (wbe_window_evhdl_node_t));
      winevhdl_node->win = win;
      winevhdl_node->next = _wbe_window_backend.winevhdl_head;
      _wbe_window_backend.winevhdl_head = winevhdl_node;
    }

  cb_node->callback = callback;
  cb_node->arg = arg;
  cb_node->next = winevhdl_node->refresh_head;
  if (winevhdl_node->refresh_head == NULL)
    glfwSetWindowRefreshCallback(win, wbe_glfw_callback_refresh);
  winevhdl_node->refresh_head = cb_node;
}

void wbe_window_handle_events(void)
{
  wbe_window_evhdl_node_t *winevhdl_node = _wbe_window_backend.winevhdl_head;
  wbe_window_input_cb_node_t *input_cb_node;
  wbe_window_resize_cb_node_t *resize_cb_node;
  wbe_window_focus_cb_node_t *focus_cb_node;
  wbe_window_refresh_cb_node_t *refresh_cb_node;

  glfwPollEvents();

  while (winevhdl_node != NULL)
    {
      if (winevhdl_node->new_input == WBE_TRUE)
        {
          for (input_cb_node = winevhdl_node->input_head;
               input_cb_node != NULL;
               input_cb_node = input_cb_node->next)
            input_cb_node->callback(&(winevhdl_node->input),
                                    input_cb_node->arg);
          winevhdl_node->new_input = WBE_FALSE;
          WBE_SET_BIT(winevhdl_node->input.buttons, 3, 0);
          WBE_SET_BIT(winevhdl_node->input.buttons, 4, 0);
        }

      if (winevhdl_node->resized == WBE_TRUE)
        {
          for (resize_cb_node = winevhdl_node->resize_head;
               resize_cb_node != NULL;
               resize_cb_node = resize_cb_node->next)
            resize_cb_node->callback(winevhdl_node->width,
                                     winevhdl_node->height,
                                     resize_cb_node->arg);
          winevhdl_node->resized = WBE_FALSE;
        }

      if (winevhdl_node->focus_changed == WBE_TRUE)
        {
          for (focus_cb_node = winevhdl_node->focus_head;
               focus_cb_node != NULL;
               focus_cb_node = focus_cb_node->next)
            focus_cb_node->callback(winevhdl_node->focus_state,
                                    focus_cb_node->arg);
          winevhdl_node->focus_changed = WBE_FALSE;
        }

      if (winevhdl_node->need_refresh == WBE_TRUE)
        {
          for (refresh_cb_node = winevhdl_node->refresh_head;
               refresh_cb_node != NULL;
               refresh_cb_node = refresh_cb_node->next)
            refresh_cb_node->callback(refresh_cb_node->arg);
          winevhdl_node->need_refresh = WBE_FALSE;
        }

      winevhdl_node = winevhdl_node->next;
    }
}

wbe_cursor_t *wbe_cursor_init(pbt_cursor_t *pbt_cursor)
{
  GLFWimage image = {.width = pbt_cursor->pixbuf.width,
                     .height = pbt_cursor->pixbuf.height,
                     .pixels = pbt_cursor->pixbuf.pixels};
  return glfwCreateCursor(&image, pbt_cursor->xhot, pbt_cursor->yhot);
}
