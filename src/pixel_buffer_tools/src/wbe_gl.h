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

#pragma once

#include "pbt_type.h"

EXTERN_C_BEGIN

#include "wbe_type.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

wbe_bool_t wbe_gl_shader_program_init(GLuint *shader_program,
                                      GLchar *vshader_str,
                                      GLchar *fshader_str);

#define _wbe_gl_resize(width, height) glViewport(0, 0, (width), (height))

#define wbe_gl_del_texture(_texture_id) glDeleteTextures(1, (_texture_id))

void wbe_gl_gen_texture(GLuint *texture_id);

#define wbe_gl_bind_texture(_texture_id)        \
  glBindTexture(GL_TEXTURE_2D, (_texture_id))

void wbe_gl_texture_destroy(void);

void wbe_gl_texture_init(void);

void _wbe_gl_texture_load(unsigned char *buffer,
                          const unsigned int width,
                          const unsigned int height);

void wbe_gl_texture_put_rect(GLfloat xmin, GLfloat ymin,
                             GLfloat xmax, GLfloat ymax);

void wbe_gl_texture_refresh(void);

void wbe_gl_texture_n_color_destroy(void);

void wbe_gl_texture_n_color_init(void);

void _wbe_gl_texture_n_color_put_rect(GLfloat xmin, GLfloat ymin,
                                      GLfloat xmax, GLfloat ymax,
                                      GLfloat *color_vec4);

void wbe_gl_color_destroy(void);

void wbe_gl_color_init(void);

void wbe_gl_color_put_line(GLfloat x1, GLfloat y1,
                           GLfloat x2, GLfloat y2,
                           GLfloat *color_vec3);

void wbe_gl_color_put_rect(GLfloat xmin, GLfloat ymin,
                           GLfloat xmax, GLfloat ymax,
                           GLfloat *color_vec3);

#define wbe_gl_flush() glFlush()

EXTERN_C_END
