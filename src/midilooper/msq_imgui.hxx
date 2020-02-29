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

#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <imgui.h>

#include <list>
#include <vector>
using namespace std;

#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>

#include <iostream>
#include <string>

class file_elt
{
public:
  string name;
  bool is_directory = false;
  file_elt(char *name_arg, bool is_directory_arg)
  {
    is_directory = is_directory_arg;
    name = name_arg;
    name += is_directory_arg ? "/" : "";
  }
};

bool list_compare_file_elt(const file_elt *first, const file_elt *second);

class directory_array
{
  list<file_elt *> file_list;
  char *dir_path;
  void destroy_list(void)
  {
    if (!file_list.empty())
      {
        free(dir_path);
        do
          {
            delete file_list.front();
            file_list.pop_front();
          } while (!file_list.empty());
      }
  }

public:
  ~directory_array(void)
  {
    destroy_list();
  }
  directory_array(void)
  {
  }
  directory_array(const char *path)
  {
    load_directory(path);
  }
  bool load_directory(const char *path)
  {
    DIR *directory = opendir(path);
    struct dirent *readdir_ret;

    if (directory == NULL)
      return false;

    destroy_list();

    while ((readdir_ret = readdir(directory)) != NULL)
      {
        file_list.push_back(new file_elt(readdir_ret->d_name,
                                         readdir_ret->d_type == DT_DIR));
      }
    closedir(directory);
    file_list.sort(list_compare_file_elt);
    dir_path = strdup(path);
    return true;
  }
  list<file_elt *> &get_list(void)
  {
    return file_list;
  }
  size_t get_size(void)
  {
    return file_list.size();
  }
  const char *get_path(void)
  {
    return dir_path;
  }
};

#include "msq_gui.h"

class msq_imgui_dialog
{
  GLuint shader_program = 0;
  GLuint vertex_buffer = 0;
  GLuint index_buffer = 0;
  GLuint font_texture = 0;
  GLint i_dpy_size_var_pos = 0;
  GLint i_position_var_pos = 0;
  GLint i_texcoo_var_pos = 0;
  GLint i_color_var_pos = 0;
  directory_array *dir_array;
  vector<char *> popup_str_list;
  unsigned int frame_height_with_spacing;
  unsigned int frame_height;
  unsigned int font_size;
public:
  GLFWwindow *glfw_win;
  msq_dialog_iface_t *dialog_iface = NULL;
  bool hide_on_focus_lost = false;
  void init_font(void);
  ~msq_imgui_dialog(void);
  msq_imgui_dialog(msq_dialog_iface_t *dialog_iface, msq_gui_theme_t *theme);
  void free_popup_list(void);
  void popup_file_browser(void);
  void popup_list(void);
  void popup_string_input(void);
  void render_file_browser(void);
  void render_list(void);
  void render_string_input(void);
  void new_frame(void);
  void render_frame(void);
  void handle_window(void);
  bool window_mapped(void);
  void map_window(void);
  void set_style(msq_gui_theme_t *theme);
  void unmap_window(void);
  void get_cursor_pos(int *xpos, int *ypos);
  void set_pos_at_cursor(void);
};
