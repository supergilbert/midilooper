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

#include <stdio.h>              /* fprintf */
#include <stdlib.h>             /* malloc */
#include <string.h>             /* memcpy */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "wbe_type.h"

/* TODO:
   Extract texture_id from struct and make wbe_gl_texncol no more depend on
   wbe_gl_texture. */
typedef struct
{
  GLuint shader_program;
  /* GLuint texture_id; */
  GLint i_position_var_pos;
  /* GLint i_texcoo_var_pos; */
} wbe_gl_texture_t;

typedef struct
{
  GLuint shader_program;
  GLint i_position_var_pos;
  GLint i_color_var_pos;
} wbe_gl_texture_n_color_t;

typedef struct
{
  GLuint shader_program;
  GLint i_position_var_pos;
  GLint i_color_var_pos;
} wbe_gl_color_t;

static GLfloat color_rgba[4];
static GLfloat plain_vertices[12];
/* static GLfloat plain_texcoos[12]; */
static GLfloat tmp_vertices[12];

static wbe_bool_t _gl_texture_initialised = WBE_FALSE;
static wbe_gl_texture_t _gl_texture_hdl = {};

static wbe_bool_t _gl_texture_n_color_initialised = WBE_FALSE;
static wbe_gl_texture_n_color_t _gl_texture_n_color_hdl = {};

static wbe_bool_t _gl_color_initialised = WBE_FALSE;
static wbe_gl_color_t _gl_color_hdl = {};

GLuint wbe_gl_load_shader(GLenum type, const char *shader_src)
{
  GLuint shader;
  GLint compiled;
  GLint info_len;
  char* info_log;

  shader = glCreateShader(type);

  if (shader == 0)
    {
      fprintf(stderr, "Unable to create shader.\n");
      return 0;
    }

  glShaderSource(shader, 1, &shader_src, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (compiled == 0)
    {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);

      if (info_len > 1)
        {
          info_log = malloc(sizeof(char) * info_len);

          glGetShaderInfoLog(shader, info_len, NULL, info_log);
          fprintf(stderr, "Error compiling shader:\n%s\n", info_log);
          free(info_log);
        }
      glDeleteShader(shader);
      return 0;
    }

  return shader;
}

wbe_bool_t wbe_gl_shader_program_init(GLuint *shader_program,
                                      GLchar *vshader_str,
                                      GLchar *fshader_str)
{
  GLuint vertex_shader;
  GLuint fragment_shader;
  GLint linked;
  GLint info_len;
  char* info_log;

  /* TODO: read doc on shader deletion */

  vertex_shader = wbe_gl_load_shader(GL_VERTEX_SHADER, vshader_str);
  if (vertex_shader == 0)
    {
      fprintf(stderr, "Unable to load vertex shader.\n");
      return WBE_FALSE;
    }
  fragment_shader = wbe_gl_load_shader(GL_FRAGMENT_SHADER, fshader_str);
  if (fragment_shader == 0)
    {
      fprintf(stderr, "Unable to load fragment shader.\n");
      return WBE_FALSE;
    }

  *shader_program = glCreateProgram();

  if (*shader_program == 0)
    return WBE_FALSE;

  glAttachShader(*shader_program, vertex_shader);
  glAttachShader(*shader_program, fragment_shader);

  /* glBindAttribLocation(*shader_program, 0, "vPosition"); */

  glLinkProgram(*shader_program);

  glGetProgramiv(*shader_program, GL_LINK_STATUS, &linked);

  if (linked == 0)
    {
      glGetProgramiv(*shader_program, GL_INFO_LOG_LENGTH, &info_len);

      if (info_len > 1)
        {
          info_log = malloc(sizeof(char) * info_len);

          glGetProgramInfoLog(*shader_program, info_len, NULL, info_log);
          fprintf(stderr, "Error linking program:\n%s", info_log);

          free(info_log);
        }

      glDeleteProgram(*shader_program);
      return WBE_FALSE;
    }
  return WBE_TRUE;
}

#define DEFAULT_PXL_FORMAT GL_RGBA

#define set_counter_clock_square_triangles(_vtx, xmin, ymin, xmax, ymax) \
  _vtx[0]  = xmax; _vtx[1]  = ymax;                                     \
  _vtx[2]  = xmin; _vtx[3]  = ymax;                                     \
  _vtx[4]  = xmin; _vtx[5]  = ymin;                                     \
  _vtx[6]  = xmax; _vtx[7]  = ymin;                                     \
  _vtx[8]  = xmax; _vtx[9]  = ymax;                                     \
  _vtx[10] = xmin; _vtx[11] = ymin

