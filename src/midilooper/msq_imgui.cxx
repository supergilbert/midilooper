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
#include <wbe_glfw.h>
#include <wbe_gl.h>
}

#include "pbt_default_font.h"

#include "msq_imgui.hxx"
#include <examples/imgui_impl_glfw.h>

bool list_compare_file_elt(const file_elt *first, const file_elt *second)
{
  size_t idx = 0;
  size_t max = first->name.size();

  if (max > second->name.size())
    max = second->name.size();

  while (idx < max)
    {
      if (first->name[idx] < second->name[idx])
        return true;
      else
        return false;
      idx++;
    }
  return first->name.size() < second->name.size();
}

void msq_imgui_dialog::free_popup_list(void)
{
  for (char *str : popup_str_list)
    free(str);
  popup_str_list.clear();
}

void msq_imgui_dialog::get_cursor_pos(int *xpos, int *ypos)
{
 double xpos_cursor, ypos_cursor;

 glfwGetCursorPos(glfw_win, &xpos_cursor, &ypos_cursor);
 glfwGetWindowPos(glfw_win, xpos, ypos);
 *xpos += xpos_cursor;
 *ypos += ypos_cursor;
}

void msq_imgui_dialog::set_pos_at_cursor(void)
{
  int xpos, ypos;

  get_cursor_pos(&xpos, &ypos);
  glfwSetWindowPos(glfw_win, xpos, ypos);
}

#define MAX_POPUP_SHOWN_ELT 12

void msq_imgui_dialog::popup_list(void)
{
  int width = 0, height;
  size_t idx;

  free_popup_list();

  for (idx = 0; idx < dialog_iface->str_list_len; idx++)
    {
      const char *str_elt = dialog_iface->str_list[idx];
      if (width < (int) strlen(str_elt))
        width = strlen(str_elt);
      popup_str_list.push_back(strdup(str_elt));
    }

  width = width * font_size;
  if (popup_str_list.size() > MAX_POPUP_SHOWN_ELT)
    height = MAX_POPUP_SHOWN_ELT * frame_height;
  else
    height = popup_str_list.size() * frame_height;
  glfwSetWindowSize(glfw_win, width, height);
  msq_dialog_activate(dialog_iface);
  hide_on_focus_lost = true;
  map_window();
  set_pos_at_cursor();
}

#define TMP_DIALOG_WIDTH 400
#define TMP_DIALOG_HEIGHT 300

void msq_imgui_dialog::popup_file_browser(void)
{
  glfwSetWindowSize(glfw_win, TMP_DIALOG_WIDTH, TMP_DIALOG_HEIGHT);
  msq_dialog_activate(dialog_iface);
  hide_on_focus_lost = false;
  map_window();
}

void msq_imgui_dialog::popup_text(void)
{
  unsigned int width = font_size * 14;

  glfwSetWindowSize(glfw_win, width, frame_height);
  msq_dialog_activate(dialog_iface);
  hide_on_focus_lost = false;
  map_window();
  set_pos_at_cursor();
}

void msq_imgui_dialog::popup_string_input(void)
{
  unsigned int width = font_size * 14;

  glfwSetWindowSize(glfw_win, width, frame_height);
  msq_dialog_activate(dialog_iface);
  hide_on_focus_lost = false;
  map_window();
  set_pos_at_cursor();
}

