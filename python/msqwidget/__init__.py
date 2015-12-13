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
    print "PyGtk 2.8 or later required"
    raise SystemExit

from wgttools import Xpos2Tick, Ypos2Note, MIDI_NOTEOFF_EVENT, MIDI_NOTEON_EVENT, evwr_to_repr_list

# Quater note x size
MIN_QNOTE_XSZ = 10
DEFAULT_QNOTE_XSZ = 70
MAX_QNOTE_XSZ = 360

DEFAULT_FONT_NAME = "-misc-fixed-medium-r-normal--10-70-100-100-c-60-iso8859-1"
# DEFAULT_FONT_NAME = "-misc-fixed-medium-r-normal--6-60-75-75-c-40-iso8859-*"
# DEFAULT_FONT_NAME = "-misc-fixed-medium-r-normal--13-120-75-75-c-120-*-*"

NOTE_MAX = 127

default_font        = gdk.Font(DEFAULT_FONT_NAME)
DEFAULT_NOTE_YSZ    = default_font.string_height("C -10X") + 4


class ProgressLineListener(object):
    def __init__(self):
        self.prev_line_xpos   = None
        self.prev_line_ypos   = 0
        self.prev_line_height = 0

    def clear_progressline(self):
        if not self.window:
            return
        self.buffer_refresh_area([self.prev_line_xpos,
                                  self.prev_line_ypos,
                                  1,
                                  self.prev_line_height])
        self.prev_line_xpos = None

    def _update_pos(self, xpos):
        if not self.window:
            return
        line_height = self.window.get_size()[1]
        line_ypos = 0

        if self.prev_line_xpos:
            self.buffer_refresh_area([self.prev_line_xpos,
                                      self.prev_line_ypos,
                                      1,
                                      self.prev_line_height])

        xpos_adj = xpos - self.xadj

        self.window.draw_line(self.style.fg_gc[gtk.STATE_NORMAL],
                              xpos_adj, line_ypos, xpos_adj, line_ypos + line_height)

        if self.prev_line_ypos != line_ypos:
            self.prev_line_ypos = line_ypos
        if self.prev_line_height != line_height:
            self.prev_line_height = line_height
        self.prev_line_xpos = xpos_adj


class MsqHBarTimeWidget(gtk.Widget, Xpos2Tick):
    def do_realize(self):
        self.set_flags(gtk.REALIZED)
        self.window = gdk.Window(self.get_parent_window(),
                                 width=self.allocation.width,
                                 height=self.allocation.height,
                                 window_type=gdk.WINDOW_CHILD,
                                 wclass=gdk.INPUT_OUTPUT,
                                 event_mask=self.get_events() | gdk.EXPOSURE_MASK)
        self.window.set_user_data(self)
        self.window.move_resize(*self.allocation)

        self.dark_gc = self.style.dark_gc[gtk.STATE_NORMAL]
        self.fg_gc = self.style.fg_gc[gtk.STATE_NORMAL]

        self.setting.hadj.set_all(0,
                                  0,
                                  self.setting.getmaxwidth(),
                                  0,
                                  0,
                                  self.window.get_size()[0])

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.height = self.height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        self.setting.hadj.set_page_size(self.allocation.width)
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def draw_area(self, area):
        self.window.draw_rectangle(self.dark_gc,
                                   True,
                                   area.x,
                                   area.y,
                                   area.width,
                                   area.height)

        xmax = area.x + area.width + self.xadj
        ymax = area.y + area.height
        # if ymax > self.max_height:
        #     ymax = self.max_height
        xpos = (area.x + self.xadj) - ((area.x + self.xadj) % self.setting.qnxsz)
        time_pos = xpos / self.setting.qnxsz

        mypos = self.font_height * 2
        nypos = self.font_height * 5 / 2
        while xpos <= xmax:
            if (time_pos % self.setting.mlen) == 0:
                self.window.draw_string(default_font, self.fg_gc, xpos - self.xadj, 2 + self.font_height, "%i" % time_pos)
                ypos = mypos
            else:
                ypos = nypos
            if area.y > ypos:
                ypos = area.y
            self.window.draw_line(self.fg_gc, xpos - self.xadj, ypos, xpos - self.xadj, ymax)
            time_pos += 1
            xpos += self.setting.qnxsz

        ypos = self.height - 1
        self.window.draw_line(self.fg_gc, area.x, ypos, area.x + area.width, ypos)

    def do_expose_event(self, event):
        self.draw_area(event.area)

    def __init__(self, setting):
        gtk.Widget.__init__(self)

        self.setting = setting

        self.font_height = default_font.string_height("3600")
        self.height = (self.font_height + 1) * 3
        Xpos2Tick.__init__(self)

