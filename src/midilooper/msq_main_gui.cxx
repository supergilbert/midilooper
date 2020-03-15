// Copyright 2012-2020 Gilbert Romer

// This file is part of midilooper.

// midilooper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// midilooper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU Gneneral Public License
// along with midilooper.  If not, see <http://www.gnu.org/licenses/>.

extern "C"
{
#include <pbt_gadget.h>
#include <pbt_font.h>
#include <pbt_draw.h>
#include <wbe_glfw.h>
#include <pbt_gadget_window.h>
#include <pbt_event_handler.h>

#include "msq_gui.h"
#include "msq_track_editor.h"
}

#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libconfig.h>

#define TMP_DEFAULT_WIDTH 300

using namespace std;

#include "msq_imgui.hxx"

typedef struct
{
  engine_ctx_t *engine_ctx;
  msq_gui_theme_t *theme;
  pbt_ggt_drawarea_t header;
  pbt_ggt_ctnr_t vlist;
  pbt_ggt_ctnr_t vctnr;
  pbt_ggt_ctnr_t hctnr_button;
  pbt_wgt_button_t add_button;
  void *main_win_addr;
  pbt_ggt_node_t vctnr_node;
  pbt_ggt_t ggt;
} msq_list_t;

class midilooper_main_window
{
  track_editor_theme_t track_editor_theme = {};
  pbt_ggt_ctnr_t root_vctnr = {};
  msq_transport_iface_t transport_iface = {};
  msq_margin_ggt_t margin = {};
  pbt_ggt_ctnr_t root_child_vctnr = {};
  bool wait_key_binding = false;
  msq_time_t wait_since;

  char **gen_output_str_list(size_t *str_list_len);

public:
  track_ctx_t *track_waiting_binding = NULL;
  string save_path;
  engine_ctx_t *engine_ctx = NULL;
  msq_gui_theme_t theme = {};
  pbt_ggt_win_t ggt_win = {};
  pbt_ggt_ctnr_t menubar_hctnr = {};
  pbt_wgt_button_t menubar_file_button = {};
  bool should_close = false;
  msq_dialog_iface_t dialog_iface = {};
  output_t *dialog_output;
  track_editor_t *dialog_track;
  msq_list_t output_list = {};
  msq_list_t track_list = {};
  msq_imgui_dialog *imgui_dialog = NULL;

  void _init_main_window(char *filename);
  midilooper_main_window(void);
  midilooper_main_window(char *filename);
  ~midilooper_main_window(void);
  void handle_dialog(void);
  void handle_windows(void);
  bool closed(void);
  void show_filemenu_dialog(void);
  void show_output_dialog(output_t *output);
  void show_set_output(track_editor_t *track_editor);
  void show_track_dialog(track_editor_t *track_editor);
  void rename_track(track_editor_t *track_editor, char *new_name);
  output_t *get_output(size_t idx);
  void add_output(char *new_name);
  void add_track(char *new_name);
  void remove_output(output_t *output);
  void remove_track(track_editor_t *track_editor);
  void save_file_as(const char *str);
  void set_add_key_binding_mode(void);
  void show_add_track(void);
  void show_add_output(void);
  void redraw(void);
  void refresh(void);
  void handle_key_bindings(wbe_window_input_t *winev);
  void run(void);
};

void _msq_draw_list_title(pbt_pbarea_t *pbarea,
                          const char *title,
                          msq_gui_theme_t *theme)
{
  unsigned int width, size, line_ypos;

  pbt_font_get_string_width(&(theme->theme.font), title, &width);
  size = (pbarea->width / 2) -  (width / 2);
  line_ypos = theme->theme.font.max_height / 2;

  pbt_pbarea_put_hline(pbarea,
                       0, line_ypos,
                       size,
                       theme->theme.window_fg);
  pbt_pbarea_printf(pbarea, &(theme->theme.font),
                    theme->theme.window_fg,
                    size, 0,
                    title);
  pbt_pbarea_put_hline(pbarea,
                       pbarea->width - size, line_ypos,
                       size,
                       theme->theme.window_fg);
}

#define OUTPUTS_TITLE " OUTPUTS "

typedef struct
{
  output_t *output;
  midilooper_main_window *main_win;
  pbt_wgt_t wgt;
} msq_output_node_t;

void msq_output_node_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  msq_output_node_t *output_node = (msq_output_node_t *) wgt->priv;

  pbt_pbarea_fill(&(ggt->pbarea),
                  output_node->main_win->theme.theme.frame_bg);
  pbt_pbarea_printf(&(ggt->pbarea),
                    &(output_node->main_win->theme.theme.font),
                    output_node->main_win->theme.theme.frame_fg,
                    0, 0,
                    output_get_name(output_node->output));
}

unsigned int msq_output_node_get_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  msq_output_node_t *output_node = (msq_output_node_t *) wgt->priv;

  return output_node->main_win->theme.theme.font.max_height * 2;
}

pbt_bool_t msq_output_node_set_focus_cb(pbt_ggt_t *ggt,
                                        wbe_window_input_t *winev,
                                        void *output_node_addr)
{
  msq_output_node_t *output_node = (msq_output_node_t *) output_node_addr;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_FALSE)
    return PBT_FALSE;

  if (WBE_GET_BIT(winev->buttons, 1) == 1)
    {
      output_node->main_win->show_output_dialog(output_node->output);
      return PBT_TRUE;
    }
  return PBT_FALSE;
}

pbt_bool_t msq_output_node_unset_focus_cb(pbt_ggt_t *ggt,
                                        wbe_window_input_t *winev,
                                        void *output_node_addr)
{
  if (WBE_GET_BIT(winev->buttons, 1) == 0)
    return PBT_TRUE;
  return PBT_FALSE;
}

void msq_output_node_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  wgt->ggt_win = ggt_win;
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           msq_output_node_set_focus_cb,
                           wgt->priv);
  pbt_evh_add_unset_focus_cb(&(ggt_win->evh),
                             &(wgt->ggt),
                             msq_output_node_unset_focus_cb,
                             wgt->priv);
}

