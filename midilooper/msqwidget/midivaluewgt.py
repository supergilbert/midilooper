# Copyright 2012-2014 Gilbert Romer

# This file is part of gmidilooper.

# gmidilooper is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# gmidilooper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU Gneneral Public License
# along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>.

import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

from wgttools import *

NO_MODE     = 0
EDIT_MODE   = 1
WRITE_MODE  = 2
SELECT_MODE = 3

class MsqValueWidget(gtk.Widget, Xpos2Tick):
    def resize_wgt(self):
        width = self.setting.getmaxwidth()
        self.set_size_request(width, -1)

    def draw_bar_area(self, tick, val, maxheight, area):
        xpos = self.tick2xpos(tick)
        ypos = (127 - val) * maxheight / 127
        if area.y < ypos:
            if ypos <= area.y + area.height:
                height = area.height + area.y - ypos
                self.window.draw_rectangle(self.fg_gc,
                                           True,
                                           xpos,
                                           ypos,
                                           self.bar_width,
                                           height)
        else:
            self.window.draw_rectangle(self.fg_gc,
                                       True,
                                       xpos,
                                       area.y,
                                       self.bar_width,
                                       area.height)

    def _draw_note_area(self, ev, maxheight, area):
        if ev[2] == self.param:
            self.draw_bar_area(ev[0], ev[4], maxheight, area)

    def _draw_ctrl_area(self, ev, maxheight, area):
        if ev[2] == MIDI_CTRL_EVENT and ev[3] == self.param:
            self.draw_bar_area(ev[0], ev[4], maxheight, area)

    def _get_note_tick(self, tick_min, ev_list):
        if self.setting.note_widget.selection and len(ev_list) != 0:
            notelist = evwr_to_repr_list(self.setting.note_widget.selection)
            ev_list.reverse()
            for ev in ev_list:
                if ev[2] == self.param and evrepr_is_in_notelist(ev, notelist):
                    return ev[0]
        return None

    def _get_ctrl_tick(self, tick_min, ev_list):
        if len(ev_list) != 0:
            ev_list.reverse()
            for ev in ev_list:
                if ev[2] == MIDI_CTRL_EVENT and ev[3] == self.param:
                    return ev[0]
        return tick_min

    def ypos_to_val(self, ypos, maxheight):
        return (maxheight - ypos) * 127 / maxheight

    def _write_notebar(self):
        maxheight = self.window.get_size()[1]
        note_sel_idx = 0 if self.param == MIDI_NOTEON_EVENT else 1
        if self.setting.note_widget.selection:
            for bar in self.data_cache:
                note_list = map(lambda ev_note_onoff: ev_note_onoff[note_sel_idx],
                                self.setting.note_widget.selection)
                evwr_list = filter(lambda evwr: evwr.get_event()[0] == bar[0],
                                   note_list)
                if evwr_list:
                    value = self.ypos_to_val(bar[1], maxheight)
                    for evwr in evwr_list:
                        evwr.set_note_vel(value)
        # return (bar[0], self.setting.chan_num, self.param, ???, self.ypos_to_val(bar[1]))

    def _write_ctrlbar(self):
        maxheight = self.window.get_size()[1]
        ev_list = []
        for bar in self.data_cache:
            ev_list.append((bar[0],
                            self.setting.chan_num,
                            MIDI_CTRL_EVENT,
                            self.param,
                            self.ypos_to_val(bar[1], maxheight)))
        if len(ev_list): # tmp
            self.setting.track.add_evrepr_list(ev_list)

    def __init__(self, setting):
        gtk.Widget.__init__(self)
        setting.value_widget = self
        self.setting = setting
        self.bar_width = 3
        self._draw_val_func = self._draw_note_area
        self._get_tick_func = self._get_note_tick
        self._write_bar_func = self._write_notebar
        self.param = MIDI_NOTEON_EVENT
        self.wgt_mode = NO_MODE
        self.data_cache = []
        self.select_area = None
        self.selection = None

    def set_note_mode(self, value):
        self._draw_val_func  = self._draw_note_area
        self._get_tick_func  = self._get_note_tick
        self._write_bar_func = self._write_notebar
        self.param = value

    def set_ctrl_mode(self, value):
        self._draw_val_func  = self._draw_ctrl_area
        self._get_tick_func  = self._get_ctrl_tick
        self._write_bar_func = self._write_ctrlbar
        self.param = value

    def draw_bar(self, tick, ypos):
        winsize = self.window.get_size()
        xpos = self.tick2xpos(tick)
        height = winsize[1] - ypos
        if height >= 0:
            self.window.draw_rectangle(self.bg_gc,
                                       True,
                                       xpos,
                                       0,
                                       self.bar_width,
                                       winsize[1])
            self.window.draw_rectangle(self.fg_gc,
                                       True,
                                       xpos,
                                       ypos,
                                       self.bar_width,
                                       height)

    def is_in_note_mode(self):
        return self._draw_note_area == self._draw_val_func

    def draw_value_at(self, tick_max, ypos):
        ypos = ypos if ypos >= 0 else 0
        tick_min = self.setting.quantify_tick(tick_max)
        ev_list = self.setting.track.getall_midicev(self.setting.chan_num,
                                                    tick_min,
                                                    tick_max)
        tick = self._get_tick_func(tick_min, ev_list)
        if tick != None:
            self.draw_bar(tick, int(ypos))
            return [tick, int(ypos)]
        else:
            return None

    def handle_button_press(self, widget, event):
        self.grab_focus()
        if event.button == 3:
            self.wgt_mode = EDIT_MODE
            self.window.set_cursor(cursor_pencil)
        elif event.button == 1:
            if self.wgt_mode == EDIT_MODE:
                self.wgt_mode = WRITE_MODE
                tick = self.xpos2tick(event.x)
                bar = self.draw_value_at(tick, event.y)
                if bar:
                    self.data_cache = []
                    self.data_cache.append(bar)
            else:
                self.data_cache = event.x
                self.wgt_mode = SELECT_MODE

    def unset_selection(self):
        if self.selection:
            area = gdk.Rectangle(int(self.selection[0]),
                                 0,
                                 int(self.selection[1] - self.selection[0]),
                                 int(self.window.get_size()[1]))
            self.selection = None
            self.draw_area(area)

    def handle_button_release(self, widget, event):
        if event.button == 1:
            if self.wgt_mode == WRITE_MODE:
                self.data_cache.sort(key=lambda bar: bar[0])
                self._write_bar_func()
                self.setting.note_widget.redraw_selection()
                self.data_cache = []
                self.wgt_mode = EDIT_MODE
            elif self.wgt_mode == SELECT_MODE:
                if self.selection:
                    old_area = gdk.Rectangle(int(self.selection[0]),
                                             0,
                                             int(self.selection[1] - self.selection[0]),
                                             int(self.window.get_size()[1]))
                else:
                    old_area = None
                if self.is_in_note_mode():
                    self.selection = None
                else:
                    if event.x < self.data_cache:
                        self.selection = [event.x, self.data_cache]
                    elif event.x > self.data_cache:
                        self.selection = [self.data_cache, event.x]
                    else:
                        self.selection = None
                    if old_area:
                        self.draw_area(old_area)
                    self.setting.note_widget.unset_selection()
                if self.select_area:
                    self.draw_area(self.select_area)
                    self.select_area = None
                self.wgt_mode = NO_MODE
                self.window.set_cursor(current_cursor)
        else:
            self.wgt_mode = NO_MODE
            self.window.set_cursor(current_cursor)

    @staticmethod
    def bar_in_list(bar, bar_list):
        for tmp_bar in bar_list:
            if tmp_bar[0] == bar[0]:
                return tmp_bar
        return None

    def handle_motion(self, widget, event):
        if self.wgt_mode == WRITE_MODE:
            tick = self.xpos2tick(event.x)
            bar = self.draw_value_at(tick, event.y)
            if bar:
                found_bar = self.bar_in_list(bar, self.data_cache)
                if found_bar:
                    found_bar[1] = bar[1]
                else:
                    self.data_cache.append(bar)
        elif self.wgt_mode == SELECT_MODE:
            if event.x > self.data_cache:
                xmax = int(event.x)
                xmin = int(self.data_cache)
            else:
                xmax = int(self.data_cache)
                xmin = int(event.x)
            if self.select_area:
                self.draw_area(self.select_area)
            ymax = int(self.window.get_size()[1])
            ymin = 0
            self.select_area = gtk.gdk.Rectangle(xmin - 2 - self.bar_width,
                                                 ymin,
                                                 xmax - xmin + 4 + self.bar_width,
                                                 ymax)
            cr = self.window.cairo_create()
            cr.set_source_rgb(0, 0, 0)
            cr.set_line_width(2)
            cr.rectangle(xmin, ymin + 1, xmax - xmin, ymax - 2)
            cr.stroke()

    def handle_key_release(self, widget, event):
        # Deleting control event
        if (not self.is_in_note_mode()) and self.selection:
            xmin = self.selection[0]
            xmax = self.selection[1]
            self.selection = None

            tick_min = self.xpos2tick(xmin)
            tick_max = self.xpos2tick(xmax)
            if tick_min < 0:
                tick_min = 0

            evwr_list = self.setting.track.sel_ctrl_evwr(self.setting.chan_num,
                                                         tick_min,
                                                         tick_max,
                                                         self.param)
            self.setting.track._delete_evwr_list(evwr_list)

            self.draw_value_area(int(xmin), int(xmax - xmin))

    def do_realize(self):
        self.set_flags(gtk.REALIZED)
        self.window = gdk.Window(self.get_parent_window(),
                                 width=self.allocation.width,
                                 height=self.allocation.height,
                                 window_type=gdk.WINDOW_CHILD,
                                 wclass=gdk.INPUT_OUTPUT,
                                 event_mask=self.get_events() | gdk.EXPOSURE_MASK | gdk.BUTTON_PRESS_MASK | gdk.BUTTON_RELEASE_MASK | gdk.KEY_PRESS_MASK | gdk.POINTER_MOTION_MASK | gdk.POINTER_MOTION_HINT_MASK)
        self.window.set_user_data(self)
        self.window.move_resize(*self.allocation)

        self.fg_gc = self.style.fg_gc[gtk.STATE_NORMAL]
        self.dark_gc = self.style.dark_gc[gtk.STATE_NORMAL]
        self.bg_gc = self.style.bg_gc[gtk.STATE_NORMAL]

        self.window.set_cursor(current_cursor)

        self.connect("key_release_event", self.handle_key_release)
        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("motion_notify_event", self.handle_motion)
        self.set_can_focus(True)

    def do_unrealize(self):
        self.window.set_user_data(None)

    def draw_area(self, area):
        self.window.draw_rectangle(self.bg_gc,
                                   True,
                                   area.x,
                                   area.y,
                                   area.width,
                                   area.height)
        if not self.is_in_note_mode() and self.selection:
            if self.selection[0] <= area.x:
                xmin = area.x
            else:
                xmin = int(self.selection[0])
            xmax = area.x + area.width
            if xmin <= xmax and xmin < self.selection[1]:
                if self.selection[1] <= xmax:
                    width = int(self.selection[1] - xmin)
                else:
                    width = xmax - xmin
                self.window.draw_rectangle(self.dark_gc, True, xmin, area.y, width, area.height)
        tick_min = self.xpos2tick(area.x)
        tick_max = self.xpos2tick(area.x + area.width)
        ev_list = self.setting.track.getall_midicev(self.setting.chan_num, tick_min, tick_max)
        winsize = self.window.get_size()
        for ev in ev_list:
            self._draw_val_func(ev, winsize[1], area)

    def draw_value_area(self, x, width):
        "Used for drawing the area without height limitation of the area"
        area = gtk.gdk.Rectangle(x, 0, width, self.window.get_size()[1])
        self.draw_area(area)

    def do_expose_event(self, event):
        self.draw_area(event.area)

gobject.type_register(MsqValueWidget)