class MsqVBarNoteWidget(gtk.Widget, Ypos2Note):
    def clear_note(self):
        if self.last_shown_note:
            xpos = self.piano_xpos
            width = self.width - xpos
            height = self.setting.noteysz
            ypos = self.note2ypos(self.last_shown_note)
            area = gtk.gdk.Rectangle(xpos, ypos, width, height)
            self.draw_area(area)
            self.last_shown_note = None

    def draw_area(self, area):
        xmax = area.x + area.width
        ymax = area.y + area.height

        self.window.draw_rectangle(self.bg_gc,
                                   True,
                                   area.x,
                                   area.y,
                                   area.width,
                                   area.height)

        if area.x > self.piano_xpos:
            piano_xpos = area.x
        else:
            piano_xpos = self.piano_xpos
            self.window.draw_line(self.fg_gc, piano_xpos, area.y, piano_xpos, ymax)
        win_xsize , win_ysize = self.window.get_size()
        if (xmax >= win_xsize):
            self.window.draw_line(self.fg_gc, win_xsize - 1, area.y, win_xsize - 1, ymax)

        ymax = ymax + self.yadj
        ypos = area.y + self.yadj
        ymod = ypos % self.setting.noteysz
        if ymod:
            ypos = ypos + self.setting.noteysz - ymod
        note_pos = ypos / self.setting.noteysz
        octave = ((128 - note_pos) / 12) - 1
        while ypos < ymax:
            yadj = ypos - self.yadj
            self.window.draw_line(self.fg_gc, piano_xpos, yadj, xmax, yadj)
            note_pos += 1
            note_val = (128 - note_pos)
            note_key = note_val % 12
            if note_key == 0:
                if area.x < piano_xpos:
                    self.window.draw_line(self.fg_gc, area.x, yadj + self.setting.noteysz, piano_xpos, yadj + self.setting.noteysz)
                self.window.draw_string(default_font, self.fg_gc, 1, yadj + self.setting.noteysz - 1, "%03d C %i" % (note_val, octave))
                octave -= 1
            elif note_key == 1 or note_key == 3 or note_key == 6 or note_key == 8 or note_key == 10:
                self.window.draw_rectangle(self.fg_gc, True, piano_xpos, yadj, (xmax - piano_xpos), self.setting.noteysz)
            ypos += self.setting.noteysz

    def show_note(self, note):
        xpos = self.piano_xpos
        width = self.width - xpos
        height = self.setting.noteysz

        if self.last_shown_note:
            ypos = (127 - self.last_shown_note) * self.setting.noteysz - self.yadj
            area = gtk.gdk.Rectangle(xpos, ypos, width, height)
            self.draw_area(area)

        ypos = (127 - note) * self.setting.noteysz - self.yadj
        cr = self.window.cairo_create()
        cr.set_source_rgba(0.5, 0.5, 0.5, 0.5)
        cr.rectangle(xpos, ypos, width, height)
        cr.fill()
        self.last_shown_note = note

    def handle_button_press(self, widget, event):
        if event.x < self.piano_xpos:
            return
        note = self.ypos2note(int(event.y))
        self.show_note(note)
        port = self.setting.track.get_output()

        self.setting.track.play_note(self.setting.chan_num,
                                     MIDI_NOTEON_EVENT,
                                     note,
                                     int(self.setting.note_valadj.get_value()))
        self.last_play_note = note

    def handle_button_release(self, widget, event):
        self.clear_note()
        note = self.ypos2note(int(event.y))
        port = self.setting.track.get_output()
        self.setting.track.play_note(self.setting.chan_num,
                                     MIDI_NOTEOFF_EVENT,
                                     note,
                                     DEFAULT_NOTEOFF_VAL)
        self.last_play_note = None

    def handle_motion(self, widget, event):
        if self.last_play_note == None:
            return
        note = self.ypos2note(int(event.y))
        self.show_note(note)
        # if self.tracked and self.tracked.sequencer.isrunning(): # tmp hack to prevent segfault
        #     print "Can't play note while sequencer is running"
        #     return
        if note != self.last_play_note:
            self.setting.track.play_note(self.setting.chan_num,
                                         MIDI_NOTEOFF_EVENT,
                                         self.last_play_note,
                                         DEFAULT_NOTEOFF_VAL)
            self.setting.track.play_note(self.setting.chan_num,
                                         MIDI_NOTEON_EVENT,
                                         note,
                                         int(self.setting.note_valadj.get_value()))
            self.last_play_note = note

    def __init__(self, setting):
        gtk.Widget.__init__(self)

        self.setting = setting
        self.width = default_font.string_width("00 C -10X") * 2
        self.piano_xpos = self.width / 2

        self.last_play_note = None

        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("motion_notify_event", self.handle_motion)
        Ypos2Note.__init__(self)

    def do_realize(self):
        self.set_flags(gtk.REALIZED)
        self.window = gdk.Window(self.get_parent_window(),
                                 width=self.allocation.width,
                                 height=self.allocation.height,
                                 window_type=gdk.WINDOW_CHILD,
                                 wclass=gdk.INPUT_OUTPUT,
                                 event_mask=self.get_events() | gdk.EXPOSURE_MASK | gdk.BUTTON_PRESS_MASK | gdk.BUTTON_RELEASE_MASK | gdk.POINTER_MOTION_MASK | gdk.POINTER_MOTION_HINT_MASK)
        self.window.set_user_data(self)
        self.window.move_resize(*self.allocation)

        self.fg_gc = self.style.fg_gc[gtk.STATE_NORMAL]
        self.bg_gc = self.style.bg_gc[gtk.STATE_NORMAL]

        self.last_shown_note = None

        self.setting.vadj.set_all(self.note2ypos(76),
                                  0,
                                  self.setting.getmaxheight(),
                                  0,
                                  0,
                                  self.window.get_size()[1])

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width = self.width
        # requisition.height = self.max_height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        self.setting.vadj.set_page_size(self.allocation.height)
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def do_expose_event(self, event):
        self.draw_area(event.area)