void msq_output_node_init(msq_output_node_t *output_node,
                          output_t *output,
                          midilooper_main_window *main_win)
{
  output_node->output = output;
  output_node->main_win = main_win;

  output_node->wgt.priv = output_node;

  output_node->wgt.ggt.priv = &(output_node->wgt);
  output_node->wgt.ggt.get_min_width = pbt_ggt_return_zero;
  output_node->wgt.ggt.get_max_width = pbt_ggt_return_zero;
  output_node->wgt.ggt.get_min_height = msq_output_node_get_height;
  output_node->wgt.ggt.get_max_height = msq_output_node_get_height;
  output_node->wgt.ggt.update_area_cb = pbt_ggt_memcpy_area;;
  output_node->wgt.ggt.draw_cb = msq_output_node_draw_cb;
  output_node->wgt.ggt.destroy_cb = pbt_wgt_evnode_destroy;

  output_node->wgt.init_ev_cb = msq_output_node_init_ev;
}

typedef struct
{
  msq_output_node_t output_node;
  pbt_ggt_ctnr_t vctnr;
  pbt_ggt_node_it_t node_it;
} msq_output_line_t;

pbt_ggt_node_t *msq_output_node_add(msq_list_t *output_list,
                                    output_t *output)
{
  msq_output_line_t *output_line =
    (msq_output_line_t *) malloc(sizeof (msq_output_line_t));
  pbt_ggt_t *ggt_wrapper = (pbt_ggt_t *) malloc(sizeof (pbt_ggt_t));

  memset(output_line, 0, sizeof (msq_output_line_t));
  memset(ggt_wrapper, 0, sizeof (pbt_ggt_t));

  msq_output_node_init(&(output_line->output_node),
                       output,
                       (midilooper_main_window *) output_list->main_win_addr);
  pbt_ggt_vctnr_init(&(output_line->vctnr));
  pbt_ggt_ctnr_add_static_separator(&(output_line->vctnr),
                                    output_list->theme->default_separator,
                                    output_list->theme->theme.window_bg);
  pbt_ggt_add_child_wgt(&(output_line->vctnr), &(output_line->output_node));

  pbt_ggt_wrapper_init(ggt_wrapper,
                       output_line,
                       &(output_line->vctnr.ggt),
                       GADGET);

  pbt_ggt_node_it_init_ggt_add_child(&(output_line->node_it),
                                     &(output_list->vlist.ggt),
                                     ggt_wrapper,
                                     GADGET);
  return  output_line->node_it.node;
}

void msq_draw_output_header_cb(pbt_pbarea_t *pbarea, void *output_list_addr)
{
  msq_list_t *output_list = (msq_list_t *) output_list_addr;

  pbt_pbarea_fill(pbarea, output_list->theme->theme.window_bg);
  _msq_draw_list_title(pbarea,
                       OUTPUTS_TITLE,
                       output_list->theme);
}

void add_output_dialog_cb(void *main_win_addr)
{
  midilooper_main_window *main_win =
    (midilooper_main_window *) main_win_addr;

  main_win->show_add_output();
}

void msq_output_list_del_node(msq_list_t *output_list,
                              output_t *output)
{
  pbt_ggt_node_t *ggt_node;
  pbt_ggt_t *ggt_wrapper;
  msq_output_line_t *output_line;

  for (ggt_node = output_list->vlist.ggt.childs;
       ggt_node != NULL;
       ggt_node = ggt_node->next)
    {
      ggt_wrapper = (pbt_ggt_t *) ggt_node->priv.ggt_addr;
      output_line = (msq_output_line_t *) ggt_wrapper->priv;
      if (output_line->output_node.output == output)
        {
          pbt_ggt_node_it_del(&(output_line->node_it));
          return;
        }
    }
}

void msq_list_destroy_cb(pbt_ggt_t *ggt)
{
  pbt_ggt_t *vctnr_ggt = (pbt_ggt_t *) ggt->childs->priv.ggt_addr;

  _pbt_ggt_destroy(vctnr_ggt);
}

void msq_output_list_init(msq_list_t *msq_list,
                          midilooper_main_window *main_win)
{
  unsigned int width;

  msq_list->engine_ctx = main_win->engine_ctx;
  msq_list->theme = &(main_win->theme);
  msq_list->main_win_addr = main_win;

  pbt_font_get_string_width(&(msq_list->theme->theme.font),
                            "-" OUTPUTS_TITLE "-",
                            &width);
  pbt_ggt_drawarea_init(&(msq_list->header),
                        width, 0,
                        msq_list->theme->theme.font.max_height,
                        msq_list->theme->theme.font.max_height,
                        msq_draw_output_header_cb,
                        msq_list,
                        NULL, NULL);

  pbt_ggt_vctnr_init(&(msq_list->vlist));
  pbt_ggt_add_child_ggt(&(msq_list->vlist), &(msq_list->header));
  msq_button_plus_init(&(msq_list->add_button),
                       msq_list->theme,
                       add_output_dialog_cb,
                       main_win);
  pbt_ggt_hctnr_init(&(msq_list->hctnr_button));
  pbt_ggt_add_child_wgt(&(msq_list->hctnr_button), &(msq_list->add_button));
  pbt_ggt_ctnr_add_empty(&(msq_list->hctnr_button),
                         msq_list->theme->theme.window_bg);
  pbt_ggt_vctnr_init(&(msq_list->vctnr));
  pbt_ggt_add_child_ggt(&(msq_list->vctnr), &(msq_list->vlist));
  pbt_ggt_ctnr_add_static_separator(&(msq_list->vctnr),
                                    msq_list->theme->default_separator,
                                    msq_list->theme->theme.window_bg);
  pbt_ggt_add_child_ggt(&(msq_list->vctnr), &(msq_list->hctnr_button));

  msq_list->ggt.get_min_width = pbt_ggt_wrapper_get_min_width;
  msq_list->ggt.get_max_width = pbt_ggt_wrapper_get_max_width;
  msq_list->ggt.get_min_height = pbt_ggt_wrapper_get_min_height;
  msq_list->ggt.get_max_height = pbt_ggt_wrapper_get_max_height;
  msq_list->ggt.draw_cb = pbt_ggt_wrapper_draw;
  msq_list->ggt.update_area_cb = pbt_ggt_wrapper_update_area;
  msq_list->ggt.priv = msq_list;
  msq_list->ggt.destroy_cb = msq_list_destroy_cb;
  msq_list->vctnr_node.type = GADGET;
  msq_list->vctnr_node.priv.ggt_addr = &(msq_list->vctnr.ggt);
  msq_list->vctnr_node.next = NULL;
  msq_list->ggt.childs = &(msq_list->vctnr_node);
}

#define TRACKS_TITLE " TRACKS "

typedef struct
{
  midilooper_main_window *main_win;
  track_editor_t *track_editor;
  pbt_wgt_t wgt;
} msq_track_node_t;

