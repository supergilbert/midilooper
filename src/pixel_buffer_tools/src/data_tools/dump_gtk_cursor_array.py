#!/usr/bin/python3

# Copyright 2012-2020 Gilbert Romer

# This file is part of midilooper.

# midilooper is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# midilooper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU Gneneral Public License
# along with midilooper.  If not, see <http://www.gnu.org/licenses/>.

import gi
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk

import os, sys

def gdkcursor_to_pbtcursor_str(gdk_cursor_type, cursor_name):
    cursor = Gdk.Cursor(gdk_cursor_type)
    pixbuf = cursor.get_image()
    width = pixbuf.get_width()
    height = pixbuf.get_height()
    row_stride = pixbuf.get_rowstride()
    n_channels = pixbuf.get_n_channels()
    img_data = pixbuf.get_pixels()
    unused_surface, xhot, yhot = cursor.get_surface()

    str_list = []
    y_idx = 0
    while y_idx < height:
        x_idx = 0
        while x_idx < width:
            idx = x_idx * n_channels + y_idx * row_stride
            color = []
            color.append(img_data[idx])
            color.append(img_data[idx + 1])
            color.append(img_data[idx + 2])
            if n_channels > 3:
                color.append(img_data[idx + 3])
            else:
                color.append(0xFF)
            str_list.append("%s, %s, %s, %s" % (color[0],
                                                color[1],
                                                color[2],
                                                color[3]))
            x_idx += 1
        y_idx += 1

    return  \
        "static unsigned char {cursor_name}_pixels[] = {pxl_list};\n".format(cursor_name = cursor_name,
                                                             pxl_list = "{" + ", ".join(str_list) + "}") \
        + \
        "static pbt_cursor_t " + cursor_name + " = {\n" \
        + \
        "  .pixbuf = {\n" \
        + \
        "    .width = {width},\n" \
        "    .height = {height},\n" \
        "    .pixels = {cursor_name}_pixels\n".format(width = width,
                                                      height = height,
                                                      cursor_name = cursor_name) \
        + \
        "  },\n" \
        + \
        "  .xhot = {xhot},\n" \
        "  .yhot = {yhot},\n".format(xhot = int(xhot),
                                     yhot = int(yhot)) \
        + \
        "};"

print("#pragma once\n\n"
      + '#include "pbt_cursor_inc.h"\n\n'
      + gdkcursor_to_pbtcursor_str(Gdk.CursorType.PENCIL, "pencil_cursor") + "\n\n"
      + gdkcursor_to_pbtcursor_str(Gdk.CursorType.LEFT_PTR, "left_ptr_cursor") + "\n\n"
      + gdkcursor_to_pbtcursor_str(Gdk.CursorType.FLEUR, "fleur_cursor") + "\n\n"
      + gdkcursor_to_pbtcursor_str(Gdk.CursorType.SB_H_DOUBLE_ARROW, "hdouble_arrow_cursor") + "\n\n"
      + gdkcursor_to_pbtcursor_str(Gdk.CursorType.SB_V_DOUBLE_ARROW, "vdouble_arrow_cursor"))
