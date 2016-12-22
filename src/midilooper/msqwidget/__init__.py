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
gi.require_version('PangoCairo', '1.0')
from gi.repository import Gtk, Gdk

import cairo

from midilooper.msqwidget.wgttools import Xpos2Tick, Ypos2Note, MIDI_NOTEOFF_EVENT, MIDI_NOTEON_EVENT, evwr_to_repr_list

# Quater note x size
DEFAULT_QNOTE_XSZ = 90

# for trackeditor (scale???)
MIN_QNOTE_XSZ = 10
MAX_QNOTE_XSZ = 360

MSQ_FG_COLOR = (0, 0, 0)
MSQ_BG_COLOR = (0.9, 0.9, 0.9)
poid1 = 1
poid2 = 1
poidtot = poid1 + poid2
MSQ_GR_COLOR = ((MSQ_FG_COLOR[0] * poid1) + (MSQ_BG_COLOR[0] * poid2) / poidtot,
                (MSQ_FG_COLOR[1] * poid1) + (MSQ_BG_COLOR[1] * poid2) / poidtot,
                (MSQ_FG_COLOR[2] * poid1) + (MSQ_BG_COLOR[2] * poid2) / poidtot)

NOTE_MAX = 127

def get_default_font_size():
    surface = cairo.ImageSurface(cairo.FORMAT_RGB24, 0, 0)
    context = cairo.Context(surface)
    return context.font_extents()[2]

def get_font_text_extents(text, font_size=None):
    surface = cairo.ImageSurface(cairo.FORMAT_RGB24, 0, 0)
    context = cairo.Context(surface)
    if font_size:
        context.set_font_size(font_size)
    return context.text_extents(text)

def get_font_text_width(string):
    return get_font_text_extents(string)[2]

def get_font_text_height(string):
    return get_font_text_extents(string)[3]

class ProgressLineListener(object):
    def __init__(self):
        self.prev_line_xpos   = None
        self.prev_line_height = None

    def clear_progressline(self):
        if not self.prev_line_xpos:
            return
        self.paste_surface((self.prev_line_xpos - 0.5,
                            0,
                            self.prev_line_xpos + 0.5,
                            self.prev_line_height))
        self.prev_line_xpos = None

    def _update_pos(self, xpos):
        window = self.get_window()
        if not window:
            return
        line_height = window.get_height()
        line_ypos = 0

        if self.prev_line_xpos:
            self.paste_surface((self.prev_line_xpos - 0.5,
                                0,
                                self.prev_line_xpos + 0.5,
                                self.prev_line_height))
        xpos_adj = xpos - self.xadj + 0.5

        cr_ctx = window.cairo_create()
        cr_ctx.set_line_width(1)
        cr_ctx.move_to(xpos_adj, 0.0)
        cr_ctx.line_to(xpos_adj, line_height)
        cr_ctx.stroke()

        if self.prev_line_height != line_height:
            self.prev_line_height = line_height
        self.prev_line_xpos = xpos_adj


def get_mark_surface_height(font_size):
    textext = get_font_text_extents("StartEnd", font_size)
    return textext[3] + (textext[0] * 2) # /!\ do not to modify create_mark_surface

def create_mark_surface(text,
                        window,
                        font_size,
                        fg_color=MSQ_BG_COLOR,
                        bg_color=MSQ_FG_COLOR):
    textext = get_font_text_extents(text, font_size)

    width = textext[4] + textext[0]
    height = textext[3] + 6 # /!\ do not to modify get_mark_surface_height
    ypos = 3.0 - textext[1]
    surface = window.create_similar_surface(cairo.CONTENT_COLOR,
                                            width,
                                            height)
    context = cairo.Context(surface)
    context.set_font_size(font_size)
    context.set_source_rgb(*bg_color)
    context.paint()
    context.set_source_rgb(*fg_color)
    context.move_to(0, ypos)
    context.show_text(text)
    return surface