from notegridevt import MsqNGWEventHdl, DEFAULT_NOTEON_VAL, DEFAULT_NOTEOFF_VAL

NOTE_PX_SIZE = 8


class ChannelEditorSetting(object):

    def getmaxwidth(self):
        # The "+ 1" in the following operation is a tmp hack
        # Need to display loop and to take the last note as a reference to set
        # the width.
        return self.qnxsz * (((self.getlen() + self.getstart()) / self.getppq()) + 1)

    def getmaxheight(self):
        return (NOTE_MAX + 1) * self.noteysz

    def getppq(self):
        return self.sequencer.getppq()

    def getlen(self):
        return self.track.get_len()

    def getstart(self):
        return self.track.get_start()

    def quantify_tick(self, tick):
        return int(tick / self.tick_res) * self.tick_res

    def __init__(self, track, sequencer, chan_num=0, qnxsz=DEFAULT_QNOTE_XSZ):
        self.track = track
        self.sequencer = sequencer

        self.qnxsz   = qnxsz
        self.noteysz = DEFAULT_NOTE_YSZ

        self.mlen = 4   # number of beat per measure

        self.tick_res = int(self.getppq() / 4)

        self.chan_num = chan_num
        self.note_widget = None
        self.value_widget = None
        self.note_valadj = gtk.Adjustment(DEFAULT_NOTEON_VAL, 0, 127, 1)

        self.hadj = gtk.Adjustment()
        self.vadj = gtk.Adjustment()


