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
    def draw_progressline(self, area):
        if self.line_xpos >= area.x and self.line_xpos <= (area.x, + area.width):
            ymin = area.y
            ymax = area.y + area.height
            self.window.draw_line(self.style.fg_gc[gtk.STATE_NORMAL],
                                  self.line_xpos, ymin, self.line_xpos, ymax)

    def clear_progressline(self):
        if not self.line_cache:
            return
        lwidth, lheight = self.line_cache.get_size()
        self.window.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                  self.line_cache, 0, 0, self.line_xpos, 0, 1, lheight)

    def _update_pos(self, pos):
        if not self.window:
            return
        width, height = self.window.get_size()
        if self.line_cache == None:
            self.line_cache = gtk.gdk.Pixmap(self.window, 1, height)
        else:
            lwidth, lheight = self.line_cache.get_size()
            self.window.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                      self.line_cache, 0, 0, self.line_xpos, 0, 1, lheight)
            if lheight != height:
                self.line_cache = gtk.gdk.Pixmap(self.window, 1, height)

        self.line_cache.draw_drawable(self.style.fg_gc[gtk.STATE_NORMAL],
                                      self.window, pos, 0, 0, 0, 1, height)
        self.window.draw_line(self.style.fg_gc[gtk.STATE_NORMAL],
                              pos, 0, pos, height)
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

        # self.dark = self.window.new_gc()

        # self.light = self.window.new_gc()
        # self.light.set_foreground(self.style.dark_gc[gtk.STATE_NORMAL].foreground)

        # self.bg_gc = self.window.new_gc()
        # self.bg_gc.set_foreground(self.style.bg_gc[gtk.STATE_NORMAL].foreground)

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
    def __init__(self, font_name=DEFAULT_FONT_NAME):
        gtk.Widget.__init__(self)

        self.font = gdk.Font(font_name)
        self.font_height = self.font.string_height("C -10X")
        self.width = self.font.string_width("C -10X") * 2
        self.piano_xpos = self.width / 2

        # self.ypadsz = self.font_height + 3
        self.ypadsz = self.font_height + 1
        self.max_height = (NOTE_MAX + 1) * self.ypadsz + 1

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

        self.dark = self.window.new_gc()

        self.bg_gc = self.window.new_gc()
        self.bg_gc.set_foreground(self.style.bg_gc[gtk.STATE_NORMAL].foreground)

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = self.width
        requisition.height = self.max_height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def do_expose_event(self, event):
        area = event.area
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
            self.window.draw_line(self.dark, piano_xpos, area.y, piano_xpos, ymax)
        win_xsize , win_ysize = self.window.get_size()
        if (xmax >= win_xsize):
            self.window.draw_line(self.dark, win_xsize - 1, area.y, win_xsize - 1, ymax)

        while ypos < ymax:
            self.window.draw_line(self.dark, piano_xpos, ypos, xmax, ypos)
            note_pos += 1
            note_val = (128 - note_pos) % 12
            if note_val == 0:
                if area.x < piano_xpos:
                    self.window.draw_line(self.dark, area.x, ypos + self.ypadsz, piano_xpos, ypos + self.ypadsz)
                self.window.draw_string(self.font, self.dark, 1, ypos + self.ypadsz - 1, "C%i" % octave)
                octave -= 1
            elif note_val == 1 or note_val == 3 or note_val == 6 or note_val == 8 or note_val == 10:
                self.window.draw_rectangle(self.dark, True, piano_xpos, ypos, (xmax - piano_xpos), self.ypadsz)
            ypos += self.ypadsz


NOTE_PX_SIZE = 8