def _crctx_paste_surface(cr_ctx, clip_extents, surface, xpos, ypos):
    width  = surface.get_width()
    height = surface.get_height()
    xend = xpos + width
    xmax = clip_extents[2]
    yend = ypos + height
    ymax = clip_extents[3]
    if clip_extents[0] <= xend and xpos <= xmax and clip_extents[1] <= yend and ypos <= ymax:
        cr_ctx.set_source_surface(surface, xpos, ypos)
        cr_ctx.rectangle(clip_extents[0],
                         clip_extents[1],
                         xmax - clip_extents[0],
                         ymax - clip_extents[1])
        cr_ctx.fill()


class MsqHBarTimeWidget(Gtk.Widget, Xpos2Tick):
    def realize_hbar_surface(self):
        window = self.get_window()
        self.start_surface = create_mark_surface(" Start ",
                                                 window,
                                                 self.font_size,
                                                 self.bg_color,
                                                 self.fg_color)
        self.end_surface   = create_mark_surface(" End ",
                                                 window,
                                                 self.font_size,
                                                 self.bg_color,
                                                 self.fg_color)

    def do_realize(self):
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
        self.set_window(window)
        window.set_user_data(self)
        window.move_resize(allocation.x,
                           allocation.y,
                           allocation.width,
                           allocation.height)

        self.setting.hadj.configure(0,                          # value
                                    0,                          # lower
                                    self.setting.getmaxwidth(), # upper
                                    0,                          # step incr
                                    0,                          # page incr
                                    allocation.width)           # page size

        # self.setting.hadj.set_lower(0)
        # self.setting.hadj.set_upper(self.setting.getmaxwidth())
        # self.setting.hadj.set_page_size(window.get_width())
        # self.setting.hadj.set_page_increment(0)
        # self.setting.hadj.set_step_increment(0)
        # self.setting.hadj.set_value(0)
        self.realize_hbar_surface()

    def do_unrealize(self):
        self.end_surface = None
        self.start_surface = None
        self.window.set_user_data(None)

    def draw_loop(self, cr_ctx, clip_extents):
        start_tick = self.setting.getstart()
        xpos       = self.tick2xpos(start_tick)
        ypos = self.height - self.start_surface.get_height()
        _crctx_paste_surface(cr_ctx, clip_extents, self.start_surface, xpos, ypos)

        end_tick = start_tick + self.setting.getlen()
        xpos     = self.tick2xpos(end_tick)
        ypos = self.height - self.end_surface.get_height()
        _crctx_paste_surface(cr_ctx, clip_extents, self.end_surface, xpos, ypos)

    def draw_timeline(self, cr_ctx, clip_extents):
        surface = cr_ctx.get_target()
        cr_ctx.set_source_rgb(*self.bg_color)
        width = clip_extents[2] - clip_extents[0]
        height = clip_extents[3] - clip_extents[1]
        cr_ctx.rectangle(clip_extents[0],
                         clip_extents[1],
                         width,
                         height)
        cr_ctx.fill()

        xpos = (clip_extents[0] + self.xadj) - ((clip_extents[0] + self.xadj) % self.setting.qnxsz)
        time_pos = xpos / self.setting.qnxsz

        cr_ctx.set_source_rgb(*self.fg_color)
        cr_ctx.set_line_width(1)
        mypos = self.mark_height * 2
        nypos = self.mark_height * 5 / 2
        while xpos <= (clip_extents[2] + self.xadj):
            if (time_pos % self.setting.mlen) == 0:
                cr_ctx.move_to(xpos - self.xadj, 2 + self.mark_height)
                cr_ctx.show_text("%i" % time_pos)
                ypos = mypos
            else:
                ypos = nypos
            if clip_extents[1] > ypos:
                ypos = clip_extents[1]
            if ypos <= clip_extents[3]:
                cr_ctx.move_to(xpos - self.xadj + 0.5, ypos)
                cr_ctx.line_to(xpos - self.xadj + 0.5, clip_extents[3])
                cr_ctx.stroke()
            time_pos += 1
            xpos += self.setting.qnxsz

    def draw_area(self, cr_ctx, clip_extents):
        self.draw_timeline(cr_ctx, clip_extents)
        self.draw_loop(cr_ctx, clip_extents)

    def do_draw(self, cr_ctx):
        clip_extents = cr_ctx.clip_extents()
        self.draw_area(cr_ctx, clip_extents)

    def handle_button_press(self, widget, event):
        if event.button == 1:
            start_tick = self.setting.getstart()
            start_xpos = self.tick2xpos(start_tick)
            start_w    = self.start_surface.get_width()
            if start_xpos <= event.x and event.x <= start_xpos + start_w:
                self.hbar_mode = 1
                self.last_tick = start_tick
            else:
                end_tick = start_tick + self.setting.getlen()
                end_xpos = self.tick2xpos(end_tick)
                end_w    = self.end_surface.get_width()
                if end_xpos <= event.x and event.x <= end_xpos + end_w:
                    self.hbar_mode = 2
                    self.last_tick = end_tick

    def update_surface(self, last_tick, new_tick, surface):
        cr_ctx = Gdk.cairo_create(self.get_window())
        ypos = self.height - surface.get_height()
        xpos = self.tick2xpos(last_tick)
        clip_extents = (xpos, ypos, xpos + surface.get_width(), ypos + surface.get_height())
        self.draw_timeline(cr_ctx, clip_extents)
        xpos = self.tick2xpos(new_tick)
        cr_ctx.set_source_surface(surface, xpos, ypos)
        cr_ctx.paint()

    def handle_button_release(self, widget, event):
        if event.button == 1:
            if self.hbar_mode == 1:
                start_tick = self.setting.getstart()
                if self.last_tick != start_tick:
                    start_tick = self.last_tick / self.setting.getppq()
                    self.setting.set_loop(int(start_tick), int(self.setting.getlen() / self.setting.getppq()))
                    self.grid.draw_all()
            elif self.hbar_mode == 2:
                start_tick = self.setting.getstart()
                end_tick = self.setting.getstart() + self.setting.getlen()
                if self.last_tick != end_tick:
                    new_len = self.setting.getlen() + self.last_tick - end_tick
                    start_tick /= self.setting.getppq()
                    new_len /= self.setting.getppq()
                    self.setting.set_loop(int(start_tick), int(new_len))
                    self.grid.draw_all()
        self.hbar_mode = 0

    def handle_motion(self, widget, event):
        if self.hbar_mode:
            tick = self.xpos2tick(event.x)
            ppq = self.setting.getppq()
            mod = tick % ppq
            if mod:
                tick = tick - mod

            if self.hbar_mode == 1:
                if tick != self.last_tick:
                    self.update_surface(self.last_tick, tick, self.start_surface)
                    self.update_surface(self.last_tick + self.setting.getlen(), tick + self.setting.getlen(), self.end_surface)
                self.last_tick = tick
            elif self.hbar_mode == 2:
                start_tick = self.setting.getstart()
                if tick != self.last_tick and start_tick < tick:
                    self.update_surface(self.last_tick, tick, self.end_surface)
                    self.last_tick = tick

    def __init__(self, grid):
        Gtk.Widget.__init__(self)
        # GObject.GObject.__init__(self)

        self.grid    = grid
        self.setting = self.grid.setting

        self.fg_color = MSQ_FG_COLOR
        self.bg_color = MSQ_GR_COLOR

        self.font_size = get_default_font_size()
        self.mark_height = get_mark_surface_height(self.font_size)
        self.height = (self.mark_height + 1) * 3

        Xpos2Tick.__init__(self)

        self.hbar_mode = 0      # 0 NO MODE, 1 MOVE START, 2 MOVE END
        self.last_tick = self.setting.getstart()

        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("motion_notify_event", self.handle_motion)

        self.set_size_request(-1, self.height)