void msq_track_node_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  msq_track_node_t *track_node = (msq_track_node_t *) wgt->priv;
  track_ctx_t *track_ctx = track_node->track_editor->editor_ctx.track_ctx;
  unsigned int output_str_width;
  std::string output_name;
  byte_t string_key_bindings[MSQ_BINDINGS_KEY_MAX];
  size_t array_size;

  if (track_ctx->output != NULL)
    output_name = output_get_name(track_ctx->output);
  else
    output_name = "No output";

  pbt_font_get_string_width(&(track_node->main_win->theme.theme.font),
                            output_name.c_str(),
                            &output_str_width);

  array_size =
    _fill_byte_array_w_track_bindings(string_key_bindings,
                                      MSQ_BINDINGS_KEY_MAX,
                                      &(track_ctx->engine->bindings.keypress),
                                      track_ctx);

  string_key_bindings[array_size] = '\0';

  pbt_pbarea_fill(&(ggt->pbarea),
                  track_node->main_win->theme.theme.frame_bg);
  pbt_pbarea_printf(&(ggt->pbarea),
                    &(track_node->main_win->theme.theme.font),
                    track_node->main_win->theme.theme.frame_fg,
                    0, 0,
                    track_ctx->track->name);
  pbt_pbarea_printf(&(ggt->pbarea),
                    &(track_node->main_win->theme.theme.font),
                    track_node->main_win->theme.theme.frame_fg,
                    _pbt_ggt_width(ggt) - output_str_width, 0,
                    output_name.c_str());
  pbt_pbarea_printf(&(ggt->pbarea),
                    &(track_node->main_win->theme.theme.font),
                    track_node->main_win->theme.theme.frame_fg,
                    0, track_node->main_win->theme.theme.font.max_height,
                    "Bindings: %s", string_key_bindings);
  pbt_pbarea_fillrect(&(ggt->pbarea),
                      track_node->main_win->theme.default_separator,
                      _pbt_ggt_height(ggt)
                      - track_node->main_win->theme.theme.font.max_height,
                      _pbt_ggt_width(ggt)
                      - (track_node->main_win->theme.default_separator * 2),
                      track_node->main_win->theme.theme.font.max_height / 2,
                      track_node->main_win->theme.theme.window_bg);
}

unsigned int msq_track_node_get_width(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  msq_track_node_t *track_node = (msq_track_node_t *) wgt->priv;

  return track_node->main_win->theme.theme.font.max_height * 18;
}

unsigned int msq_track_node_get_height(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  msq_track_node_t *track_node = (msq_track_node_t *) wgt->priv;

  return track_node->main_win->theme.theme.font.max_height * 4;
}

pbt_bool_t msq_track_node_set_focus_cb(pbt_ggt_t *ggt,
                                       wbe_window_input_t *winev,
                                       void *track_node_addr)
{
  msq_track_node_t *track_node =
    (msq_track_node_t *) track_node_addr;
  track_editor_t *track_editor;

  if (_PBT_IS_IN_GGT(ggt, winev->xpos, winev->ypos) == PBT_FALSE)
    return PBT_FALSE;

  track_editor = track_node->track_editor;
  if (WBE_GET_BIT(winev->buttons, 0) == 1)
    {
      if (wbe_window_mapped(track_editor->ggt_win.pb_win.win_be) == WBE_TRUE)
        wbe_window_unmap(track_editor->ggt_win.pb_win.win_be);
      else
        {
          wbe_window_map(track_editor->ggt_win.pb_win.win_be);
          wbe_pbw_put_buffer(&(track_editor->ggt_win.pb_win));
        }
      return PBT_TRUE;
    }
  else if (WBE_GET_BIT(winev->buttons, 1) == 1)
    {
      track_node->main_win->show_track_dialog(track_editor);
      return PBT_TRUE;
    }
  return PBT_FALSE;
}

pbt_bool_t msq_track_node_unset_focus_cb(pbt_ggt_t *ggt,
                                         wbe_window_input_t *winev,
                                         void *track_node_addr)
{
  if ((WBE_GET_BIT(winev->buttons, 0) == 0)
      && (WBE_GET_BIT(winev->buttons, 1) == 0))
    return PBT_TRUE;
  return PBT_FALSE;
}

void msq_track_node_init_ev(pbt_wgt_t *wgt, pbt_ggt_win_t *ggt_win)
{
  wgt->ggt_win = ggt_win;
  pbt_evh_add_set_focus_cb(&(ggt_win->evh),
                           &(wgt->ggt),
                           msq_track_node_set_focus_cb,
                           wgt->priv);
  pbt_evh_add_unset_focus_cb(&(ggt_win->evh),
                             &(wgt->ggt),
                             msq_track_node_unset_focus_cb,
                             wgt->priv);
}

void msq_track_node_destroy(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  msq_track_node_t *track_node = (msq_track_node_t *) wgt->priv;

  pbt_wgt_evnode_destroy(ggt);
  pbt_ggt_win_destroy(&(track_node->track_editor->ggt_win));
}

void msq_track_node_init(msq_track_node_t *track_node,
                         track_editor_t *track_editor,
                         midilooper_main_window *main_win)
{
  track_node->track_editor = track_editor;
  track_node->main_win = main_win;

  track_node->wgt.priv = track_node;

  track_node->wgt.ggt.priv = &(track_node->wgt);
  track_node->wgt.ggt.get_min_width = msq_track_node_get_width;
  track_node->wgt.ggt.get_max_width = pbt_ggt_return_zero;
  track_node->wgt.ggt.get_min_height = msq_track_node_get_height;
  track_node->wgt.ggt.get_max_height = msq_track_node_get_height;
  track_node->wgt.ggt.update_area_cb = pbt_ggt_memcpy_area;;
  track_node->wgt.ggt.draw_cb = msq_track_node_draw_cb;
  track_node->wgt.ggt.destroy_cb = msq_track_node_destroy;

  track_node->wgt.init_ev_cb = msq_track_node_init_ev;
}