class MsqNoteGridWidget(gtk.Widget, ProgressLineListener, MsqNGWEventHdl, Xpos2Tick, Ypos2Note):
    def update_pos(self, tickpos):
        xpos = int(self.setting.track.get_loop_pos(tickpos) * self.setting.qnxsz / self.setting.getppq())
        self._update_pos(xpos)

    def clear_progress(self):
        self.clear_progressline()

    def set_scale(self, note):
        self.scale_notes = [(note + 1) % 12,
                            (note + 3) % 12,
                            (note + 6) % 12,
                            (note + 8) % 12,
                            (note + 10) % 12]

    def __init__(self, track, sequencer, chan_num=0):
        gtk.Widget.__init__(self)
        self.setting = ChannelEditorSetting(track, sequencer, chan_num)
        self.setting.note_widget = self
        Xpos2Tick.__init__(self)
        Ypos2Note.__init__(self)
        MsqNGWEventHdl.__init__(self)
        ProgressLineListener.__init__(self)
        self.max_height = (NOTE_MAX + 1) * self.setting.noteysz + 1
        self.selection = None
        self.buffer_img = None  # TODO search for the best buffer
        # self.set_flags(gtk.CAN_DEFAULT)
        self.set_scale(0)

    def draw_note(self, drawable, note_on, note_off, selected=False):
        xmin = self.tick2xpos(note_on[0])
        xmax = self.tick2xpos(note_off[0])
        width = xmax - xmin
        ypos = self.note2ypos(note_on[3]) + 1
        self.draw_note_rectangle(drawable,
                                 xmin, ypos, width, self.setting.noteysz - 1,
                                 note_on[4], selected)

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

        self.buffer_img = gtk.gdk.Pixmap(self.window, self.allocation[2], self.allocation[3], -1)

        self.grid_fg = gtk.gdk.Color(0.0, 0.0, 0.0)
        self.grid_scale_bg = gtk.gdk.Color(0.6, 0.6, 0.6)
        self.grid_bg = gtk.gdk.Color(0.8, 0.8, 0.8)
        self.grid_light = gtk.gdk.Color(((self.grid_fg.red   * 4) + self.grid_bg.red)   / 5,
                                        ((self.grid_fg.green * 4) + self.grid_bg.green) / 5,
                                        ((self.grid_fg.blue  * 4) + self.grid_bg.blue)  / 5)
        self.grid_res_col = gtk.gdk.Color((self.grid_light.red   + self.grid_bg.red)   / 2,
                                          (self.grid_light.green + self.grid_bg.green) / 2,
                                          (self.grid_light.blue  + self.grid_bg.blue)  / 2)
        self.sel_col = gtk.gdk.Color(0.5, 0.9, 0.5)
        self.realize_noteonoff_handler()

        self.setting.vadj.set_value((self.setting.vadj.get_lower() + self.setting.vadj.get_upper() / 2))
        self.setting.vadj.value_changed()

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = 320
        requisition.height = 240

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)
            self.buffer_img = gtk.gdk.Pixmap(self.window, self.allocation[2], self.allocation[3], -1)

    def draw_grid(self, area):
        cr = self.buffer_img.cairo_create()
        cr.set_source_color(self.grid_bg)
        cr.rectangle(area.x, area.y, area.width, area.height)
        cr.fill()

        xmax = area.x + area.width
        ymax = area.y + area.height
        if ymax > self.max_height:
            ymax = self.max_height

        cr.set_line_width(1)

        note = self.ypos2note(area.y)
        ypos = self.note2ypos(note)
        while ypos < ymax:
            if (note % 12) in self.scale_notes:
                cr.set_source_color(self.grid_scale_bg)
                scale_height = self.setting.noteysz if (ypos  + self.setting.noteysz) <= ymax else (ymax - ypos)
                if ypos < area.y:
                    scale_height = scale_height - (area.y - ypos)
                    ypos = area.y
                if scale_height > 0:
                    cr.rectangle(area.x, ypos, area.width, scale_height)
                    cr.fill()
            note -= 1
            ypos = self.note2ypos(note)

        # Detecting first line to draw
        tick = self.xpos2tick(area.x)
        modtick = tick % self.setting.tick_res
        if modtick != 0:
            tick = tick + self.setting.tick_res - modtick
        xpos = self.tick2xpos(tick)
        while xpos <= xmax:
            if (tick % self.setting.getppq()) == 0:
                if (tick % self.setting.mlen) == 0:
                    cr.set_source_color(self.grid_fg)
                else:
                    cr.set_source_color(self.grid_light)
            else:
                cr.set_source_color(self.grid_res_col)
            cr.move_to(xpos + 0.5, area.y)
            cr.line_to(xpos + 0.5, ymax + 0.5)
            cr.stroke()
            tick += self.setting.tick_res
            xpos = self.tick2xpos(tick)

        note = self.ypos2note(area.y)
        ypos = self.note2ypos(note)
        while ypos < ymax:
            if ((note + 1) % 12) == 0:
                cr.set_source_color(self.grid_fg)
            else:
                cr.set_source_color(self.grid_light)
            cr.move_to(area.x, ypos + 0.5)
            cr.line_to(xmax,   ypos + 0.5)
            cr.stroke()
            note -= 1
            ypos = self.note2ypos(note)

    def is_in_note_list(self, event, note_list):
        if not note_list:
            return False
        for ev_noteon, ev_noteoff in note_list:
            if event == ev_noteon or event == ev_noteoff:
                return True
        return False

    def draw_note_rectangle(self, drawable, x, y, width, height, value, selected):
        def ponder_color(coef, color1, color2):
            def ponder_value(coef, value1, value2):
                dist = value2 - value1
                return value1 + (dist * coef)
            red   = ponder_value(coef, color1[0], color2[0])
            green = ponder_value(coef, color1[1], color2[1])
            blue  = ponder_value(coef, color1[2], color2[2])
            return red, green, blue
        def get_note_color(value):
            if value > 255.0 or value < 0.0:
                print "ERROR in get_note_color value is not between 0 255"
                return
            first_color  = [50.0, 50.0, 70.0]
            second_color = [0.0, 179.0, 0.0]
            third_color  = [217.0, 217.0, 0.0]
            fouth_color  = [255.0, 0.0, 0.0]

            first_threshold  = 64.0
            second_threshold = 96.0

            red_value   = 0.0
            green_value = 0.0
            blue_value  = 0.0

            if value <= first_threshold:
                coef = value / first_threshold
                red_value, green_value, blue_value = ponder_color(coef, first_color, second_color)
            elif value <= second_threshold:
                coef = (value - first_threshold) / (second_threshold - first_threshold)
                red_value, green_value, blue_value = ponder_color(coef, second_color, third_color)
            else:
                coef = (value - second_threshold) / (127.0 - second_threshold)
                red_value, green_value, blue_value = ponder_color(coef, third_color, fouth_color)

            red_value   = red_value / 256.0
            green_value = green_value / 256.0
            blue_value  = blue_value / 256.0
            return gtk.gdk.Color(red=red_value, green=green_value, blue=blue_value)

        note_color = get_note_color(value)
        cr = drawable.cairo_create()
        cr.set_source_color(note_color)
        cr.rectangle(x, y, width, height)
        cr.fill()

        if selected:
            cr.set_source_color(self.sel_col)
        else:
            cr.set_source_color(self.grid_fg)
        cr.set_line_width(1)
        cr.rectangle(x + 0.5, y - 0.5, width, height + 1)
        cr.stroke()

        # if selected:
        #     cr.set_source_rgba(0, 0, 0, 0.5)
        #     cr.rectangle(x, y, width, height)
        #     cr.fill()

    def get_notes(self, rectangle):
        tick_min = self.xpos2tick(rectangle.x)
        tick_max = self.xpos2tick(rectangle.x + rectangle.width)
        note_max = self.ypos2note(rectangle.y)
        note_min = self.ypos2note(rectangle.y + rectangle.height)

        if tick_min < 0: tick_min = 0
        if note_min < 0: note_min = 0
        return self.setting.track.sel_noteonoff_repr(self.setting.chan_num,
                                                     tick_min,
                                                     tick_max,
                                                     note_min,
                                                     note_max)

    def draw_notes_bar(self, drawable, area):
        note_list = self.get_notes(area)
        selected_notes = None
        if self.selection:
            selected_notes = evwr_to_repr_list(self.selection)
        for ev_on, ev_off in note_list:
            self.draw_note(drawable, ev_on, ev_off, self.is_in_note_list(ev_on, selected_notes))
        if self.value_wgt.window and self.value_wgt.is_in_note_mode():
            self.value_wgt.draw_value_area(area.x, area.width)

    def buffer_refresh_area(self, area):
        self.window.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                  self.buffer_img,
                                  area[0],
                                  area[1],
                                  area[0],
                                  area[1],
                                  area[2],
                                  area[3])

    def draw_loop_veil(self, area):
        cr = self.buffer_img.cairo_create()
        cr.set_source_rgba(0, 0, 0, 0.5)

        tickstart = self.setting.getstart()
        xmax = area.x + area.width

        xstart = self.tick2xpos(tickstart)
        if area.x < xstart:
            cr.rectangle(area.x, area.y, xstart - area.x, area.height)
            cr.fill()

        xend = self.tick2xpos(tickstart + self.setting.getlen())
        if xend < area.x:
            xend = area.x
        if xend < xmax:
            cr.rectangle(xend, area.y, xmax - xend, area.height)
            cr.fill()

    def draw_area(self, area):
        self.draw_grid(area)
        self.draw_notes_bar(self.buffer_img, area)
        self.draw_loop_veil(area)
        self.buffer_refresh_area(area)

    def do_expose_event(self, event):
        self.draw_area(event.area)


gobject.type_register(MsqHBarTimeWidget)
gobject.type_register(MsqVBarNoteWidget)
gobject.type_register(MsqNoteGridWidget)