class MsqVBarNoteWidget(Gtk.Widget, Ypos2Note):
    def clear_note(self):
        if self.last_shown_note:
            xpos = self.piano_xpos
            height = self.setting.noteysz
            ypos = self.note2ypos(self.last_shown_note)
            clip_extents = (xpos, ypos, self.width, ypos + height)
            self.draw_area(Gdk.cairo_create(self.get_window()), clip_extents)
            self.last_shown_note = None

    def draw_area(self, cr_ctx, clip_extents):
        xmin = int(clip_extents[0])
        ymin = int(clip_extents[1])
        xmax = int(clip_extents[2])
        ymax = int(clip_extents[3])
        width  = xmax - xmin
        height = ymax - ymin

        # Drawing background
        cr_ctx.set_source_rgb(*self.bg_color)
        cr_ctx.rectangle(xmin,
                         ymin,
                         width,
                         height)
        cr_ctx.fill()

        cr_ctx.set_line_width(1)
        cr_ctx.set_source_rgb(*self.fg_color)

        # Drawing vertical lines
        piano_xpos = self.piano_xpos
        if xmin > self.piano_xpos:
            piano_xpos = xmin
        else:
            cr_ctx.move_to(piano_xpos + .5, ymin)
            cr_ctx.line_to(piano_xpos + .5, ymax)
            cr_ctx.stroke()
        win_width = self.get_window().get_width()
        if xmax >= win_width:
            cr_ctx.move_to(win_width + .5, ymin)
            cr_ctx.line_to(win_width + .5, ymax)
            cr_ctx.stroke()

        # Drawing piano keys
        ymax_adj = ymax + self.yadj
        ypos_adj = ymin + self.yadj

        note_pos = int(ypos_adj / self.setting.noteysz)
        ymod = ypos_adj % self.setting.noteysz
        if ymod:
            ypos_adj -= ymod
        while ypos_adj <= ymax_adj:
            note_pos += 1
            note_val = (128 - note_pos)
            note_key = note_val % 12

            ypos = ypos_adj - self.yadj
            if note_key == 11:
                cr_ctx.move_to(xmin, ypos + .5)
                cr_ctx.line_to(xmax, ypos + .5)
                cr_ctx.stroke()
            else:
                cr_ctx.move_to(piano_xpos, ypos + .5)
                cr_ctx.line_to(xmax,       ypos + .5)
                cr_ctx.stroke()
            if note_key == 0:
                if xmin < piano_xpos:
                    cr_ctx.move_to(1, ypos + self.setting.noteysz - 1)
                    octave = ((128 - note_pos) / 12) - 1
                    cr_ctx.show_text("%03d C %i" % (note_val, octave))
            elif note_key == 1 or note_key == 3 or note_key == 6 or note_key == 8 or note_key == 10:
                cr_ctx.rectangle(piano_xpos, ypos, (xmax - piano_xpos), self.setting.noteysz)
                cr_ctx.fill()
            ypos_adj += self.setting.noteysz

    def do_draw(self, cr_ctx):
        clip_extents = cr_ctx.clip_extents()
        self.draw_area(cr_ctx, clip_extents)

    def show_note(self, note):
        xpos = self.piano_xpos
        width = self.width - xpos
        height = self.setting.noteysz

        cr_ctx = Gdk.cairo_create(self.get_window())

        if self.last_shown_note:
            ypos = self.note2ypos(self.last_shown_note)
            clip_extents = (xpos, ypos, self.width, ypos + height)
            self.draw_area(cr_ctx, clip_extents)

        ypos = self.note2ypos(note)
        cr_ctx.set_source_rgba(0.5, 0.5, 0.5, 0.5)
        cr_ctx.rectangle(xpos, ypos, width, height)
        cr_ctx.fill()
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
        GObject.GObject.__init__(self)

        self.setting = setting
        self.width = int(get_font_text_width("00 C -10X") * 3 + 1)
        self.piano_xpos = int(self.width / 2)

        self.last_play_note = None

        Ypos2Note.__init__(self)

        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("motion_notify_event", self.handle_motion)

        self.set_size_request(self.width, -1)

    def do_realize(self):
        # self.set_flags(Gtk.REALIZED)
        self.set_realized(True)
        allocation = self.get_allocation()
        winattr = Gdk.WindowAttr()
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
        self.set_window(window)
        window.set_user_data(self)
        window.move_resize(allocation.x,
                           allocation.y,
                           allocation.width,
                           allocation.height)

        self.fg_color = MSQ_FG_COLOR
        self.bg_color = MSQ_BG_COLOR

        self.last_shown_note = None

        self.setting.vadj.configure(self.note2ypos(76),          # value
                                    0,                           # lower
                                    self.setting.getmaxheight(), # upper
                                    0,                           # step incr
                                    0,                           # page incr
                                    allocation.height)           # page size

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width = self.width

    def do_size_allocate(self, allocation):
        self.set_allocation(allocation)
        if self.get_realized():
            window = self.get_window()
            window.move_resize(allocation.x,
                               allocation.y,
                               allocation.width,
                               allocation.height)


