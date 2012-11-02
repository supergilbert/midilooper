#!/usr/bin/python

import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

#import sys

DEFAULT_XPADSZ = 140
#DEFAULT_FONT_NAME = "-misc-fixed-medium-r-normal--10-70-100-100-c-60-iso8859-1"
DEFAULT_FONT_NAME = "-misc-fixed-medium-r-normal--6-60-75-75-c-40-iso8859-*"

NOTE_MAX = 127

MIDI_NOTEOFF_EVENT = 0x8
MIDI_NOTEON_EVENT  = 0x9

DEFAULT_NOTEON_VAL = 100

DEFAULT_YPADSZ = gdk.Font(DEFAULT_FONT_NAME).string_height("C -10X") + 4

class ProgressLineWidget(object):
    def clear_progressline(self):
        if not self.line_cache:
            return
        lwidth, lheight = self.line_cache.get_size()
        self.window.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                  self.line_cache, 0, 0, self.line_xpos, self.line_ypos, 1, lheight)

    def _update_pos(self, pos):
        if not self.window:
            return
        height = None
        line_ypos = None
        if self.vadj:
            line_ypos = int(self.vadj.get_value())
            height = int(self.vadj.get_page_size())
        else:
            width, height = self.window.get_size()
            line_ypos = 0
        if self.line_cache == None:
            self.line_cache = gtk.gdk.Pixmap(self.window, 1, height)
        else:
            lwidth, lheight = self.line_cache.get_size()
            self.window.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                      self.line_cache, 0, 0, self.line_xpos, self.line_ypos, 1, lheight)
            if lheight != height:
                self.line_cache = gtk.gdk.Pixmap(self.window, 1, height)
        if self.line_ypos != line_ypos:
            self.line_ypos = line_ypos

        self.line_cache.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                      self.window, pos, line_ypos, 0, 0, 1, height)
        self.window.draw_line(self.style.fg_gc[gtk.STATE_NORMAL],
                              pos, line_ypos, pos, line_ypos + height)
        self.line_xpos = pos


class MsqHBarTimeWidget(gtk.Widget):
    def set_len(self, track_len):
        self.hbarlen = self.xpadsz * track_len
        self.set_size_request(self.hbarlen, self.height)

    def __init__(self, track_len, mlen=4, xpadsz=DEFAULT_XPADSZ, font_name=DEFAULT_FONT_NAME):
        gtk.Widget.__init__(self)

        self.mlen = mlen
        self.xpadsz = xpadsz
        self.hbarlen = xpadsz * track_len

        self.font = gdk.Font(font_name)
        self.font_height = self.font.string_height("3600")
        self.height = (self.font_height + 1) * 3

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

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = self.hbarlen
        requisition.height = self.height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def do_expose_event(self, event):
        area = event.area
        self.window.draw_rectangle(self.dark_gc,
                                   True,
                                   area.x,
                                   area.y,
                                   area.width,
                                   area.height)
        xmax = area.x + area.width
        ymax = area.y + area.height
        # if ymax > self.max_height:
        #     ymax = self.max_height
        xpos = area.x - (area.x % self.xpadsz)
        time_pos = xpos / self.xpadsz

        mypos = self.font_height * 2
        nypos = self.font_height * 5 / 2
        while xpos <= xmax:
            if (time_pos % self.mlen) == 0:
                self.window.draw_string(self.font, self.fg_gc, xpos, 2 + self.font_height, "%i" % time_pos)
                ypos = mypos
            else:
                ypos = nypos
            if area.y > ypos:
                ypos = area.y
            self.window.draw_line(self.fg_gc, xpos, ypos, xpos, ymax)
            time_pos += 1
            xpos += self.xpadsz
        ypos = self.height - 1
        self.window.draw_line(self.fg_gc, area.x, ypos, area.x + area.width, ypos)