void msq_imgui_dialog::new_frame(void)
{
  ImGuiIO &io = ImGui::GetIO();
  int width, height;

  glfwMakeContextCurrent(glfw_win);
  glfwGetWindowSize(glfw_win, &width, &height);
  io.DisplaySize.x = width;
  io.DisplaySize.y = height;
  glUniform2f(i_dpy_size_var_pos,
              (GLfloat) width,
              (GLfloat) height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void msq_imgui_dialog::render_frame(void)
{
  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();
  ImGuiIO& io = ImGui::GetIO();
  int fb_width =
    (int) (draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height =
    (int) (draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
  ImDrawList* cmd_list;
  ImDrawIdx* idx_buffer_offset;
  ImDrawCmd* pcmd;

  glUseProgram(shader_program);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glActiveTexture(GL_TEXTURE0);

  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  glVertexAttribPointer(i_position_var_pos,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof (ImDrawVert),
                        (const void *) IM_OFFSETOF(ImDrawVert, pos));
  glEnableVertexAttribArray(i_position_var_pos);

  glVertexAttribPointer(i_texcoo_var_pos,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof (ImDrawVert),
                        (const void *) IM_OFFSETOF(ImDrawVert, uv));
  glEnableVertexAttribArray(i_texcoo_var_pos);

  glVertexAttribPointer(i_color_var_pos,
                        4,
                        GL_UNSIGNED_BYTE,
                        GL_TRUE,
                        sizeof (ImDrawVert),
                        (const void *) IM_OFFSETOF(ImDrawVert, col));
  glEnableVertexAttribArray(i_color_var_pos);

  for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
      cmd_list = draw_data->CmdLists[n];
      idx_buffer_offset = 0;

      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER,
                   cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                   cmd_list->VtxBuffer.Data,
                   GL_STREAM_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                   cmd_list->IdxBuffer.Data,
                   GL_STREAM_DRAW);

      for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
          pcmd = &cmd_list->CmdBuffer[cmd_i];
          if (pcmd->UserCallback)
            pcmd->UserCallback(cmd_list, pcmd);
          else
            {
              glBindTexture(GL_TEXTURE_2D,
                            (GLuint) (intptr_t) pcmd->TextureId);
              glScissor((int)pcmd->ClipRect.x,
                        (int)(fb_height - pcmd->ClipRect.w),
                        (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                        (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
              glDrawElements(GL_TRIANGLES,
                             pcmd->ElemCount,
                             GL_UNSIGNED_SHORT,
                             idx_buffer_offset);
            }
          idx_buffer_offset += pcmd->ElemCount;
        }
    }
  glFlush();
}

#include <iostream>
#include <string>

#define BUFFSIZE 128

void msq_imgui_dialog::render_file_browser(void)
{
  ImGuiIO &io = ImGui::GetIO();
  bool bool_var = true;
  static char buff[BUFFSIZE] = {};
  static file_elt *current_item = NULL;
  int current_height;
  int height;
  ImGuiStyle* style = &ImGui::GetStyle();
  std::string new_dir_path;

  glfwGetWindowSize(glfw_win, NULL, &current_height);
  height = current_height - (3 * (int) ImGui::GetFrameHeightWithSpacing()
                             + style->ItemSpacing.y);

  new_frame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);
  ImGui::SetNextWindowBgAlpha(1.0f);
  ImGui::Begin("File browser",
               &bool_var,
               ImGuiWindowFlags_NoTitleBar
               | ImGuiWindowFlags_NoDecoration
               | ImGuiWindowFlags_NoMove
               | ImGuiWindowFlags_AlwaysUseWindowPadding
               | ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("Path: %s", dir_array->get_path());

  ImGui::ListBoxHeader("", ImVec2(-1, height));
  for (file_elt *elt : dir_array->get_list())
    if (ImGui::Selectable(elt->name.c_str(), elt == current_item))
      {
        if (elt->is_directory)
          {
            if (elt->name == "../")
              {
                new_dir_path = dir_array->get_path();
                if (new_dir_path != "/")
                  new_dir_path.erase(new_dir_path.length() - 1);
                std::size_t found = new_dir_path.find_last_of("/");
                if (found == 0)
                  new_dir_path = "/";
                else
                  new_dir_path.erase(found);
              }
            else if (elt->name !=  "./")
              {
                new_dir_path = dir_array->get_path();
                if (new_dir_path[new_dir_path.length() - 1] != '/')
                  new_dir_path += "/";
                new_dir_path += elt->name;
              }
          }
        else
          {
            current_item = elt;
            strncpy(buff, current_item->name.c_str(), BUFFSIZE);
          }
      }
  ImGui::ListBoxFooter();

  ImGui::PushItemWidth(-1);
  ImGui::InputText("Enter a filename", buff, BUFFSIZE);
  ImGui::PopItemWidth();
  if (ImGui::Button("Ok"))
    {
      new_dir_path = dir_array->get_path();
      new_dir_path += "/";
      new_dir_path += buff;
      strcpy(buff, new_dir_path.c_str());
      msq_dialog_result_str(dialog_iface, buff);
    }
  ImGui::SameLine();
  if (ImGui::Button("Cancel"))
    msq_dialog_desactivate(dialog_iface);
  ImGui::SameLine();
  ImGui::End();

  if (io.NavInputs[ImGuiNavInput_Cancel] == 1.0f)
    msq_dialog_desactivate(dialog_iface);

  render_frame();
  if (new_dir_path.size() != 0)
    dir_array->load_directory(new_dir_path.c_str());

}

void msq_imgui_dialog::render_list(void)
{
  ImGuiIO& io = ImGui::GetIO();
  bool bool_var = true;
  size_t idx;
  bool got_result = false;

  new_frame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);
  ImGui::SetNextWindowBgAlpha(1.0f);
  ImGui::Begin("list",
               &bool_var,
               ImGuiWindowFlags_NoTitleBar
               | ImGuiWindowFlags_NoDecoration
               | ImGuiWindowFlags_NoMove
               | ImGuiWindowFlags_AlwaysUseWindowPadding
               | ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::ListBoxHeader("", ImVec2(-1, -1));
  for (idx = 0; idx < popup_str_list.size(); idx++)
    if (ImGui::Selectable(popup_str_list[idx]))
      {
        got_result = true;
        break;
      }
  ImGui::ListBoxFooter();

  ImGui::End();

  if (io.NavInputs[ImGuiNavInput_Cancel] == 1.0f)
    msq_dialog_desactivate(dialog_iface);

  render_frame();

  if (got_result)
    msq_dialog_result_idx(dialog_iface, idx);
}

void msq_imgui_dialog::render_string_input(void)
{
  ImGuiIO& io = ImGui::GetIO();
  bool bool_var = true;
  static char buff[BUFFSIZE] = {};
  bool got_result = false;

  new_frame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);
  ImGui::SetNextWindowBgAlpha(1.0f);
  ImGui::Begin("string input",
               &bool_var,
               ImGuiWindowFlags_NoTitleBar
               | ImGuiWindowFlags_NoDecoration
               | ImGuiWindowFlags_NoMove
               | ImGuiWindowFlags_AlwaysUseWindowPadding
               | ImGuiWindowFlags_AlwaysAutoResize);
  if (ImGui::InputText("Enter a name",
                       buff,
                       BUFFSIZE,
                       ImGuiInputTextFlags_EnterReturnsTrue))
    got_result = true;
  ImGui::SetKeyboardFocusHere(0);

  ImGui::End();

  if (io.NavInputs[ImGuiNavInput_Cancel] == 1.0f)
    msq_dialog_desactivate(dialog_iface);

  render_frame();

  if (got_result)
    {
      // glfwHideWindow(glfw_win);
      msq_dialog_result_str(dialog_iface, buff);
    }
}

void msq_imgui_dialog::render_text(void)
{
  ImGuiIO& io = ImGui::GetIO();
  bool bool_var = true;

  new_frame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);
  ImGui::SetNextWindowBgAlpha(1.0f);
  ImGui::Begin("string",
               &bool_var,
               ImGuiWindowFlags_NoTitleBar
               | ImGuiWindowFlags_NoDecoration
               | ImGuiWindowFlags_NoMove
               | ImGuiWindowFlags_AlwaysUseWindowPadding
               | ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("%s", dialog_iface->str);
  ImGui::End();

  if (io.NavInputs[ImGuiNavInput_Cancel] == 1.0f)
    msq_dialog_desactivate(dialog_iface);

  render_frame();
}

void msq_imgui_dialog::handle_window(void)
{
  if (dialog_iface->activated && window_mapped())
    {
      if (glfwWindowShouldClose(glfw_win) == GLFW_TRUE)
        unmap_window();
      else
        {
          if (dialog_iface->type == LIST)
            render_list();
          else if (dialog_iface->type == FILE_BROWSER)
            render_file_browser();
          else if (dialog_iface->type == STRING)
            render_text();
          else
            render_string_input();
        }
    }
}

bool msq_imgui_dialog::window_mapped(void)
{
  return glfwGetWindowAttrib(glfw_win, GLFW_VISIBLE) == GLFW_TRUE;
}

void msq_imgui_dialog::map_window(void)
{
  glfwSetWindowShouldClose(glfw_win, GLFW_FALSE);
  glfwShowWindow(glfw_win);
}

void msq_imgui_dialog::unmap_window(void)
{
  glfwHideWindow(glfw_win);
}

void msq_imgui_dialog::init_font(void)
{
  int font_width, font_height;
  unsigned char* font_pixels;
  ImGuiIO &io = ImGui::GetIO();
  ImFontConfig imfont_config;

  imfont_config.FontDataOwnedByAtlas = false;

  // io.Fonts->AddFontFromFileTTF(DEFAULT_FONT_PATH, DEFAULT_FONT_SIZE);
  io.Fonts->AddFontFromMemoryTTF(_get_pbt_default_font_ptr(),
                                 _get_pbt_default_font_len(),
                                 DEFAULT_FONT_SIZE,
                                 &imfont_config);
  io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
  glGenTextures(1, &font_texture);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  // glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               font_width,
               font_height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               font_pixels);
  io.Fonts->TexID = (ImTextureID)(intptr_t) font_texture;
}

msq_imgui_dialog::~msq_imgui_dialog(void)
{
  free_popup_list();
  ImGui_ImplGlfw_Shutdown();
  delete dir_array;
  ImGui::DestroyContext();
  glDeleteProgram(shader_program);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &index_buffer);
  if (font_texture != 0)
    glDeleteTextures(1, &font_texture);
  glfwDestroyWindow(glfw_win);
}

#define BYTEARRAY_TO_IMVEC4_COLOR(byte_color)                      \
  ImVec4(((float) (byte_color)[0]) / 255.0f,                       \
         ((float) (byte_color)[1]) / 255.0f,                       \
         ((float) (byte_color)[2]) / 255.0f,                       \
         1.0f)

void msq_imgui_dialog::set_style(msq_gui_theme_t *theme)
{
  ImGuiStyle * style = &ImGui::GetStyle();

  style->WindowRounding = 0.0f;
  style->ScrollbarRounding = 0.0f;
  style->WindowBorderSize = 0.0f;
  style->ChildBorderSize = 0.0f;
  style->PopupBorderSize = 0.0f;
  style->FrameBorderSize = 0.0f;
  style->TabBorderSize = 0.0f;

  style->WindowPadding = ImVec2(0.0f, 0.0f);

  for (int idx = 0; idx < ImGuiCol_COUNT; idx++)
    style->Colors[idx] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

  style->Colors[ImGuiCol_Text] = BYTEARRAY_TO_IMVEC4_COLOR(theme->theme.frame_fg);
  style->Colors[ImGuiCol_WindowBg] = BYTEARRAY_TO_IMVEC4_COLOR(theme->theme.window_bg);
  style->Colors[ImGuiCol_FrameBg] = BYTEARRAY_TO_IMVEC4_COLOR(theme->theme.frame_bg);

  style->Colors[ImGuiCol_Header] = BYTEARRAY_TO_IMVEC4_COLOR(theme->theme.frame_bg);
  style->Colors[ImGuiCol_HeaderHovered] = BYTEARRAY_TO_IMVEC4_COLOR(theme->theme.wgt_hovered_bg);
  style->Colors[ImGuiCol_HeaderActive] = BYTEARRAY_TO_IMVEC4_COLOR(theme->theme.wgt_normal_bg);

  style->Colors[ImGuiCol_Button] = style->Colors[ImGuiCol_Header];
  style->Colors[ImGuiCol_ButtonHovered] = style->Colors[ImGuiCol_HeaderHovered];
  style->Colors[ImGuiCol_ButtonActive] = style->Colors[ImGuiCol_HeaderActive];

  style->Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);


  style->Colors[ImGuiCol_ScrollbarBg] = style->Colors[ImGuiCol_WindowBg];
  style->Colors[ImGuiCol_ScrollbarGrab] = style->Colors[ImGuiCol_Header];
  style->Colors[ImGuiCol_ScrollbarGrabHovered] = style->Colors[ImGuiCol_HeaderHovered];
  style->Colors[ImGuiCol_ScrollbarGrabActive] = style->Colors[ImGuiCol_HeaderActive];
}