class MsqNoteGridWidget(gtk.Widget, ProgressLineWidget):
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
        self.line_cache = None
        self.selection = []
        self.to_paste = []
        self.rect_select_start = None
        self.rect_select = None
        self.paste_area = None
        self.selecting = False
        self.paste_motion = False

    def refresh_rectangle(self, x, y, width, height):
        self.draw_all(gtk.gdk.Rectangle(x, y, width, height))

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
        self.refresh_rectangle(xpos, ypos, xsize + NOTE_PX_SIZE, self.ypadsz + NOTE_PX_SIZE)
        # self.draw_all(gtk.gdk.Rectangle(xpos, ypos, xsize + NOTE_PX_SIZE, self.ypadsz + NOTE_PX_SIZE))

    def xpos2tick(self, xpos):
        return int(xpos * self.ppq / self.xpadsz)

    def ypos2noteval(self, ypos):
        return int(127 - (ypos / self.ypadsz))

    def add_note(self, xpos, ypos):
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]
        note = self.ypos2noteval(int(ypos))

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


    def handle_button_release(self, widget, event):
        if event.button == 3:
            self.window.set_cursor(self.cursor_arrow)
            self.button3down = False
        if event.button == 1:
            if self.paste_motion:
                self.paste_note(event.x, event.y)
                self.paste_motion = False
                self.paste_area = None
                return
            if self.button3down:
                self.add_note(event.x, event.y)
            else:
                if self.rect_select: # clear rectangle selection
                    self.draw_all(self.rect_select) # temporary change draw_all to reversible effect

                if self.rect_select:
                    self.selection = self.select_note(self.rect_select)
                    if len(self.selection): # render new selection
                        self.draw_all(self.get_notelist_area(self.selection))

                # self.rect_select = None
                self.rect_select_start = None

    def handle_button_press(self, widget, event):
        self.grab_focus()
        if self.paste_motion:
            return
        if event.button == 3:
            self.window.set_cursor(self.cursor_pencil)
            self.button3down = True
        elif event.button == 1 and not self.button3down:
            sel_area = self.get_notelist_area(self.selection)
            self.selection = []
            self.rect_select = None
            if sel_area:    # clear previous selection
                self.draw_all(sel_area)
            self.rect_select_start = (event.x, event.y)
            self.selecting = True

    def get_xpos(self, tick):
        return tick * self.xpadsz / self.ppq
    def get_ypos(self, note):
        return (127 - note) * self.ypadsz

    def draw_note(self, note_on, note_off):
        xmin = self.get_xpos(note_on[0])
        xmax = self.get_xpos(note_off[0])
        width = xmax - xmin
        ypos = self.get_ypos(note_on[3])
        self.draw_note_rectangle(False,
                                 xmin, ypos, width, self.ypadsz - 1,
                                 note_on[4])

    def diff_note(self, note, tick_diff, note_diff):
        note = (note[0] + tick_diff,
                note[1],
                note[2],
                note[3] + note_diff,
                note[4])
        return note

    def paste_note(self, xpos, ypos):
        note_max = max(self.to_paste, key=lambda x: x[0][3])[0][3]
        tick_min = min(self.to_paste, key=lambda x: x[0][0])[0][0]

        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]

        note_diff = note - note_max
        tick_diff = tick - tick_min

        note_min = note_max + note_diff
        tick_max = tick_min + tick_diff

        for note_on, note_off in self.to_paste:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            self.track.add_note_event(*diff_note_on)
            self.track.add_note_event(*diff_note_off)
        if self.paste_area:
            self.draw_all(self.paste_area)


    def draw_paste(self, xpos, ypos):
        if self.paste_area:
            self.draw_all(self.paste_area)
        note_max = max(self.to_paste, key=lambda x: x[0][3])[0][3]
        tick_min = min(self.to_paste, key=lambda x: x[0][0])[0][0]

        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]

        note_diff = note - note_max
        tick_diff = tick - tick_min

        note_min = note_max + note_diff
        tick_max = tick_min + tick_diff

        for note_on, note_off in self.to_paste:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            self.draw_note(diff_note_on, diff_note_off)
            if note_min > note_off[3]:
                note_min = note_off[3]
            if tick_max < note_off[0]:
                tick_max = note_off[0]
        xmin = self.get_xpos(tick_min + tick_diff)
        xmax = self.get_xpos(tick_max + tick_diff)
        ymin = self.get_ypos(note_max + note_diff)
        ymax = self.get_ypos(note_min + note_diff)
        self.paste_area = gtk.gdk.Rectangle(xmin - 1,
                                            ymin - 1,
                                            xmax - xmin + 2,
                                            ymax - ymin + self.ypadsz + 2)

    def handle_motion(self, widget, event):
        if event.is_hint and self.rect_select_start:
            xmin = None
            xmax = None
            ymin = None
            ymax = None
            if event.x > self.rect_select_start[0]:
                xmax = event.x
                xmin = self.rect_select_start[0]
            else:
                xmax = self.rect_select_start[0]
                xmin = event.x
            if event.y > self.rect_select_start[1]:
                ymax = event.y
                ymin = self.rect_select_start[1]
            else:
                ymax = self.rect_select_start[1]
                ymin = event.y
            if self.rect_select:
                self.draw_all(self.rect_select)
            cr = self.window.cairo_create()
            # import cairo
            # cr.set_operator(cairo.OPERATOR_DEST_IN)
            # cr.set_operator(cairo.OPERATOR_DIFFERENCE)
            # cr.set_source_color(gtk.gdk.Color(255, 1, 1))
            self.rect_select = gtk.gdk.Rectangle(int(xmin), int(ymin), int(xmax - xmin), int(ymax - ymin))
            cr.set_source_rgba(0, 0, 0, 0.5)
            cr.rectangle(int(xmin), int(ymin), int(xmax - xmin), int(ymax - ymin))
            cr.fill()
        if self.paste_motion == True:
            self.draw_paste(event.x, event.y)

    def delete_selection(self):
        self.track.lock()
        if self.track.is_handled():
            for ev_noteon, ev_noteoff in self.selection:
                self.track.event2trash(ev_noteon)
                self.track.event2trash(ev_noteoff)
        else:
            for ev_noteon, ev_noteoff in self.selection:
                ev_noteon.del_event()
                ev_noteoff.del_event()
        self.track.unlock()
        self.selection = []

    def handle_key_press(self, widget, event):
        self.grab_focus()
        keyname = gtk.gdk.keyval_name(event.keyval)
        # print "Key %s (%d) was pressed" % (keyname, event.keyval)
        if event.state & gtk.gdk.CONTROL_MASK:
            if keyname == "v":
                self.paste_motion = True
            if keyname == "c":
                self.to_paste = []
                for note_on, note_off in self.selection:
                    self.to_paste.append((note_on.get_event(), note_off.get_event()))
        if event.keyval == gtk.keysyms.Delete or event.keyval == gtk.keysyms.BackSpace:
            sel_area = self.get_notelist_area(self.selection)
            self.delete_selection()
            if sel_area:
                self.draw_all(sel_area)

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

        self.grid_fg = self.style.fg[gtk.STATE_ACTIVE]
        self.grid_bg = self.style.bg[gtk.STATE_ACTIVE]
        self.grid_light = gtk.gdk.Color((self.grid_fg.red + self.grid_bg.red) / 2,
                                        (self.grid_fg.green + self.grid_bg.green) / 2,
                                        (self.grid_fg.blue + self.grid_bg.blue) / 2)

        self.dark = self.window.new_gc()

        self.bg_gc = self.window.new_gc()
        self.bg_gc.set_foreground(self.style.bg_gc[gtk.STATE_NORMAL].foreground)

        self.light = self.window.new_gc()
        self.light.set_foreground(self.style.dark_gc[gtk.STATE_NORMAL].foreground)

        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("key_press_event", self.handle_key_press)
        self.connect("motion_notify_event", self.handle_motion)
        self.cursor_arrow = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.cursor_pencil = gtk.gdk.Cursor(gtk.gdk.PENCIL)
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
            cr.move_to(area.x + 0.5, ypos + 0.5)
            cr.line_to(xmax + 0.5, ypos + 0.5)
            cr.stroke()
            # gc = self.dark if ((128 - note_pos) % 12) == 0 else self.light
            # self.window.draw_line(gc, area.x, ypos, xmax, ypos)
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
            first_color  = [130.0, 130.0, 130.0]
            second_color = [0.0, 255.0, 0.0]
            third_color  = [255.0, 255.0, 0.0]
            fouth_color  = [255.0, 0.0, 0.0]

            first_threshold  = 96.0
            second_threshold = 112.0

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
            self.window.draw_rectangle(self.dark, False, x - 1, y - 1, width + 1, height + 1)

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
                        ypos = ((127 - ev_note) * self.ypadsz)
                        ysize = self.ypadsz - 1
                        if ypos > ymax or (ypos + ysize) < area.y:
                            continue
                        if (ypos + ysize) > ymax:
                            ysize = ymax - ypos
                        self.draw_note_rectangle(self.is_selected(event), area.x, ypos + 1, area.width, ysize, noteon[1])
                continue

            # Handling note on area position
            ypos = ((127 - ev_note) * self.ypadsz)
            ysize = self.ypadsz - 1
            if ypos > ymax:
                continue
            if (ypos + ysize) < area.y:
                continue
            if (ypos + ysize) > ymax:
                ysize = ymax - ypos

            if ysize > 0:
                if ev_type == MIDI_NOTEON_EVENT:
                    noteon_list.append((ev_note, ev_val, xpos, ypos + 1, ysize))
                else: # MIDI_NOTEOFF_EVENT
                    noteon = pop_1st_note_in_list(ev_note, noteon_list)
                    if noteon == None:
                        noteon = pop_1st_note_in_list(ev_note, noteon_bkp)
                    if noteon == None:
                        # Handling noteoff with no preceding noteon
                        xsize = xpos - area.x
                        if xsize > 0:
                            self.draw_note_rectangle(self.is_selected(event), area.x, ypos + 1, xsize, ysize, noteon[1])
                    else:
                        xsize = xpos - noteon[2]
                        self.draw_note_rectangle(self.is_selected(event), noteon[2], ypos + 1, xsize, ysize, noteon[1])

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