class MsqVBarNoteWidget(gtk.Widget):
    def clear_note(self):
        if self.last_shown_note:
            xpos = self.piano_xpos
            width = self.width - xpos
            height = self.ypadsz
            ypos = (127 - self.last_shown_note) * self.ypadsz
            area = gtk.gdk.Rectangle(xpos, ypos, width, height)
            self.draw_area(area)
            self.last_shown_note = None

    def ypos2noteval(self, ypos):
        return 127 - (ypos / self.ypadsz)

    def handle_button_press(self, widget, event, grid):
        if event.x < self.piano_xpos:
            return
        note = self.ypos2noteval(int(event.y))
        self.show_note(note)
        port = grid.track.get_port()
        if port:
            port.send_note(grid.note_param["channel"],
                           MIDI_NOTEON_EVENT,
                           note,
                           grid.note_param["val_on"])
            self.last_play_note = note

    def handle_button_release(self, widget, event, grid):
        self.clear_note()
        note = self.ypos2noteval(int(event.y))
        port = grid.track.get_port()
        if port:
            port.send_note(grid.note_param["channel"],
                           MIDI_NOTEOFF_EVENT,
                           note,
                           grid.note_param["val_off"])
            self.last_play_note = None

    def handle_motion(self, widget, event, grid):
        if self.last_play_note == None:
            return
        note = self.ypos2noteval(int(event.y))
        self.show_note(note)
        port = grid.track.get_port()
        if port and note != self.last_play_note:
            port.send_note(grid.note_param["channel"],
                           MIDI_NOTEOFF_EVENT,
                           self.last_play_note,
                           grid.note_param["val_off"])
            port.send_note(grid.note_param["channel"],
                           MIDI_NOTEON_EVENT,
                           note,
                           grid.note_param["val_on"])
            self.last_play_note = note

    def show_note(self, note):
        xpos = self.piano_xpos
        width = self.width - xpos
        height = self.ypadsz

        if self.last_shown_note:
            ypos = (127 - self.last_shown_note) * self.ypadsz
            area = gtk.gdk.Rectangle(xpos, ypos, width, height)
            self.draw_area(area)

        ypos = (127 - note) * self.ypadsz
        cr = self.window.cairo_create()
        cr.set_source_rgba(0, 0, 0, 0.5)
        cr.set_source_rgba(0.5, 0.5, 0.5, 0.5)
        cr.rectangle(xpos, ypos, width, height)
        cr.fill()
        self.last_shown_note = note

    def __init__(self, font_name=DEFAULT_FONT_NAME):
        gtk.Widget.__init__(self)

        self.font = gdk.Font(font_name)
        self.font_height = self.font.string_height("C -10X")
        self.width = self.font.string_width("00 C -10X") * 2
        self.piano_xpos = self.width / 2

        self.ypadsz = self.font_height + 1
        self.max_height = (NOTE_MAX + 1) * self.ypadsz + 1
        self.last_play_note = None

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

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = self.width
        requisition.height = self.max_height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def draw_area(self, area):
        xmax = area.x + area.width
        ymax = area.y + area.height
        ypos = area.y - (area.y % self.ypadsz)
        note_pos = ypos / self.ypadsz
        octave = ((128 - note_pos) / 12) - 2

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

        while ypos < ymax:
            self.window.draw_line(self.fg_gc, piano_xpos, ypos, xmax, ypos)
            note_pos += 1
            note_val = (128 - note_pos)
            note_key = note_val % 12
            if note_key == 0:
                if area.x < piano_xpos:
                    self.window.draw_line(self.fg_gc, area.x, ypos + self.ypadsz, piano_xpos, ypos + self.ypadsz)
                self.window.draw_string(self.font, self.fg_gc, 1, ypos + self.ypadsz - 1, "%03d C%i" % (note_val, octave))
                octave -= 1
            elif note_key == 1 or note_key == 3 or note_key == 6 or note_key == 8 or note_key == 10:
                self.window.draw_rectangle(self.fg_gc, True, piano_xpos, ypos, (xmax - piano_xpos), self.ypadsz)
            ypos += self.ypadsz

    def do_expose_event(self, event):
        self.draw_area(event.area)


from notegridwgt import MsqNGWHandleEvent

NOTE_PX_SIZE = 8