void wbe_gl_gen_texture(GLuint *texture_id)
{
  glGenTextures(1, texture_id);
  glBindTexture(GL_TEXTURE_2D, *texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void wbe_gl_texture_init(void)
{
  GLchar vshader_str[] =
    "attribute vec2 i_position;"
    "varying vec2 v_texcoo;"
    "void main()"
    "{"
    "  v_texcoo[0] = i_position[0] * 0.5 + 0.5;"
    "  v_texcoo[1] = -i_position[1] * 0.5 + 0.5;"
    "  gl_Position = vec4(i_position, 0.0, 1.0);"
    "}";
  GLchar fshader_str[] =
    "varying vec2 v_texcoo;"
    "uniform sampler2D i_texture;"
    "void main()"
    "{"
    "  gl_FragColor = texture2D(i_texture, v_texcoo);"
    "}";

  if (_gl_texture_initialised == WBE_TRUE)
    return;

  if (wbe_gl_shader_program_init(&(_gl_texture_hdl.shader_program),
                                 vshader_str,
                                 fshader_str) == WBE_FALSE)
    {
      fprintf(stderr, "!!! ERROR: Unable to initialise shader program.\n");
      return;
    }

  set_counter_clock_square_triangles(plain_vertices,
                                     -1.0f, -1.0f, 1.0f, 1.0f);
  /* set_counter_clock_square_triangles(plain_texcoos, */
  /*                                    0.0f, 1.0f, 1.0f, 0.0f); */

  _gl_texture_hdl.i_position_var_pos =
    glGetAttribLocation(_gl_texture_hdl.shader_program,
                        "i_position");

  /* _gl_texture_hdl.i_texcoo_var_pos = */
  /*   glGetAttribLocation(_gl_texture_hdl.shader_program, */
  /*                       "i_texcoo"); */

  _gl_texture_initialised = WBE_TRUE;
}

void wbe_gl_texture_destroy(void)
{
  glDeleteProgram(_gl_texture_hdl.shader_program);
}

void _wbe_gl_texture_put_vtx(GLfloat *vertices)
{
  glUseProgram(_gl_texture_hdl.shader_program);

  glVertexAttribPointer(_gl_texture_hdl.i_position_var_pos,
                        2, GL_FLOAT, GL_FALSE, 0, vertices);
  glEnableVertexAttribArray(_gl_texture_hdl.i_position_var_pos);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void _wbe_gl_texture_load(unsigned char *buffer,
                          const unsigned int width,
                          const unsigned int height)
{
  glTexImage2D(GL_TEXTURE_2D,
               0, DEFAULT_PXL_FORMAT,
               width, height,
               0,
               DEFAULT_PXL_FORMAT,
               GL_UNSIGNED_BYTE,
               buffer);
}

void wbe_gl_texture_refresh(void)
{
  /* glVertexAttribPointer(_gl_texture_hdl.i_texcoo_var_pos, */
  /*                       2, GL_FLOAT, GL_FALSE, 0, plain_texcoos); */
  /* glEnableVertexAttribArray(_gl_texture_hdl.i_texcoo_var_pos); */

  _wbe_gl_texture_put_vtx(plain_vertices);
}

void wbe_gl_texture_put_rect(GLfloat xmin, GLfloat ymin,
                             GLfloat xmax, GLfloat ymax)
{
  /* glVertexAttribPointer(_gl_texture_hdl.i_texcoo_var_pos, */
  /*                       2, GL_FLOAT, GL_FALSE, 0, plain_texcoos); */
  /* glEnableVertexAttribArray(_gl_texture_hdl.i_texcoo_var_pos); */

  set_counter_clock_square_triangles(tmp_vertices,
                                     xmin, ymin, xmax, ymax);

  _wbe_gl_texture_put_vtx(tmp_vertices);
}

void wbe_gl_texture_n_color_destroy(void)
{
  glDeleteProgram(_gl_texture_n_color_hdl.shader_program);
}

void wbe_gl_texture_n_color_init(void)
{
  GLchar vshader_str[] =
    "attribute vec2 i_position;"
    "varying vec2 v_texcoo;"
    "void main()"
    "{"
    "  v_texcoo[0] = i_position[0] * 0.5 + 0.5;"
    "  v_texcoo[1] = -i_position[1] * 0.5 + 0.5;"
    "  gl_Position = vec4(i_position, 0.0, 1.0);"
    "}";
  GLchar fshader_str[] =
    "varying vec2 v_texcoo;"
    "uniform vec4 i_color;"
    "uniform sampler2D i_texture;"
    "void main()"
    "{"
    "  gl_FragColor = mix(texture2D(i_texture, v_texcoo),"
    "                               vec4(vec3(i_color), 1.0),"
    "                               i_color.a);"
    "}";

  if (_gl_texture_n_color_initialised == WBE_TRUE)
    return;

  wbe_gl_texture_init();

  if (wbe_gl_shader_program_init(&(_gl_texture_n_color_hdl.shader_program),
                                 vshader_str,
                                 fshader_str) == WBE_FALSE)
    {
      fprintf(stderr, "Unable to initialise shader program.\n");
      return;
    }
  _gl_texture_n_color_hdl.i_position_var_pos =
    glGetAttribLocation(_gl_texture_n_color_hdl.shader_program,
                        "i_position");

  _gl_texture_n_color_hdl.i_color_var_pos =
    glGetUniformLocation(_gl_texture_n_color_hdl.shader_program,
                         "i_color");

  _gl_texture_n_color_initialised = WBE_TRUE;
}


void _wbe_gl_texture_n_color_put_rect(GLfloat xmin, GLfloat ymin,
                                      GLfloat xmax, GLfloat ymax,
                                      GLfloat *color_vec4)
{
  set_counter_clock_square_triangles(tmp_vertices,
                                     xmin, ymin, xmax, ymax);
  memcpy(color_rgba, color_vec4, 4 * sizeof (GLfloat));

  glUseProgram(_gl_texture_n_color_hdl.shader_program);

  glVertexAttribPointer(_gl_texture_n_color_hdl.i_position_var_pos,
                        2, GL_FLOAT, GL_FALSE, 0, tmp_vertices);
  glEnableVertexAttribArray(_gl_texture_n_color_hdl.i_position_var_pos);

  glUniform4fv(_gl_texture_n_color_hdl.i_color_var_pos, 1, color_rgba);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}


void wbe_gl_color_destroy(void)
{
  glDeleteProgram(_gl_color_hdl.shader_program);
}

void wbe_gl_color_init(void)
{
  GLchar vshader_str[] =
    "attribute vec2 i_position;"
    "void main()"
    "{"
    "  gl_Position = vec4(i_position, 0.0, 1.0);"
    "}";
  GLchar fshader_str[] =
    "uniform vec4 i_color;"
    "void main()"
    "{"
    "  gl_FragColor = i_color;"
    "}";

  if (_gl_color_initialised == WBE_TRUE)
    return;

  if (wbe_gl_shader_program_init(&(_gl_color_hdl.shader_program),
                                 vshader_str,
                                 fshader_str) == WBE_FALSE)
    {
      fprintf(stderr, "Unable to initialise shader program.\n");
      return;
    }

  _gl_color_hdl.i_position_var_pos =
    glGetAttribLocation(_gl_color_hdl.shader_program,
                        "i_position");

  _gl_color_hdl.i_color_var_pos =
    glGetUniformLocation(_gl_color_hdl.shader_program,
                         "i_color");

  _gl_color_initialised = WBE_TRUE;
}

void wbe_gl_color_put_line(GLfloat x1, GLfloat y1,
                           GLfloat x2, GLfloat y2,
                           GLfloat *color_vec4)
{
  tmp_vertices[0] = x1;
  tmp_vertices[1] = y1;
  tmp_vertices[2] = x2;
  tmp_vertices[3] = y2;
  memcpy(color_rgba, color_vec4, 4 * sizeof (GLfloat));

  glUseProgram(_gl_color_hdl.shader_program);

  glVertexAttribPointer(_gl_color_hdl.i_position_var_pos,
                        2, GL_FLOAT, GL_FALSE, 0, tmp_vertices);
  glEnableVertexAttribArray(_gl_color_hdl.i_position_var_pos);

  glUniform4fv(_gl_color_hdl.i_color_var_pos, 1, color_rgba);

  glDrawArrays(GL_LINES, 0, 2);
}

void wbe_gl_color_put_rect(GLfloat xmin, GLfloat ymin,
                           GLfloat xmax, GLfloat ymax,
                           GLfloat *color_vec4)
{
  set_counter_clock_square_triangles(tmp_vertices,
                                     xmin, ymin, xmax, ymax);
  memcpy(color_rgba, color_vec4, 4 * sizeof (GLfloat));

  glUseProgram(_gl_color_hdl.shader_program);

  glVertexAttribPointer(_gl_color_hdl.i_position_var_pos,
                        2, GL_FLOAT, GL_FALSE, 0, tmp_vertices);
  glEnableVertexAttribArray(_gl_color_hdl.i_position_var_pos);

  glUniform4fv(_gl_color_hdl.i_color_var_pos, 1, color_rgba);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