void msq_imgui_dialog_focus_cb(wbe_bool_t focused, void *msq_imgui_arg)
{
  msq_imgui_dialog *imgui_dialog = (msq_imgui_dialog *) msq_imgui_arg;

  if (imgui_dialog->hide_on_focus_lost)
    {
      if (focused == GLFW_FALSE)
        {
          msq_dialog_desactivate(imgui_dialog->dialog_iface);
          imgui_dialog->unmap_window();
        }
    }
  else if (focused == GLFW_FALSE)
    glfwFocusWindow(imgui_dialog->glfw_win);
}

#include <stdlib.h>             // abort

msq_imgui_dialog::msq_imgui_dialog(msq_dialog_iface_t *dialog_iface_arg, msq_gui_theme_t *theme)
{
  GLchar vshader_str[] =
    "uniform vec2 i_dpy_size;"
    "attribute vec2 i_position;"
    "attribute vec2 i_texcoo;"
    "attribute vec4 i_color;"
    "varying vec2 v_texcoo;"
    "varying vec4 v_color;"
    "void main()"
    "{"
    "  v_color = i_color;"
    "  v_texcoo = i_texcoo;"
    "  float x_coo;"
    "  float y_coo;"
    "  x_coo = (i_position.x *  2.0 / i_dpy_size.x) - 1.0;"
    "  y_coo = (i_position.y * -2.0 / i_dpy_size.y) + 1.0;"
    "  gl_Position = vec4(x_coo, y_coo, 0.0, 1.0);"
    "}";
  GLchar fshader_str[] =
    "varying vec2 v_texcoo;"
    "varying vec4 v_color;"
    "uniform sampler2D i_texture;"
    "void main()"
    "{"
    "  gl_FragColor = v_color * texture2D(i_texture, v_texcoo);"
    "}";

  dialog_iface = dialog_iface_arg;

  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfw_win = glfwCreateWindow(TMP_DIALOG_WIDTH,
                              TMP_DIALOG_HEIGHT,
                              "dialog",
                              NULL, NULL);
  wbe_window_add_focus_cb(glfw_win, msq_imgui_dialog_focus_cb, this);
  glfwMakeContextCurrent(glfw_win);

  if (wbe_gl_shader_program_init(&shader_program,
                                 vshader_str,
                                 fshader_str) != WBE_TRUE)
    abort();

  i_dpy_size_var_pos = glGetUniformLocation(shader_program, "i_dpy_size");
  i_position_var_pos = glGetAttribLocation(shader_program, "i_position");
  i_texcoo_var_pos   = glGetAttribLocation(shader_program, "i_texcoo");
  i_color_var_pos    = glGetAttribLocation(shader_program, "i_color");

  glEnable(GL_TEXTURE_2D);

  glGenBuffers(1, &vertex_buffer);
  glGenBuffers(1, &index_buffer);

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = NULL;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  set_style(theme);

  dir_array = new directory_array();
  dir_array->load_directory("/tmp");

  ImGui_ImplGlfw_InitForOpenGL(glfw_win, true);
  init_font();
  new_frame();
  render_frame();
  frame_height_with_spacing = ImGui::GetFrameHeightWithSpacing();
  frame_height = ImGui::GetFrameHeight();
  font_size = ImGui::GetFontSize();
}