class MsqNoteGridWidget(gtk.Widget, ProgressLineWidget, MsqNGWHandleEvent):
    def resize_grid(self):
        width  = self.xpadsz * self.track.get_len() / self.ppq
        height = self.max_height
        self.set_size_request(width, height)
    def set_pad_size(self, xpadsz, ypadsz):
        self.xpadsz = xpadsz
        self.ypadsz = ypadsz
        self.max_height = (NOTE_MAX + 1) * self.ypadsz + 1

    def update_pos(self, tickpos):
        xpos = int((tickpos % self.track.get_len()) * self.xpadsz / self.ppq)
        self._update_pos(xpos)

    def clear_progress(self):
        self.clear_progressline()

    def __init__(self, chan_num, track, mlen=4, ppq=48, xpadsz=DEFAULT_XPADSZ, ypadsz=DEFAULT_YPADSZ, sequencer=None):
        gtk.Widget.__init__(self)
        self.sequencer = sequencer
        self.chan_num = chan_num
        self.mlen = mlen # 4/4 or 3/4
        self.ppq = ppq
        self.set_pad_size(xpadsz, ypadsz)
        self.track = track
        self.note_param = {"channel": 0, "val_on": DEFAULT_NOTEON_VAL, "val_off": 0, "len": ppq / 4, "quant": ppq / 4}
        self.button3down = False
        self.vadj = None
        self.line_cache = None
        self.line_ypos = 0
        self.selection = []
        self.to_paste = []
        self.to_move = None
        self.inc_note_left  = None
        self.inc_note_right = None
        self.rect_select_start = None
        self.rect_select = None
        self.paste_area = None
        self.paste_motion = False
        self.ctrl_click = False

    def refresh_note(self, tick, note, len, val_on, val_off=0):
        xpos = tick * self.xpadsz / self.ppq
        xsize = len * self.xpadsz / self.ppq
        ypos = ((127 - note) * self.ypadsz) + 1
        if xpos > (NOTE_PX_SIZE / 2):
            xpos = xpos - (NOTE_PX_SIZE / 2)
        else:
            xpos = 0
        if ypos > (NOTE_PX_SIZE / 2):
            ypos = ypos - (NOTE_PX_SIZE / 2)
        else:
            ypos = 0
        self.draw_all(gtk.gdk.Rectangle(xpos, ypos, xsize + NOTE_PX_SIZE, self.ypadsz + NOTE_PX_SIZE))

    def xpos2tick(self, xpos):
        return int(xpos * self.ppq / self.xpadsz)

    def ypos2noteval(self, ypos):
        return int(127 - (ypos / self.ypadsz))

    def get_match_note(self, tick, channel, note_type, note):
        match_list = []
        self.track.lock()
        for evwr in self.track:
            event = evwr.get_event()
            if (tick == event[0]
                and channel == event[1]
                and note_type == event[2]
                and note == event[3]):
                match_list.append(evwr.copy())
        self.track.unlock()
        return match_list

    def add_note(self, xpos, ypos):
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]
        note = self.ypos2noteval(int(ypos))

        self.track.lock()
        self.track.add_note_event(tick,
                                  self.note_param["channel"],
                                  MIDI_NOTEON_EVENT,
                                  note,
                                  self.note_param["val_on"])
        self.track.add_note_event(tick + self.note_param["len"],
                                  self.note_param["channel"],
                                  MIDI_NOTEOFF_EVENT,
                                  note,
                                  self.note_param["val_off"])
        self.track.unlock()

        self.refresh_note(tick,
                          note,
                          self.note_param["len"],
                          self.note_param["val_on"],
                          self.note_param["val_off"])

    def get_notelist_area(self, note_list):
        if len(note_list) == 0:
            return None
        evwr_noteon, evwr_noteoff = note_list[0]
        ev_noteon = evwr_noteon.get_event()
        ev_noteoff = evwr_noteoff.get_event()
        min_tick = ev_noteon[0]
        max_tick = ev_noteoff[0]
        min_note = ev_noteon[3]
        max_note = ev_noteon[3]
        for evwr_noteon, evwr_noteoff in note_list:
            ev_noteon = evwr_noteon.get_event()
            ev_noteoff = evwr_noteoff.get_event()
            if ev_noteon[0] < min_tick:
                min_tick = ev_noteon[0]
            if ev_noteoff[0] > max_tick:
                max_tick = ev_noteoff[0]
            if ev_noteon[3] < min_note:
                min_note = ev_noteon[3]
            if ev_noteon[3] > max_note:
                max_note = ev_noteon[3]
        xmax = max_tick * self.xpadsz / self.ppq
        xmin = min_tick * self.xpadsz / self.ppq
        ymax = ((127 - min_note) * self.ypadsz)
        ymin = ((127 - max_note) * self.ypadsz)
        return gtk.gdk.Rectangle(xmin - 1, ymin - 1, xmax - xmin + 2, ymax - ymin + self.ypadsz + 2)


    def select_note(self, rectangle):
        tick_min = self.xpos2tick(rectangle.x)
        tick_max = self.xpos2tick(rectangle.x + rectangle.width)
        note_max = self.ypos2noteval(rectangle.y)
        note_min = self.ypos2noteval(rectangle.y + rectangle.height)

        noteon = {}
        selection = []

        self.track.lock()
        for evwr in self.track:
            event = evwr.get_event()
            ev_tick = event[0]
            ev_type = event[2]
            if ev_type != MIDI_NOTEON_EVENT and ev_type != MIDI_NOTEOFF_EVENT:
                continue
            ev_channel = event[1]
            if ev_channel != self.chan_num:
                continue
            ev_note = event[3]
            if ev_note < note_min or ev_note > note_max:
                continue

            if ev_tick < tick_min:
                if ev_type == MIDI_NOTEON_EVENT:
                    noteon[ev_note] = evwr.copy()
                else:
                    if noteon.has_key(ev_note):
                        noteon.pop(ev_note)
                continue
            elif ev_tick <= tick_max:
                if ev_type == MIDI_NOTEON_EVENT:
                    noteon[ev_note] = evwr.copy()
                else:
                    if noteon.has_key(ev_note):
                        selection.append((noteon[ev_note], evwr.copy()))
                        noteon.pop(ev_note)
                continue
            else:
                if ev_type == MIDI_NOTEOFF_EVENT:
                    if noteon.has_key(ev_note):
                        selection.append((noteon[ev_note], evwr.copy()))
                        noteon.pop(ev_note)
        self.track.unlock()

        return selection

    def get_xpos(self, tick):
        return tick * self.xpadsz / self.ppq
    def get_ypos(self, note):
        return (127 - note) * self.ypadsz

    def draw_note(self, note_on, note_off, selected=False):
        xmin = self.get_xpos(note_on[0])
        xmax = self.get_xpos(note_off[0])
        width = xmax - xmin
        ypos = self.get_ypos(note_on[3]) + 1
        self.draw_note_rectangle(selected,
                                 xmin, ypos, width, self.ypadsz - 1,
                                 note_on[4])

    def draw_note_list(self, note_list, selected=False):
        tick_min = note_list[0][0][0]
        tick_max = note_list[0][0][0]
        note_min = note_list[0][0][3]
        note_max = note_list[0][0][3]
        for note_on, note_off in note_list:
            self.draw_note(note_on, note_off, selected=selected)
            if tick_max < note_off[0]:
                tick_max = note_off[0]
            if tick_min > note_on[0]:
                tick_min = note_on[0]
            if note_min > note_on[3]:
                note_min = note_on[3]
            if note_max < note_on[3]:
                note_max = note_on[3]
                note_max = note_on[3]
        xmin = self.get_xpos(tick_min)
        xmax = self.get_xpos(tick_max)
        ymin = self.get_ypos(note_max)
        ymax = self.get_ypos(note_min)
        self.paste_area = gtk.gdk.Rectangle(xmin - 2,
                                            ymin - 2,
                                            xmax - xmin + 4,
                                            ymax - ymin + self.ypadsz + 4)

    def coo_under_notelist(self, xpos, ypos, note_list):
        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)
        for evwr_on, evwr_off in self.selection:
            ev_on = evwr_on.get_event()
            note_on = ev_on[3]
            if note == note_on:
                tick_on = ev_on[0]
                ev_off = evwr_off.get_event()
                tick_off = ev_off[0]
                if tick >= tick_on and tick <= tick_off:
                    return ev_on, ev_off, tick
        return None

    def refresh_cursor(self, xpos, ypos, ev_state):
        ev_on_off_tick = self.coo_under_notelist(xpos, ypos, self.selection)
        if ev_on_off_tick:
            if ev_state & gtk.gdk.SHIFT_MASK:
                len1 = ev_on_off_tick[2] - ev_on_off_tick[0][0]
                len2 = ev_on_off_tick[1][0] - ev_on_off_tick[2]
                if len1 > len2:
                    if self.current_cursor != self.cursor_move_left:
                        self.window.set_cursor(self.cursor_move_left)
                        self.current_cursor = self.cursor_move_left
                else:
                    if self.current_cursor != self.cursor_move_right:
                        self.window.set_cursor(self.cursor_move_right)
                        self.current_cursor = self.cursor_move_right
            elif self.current_cursor != self.cursor_move:
                self.window.set_cursor(self.cursor_move)
                self.current_cursor = self.cursor_move
        else:
            if self.current_cursor != self.cursor_arrow:
                self.window.set_cursor(self.cursor_arrow)
                self.current_cursor = self.cursor_arrow

    def delete_selection(self):
        selarea = self.get_notelist_area(self.selection)
        self.track.lock()
        if self.track.is_handled():
            for ev_noteon, ev_noteoff in self.selection:
                self.track.event2trash(ev_noteon)
                self.track.event2trash(ev_noteoff)
        else:
            for ev_noteon, ev_noteoff in self.selection:
                ev_noteon._del_event()
                ev_noteoff._del_event()
        self.track.unlock()
        self.selection = []
        if selarea:
            selarea.x = selarea.x - 2
            selarea.y = selarea.y - 2
            selarea.width = selarea.width + 4
            selarea.height = selarea.height + 4
            self.draw_all(selarea)

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

        self.grid_fg = gtk.gdk.Color(0.0, 0.0, 0.0)
        self.grid_bg = gtk.gdk.Color(0.4, 0.4, 0.4)
        self.grid_light = gtk.gdk.Color((self.grid_fg.red + self.grid_bg.red) / 2,
                                        (self.grid_fg.green + self.grid_bg.green) / 2,
                                        (self.grid_fg.blue + self.grid_bg.blue) / 2)

        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("key_press_event", self.handle_key_press)
        self.connect("motion_notify_event", self.handle_motion)
        self.cursor_arrow = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.cursor_pencil = gtk.gdk.Cursor(gtk.gdk.PENCIL)
        self.cursor_move = gtk.gdk.Cursor(gtk.gdk.FLEUR)
        # self.cursor_move_left = gtk.gdk.Cursor(gtk.gdk.SB_RIGHT_ARROW)
        # self.cursor_move_right = gtk.gdk.Cursor(gtk.gdk.SB_LEFT_ARROW)
        self.cursor_move_left = gtk.gdk.Cursor(gtk.gdk.RIGHT_SIDE)
        self.cursor_move_right = gtk.gdk.Cursor(gtk.gdk.LEFT_SIDE)
        self.current_cursor = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.window.set_cursor(self.cursor_arrow)
        self.set_can_focus(True)

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = self.xpadsz * self.track.get_len() / self.ppq
        requisition.height = self.max_height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def draw_grid(self, area):
        xmax = area.x + area.width
        ymax = area.y + area.height
        if ymax > self.max_height:
            ymax = self.max_height
        xpos = area.x - (area.x % self.xpadsz)
        ypos = area.y - (area.y % self.ypadsz)
        time_pos = xpos / self.xpadsz
        note_pos = ypos / self.ypadsz

        cr = self.window.cairo_create()
        cr.set_source_color(self.grid_bg)
        cr.rectangle(area.x, area.y, area.width, area.height)
        cr.fill()

        cr.set_line_width(1)
        # dx = 0.5
        # dy = dx
        # cr.device_to_user_distance(dx, dy)
        # cr.set_line_width(dx if dx > dy else dy)
        # import pdb; pdb.set_trace()
        while xpos <= xmax:
            if  (time_pos % self.mlen) == 0:
                cr.set_source_color(self.grid_fg)
            else:
                cr.set_source_color(self.grid_light)
            cr.move_to(xpos + 0.5, area.y + 0.5)
            cr.line_to(xpos + 0.5, ymax + 0.5)
            cr.stroke()
            time_pos += 1
            xpos += self.xpadsz

        while ypos < ymax:
            if ((128 - note_pos) % 12) == 0:
                cr.set_source_color(self.grid_fg)
            else:
                cr.set_source_color(self.grid_light)
            cr.move_to(area.x - 0.5, ypos + 0.5)
            cr.line_to(xmax + 0.5, ypos + 0.5)
            cr.stroke()
            note_pos += 1
            ypos += self.ypadsz

    def is_selected(self, event):
        for ev_noteon, ev_noteoff in self.selection:
            if event == ev_noteon.get_event() or event == ev_noteoff.get_event():
                return True
        return False

    def draw_note_rectangle(self, selected, x, y, width, height, value):
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
            first_color  = [140.0, 140.0, 140.0]
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
        cr = self.window.cairo_create()
        cr.set_source_color(note_color)
        cr.rectangle(x, y, width, height)
        cr.fill()
        if selected:
            cr.set_source_color(self.grid_fg)
            cr.set_line_width(1.5)
            cr.rectangle(x - 1.0, y - 1.0, width + 2.0, height + 2.0)
            cr.stroke()

    def draw_notes_bar(self, area):
        xmax = area.x + area.width
        ymax = area.y + area.height
        if ymax > self.max_height: # Handling max notes height
            ymax = self.max_height

        def pop_1st_note_in_list(note, noteon_list):
            for noteon in noteon_list:
                if noteon[0] == note:
                    noteon_list.remove(noteon)
                    return noteon

        noteon_list = []
        noteon_bkp = []
        self.track.lock()
        for evwr in self.track:
            event = evwr.get_event()
            ev_chan = event[1]
            ev_type = event[2]
            if ev_chan != self.chan_num or not ev_type in (MIDI_NOTEOFF_EVENT, MIDI_NOTEON_EVENT):
                continue
            tick = event[0]
            ev_note = event[3]
            ev_val = event[4]
            xpos = tick * self.xpadsz / self.ppq
            # Handling possible couple of 'note on' and 'note off' around area position
            if xpos < area.x:
                if ev_type == MIDI_NOTEON_EVENT:
                    noteon_bkp.append((ev_note, ev_val, xpos))
                elif ev_type == MIDI_NOTEOFF_EVENT:
                    pop_1st_note_in_list(ev_note, noteon_bkp)
                continue
            if xpos > xmax:
                if ev_type == MIDI_NOTEOFF_EVENT:
                    noteon = pop_1st_note_in_list(ev_note, noteon_bkp)
                    if noteon:
                        ypos = self.get_ypos(ev_note) + 1
                        ysize = self.ypadsz - 1
                        if ypos > ymax or (ypos + ysize) < area.y:
                            continue
                        if (ypos + ysize) > ymax:
                            ysize = ymax - ypos
                        self.draw_note_rectangle(self.is_selected(event), area.x, ypos, area.width, ysize, noteon[1])
                continue

            # Handling note on area position
            ypos = self.get_ypos(ev_note) + 1
            ysize = self.ypadsz - 1
            if ypos > ymax:
                continue
            if (ypos + ysize) < area.y:
                continue
            if (ypos + ysize) > ymax:
                ysize = ymax - ypos

            if ysize > 0:
                if ev_type == MIDI_NOTEON_EVENT:
                    noteon_list.append((ev_note, ev_val, xpos, ypos, ysize))
                else: # MIDI_NOTEOFF_EVENT
                    noteon = pop_1st_note_in_list(ev_note, noteon_list)
                    if noteon == None:
                        noteon = pop_1st_note_in_list(ev_note, noteon_bkp)
                    if noteon == None:
                        # Handling noteoff with no preceding noteon
                        xsize = xpos - area.x
                        if xsize > 0:
                            self.draw_note_rectangle(self.is_selected(event), area.x, ypos, xsize, ysize, noteon[1])
                    else:
                        xsize = xpos - noteon[2]
                        self.draw_note_rectangle(self.is_selected(event), noteon[2], ypos, xsize, ysize, noteon[1])

        # Handling noteon with no noteoff (to resolve some scroll problem)
        for noteon in noteon_list:
            xsize = area.x + area.width - noteon[2]
            self.draw_note_rectangle(self.is_selected(event), noteon[2], noteon[3], xsize, noteon[4], noteon[1])
        self.track.unlock()


    def draw_all(self, area):
        self.draw_grid(area)
        self.draw_notes_bar(area)

    def do_expose_event(self, event):
        self.draw_all(event.area)

gobject.type_register(MsqHBarTimeWidget)
gobject.type_register(MsqVBarNoteWidget)
gobject.type_register(MsqNoteGridWidget)