void msq_track_node_draw_progress(msq_track_node_t *track_node)
{
  track_editor_ctx_t *editor_ctx = &(track_node->track_editor->editor_ctx);
  unsigned int tick = engine_get_tick(editor_ctx->track_ctx->engine);
  unsigned size =
    _pbt_ggt_width(&(track_node->wgt.ggt))
    - (track_node->main_win->theme.default_separator * 2);
  unsigned xpos =
    size * (tick % editor_ctx->track_ctx->loop_len)
    / editor_ctx->track_ctx->loop_len;

  wbe_pbw_make_context(&(track_node->wgt.ggt_win->pb_win));
  pbt_wgt_gl_fillrect(&(track_node->wgt),
                      track_node->main_win->theme.default_separator,
                      pbt_ggt_height(&(track_node->wgt))
                      - track_node->main_win->theme.theme.font.max_height,
                      xpos,
                      track_node->main_win->theme.theme.font.max_height / 2,
                      track_node->main_win->theme.theme.frame_fg);
  pbt_wgt_gl_fillrect(&(track_node->wgt),
                      track_node->main_win->theme.default_separator + xpos,
                      pbt_ggt_height(&(track_node->wgt))
                      - track_node->main_win->theme.theme.font.max_height,
                      pbt_ggt_width(&(track_node->wgt))
                      - (track_node->main_win->theme.default_separator * 2)
                      - xpos,
                      track_node->main_win->theme.theme.font.max_height / 2,
                      track_node->main_win->theme.theme.window_bg);
  wbe_gl_flush();
}

typedef struct
{
  msq_track_node_t track_node;
  pbt_wgt_button_t mute_button;
  pbt_ggt_ctnr_t hctnr;
  pbt_ggt_ctnr_t vctnr;
  pbt_ggt_node_it_t node_it;
  track_editor_t track_editor;
} msq_track_line_t;

void msq_mute_button_cb(void *track_line_addr)
{
  msq_track_line_t *track_line = (msq_track_line_t *) track_line_addr;
  track_ctx_t *track_ctx = track_line->track_editor.editor_ctx.track_ctx;

  trackctx_toggle_mute(track_ctx);
}

void _msq_mute_button_draw_cb(pbt_ggt_t *ggt)
{
  pbt_wgt_t *wgt = (pbt_wgt_t *) ggt->priv;
  pbt_wgt_button_t *mute_button = (pbt_wgt_button_t *) wgt->priv;
  msq_track_line_t *track_line = (msq_track_line_t *) mute_button->cb_arg;
  pbt_pixbuf_t *track_mute_imgs =
    track_line->track_editor.editor_ctx.theme->global_theme->track_mute_imgs;
  track_ctx_t *track_ctx = track_line->track_editor.editor_ctx.track_ctx;

  if (track_ctx->mute == MSQ_FALSE)
    {
      if (mute_button->pb_released != &(track_mute_imgs[0]))
        {
          mute_button->pb_released = &(track_mute_imgs[0]);
          mute_button->pb_pressed = &(track_mute_imgs[1]);
          mute_button->pb_hovered = &(track_mute_imgs[2]);
          mute_button->bg_released = track_mute_imgs[0].pixels;
          mute_button->bg_pressed = track_mute_imgs[1].pixels;
          mute_button->bg_hovered = track_mute_imgs[2].pixels;
        }
    }
  else
    if (mute_button->pb_released != &(track_mute_imgs[1]))
      {
        mute_button->pb_released = &(track_mute_imgs[1]);
        mute_button->pb_pressed = &(track_mute_imgs[0]);
        mute_button->pb_hovered = &(track_mute_imgs[3]);
        mute_button->bg_released = track_mute_imgs[1].pixels;
        mute_button->bg_pressed = track_mute_imgs[0].pixels;
        mute_button->bg_hovered = track_mute_imgs[3].pixels;
      }

  pbt_wgt_button_draw_cb(ggt);
}

msq_track_line_t *msq_track_node_add(msq_list_t *track_list,
                                     track_ctx_t *track_ctx,
                                     track_editor_theme_t *track_editor_theme,
                                     msq_dialog_iface_t *dialog_iface,
                                     msq_transport_iface_t *transport_iface)
{
  msq_track_line_t *track_line =
    (msq_track_line_t *) malloc(sizeof (msq_track_line_t));
  pbt_ggt_t *ggt_wrapper = (pbt_ggt_t *) malloc(sizeof (pbt_ggt_t));

  memset(track_line, 0, sizeof (msq_track_line_t));
  memset(ggt_wrapper, 0, sizeof (pbt_ggt_t));

  track_editor_init(&(track_line->track_editor),
                    track_editor_theme,
                    dialog_iface,
                    transport_iface,
                    track_ctx,
                    100,
                    10,
                    track_ctx->engine->ppq / 4,
                    0);

  msq_track_node_init(&(track_line->track_node),
                      &(track_line->track_editor),
                      (midilooper_main_window *) track_list->main_win_addr);
  pbt_wgt_button_init(&(track_line->mute_button),
                      &(track_editor_theme->global_theme->track_mute_imgs[0]),
                      &(track_editor_theme->global_theme->track_mute_imgs[1]),
                      &(track_editor_theme->global_theme->track_mute_imgs[2]),
                      track_editor_theme->global_theme->track_mute_imgs[0].pixels,
                      track_editor_theme->global_theme->track_mute_imgs[1].pixels,
                      track_editor_theme->global_theme->track_mute_imgs[2].pixels,
                      msq_theme_cursor_finger(track_editor_theme->global_theme),
                      msq_theme_cursor_arrow(track_editor_theme->global_theme),
                      msq_mute_button_cb,
                      track_line);
  track_line->mute_button.wgt.ggt.draw_cb = _msq_mute_button_draw_cb;
  pbt_ggt_hctnr_init(&(track_line->hctnr));
  pbt_ggt_add_child_wgt(&(track_line->hctnr), &(track_line->track_node));
  pbt_ggt_ctnr_add_static_separator(&(track_line->hctnr),
                                    track_list->theme->default_separator,
                                    track_list->theme->theme.window_bg);
  pbt_ggt_add_child_wgt(&(track_line->hctnr), &(track_line->mute_button));

  pbt_ggt_vctnr_init(&(track_line->vctnr));
  pbt_ggt_ctnr_add_static_separator(&(track_line->vctnr),
                                    track_list->theme->default_separator,
                                    track_list->theme->theme.window_bg);
  pbt_ggt_add_child_ggt(&(track_line->vctnr), &(track_line->hctnr));

  pbt_ggt_wrapper_init(ggt_wrapper,
                       track_line,
                       &(track_line->vctnr.ggt),
                       GADGET);

  pbt_ggt_node_it_init_ggt_add_child(&(track_line->node_it),
                                     &(track_list->vlist.ggt),
                                     ggt_wrapper,
                                     GADGET);
  return track_line;
}

