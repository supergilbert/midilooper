# Copyright 2012-2016 Gilbert Romer

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

from gi.repository import GObject
import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from gi.repository import Gdk

from midilooper.msqwidget.wgttools import *

NO_MODE     = 0
EDIT_MODE   = 1
WRITE_MODE  = 2
SELECT_MODE = 3
DEFVAL_MODE = 4

class MsqValueWidget(Gtk.Widget, Xpos2Tick):
    def draw_bar_area(self, cr_ctx, tick, ypos, clip_extents):
        xpos = self.tick2xpos(tick)
        if clip_extents[1] < ypos:
            if ypos <= clip_extents[3]:
                height = clip_extents[3] - ypos
                cr_ctx.set_source_rgb(*self.fg_color)
                cr_ctx.rectangle(xpos, ypos, self.bar_width, height)
                cr_ctx.fill()
        else:
            cr_ctx.set_source_rgb(*self.fg_color)
            cr_ctx.rectangle(xpos,
                             clip_extents[1],
                             self.bar_width,
                             clip_extents[3] - clip_extents[1])
            cr_ctx.fill()

    def _draw_note_area(self, cr_ctx, ev, maxheight, area):
        if ev[2] == self.param:
            ypos = (127 - ev[4]) * maxheight / 127
            self.draw_bar_area(cr_ctx, ev[0], ypos, area)

    def _draw_ctrl_area(self, cr_ctx, ev, maxheight, area):
        if ev[2] == MIDI_CTRL_EVENT and ev[3] == self.ctrl_num:
            ypos = (127 - ev[4]) * maxheight / 127
            self.draw_bar_area(cr_ctx, ev[0], ypos, area)

    def _draw_pitch_area(self, cr_ctx, ev, maxheight, area):
        if ev[2] == MIDI_PITCH_EVENT:
            ypos = (16383 - ((ev[4] * 128) + ev[3])) * maxheight / 16383
            self.draw_bar_area(cr_ctx, ev[0], ypos, area)

    def _get_note_tick(self, tick_min, ev_list):
        if self.setting.note_widget.selection and len(ev_list) != 0:
            notelist = evwr_to_repr_list(self.setting.note_widget.selection)
            ev_list.reverse()
            for ev in ev_list:
                if ev[2] == self.param and evrepr_is_in_notelist(ev, notelist):
                    return ev[0]
        return None

    def _get_notnote_tick(self, evtype, tick_min, ev_list):
        if len(ev_list) != 0:
            ev_list.reverse()
            for ev in ev_list:
                if ev[2] == evtype and ev[3] == self.param:
                    return ev[0]
        return tick_min

    def _get_ctrl_tick(self, tick_min, ev_list):
        return self._get_notnote_tick(MIDI_CTRL_EVENT, tick_min, ev_list)

    def _get_pitch_tick(self, tick_min, ev_list):
        return self._get_notnote_tick(MIDI_PITCH_EVENT, tick_min, ev_list)

    def ypos_to_val(self, ypos, maxheight):
        return int((maxheight - ypos) * 127 / maxheight)

    def _write_notebar(self):
        maxheight = self.get_window().get_height()
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

    def _write_ctrlbar(self):
        maxheight = self.get_window().get_height()
        ev_list = []
        for bar in self.data_cache:
            ev_list.append((bar[0],
                            self.setting.chan_num,
                            MIDI_CTRL_EVENT,
                            self.ctrl_num,
                            self.ypos_to_val(bar[1], maxheight)))
        if len(ev_list): # tmp
            self.setting.track.add_evrepr_list(ev_list)

    def _write_pitchbar(self):
        maxheight = self.get_window().get_height()
        ev_list = []
        for bar in self.data_cache:
            val = ((maxheight - bar[1]) * 16383) // maxheight
            Hval = val // 128
            Lval = val - (Hval * 128)
            ev_list.append((bar[0],
                            self.setting.chan_num,
                            MIDI_PITCH_EVENT,
                            Lval,
                            Hval))
        if len(ev_list): # tmp
            self.setting.track.add_evrepr_list(ev_list)

    def set_note_mode(self):
        self._draw_val_func  = self._draw_note_area
        self._get_tick_func  = self._get_note_tick
        self._write_bar_func = self._write_notebar
        self.param = MIDI_NOTEON_EVENT
        self.adjwgt.set_adjustment(self.setting.note_valadj)
        if self.spinbut:
            self.spinbut.set_adjustment(self.setting.note_valadj)
        self.setting.note_valadj.value_changed()

    def set_ctrl_mode(self, ctrl_num):
        self._draw_val_func  = self._draw_ctrl_area
        self._get_tick_func  = self._get_ctrl_tick
        self._write_bar_func = self._write_ctrlbar
        self.ctrl_num        = ctrl_num
        self.param = MIDI_CTRL_EVENT
        self.adjwgt.set_adjustment(self.ctrl_adj)
        if self.spinbut:
            self.spinbut.set_adjustment(self.ctrl_adj)
        self.ctrl_adj.value_changed()

    def set_pitch_mode(self):
        self._draw_val_func  = self._draw_pitch_area
        self._get_tick_func  = self._get_pitch_tick
        self._write_bar_func = self._write_pitchbar
        self.param = MIDI_PITCH_EVENT
        self.adjwgt.set_adjustment(self.pitch_adj)
        if self.spinbut:
            self.spinbut.set_adjustment(self.pitch_adj)
        self.pitch_adj.value_changed()

    def __init__(self, setting, adjwgt):
        GObject.GObject.__init__(self)
        setting.value_widget = self
        self.setting = setting
        Xpos2Tick.__init__(self)
        self.bar_width = 3
        self.adjwgt = adjwgt
        self.spinbut = None

        self.ctrl_adj  = Gtk.Adjustment(value=64.0,
                                        lower=0.0,
                                        upper=127.0,
                                        step_increment=1.0)
        self.pitch_adj = Gtk.Adjustment(value=8192.0,
                                        lower=0.0,
                                        upper=16384.0,
                                        step_increment=1.0)
        self.set_note_mode()

        self.fg_color = (0, 0, 0)
        self.bg_color = (0.8, 0.8, 0.8)
        self.dark_color = (0.6, 0.6, 0.6)

        self.wgt_mode = NO_MODE
        self.data_cache = []
        self.select_range = None
        self.selection = None
        self.ctrl_num = 0

    def draw_bar(self, tick, ypos):
        window = self.get_window()
        win_height = window.get_height()
        xpos = self.tick2xpos(tick)
        height = win_height - ypos
        if height >= 0:
            cr_ctx = window.cairo_create()
            cr_ctx.set_source_rgb(*self.bg_color)
            cr_ctx.rectangle(xpos, 0,    self.bar_width, win_height)
            cr_ctx.fill()
            cr_ctx = window.cairo_create()
            cr_ctx.set_source_rgb(*self.fg_color)
            cr_ctx.rectangle(xpos, ypos, self.bar_width, height)
            cr_ctx.fill()

    def is_in_note_mode(self):
        return self._draw_note_area == self._draw_val_func

    def draw_value_at(self, tick_max, ypos):
        height = self.get_window().get_height()
        if ypos > height:
            ypos = height
        elif ypos < 0:
            ypos = 0
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
        if self.wgt_mode == EDIT_MODE and event.button == 1:
            self.wgt_mode = WRITE_MODE
            tick = self.xpos2tick(event.x)
            bar = self.draw_value_at(tick, event.y)
            if bar:
                self.data_cache = []
                self.data_cache.append(bar)
        elif self.wgt_mode == NO_MODE and event.button == 1:
            self.data_cache = event.x
            self.wgt_mode = SELECT_MODE
        elif self.wgt_mode == NO_MODE and event.button == 2:
            self.wgt_mode = DEFVAL_MODE
            self.get_window().set_cursor(cursor_pencil)
        elif self.wgt_mode == NO_MODE and event.button == 3:
            self.wgt_mode = EDIT_MODE
            self.get_window().set_cursor(cursor_pencil)

    def unset_selection(self):
        if self.selection:
            clip = (int(self.selection[0]),
                    int(self.selection[1]) + 1)
            self.selection = None
            self.draw_value_clip(*clip)

    def handle_button_release(self, widget, event):
        if event.button == 1:
            if self.wgt_mode == WRITE_MODE:
                self.data_cache.sort(key=lambda bar: bar[0])
                self._write_bar_func()
                if self.select_range:
                    self.draw_value_clip(*self.select_range)
                self.setting.note_widget.redraw_selection()
                self.wgt_mode = EDIT_MODE
            elif self.wgt_mode == SELECT_MODE:
                if self.selection:
                    old_area = (int(self.selection[0]),
                                int(self.selection[1]) + 1)
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
                        self.draw_value_clip(*old_area)
                    self.setting.note_widget.unset_selection()
                if self.select_range:
                    self.draw_value_clip(*self.select_range)
                    self.select_range = None
                self.wgt_mode = NO_MODE
                self.get_window().set_cursor(current_cursor)
                self.data_cache = []
        elif event.button == 2 and self.wgt_mode == DEFVAL_MODE:
            self.data_cache.sort(key=lambda bar: bar[0])
            self._write_bar_func()
            self.setting.note_widget.redraw_selection()
            self.wgt_mode = NO_MODE
            self.get_window().set_cursor(current_cursor)
            self.data_cache = []
        else:
            self.wgt_mode = NO_MODE
            self.get_window().set_cursor(current_cursor)
            self.data_cache = []

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
            if self.select_range:
                self.draw_value_clip(*self.select_range)
            ymax = int(self.get_window().get_height())
            ymin = 0
            self.select_range = (xmin - 1, xmax + 1)
            cr = self.get_window().cairo_create()
            cr.set_source_rgb(0, 0, 0)
            cr.set_line_width(2)
            cr.rectangle(xmin, ymin + 1, xmax - xmin, ymax - 2)
            cr.stroke()
        elif self.wgt_mode == DEFVAL_MODE:
            winheight = self.get_window().get_height()
            if self.param == MIDI_NOTEON_EVENT:
                ypos = (127 - int(self.setting.note_valadj.get_value())) * winheight / 127
            elif self.param == MIDI_CTRL_EVENT:
                ypos = (127 - int(self.ctrl_adj.get_value())) * winheight / 127
            elif self.param == MIDI_PITCH_EVENT:
                ypos = (16383 - int(self.pitch_adj.get_value())) * winheight / 16383
            tick = self.xpos2tick(event.x)
            bar = self.draw_value_at(tick, ypos)
            if bar:
                found_bar = self.bar_in_list(bar, self.data_cache)
                if found_bar:
                    found_bar[1] = bar[1]
                else:
                    self.data_cache.append(bar)

    def handle_key_press(self, widget, event):
        # Deleting control event
        if (not self.is_in_note_mode()) and self.selection and (event.keyval == Gdk.KEY_Delete or event.keyval == Gdk.KEY_BackSpace):
            xmin = self.selection[0]
            xmax = self.selection[1]
            self.selection = None

            tick_min = self.xpos2tick(xmin)
            tick_max = self.xpos2tick(xmax)
            if tick_min < 0:
                tick_min = 0

            if self.param == MIDI_CTRL_EVENT:
                evwr_list = self.setting.track.sel_ctrl_evwr(self.setting.chan_num,
                                                             tick_min,
                                                             tick_max,
                                                             self.ctrl_num)
            else:
                evwr_list = self.setting.track.sel_pitch_evwr(self.setting.chan_num,
                                                              tick_min,
                                                              tick_max)
            self.setting.track._delete_evwr_list(evwr_list)

            self.draw_value_clip(int(xmin), int(xmax) + 1)

    def do_realize(self):
        # self.set_flags(Gtk.REALIZED)
        self.set_realized(True)
        winattr = Gdk.WindowAttr()
        allocation = self.get_allocation()
        winattr.width = allocation.width
        winattr.height = allocation.height
        winattr.window_type = Gdk.WindowType.CHILD
        winattr.wclass = Gdk.WindowWindowClass.INPUT_OUTPUT
        winattr_type = Gdk.WindowAttributesType(0)
        window = Gdk.Window(self.get_parent_window(), winattr, winattr_type)
        window.set_events(Gdk.EventMask.EXPOSURE_MASK
                          | Gdk.EventMask.BUTTON_PRESS_MASK
                          | Gdk.EventMask.BUTTON_RELEASE_MASK
                          | Gdk.EventMask.KEY_PRESS_MASK
                          | Gdk.EventMask.POINTER_MOTION_MASK
                          | Gdk.EventMask.POINTER_MOTION_HINT_MASK
                          | Gdk.EventMask.SCROLL_MASK)
        window.set_user_data(self)
        window.move_resize(allocation.x,
                           allocation.y,
                           allocation.width,
                           allocation.height)
        self.set_window(window)


        window.set_cursor(current_cursor)

        self.connect("key_press_event", self.handle_key_press)
        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("motion_notify_event", self.handle_motion)
        self.set_can_focus(True)

    def do_unrealize(self):
        self.get_window().set_user_data(None)

    def draw_clip(self, cr_ctx, clip_extents):
        xmin = clip_extents[0]
        ymin = clip_extents[1]
        xmax = clip_extents[2]
        ymax = clip_extents[3]

        cr_ctx.set_source_rgb(*self.bg_color)
        cr_ctx.rectangle(xmin, ymin, xmax - xmin, ymax - ymin)
        cr_ctx.fill()

        if not self.is_in_note_mode() and self.selection:
            if self.selection[0] < xmax and xmin < self.selection[1]:
                selxmin = self.selection[0] if xmin <= self.selection[0] else xmin
                selxmax = self.selection[1] if self.selection[1] <= xmax else xmax
                cr_ctx.set_source_rgb(*self.dark_color)
                cr_ctx.rectangle(selxmin, ymin, selxmax - selxmin, ymax - ymin)
                cr_ctx.fill()


        xmin = xmin - self.bar_width
        if xmin < 0:
            xmin = 0
        tick_min = self.xpos2tick(xmin)
        tick_max = self.xpos2tick(xmax)
        ev_list = self.setting.track.getall_midicev(self.setting.chan_num, tick_min, tick_max)
        window = self.get_window()
        winheigt = window.get_height() - 1 # keep one pixel to mark an event
        for ev in ev_list:
            self._draw_val_func(cr_ctx, ev, winheigt, clip_extents)

    def do_draw(self, cr_ctx):
        clip_extents = cr_ctx.clip_extents()
        self.draw_clip(cr_ctx, clip_extents)

    def draw_value_clip(self, xmin, xmax):
        window = self.get_window()
        clip_extents = (xmin, 0, xmax, window.get_height())
        self.draw_clip(window.cairo_create(), clip_extents)

GObject.type_register(MsqValueWidget)