from midilooper.msqwidget.notegridevt import MsqNGWEventHdl, DEFAULT_NOTEON_VAL, DEFAULT_NOTEOFF_VAL

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

    def set_loop(self, loop_start, loop_len):
        if self.setting_table:
            self.setting_table.set_loop(loop_start, loop_len)

    def quantify_tick(self, tick):
        return int(tick / self.tick_res) * self.tick_res

    def __init__(self, track, sequencer, chan_num=0, qnxsz=DEFAULT_QNOTE_XSZ):
        self.track = track
        self.sequencer = sequencer

        self.qnxsz   = qnxsz
        self.noteysz = int(get_font_text_height("C - 10X") * 2 + 1)

        self.mlen = 4   # number of beat per measure

        self.tick_res = int(self.getppq() / 4)

        self.chan_num = chan_num
        self.note_widget = None
        self.value_widget = None
        self.note_valadj = Gtk.Adjustment.new(DEFAULT_NOTEON_VAL, 0, 127, 1, 0, 0)

        self.hadj = Gtk.Adjustment()
        self.vadj = Gtk.Adjustment()

        self.setting_table = None


class MsqNoteGridWidget(Gtk.Widget, ProgressLineListener, MsqNGWEventHdl, Xpos2Tick, Ypos2Note):
    def update_pos(self, tickpos):
        xpos = int(self.setting.track.get_loop_pos(tickpos) * self.setting.qnxsz / self.setting.getppq())
        self._update_pos(xpos)

    def clear_progress(self):
        self.clear_progressline()

    def set_notes_scale(self, note):
        self.scale_notes = [(note + 1) % 12,
                            (note + 3) % 12,
                            (note + 6) % 12,
                            (note + 8) % 12,
                            (note + 10) % 12]

    def __init__(self, track, sequencer, chan_num=0):
        GObject.GObject.__init__(self)
        self.setting = ChannelEditorSetting(track, sequencer, chan_num)
        self.setting.note_widget = self
        Xpos2Tick.__init__(self)
        Ypos2Note.__init__(self)
        MsqNGWEventHdl.__init__(self)
        ProgressLineListener.__init__(self)
        self.max_height = (NOTE_MAX + 1) * self.setting.noteysz + 1
        self.selection = None
        # self.set_can_default(True)
        self.cairo_surface = None
        self.set_notes_scale(0)

        self.grid_fg = (0.0, 0.0, 0.0)
        self.grid_scale_bg = (0.6, 0.6, 0.6)
        self.grid_bg = (0.8, 0.8, 0.8)
        self.grid_light = (((self.grid_fg[0]   * 4) + self.grid_bg[0]) / 5,
                           ((self.grid_fg[1]   * 4) + self.grid_bg[1]) / 5,
                           ((self.grid_fg[2]   * 4) + self.grid_bg[2]) / 5)
        self.grid_res_col = ((self.grid_fg[0] + self.grid_bg[0]) / 2,
                             (self.grid_fg[1] + self.grid_bg[1]) / 2,
                             (self.grid_fg[2] + self.grid_bg[2]) / 2)
        self.sel_col = (0.5, 0.9, 0.5)

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
        self.set_window(window)
        self.cairo_surface = window.create_similar_surface(cairo.CONTENT_COLOR,
                                                           window.get_width(),
                                                           window.get_height())
        self.realize_noteonoff_handler()

        self.setting.vadj.set_value((self.setting.vadj.get_lower() + self.setting.vadj.get_upper() / 2))
        self.setting.vadj.value_changed()

    def do_unrealize(self):
        self.cairo_surface = None
        window = self.get_window()
        window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = 640
        requisition.height = 480

    def do_size_allocate(self, allocation):
        self.set_allocation(allocation)
        if self.get_realized():
            window = self.get_window()
            window.move_resize(allocation.x,
                               allocation.y,
                               allocation.width,
                               allocation.height)
            self.cairo_surface = window.create_similar_surface(cairo.CONTENT_COLOR,
                                                               allocation.width,
                                                               allocation.height)

    def crctx_draw_grid(self, cr_ctx, clip_extents):
        xmin = clip_extents[0]
        ymin = clip_extents[1]
        xmax = clip_extents[2]
        ymax = clip_extents[3]

        cr_ctx.set_source_rgb(*self.grid_bg)
        cr_ctx.rectangle(xmin, ymin, xmax - xmin, ymax - ymin)
        cr_ctx.fill()

        if ymax > self.max_height:
            ymax = self.max_height

        cr_ctx.set_line_width(1)

        note = self.ypos2note(ymin)
        ypos = self.note2ypos(note)
        while ypos < ymax:
            if (note % 12) in self.scale_notes:
                cr_ctx.set_source_rgb(*self.grid_scale_bg)
                scale_height = self.setting.noteysz if (ypos  + self.setting.noteysz) <= ymax else (ymax - ypos)
                if ypos < ymin:
                    scale_height = scale_height - (ymin - ypos)
                    ypos = ymin
                if scale_height > 0:
                    cr_ctx.rectangle(xmin, ypos, xmax - xmin, scale_height)
                    cr_ctx.fill()
            note -= 1
            ypos = self.note2ypos(note)

        # Detecting first line to draw
        tick = self.xpos2tick(xmin)
        modtick = tick % self.setting.tick_res
        if modtick != 0:
            tick = tick + self.setting.tick_res - modtick
        xpos = self.tick2xpos(tick)
        while xpos <= xmax:
            if (tick % self.setting.getppq()) == 0:
                if (tick % self.setting.mlen) == 0:
                    cr_ctx.set_source_rgb(*self.grid_fg)
                else:
                    cr_ctx.set_source_rgb(*self.grid_light)
            else:
                cr_ctx.set_source_rgb(*self.grid_res_col)
            cr_ctx.move_to(xpos + 0.5, ymin)
            cr_ctx.line_to(xpos + 0.5, ymax + 0.5)
            cr_ctx.stroke()
            tick += self.setting.tick_res
            xpos = self.tick2xpos(tick)

        note = self.ypos2note(ymin)
        ypos = self.note2ypos(note)
        while ypos < ymax:
            if ((note + 1) % 12) == 0:
                cr_ctx.set_source_rgb(*self.grid_fg)
            else:
                cr_ctx.set_source_rgb(*self.grid_light)
            cr_ctx.move_to(xmin, ypos + 0.5)
            cr_ctx.line_to(xmax,   ypos + 0.5)
            cr_ctx.stroke()
            note -= 1
            ypos = self.note2ypos(note)

    def is_in_note_list(self, event, note_list):
        if not note_list:
            return False
        for ev_noteon, ev_noteoff in note_list:
            if event == ev_noteon or event == ev_noteoff:
                return True
        return False

    def draw_note_rectangle(self, cr_ctx, x, y, width, height, value, selected):
        def ponder_color(coef, color1, color2):
            def ponder_value(coef, value1, value2):
                dist = value2 - value1
                return value1 + (dist * coef)
            red   = ponder_value(coef, color1[0], color2[0])
            green = ponder_value(coef, color1[1], color2[1])
            blue  = ponder_value(coef, color1[2], color2[2])
            return (red, green, blue)

        def get_note_color(value):
            if value > 255.0 or value < 0.0:
                print("ERROR in get_note_color value is not between 0 255")
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
            return (red_value, green_value, blue_value)

        note_color = get_note_color(value)
        cr_ctx.set_source_rgb(*note_color)
        cr_ctx.rectangle(x, y, width, height)
        cr_ctx.fill()

        if selected:
            cr_ctx.set_source_rgb(*self.sel_col)
        else:
            cr_ctx.set_source_rgb(*self.grid_fg)
        cr_ctx.set_line_width(1)
        cr_ctx.rectangle(x + 0.5, y - 0.5, width, height + 1)
        cr_ctx.stroke()

    def draw_note(self, cr_ctx, note_on, note_off, selected=False):
        xmin = self.tick2xpos(note_on[0])
        xmax = self.tick2xpos(note_off[0])
        width = xmax - xmin
        ypos = self.note2ypos(note_on[3]) + 1
        self.draw_note_rectangle(cr_ctx,
                                 xmin, ypos, width, self.setting.noteysz - 1,
                                 note_on[4], selected)

    def get_notes(self, clip_extents):
        tick_min = self.xpos2tick(clip_extents[0])
        note_max = self.ypos2note(clip_extents[1])
        tick_max = self.xpos2tick(clip_extents[2])
        note_min = self.ypos2note(clip_extents[3])

        if tick_min < 0: tick_min = 0
        if note_min < 0: note_min = 0
        return self.setting.track.sel_noteonoff_repr(self.setting.chan_num,
                                                     tick_min,
                                                     tick_max,
                                                     note_min,
                                                     note_max)

    def crctx_draw_notes_bar(self, cr_ctx, clip_extents):
        note_list = self.get_notes(clip_extents)
        selected_notes = None
        if self.selection:
            selected_notes = evwr_to_repr_list(self.selection)
        for ev_on, ev_off in note_list:
            self.draw_note(cr_ctx, ev_on, ev_off, self.is_in_note_list(ev_on, selected_notes))
        if self.value_wgt.get_window() and self.value_wgt.is_in_note_mode():
            self.value_wgt.draw_value_clip(clip_extents[0], clip_extents[2])

    def crctx_paste_surface(self, cr_ctx, clip_extents):
        _crctx_paste_surface(cr_ctx, clip_extents, self.cairo_surface, 0, 0)

    def paste_surface(self, clip_extents):
        window = self.get_window()
        self.crctx_paste_surface(window.cairo_create(), clip_extents)

    def crctx_draw_loop_veil(self, cr_ctx, clip_extents):
        cr_ctx.set_source_rgba(0, 0, 0, 0.5)

        tickstart = self.setting.getstart()
        xmin = clip_extents[0]
        ymin = clip_extents[1]
        xmax = clip_extents[2]
        height = clip_extents[3] - clip_extents[1]

        xstart = self.tick2xpos(tickstart)
        if xmin < xstart:
            if xmax < xstart:
                cr_ctx.rectangle(xmin, ymin, xmax - xmin, height)
                cr_ctx.fill()
                return
            cr_ctx.rectangle(xmin, ymin, xstart - xmin, height)
            cr_ctx.fill()

        xend = self.tick2xpos(tickstart + self.setting.getlen())
        if xend < xmin:
            xend = xmin
        if xend < xmax:
            cr_ctx.rectangle(xend, ymin, xmax - xend, height)
            cr_ctx.fill()

    def crctx_draw_clip(self, cr_ctx, clip_extents):
        cr_ctx_surface = cairo.Context(self.cairo_surface)
        self.crctx_draw_grid(cr_ctx_surface, clip_extents)
        self.crctx_draw_notes_bar(cr_ctx_surface, clip_extents)
        self.crctx_draw_loop_veil(cr_ctx_surface, clip_extents)

    def draw_clip(self, clip_extents):
        window = self.get_window()
        cr_ctx = window.cairo_create()
        self.crctx_draw_clip(cr_ctx, clip_extents)
        self.crctx_paste_surface(cr_ctx, clip_extents)

    def do_draw(self, cr_ctx):
        clip_extents = cr_ctx.clip_extents()
        self.crctx_draw_clip(cr_ctx, clip_extents)
        self.crctx_paste_surface(cr_ctx, clip_extents)


GObject.type_register(MsqHBarTimeWidget)
GObject.type_register(MsqVBarNoteWidget)
GObject.type_register(MsqNoteGridWidget)