void msq_draw_track_header_cb(pbt_pbarea_t *pbarea, void *track_list_addr)
{
  msq_list_t *track_list = (msq_list_t *) track_list_addr;

  pbt_pbarea_fill(pbarea, track_list->theme->theme.window_bg);
  _msq_draw_list_title(pbarea,
                       TRACKS_TITLE,
                       track_list->theme);
}

void add_track_dialog_cb(void *main_win_addr)
{
  midilooper_main_window *main_win =
    (midilooper_main_window *) main_win_addr;

  main_win->show_add_track();
}

void msq_track_list_del_node(msq_list_t *track_list,
                             track_editor_t *track_editor)
{
  pbt_ggt_node_t *ggt_node;
  pbt_ggt_t *ggt_wrapper;
  msq_track_line_t *track_line;

  for (ggt_node = track_list->vlist.ggt.childs;
       ggt_node != NULL;
       ggt_node = ggt_node->next)
    {
      ggt_wrapper = (pbt_ggt_t *) ggt_node->priv.ggt_addr;
      track_line = (msq_track_line_t *) ggt_wrapper->priv;
      if (track_line->track_node.track_editor == track_editor)
        {
          pbt_ggt_node_it_del(&(track_line->node_it));
          return;
        }
    }
}

void msq_track_list_init(msq_list_t *msq_list,
                         midilooper_main_window *main_win)
{
  unsigned int width;

  msq_list->engine_ctx = main_win->engine_ctx;
  msq_list->theme = &(main_win->theme);
  msq_list->main_win_addr = main_win;

  // pbt_font_get_string_width(&(msq_list->theme->theme.font),
  //                           "-" TRACKS_TITLE "-",
  //                           &width);
  width = msq_list->theme->theme.font.max_height * 21
    + msq_list->theme->default_separator;
  pbt_ggt_drawarea_init(&(msq_list->header),
                        width, 0,
                        msq_list->theme->theme.font.max_height,
                        msq_list->theme->theme.font.max_height,
                        msq_draw_track_header_cb,
                        msq_list,
                        NULL, NULL);

  pbt_ggt_vctnr_init(&(msq_list->vlist));
  pbt_ggt_add_child_ggt(&(msq_list->vlist), &(msq_list->header));

  msq_button_plus_init(&(msq_list->add_button),
                       msq_list->theme,
                       add_track_dialog_cb,
                       main_win);
  pbt_ggt_hctnr_init(&(msq_list->hctnr_button));
  pbt_ggt_add_child_wgt(&(msq_list->hctnr_button), &(msq_list->add_button));
  pbt_ggt_ctnr_add_empty(&(msq_list->hctnr_button),
                         msq_list->theme->theme.window_bg);
  pbt_ggt_vctnr_init(&(msq_list->vctnr));
  pbt_ggt_add_child_ggt(&(msq_list->vctnr), &(msq_list->vlist));
  pbt_ggt_ctnr_add_static_separator(&(msq_list->vctnr),
                                    msq_list->theme->default_separator,
                                    msq_list->theme->theme.window_bg);
  pbt_ggt_add_child_ggt(&(msq_list->vctnr), &(msq_list->hctnr_button));

  msq_list->ggt.get_min_width = pbt_ggt_wrapper_get_min_width;
  msq_list->ggt.get_max_width = pbt_ggt_wrapper_get_max_width;
  msq_list->ggt.get_min_height = pbt_ggt_wrapper_get_min_height;
  msq_list->ggt.get_max_height = pbt_ggt_wrapper_get_max_height;
  msq_list->ggt.draw_cb = pbt_ggt_wrapper_draw;
  msq_list->ggt.update_area_cb = pbt_ggt_wrapper_update_area;
  msq_list->ggt.priv = msq_list;
  msq_list->ggt.destroy_cb = msq_list_destroy_cb;
  msq_list->vctnr_node.type = GADGET;
  msq_list->vctnr_node.priv.ggt_addr = &(msq_list->vctnr.ggt);
  msq_list->vctnr_node.next = NULL;
  msq_list->ggt.childs = &(msq_list->vctnr_node);
}

void add_track_result_cb(char *str, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  mainwin->add_track(str);
}

void add_output_result_cb(char *str, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  mainwin->add_output(str);
}

void save_file_cb(char *str, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  mainwin->save_file_as(str);
  // printf("str result: %s\n", str);
}

void filemenu_dialog_cb(size_t idx, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  if (idx == 0)
    {
      if (mainwin->save_path.empty())
        msq_dialog_filebrowser(&(mainwin->dialog_iface),
                               save_file_cb,
                               mainwin_addr);
      else
        mainwin->save_file_as(mainwin->save_path.c_str());
    }
  else if (idx == 1)
    msq_dialog_filebrowser(&(mainwin->dialog_iface),
                           save_file_cb,
                           mainwin_addr);
  else if (idx == 2)
    mainwin->should_close = true;
  return;
}

void output_rename_dialog_res_cb(char *str, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  output_set_name(mainwin->dialog_output,
                  str);
  mainwin->redraw();
}

void output_dialog_res_cb(size_t idx, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  if (idx == 0)
    msq_dialog_string_input(&(mainwin->dialog_iface),
                            output_rename_dialog_res_cb,
                            mainwin_addr);
  else if (idx == 1)
    mainwin->show_add_output();
  else if (idx == 2)
    mainwin->remove_output(mainwin->dialog_output);
}

void track_set_output_dialog_res_cb(size_t idx, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  if (idx == 0)
    mainwin->dialog_track->editor_ctx.track_ctx->output = NULL;
  else
    mainwin->dialog_track->editor_ctx.track_ctx->output =
      mainwin->get_output(idx - 1);
  pbt_ggt_draw(&(mainwin->track_list.vctnr));
  mainwin->refresh();
}

void track_rename_dialog_res_cb(char *str, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  trackctx_set_name(mainwin->dialog_track->editor_ctx.track_ctx,
                    str);
  pbt_ggt_draw(&(mainwin->track_list.vctnr));
  mainwin->refresh();
}

void track_dialog_res_cb(size_t idx, void *mainwin_addr)
{
  midilooper_main_window *mainwin =
    (midilooper_main_window *) mainwin_addr;

  switch (idx)
    {
    case 0:                     // Rename
      msq_dialog_string_input(&(mainwin->dialog_iface),
                              track_rename_dialog_res_cb,
                              mainwin_addr);
      break;
    case 1:                     // Output
      mainwin->show_set_output(mainwin->dialog_track);
      break;
    case 2:                     // Add key binding
      mainwin->set_add_key_binding_mode();
      break;
    case 3:                     // Delete key bindings
      engine_del_track_bindings(mainwin->engine_ctx,
                                mainwin->dialog_track->editor_ctx.track_ctx);
      pbt_ggt_draw(&(mainwin->track_list.vctnr));
      mainwin->refresh();
      break;
    case 4:                     // Add
      mainwin->show_add_track();
      break;
    case 5:                     // Remove
      mainwin->remove_track(mainwin->dialog_track);
      break;
    default:
      ;
    }
}

void show_filemenu_dialog_cb(void *main_window_addr)
{
  midilooper_main_window *main_window =
    (midilooper_main_window *) main_window_addr;

  main_window->show_filemenu_dialog();
}

#include "pbt_tools.h"
#include "wbe_gl.h"

void main_window_input_cb(wbe_window_input_t *winev, void *main_window_addr)
{
  midilooper_main_window *main_window =
    (midilooper_main_window *) main_window_addr;

  main_window->handle_key_bindings(winev);
  pbt_evh_handle(&(main_window->ggt_win.evh), winev);
}


void midilooper_main_window::_init_main_window(char *filename)
{
  list_iterator_t it;
  output_t *output = NULL;
  track_ctx_t *track_ctx;
  msq_track_line_t *track_line;
  int midifile_fd;
  midifile_t *midifile;

  if (filename != NULL)
    {
      save_path = filename;
      midifile_fd = open(filename, O_RDONLY);
      if (midifile_fd == -1)
        {
          fprintf(stderr, "Unable to open filename: %s\n", filename);
          abort();
        }
      midifile = read_midifile_fd(midifile_fd);
      engine_ctx = (engine_ctx_t *) malloc(sizeof (engine_ctx_t)); // TODO remove malloc
      init_engine(engine_ctx, (char *) "midilooper", 0, NULL);
      engine_read_midifile(engine_ctx, midifile);
      free_midifile(midifile);
      close(midifile_fd);
    }
  else
    {
      engine_ctx = (engine_ctx_t *) malloc(sizeof (engine_ctx_t)); // TODO bis remove malloc
      init_engine(engine_ctx, (char *) "midilooper", 0, NULL);
    }

  gui_default_theme_init(&theme);
  track_editor_default_theme_init(&track_editor_theme,
                                  &theme);

  _pbt_wgt_label_button_init(&menubar_file_button,
                             "File",
                             &(theme.theme.font),
                             theme.theme.window_fg,
                             theme.theme.window_bg,
                             theme.theme.wgt_activated_fg,
                             theme.theme.wgt_activated_bg,
                             theme.theme.frame_fg,
                             theme.theme.frame_bg,
                             theme.theme.cursor_finger,
                             theme.theme.cursor_arrow,
                             show_filemenu_dialog_cb,
                             this);
  pbt_ggt_hctnr_init(&menubar_hctnr);
  pbt_ggt_add_child_wgt(&menubar_hctnr, &menubar_file_button);
  pbt_ggt_ctnr_add_empty(&menubar_hctnr, theme.theme.window_bg);

  msq_transport_init(&transport_iface, &theme, engine_ctx);

  msq_output_list_init(&output_list, this);

  for (iter_init(&it, &(engine_ctx->output_list));
       iter_node(&it);
       iter_next(&it))
    {
      output = (output_t *) iter_node_ptr(&it);
      msq_output_node_add(&output_list, output);
    }

  msq_track_list_init(&track_list, this);

  pbt_ggt_vctnr_init(&root_child_vctnr);
  pbt_ggt_add_child_wgt(&root_child_vctnr, &transport_iface);
  pbt_ggt_ctnr_add_static_separator(&root_child_vctnr,
                                    theme.default_margin,
                                    theme.theme.window_bg);
  pbt_ggt_add_child_ggt(&root_child_vctnr, &output_list);
  pbt_ggt_ctnr_add_static_separator(&root_child_vctnr,
                                    theme.default_separator,
                                    theme.theme.window_bg);
  pbt_ggt_add_child_ggt(&root_child_vctnr, &(track_list));

  msq_margin_ggt_init_g(&margin,
                        &root_child_vctnr.ggt,
                        theme.default_margin,
                        theme.default_margin,
                        theme.default_margin,
                        theme.default_margin,
                        theme.theme.window_bg);

  pbt_ggt_vctnr_init(&root_vctnr);
  pbt_ggt_add_child_ggt(&root_vctnr, &menubar_hctnr);
  pbt_ggt_add_child_ggt(&root_vctnr, &margin);

  pbt_ggt_win_init(&ggt_win,
                   "Midilooper2",
                   &root_vctnr,
                   0, 0,
                   PBT_FALSE);
  wbe_gl_texture_n_color_init();
  wbe_gl_color_init();

  for (iter_init(&it, &(engine_ctx->track_list));
       iter_node(&it);
       iter_next(&it))
    {
      track_ctx = (track_ctx_t *) iter_node_ptr(&it);
      track_line = msq_track_node_add(&track_list,
                                    track_ctx,
                                    &track_editor_theme,
                                    &dialog_iface,
                                    &transport_iface);
      pbt_ggt_win_set_min_size(&ggt_win);
      pbt_ggt_win_init_child_ev(&ggt_win, track_line->node_it.node);
    }

  pbt_ggt_draw(&(root_vctnr));

  imgui_dialog = new msq_imgui_dialog(&(dialog_iface), &theme);

  wbe_window_map(ggt_win.pb_win.win_be);
  wbe_pbw_put_buffer(&(ggt_win.pb_win));
  wbe_window_add_input_cb(ggt_win.pb_win.win_be,
                          main_window_input_cb,
                          this);
}

midilooper_main_window::midilooper_main_window(char *filename)
{
  _init_main_window(filename);
}

midilooper_main_window::midilooper_main_window(void)
{
  _init_main_window(NULL);
}

midilooper_main_window::~midilooper_main_window(void)
{
  pbt_ggt_win_destroy(&ggt_win);

  delete imgui_dialog;

  gui_default_theme_destroy(&theme);

  uninit_engine(engine_ctx);
  free(engine_ctx);
}

void midilooper_main_window::handle_dialog(void)
{
  if (dialog_iface.need_popup == MSQ_TRUE)
    {
      if (dialog_iface.type == LIST)
        imgui_dialog->popup_list();
      else if (dialog_iface.type == FILE_BROWSER)
        imgui_dialog->popup_file_browser();
      else if (dialog_iface.type == STRING)
        imgui_dialog->popup_text();
      else if (dialog_iface.type == STRING_INPUT)
        imgui_dialog->popup_string_input();

      dialog_iface.need_popup = MSQ_FALSE;
    }

  if (dialog_iface.activated == MSQ_TRUE)
    imgui_dialog->handle_window();
  else
    if (imgui_dialog->window_mapped() == true)
      imgui_dialog->unmap_window();
}

void midilooper_main_window::handle_windows(void)
{
  pbt_ggt_node_t *ggt_node;
  pbt_ggt_t *ggt_wrapper;
  msq_track_line_t *track_line;
  wbe_window_t *win;
  track_editor_t *track_editor;

  for (ggt_node = track_list.vlist.ggt.childs->next;
       ggt_node != NULL;
       ggt_node = ggt_node->next)
    {
      ggt_wrapper = (pbt_ggt_t *) ggt_node->priv.ggt_addr;
      track_line = (msq_track_line_t *) ggt_wrapper->priv;
      track_editor = track_line->track_node.track_editor;
      win = track_editor->ggt_win.pb_win.win_be;
      if (wbe_window_closed(win) == WBE_TRUE)
        wbe_window_unmap(win);
      if (engine_is_running(engine_ctx) == MSQ_TRUE)
        {
          if (wbe_window_mapped(win) == WBE_TRUE)
            draw_progress_line(track_editor);
          msq_track_node_draw_progress(&(track_line->track_node));
        }
    }

  handle_dialog();
}

bool midilooper_main_window::closed(void)
{
  if (wbe_window_closed(ggt_win.pb_win.win_be) == WBE_TRUE || should_close)
    return true;
  return false;
}

void midilooper_main_window::show_filemenu_dialog(void)
{
  static const char *file_menu[] = {"Save", "Save as", "Quit"};

  // tmp
  msq_dialog_list(&(dialog_iface),
                  (char **) file_menu,
                  3,
                  filemenu_dialog_cb,
                  this);
}

void midilooper_main_window::show_output_dialog(output_t *output)
{
  static const char *output_menu[] = {"Rename", "Add", "Remove"};

  msq_dialog_list(&(dialog_iface),
                  (char **) output_menu,
                  3,
                  output_dialog_res_cb,
                  this);
  dialog_output = output;
}

char **midilooper_main_window::gen_output_str_list(size_t *str_list_len)
{
  list_iterator_t iter;
  output_t        *output;
  char            **ret_str, **ret_ptr;

  *str_list_len = engine_ctx->output_list.len + 1;

  ret_str = (char **) calloc(*str_list_len, sizeof (char *));
  ret_ptr = ret_str;

  *ret_ptr = strdup("No output");
  ret_ptr++;

  for (iter_init(&iter, &(engine_ctx->output_list));
       iter_node(&iter);
       iter_next(&iter))
    {
      output = (output_t *) iter_node_ptr(&iter);
      *ret_ptr = strdup(output_get_name(output));
      ret_ptr++;
    }

  return ret_str;
}

void midilooper_main_window::show_set_output(track_editor_t *track_editor)
{
  size_t str_list_len;
  char **set_output_menu = gen_output_str_list(&str_list_len);

  msq_dialog_list(&(dialog_iface),
                  set_output_menu,
                  str_list_len,
                  track_set_output_dialog_res_cb,
                  this);
  _msq_free_list(set_output_menu, str_list_len);
  dialog_track = track_editor;
}

void midilooper_main_window::show_track_dialog(track_editor_t *track_editor)
{
  static const char *track_menu[] = {"Rename",
                                     "Output",
                                     "Add key binding",
                                     "Del all bindings",
                                     "Add",
                                     "Remove"};

  msq_dialog_list(&(dialog_iface),
                  (char **) track_menu,
                  6,
                  track_dialog_res_cb,
                  this);
  dialog_track = track_editor;
}

void midilooper_main_window::refresh(void)
{
  wbe_pbw_put_buffer(&(ggt_win.pb_win));
}

void midilooper_main_window::redraw(void)
{
  _pbt_ggt_draw(ggt_win.ggt);
  refresh();
}

output_t *midilooper_main_window::get_output(size_t idx)
{
  list_iterator_t it;
  output_t *output = NULL;
  size_t current_idx;

  for (current_idx = 0,
         iter_init(&it, &(engine_ctx->output_list));
       iter_node(&it);
       iter_next(&it),
         current_idx++)
    {
      output = (output_t *) iter_node_ptr(&it);
      if (current_idx == idx)
        return output;
    }

  return NULL;
}

void midilooper_main_window::add_output(char *new_name)
{
  output_t *output = engine_create_output(engine_ctx, new_name);
  pbt_ggt_node_t *ggt_node = msq_output_node_add(&output_list, output);

  pbt_ggt_win_set_min_size(&ggt_win);
  pbt_ggt_win_init_child_ev(&ggt_win, ggt_node);
}

void midilooper_main_window::add_track(char *new_name)
{
  track_ctx_t *track_ctx = engine_create_trackctx(engine_ctx, new_name);
  msq_track_line_t *track_line = msq_track_node_add(&track_list,
                                                    track_ctx,
                                                    &track_editor_theme,
                                                    &dialog_iface,
                                                    &transport_iface);

  pbt_ggt_win_set_min_size(&ggt_win);
  pbt_ggt_win_init_child_ev(&ggt_win, track_line->node_it.node);
  wbe_window_map(track_line->track_editor.ggt_win.pb_win.win_be);
  pbt_ggt_win_put_buffer(&(track_line->track_editor.ggt_win));
}

void midilooper_main_window::remove_output(output_t *output)
{
  if (engine_is_running(engine_ctx) == MSQ_TRUE)
    {
      fprintf(stderr, "Sorry can not delete output while running.\n");
      return;
    }
  msq_output_list_del_node(&output_list, output);
  engine_delete_output(engine_ctx, output);
  pbt_ggt_win_set_min_size(&ggt_win);
}

void midilooper_main_window::remove_track(track_editor_t *track_editor)
{
  track_ctx_t *track_ctx = track_editor->editor_ctx.track_ctx;

  msq_track_list_del_node(&track_list, track_editor);
  // track_editor_destroy(track_editor);
  engine_delete_trackctx(track_ctx->engine, track_ctx);
  pbt_ggt_win_set_min_size(&ggt_win);
}

void midilooper_main_window::set_add_key_binding_mode(void)
{
  const string str1 = "Press a key from A to Z";
  const string str2 = "(abort 5 sec or press ESC)";
  unsigned int
    str_ypos = (_pbt_ggt_height(ggt_win.ggt) >> 1) - theme.theme.font.max_height,
    str_xpos,
    str_width;

  pbt_font_get_string_width(&(theme.theme.font), str2.c_str(), &str_width);
  str_xpos = (_pbt_ggt_width(ggt_win.ggt) - str_width) >> 1;

  msq_draw_veil(&(ggt_win.ggt->pbarea),
                0, _pbt_ggt_width(ggt_win.ggt),
                0, _pbt_ggt_height(ggt_win.ggt));
  pbt_pbarea_fillrect(&(ggt_win.ggt->pbarea),
                      str_xpos, str_ypos,
                      str_width, theme.theme.font.max_height << 1,
                      theme.theme.window_bg);
  pbt_pbarea_printf(&(ggt_win.ggt->pbarea),
                    &(theme.theme.font),
                    theme.theme.window_fg,
                    str_xpos, str_ypos,
                    str1.c_str());
  pbt_pbarea_printf(&(ggt_win.ggt->pbarea),
                    &(theme.theme.font),
                    theme.theme.window_fg,
                    str_xpos, str_ypos + theme.theme.font.max_height,
                    str2.c_str());
  pbt_ggt_win_put_buffer(&ggt_win);
  wait_since = wbe_get_time();
  wait_key_binding = true;
  track_waiting_binding = dialog_track->editor_ctx.track_ctx;
}

void midilooper_main_window::show_add_track(void)
{
  msq_dialog_string_input(&(dialog_iface),
                          add_track_result_cb,
                          this);
}

void midilooper_main_window::show_add_output(void)
{
  msq_dialog_string_input(&(dialog_iface),
                          add_output_result_cb,
                          this);
}

void midilooper_main_window::save_file_as(const char *file_path)
{
  engine_save_project(engine_ctx, file_path, MSQ_FALSE);
  save_path = file_path;
}

#define _check_alpha_keys(byte_array)           \
  (((byte_array)[1] != 0)                       \
   || ((byte_array)[2] != 0)                    \
   || ((byte_array)[3] != 0)                    \
   || ((byte_array)[4] != 0))

void midilooper_main_window::handle_key_bindings(wbe_window_input_t *winev)
{
  static uint32_t keys = 0;
  char key;

  if (_check_alpha_keys(winev->keys))
    {
      if (wait_key_binding)
        {
          for (key = 'A'; key <= 'Z'; key++)
            if (wbe_key_pressedA(winev->keys, key) == WBE_TRUE)
              {
                _add_binding(&(engine_ctx->bindings.keypress),
                             key,
                             track_waiting_binding);
                _pbt_ggt_draw(ggt_win.ggt);
                pbt_ggt_win_put_buffer(&ggt_win);
                track_waiting_binding = NULL;
                wait_key_binding = false;
                WBE_SET_BIT(keys, key - 'A', 1);
                return;
              }
        }
      else
        {
          for (key = 'A'; key <= 'Z'; key++)
            {
              if (wbe_key_pressedA(winev->keys, key) == WBE_TRUE)
                {
                  if (WBE_GET_BIT(keys, key - 'A') == WBE_FALSE)
                    {
                      engine_call_keypress_b(engine_ctx, key);
                      WBE_SET_BIT(keys, key - 'A', 1);
                    }
                }
              else
                WBE_SET_BIT(keys, key - 'A', 0);
            }
          if (engine_ctx->mute_state_changed == MSQ_TRUE)
            {
              _pbt_ggt_draw(ggt_win.ggt);
              pbt_ggt_win_put_buffer(&ggt_win);
            }
        }
    }
  else
    keys = 0;
}

void midilooper_main_window::run(void)
{
  while (!closed())
    {
      if (engine_is_running(engine_ctx) == MSQ_TRUE)
        glfwWaitEventsTimeout(0.05);
      else if (dialog_iface.need_update == MSQ_TRUE)
        {
          dialog_iface.need_update = MSQ_FALSE;
          glfwWaitEventsTimeout(0.05);
        }
      else if (track_waiting_binding)
        {
          if (wait_since + 5.0 < wbe_get_time())
            {
              _pbt_ggt_draw(ggt_win.ggt);
              pbt_ggt_win_put_buffer(&ggt_win);
              track_waiting_binding = NULL;
              wait_key_binding = false;
            }
          glfwWaitEventsTimeout(1.0);
        }
      else
        wbe_wait_events();

      wbe_window_handle_events();

      handle_windows();
    }
}

#define PATH_MAX_LEN 256

void load_configuration(void)
{
  char tmp_path[PATH_MAX_LEN];
  const char *tmp_str;
  config_t config;

  snprintf(tmp_path, PATH_MAX_LEN,
           "%s/.config/midilooper/midilooper.conf",
           getenv("HOME"));

  config_init(&config);
  if (config_read_file(&config, tmp_path) == CONFIG_TRUE)
    {
      if (config_lookup_string(&config, "keyboard_language", &tmp_str)
          == CONFIG_TRUE)
        {
          if (strcmp("fr", tmp_str) == 0)
            {
              printf("set keyboard language %s\n", tmp_str);
              wbe_window_backend_set_key_layout(WBE_KEY_LAYOUT_FR);
            }
          else if (strcmp("FR", tmp_str) == 0)
            {
              printf("set keyboard language %s\n", tmp_str);
              wbe_window_backend_set_key_layout(WBE_KEY_LAYOUT_FR);
            }
          else if (strcmp("us", tmp_str) == 0)
            ;                   // nothing todo default setting
          else if (strcmp("US", tmp_str) == 0)
            ;                   // bis
        }
    }
  config_destroy(&config);
}

int main(int ac, char **av)
{
  midilooper_main_window *main_window;

  if (ac > 2)
    return 1;

  load_configuration();

  wbe_pbw_backend_init();

  if (ac == 2)
    main_window = new midilooper_main_window(av[1]);
  else
    main_window = new midilooper_main_window();

  main_window->run();

  delete main_window;

  wbe_gl_color_destroy();
  wbe_gl_texture_n_color_destroy();
  wbe_pbw_backend_destroy();

  return 0;
}
