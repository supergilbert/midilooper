#!/usr/bin/python

import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

import sys

DEFAULT_XPADSZ = 20
DEFAULT_FONT_NAME = "-misc-fixed-medium-r-normal--10-70-100-100-c-60-iso8859-1"

NOTE_MAX = 127

MIDI_NOTEOFF_EVENT = 0x8
MIDI_NOTEON_EVENT  = 0x9

DEFAULT_YPADSZ = gdk.Font(DEFAULT_FONT_NAME).string_height("C -10X") + 4

class MsqHBarTimeWidget(gtk.Widget):
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
        self.width = self.font.string_width("C -10X")

        self.ypadsz = self.font_height + 4
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

        win_xsize , win_ysize = self.window.get_size()
        if (xmax >= win_xsize):
            self.window.draw_line(self.dark, win_xsize - 1, area.y, win_xsize - 1, ymax)

        while ypos < ymax:
            self.window.draw_line(self.dark, area.x, ypos, xmax, ypos)
            note_pos += 1
            ypos += self.ypadsz
            if ((128 - note_pos) % 12) == 0:
                self.window.draw_string(self.font, self.dark, 3, ypos - 1, "C %i" % octave)
                octave -= 1


NOTE_PX_SIZE = 8

note_off_xpm = ["%i %i 2 1" % (NOTE_PX_SIZE, NOTE_PX_SIZE),
                "  c None",
                "x c #000000",
                "        ",
                "        ",
                "  xxxx  ",
                "  x  x  ",
                "  x  x  ",
                "  xxxx  ",
                "        ",
                "        "]

note_on_xpm = ["%i %i 2 1" % (NOTE_PX_SIZE, NOTE_PX_SIZE),
               "  c None",
               "x c #000000",
               "        ",
               " xxxxxx ",
               " xx  xx ",
               " x xx x ",
               " x xx x ",
               " xx  xx ",
               " xxxxxx ",
               "        "]

NOTE_BAR_TYPE = 0
NOTE_POINT_TYPE = 1

class MsqNoteGridWidget(gtk.Widget):
    def set_pad_size(self, xpadsz, ypadsz):
        self.xpadsz = xpadsz
        self.ypadsz = ypadsz
        self.max_height = (NOTE_MAX + 1) * self.ypadsz + 1

    def __init__(self, track, track_len, mlen=4, ppq=48, xpadsz=DEFAULT_XPADSZ, ypadsz=DEFAULT_YPADSZ, note_type=NOTE_BAR_TYPE):
        gtk.Widget.__init__(self)
        self.mlen = mlen # 4/4 or 3/4
        self.ppq = ppq
        self.pattern = None
        self.set_pad_size(xpadsz, ypadsz)
        self.track = track
        self.track_len = track_len
        self.button_press_param = {"channel": 0, "val_on": 127, "val_off": 0, "len": ppq}
        self.note_type = note_type
        self.button3down = False

    def add_note_to_seq(self, tick, note):
        for idx, elt in enumerate(self.track):
            if elt[0] == tick:
                elt[1].append(note)
                return
            elif elt[0] > tick:
                self.track.insert(idx,
                                  (tick, [note]))
                return
        self.track.append((tick, [note]))

    def draw_note(self, tick, note, len, val_on, val_off=0):
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

    def handle_button_release(self, widget, event):
        if event.button == 3:
            self.window.set_cursor(self.cursor_arrow)
            self.button3down = False
        if event.button == 1 and self.button3down:
            tick = int(event.x * self.ppq / self.xpadsz)
            note = int(128 - (event.y / self.ypadsz))
            # import pdb; pdb.set_trace()
            noteon = (MIDI_NOTEON_EVENT,
                      self.button_press_param["channel"],
                      note,
                      self.button_press_param["val_on"])
            self.add_note_to_seq(tick, noteon)
            self.add_note_to_seq(tick + self.button_press_param["len"],
                                 (MIDI_NOTEOFF_EVENT, self.button_press_param["channel"], note, self.button_press_param["val_on"]))
            self.draw_note(tick,
                           note,
                           self.button_press_param["len"],
                           self.button_press_param["val_on"],
                           self.button_press_param["val_off"])

    def handle_button_press(self, widget, event):
        if event.button == 3:
            self.window.set_cursor(self.cursor_pencil)
            self.button3down = True

    def do_realize(self):
        self.set_flags(gtk.REALIZED)
        self.window = gdk.Window(self.get_parent_window(),
                                 width=self.allocation.width,
                                 height=self.allocation.height,
                                 window_type=gdk.WINDOW_CHILD,
                                 wclass=gdk.INPUT_OUTPUT,
                                 event_mask=self.get_events() | gdk.EXPOSURE_MASK | gdk.BUTTON_PRESS_MASK | gdk.BUTTON_RELEASE_MASK)
        self.window.set_user_data(self)
        self.window.move_resize(*self.allocation)

        self.dark = self.window.new_gc()

        self.bg_gc = self.window.new_gc()
        self.bg_gc.set_foreground(self.style.bg_gc[gtk.STATE_NORMAL].foreground)

        self.light = self.window.new_gc()
        self.light.set_foreground(self.style.dark_gc[gtk.STATE_NORMAL].foreground)

        self.cross_pxb = gtk.gdk.pixbuf_new_from_xpm_data(note_on_xpm)
        self.square_pxb = gtk.gdk.pixbuf_new_from_xpm_data(note_off_xpm)

        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.cursor_arrow = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.cursor_pencil = gtk.gdk.Cursor(gtk.gdk.PENCIL)
        self.window.set_cursor(self.cursor_arrow)


    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = self.xpadsz * self.track_len
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

        self.window.draw_rectangle(self.bg_gc, True, area.x, area.y, area.width, area.height)

        while xpos <= xmax:
            gc = self.dark if (time_pos % self.mlen) == 0 else self.light
            self.window.draw_line(gc, xpos, area.y, xpos, ymax)
            time_pos += 1
            xpos += self.xpadsz

        while ypos < ymax:
            gc = self.dark if ((128 - note_pos) % 12) == 0 else self.light
            self.window.draw_line(gc, area.x, ypos, xmax, ypos)
            note_pos += 1
            ypos += self.ypadsz

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
        for tick, midiev_list in self.track:
            # import pdb; pdb.set_trace()
            xpos = tick * self.xpadsz / self.ppq
            # Handling possible couple of 'note on' 'note off' around area position
            if xpos < area.x:
                for midiev in midiev_list:
                    if midiev[0] == MIDI_NOTEON_EVENT:
                        noteon_bkp.append((midiev[2], xpos))
                    elif midiev[0] == MIDI_NOTEOFF_EVENT:
                        pop_1st_note_in_list(midiev[2], noteon_bkp)
                continue
            if xpos > xmax:
                for midiev in filter(lambda midiev: midiev[0] == MIDI_NOTEOFF_EVENT,
                                     midiev_list):
                    note = midiev[2]
                    noteon = pop_1st_note_in_list(note, noteon_bkp)
                    if noteon:
                        ypos = ((127 - note) * self.ypadsz)
                        ysize = self.ypadsz - 1
                        if ypos > ymax or (ypos + ysize) < area.y:
                            continue
                        if (ypos + ysize) > ymax:
                            ysize = ymax - ypos
                        self.window.draw_rectangle(self.light, True, area.x, ypos + 1, area.width, ysize)
                continue

            # Handling note on area position
            midiev_list = filter(lambda midiev: midiev[0] == MIDI_NOTEOFF_EVENT or midiev[0] == MIDI_NOTEON_EVENT,
                                 midiev_list)
            for midiev in midiev_list:
                type = midiev[0]
                note = midiev[2]
                ypos = ((127 - note) * self.ypadsz)
                ysize = self.ypadsz - 1
                if ypos > ymax:
                    continue
                if (ypos + ysize) < area.y:
                    continue
                if (ypos + ysize) > ymax:
                    ysize = ymax - ypos


                if ysize > 0:
                    if type == MIDI_NOTEON_EVENT:
                        noteon_list.append((midiev[2], xpos, ypos + 1, ysize))
                    else: # MIDI_NOTEOFF_EVENT
                        noteon = pop_1st_note_in_list(note, noteon_list)
                        if noteon == None:
                            noteon = pop_1st_note_in_list(note, noteon_bkp)
                        if noteon == None:
                            # Handling noteoff with no preceding noteon
                            xsize = xpos - area.x
                            if xsize > 0:
                                self.window.draw_rectangle(self.light, True, area.x, ypos + 1, xsize, ysize)
                        else:
                            xsize = xpos - noteon[1]
                            self.window.draw_rectangle(self.light, True, noteon[1], ypos + 1, xsize, ysize)

        # Handling noteon with no noteoff (to resolve some scroll problem)
        for noteon in noteon_list:
            xsize = area.x + area.width - noteon[1]
            self.window.draw_rectangle(self.light, True, noteon[1], noteon[2], xsize, noteon[3])

    def draw_notes_points(self, area):
        xmax = area.x + area.width
        ymax = area.y + area.height
        if ymax > self.max_height: # Handling max notes height
            ymax = self.max_height
        for tick, midiev_list in self.track:
            xpos = tick * self.xpadsz / self.ppq
            if xpos < area.x:
                continue
            if xpos > xmax:
                continue
            # Handling note on area position
            midiev_list = filter(lambda midiev: midiev[0] == MIDI_NOTEOFF_EVENT or midiev[0] == MIDI_NOTEON_EVENT,
                                 midiev_list)
            for midiev in midiev_list:
                type = midiev[0]
                note = midiev[2]
                ypos = ((127 - note) * self.ypadsz) + 1
                ysize = self.ypadsz - 1
                if ypos > ymax:
                    continue
                if (ypos + ysize) < area.y:
                    continue
                if type == MIDI_NOTEON_EVENT:
                    self.window.draw_pixbuf(None, self.cross_pxb, 0, 0, xpos - (NOTE_PX_SIZE / 2), ypos + (ysize / 2) - (NOTE_PX_SIZE / 2))
                if type == MIDI_NOTEOFF_EVENT:
                    self.window.draw_pixbuf(None, self.square_pxb, 0, 0, xpos - (NOTE_PX_SIZE / 2), ypos + (ysize / 2) - (NOTE_PX_SIZE / 2))


    def draw_all(self, area):
        self.draw_grid(area)
        if self.note_type == NOTE_BAR_TYPE:
            self.draw_notes_bar(area)
        else:
            self.draw_notes_points(area)

    def set_draw_points_mode(self):
        self.note_type = NOTE_POINT_TYPE
    def set_draw_bars_mode(self):
        self.note_type = NOTE_BAR_TYPE

    def do_expose_event(self, event):
        self.draw_all(event.area)

gobject.type_register(MsqHBarTimeWidget)
gobject.type_register(MsqVBarNoteWidget)
gobject.type_register(MsqNoteGridWidget)

if __name__ == '__main__':
    seq_test = [(24,   [(MIDI_NOTEON_EVENT, 0, 0, 127),]),
                (48,  [(MIDI_NOTEOFF_EVENT, 0, 0, 127),
                       (MIDI_NOTEON_EVENT, 0, 2, 127)]),
                (96,  [(MIDI_NOTEON_EVENT, 0, 5, 127),
                       (MIDI_NOTEOFF_EVENT, 0, 2, 127)]),
                (144, [(MIDI_NOTEOFF_EVENT, 0, 5, 127),]),
                (192, [(MIDI_NOTEON_EVENT, 0, 64, 127),]),
                (192, [(MIDI_NOTEON_EVENT, 0, 127, 127),]),
                (240, [(MIDI_NOTEOFF_EVENT, 0, 127, 127),]),
                (288, [(MIDI_NOTEON_EVENT, 0, 5, 127),]),
                (336, [(MIDI_NOTEOFF_EVENT, 0, 5, 127),]),
                (676, [(MIDI_NOTEOFF_EVENT, 0, 64, 127),]),
                (2880, [(MIDI_NOTEON_EVENT, 0, 5, 127),]),
                (2928, [(MIDI_NOTEOFF_EVENT, 0, 5, 127),])]
    # MIDI_SEQ_TEST = [(192, [(MIDI_NOTEON_EVENT, 0, 64, 127),]),
    #                  (676, [(MIDI_NOTEOFF_EVENT, 0, 64, 127),])]
    test_len = 8 * 4 * 4

    win = gtk.Window()
    win.set_title('test')
    win.connect('delete_event', gtk.main_quit)

    xpadsz = DEFAULT_XPADSZ

    hbar = MsqHBarTimeWidget(test_len)
    vbar = MsqVBarNoteWidget()
    ypadsz = vbar.ypadsz
    matrix = MsqNoteGridWidget(seq_test, test_len, ypadsz=ypadsz)

    matrix_vp = gtk.Viewport()
    matrix_vp.set_size_request(320, 240)
    matrix_vp.add(matrix)

    hadj = matrix_vp.get_hadjustment()
    vadj = matrix_vp.get_vadjustment()

    hbar_vp = gtk.Viewport()
    hbar_vp.set_hadjustment(hadj)
    hbar_vp.set_size_request(320, -1)
    hbar_vp.add(hbar)

    vbar_vp = gtk.Viewport()
    vbar_vp.set_vadjustment(vadj)
    vbar_vp.set_size_request(-1, 240)
    vbar_vp.add(vbar)

    hsb = gtk.HScrollbar(hadj)
    vsb = gtk.VScrollbar(vadj)

    matrix_vp.set_shadow_type(gtk.SHADOW_NONE)
    hbar_vp.set_shadow_type(gtk.SHADOW_NONE)
    vbar_vp.set_shadow_type(gtk.SHADOW_NONE)

    table = gtk.Table(3, 3)

    def set_adjustment(widget, event, vadj, hadj, xpadsz, ypadsz):
        value = vadj.get_value()
        xinc = xpadsz * 5
        yinc = ypadsz * 5
        if event.direction == gdk.SCROLL_DOWN:
            new_val = value + yinc
            vupper = vadj.upper - vadj.page_size
            vadj.set_value(new_val if new_val <= vupper else vupper)
        elif event.direction == gdk.SCROLL_UP:
            new_val = value - yinc
            vadj.set_value(new_val if new_val >= vadj.lower else vadj.lower)
        elif event.direction == gdk.SCROLL_RIGHT:
            new_val = value + xinc
            hupper = hadj.upper - hadj.page_size
            hadj.set_value(new_val if new_val <= hupper else hupper)
        elif event.direction == gdk.SCROLL_LEFT:
            new_val = value - xinc
            hadj.set_value(new_val if new_val >= hadj.lower else hadj.lower)

    matrix_vp.connect("scroll_event", set_adjustment, vadj, hadj, xpadsz, ypadsz)
    hbar_vp.connect("scroll_event", set_adjustment, vadj, hadj, xpadsz, ypadsz)
    vbar_vp.connect("scroll_event", set_adjustment, vadj, hadj, xpadsz, ypadsz)

    table.attach(hbar_vp, 1, 2, 0, 1, gtk.FILL, 0)
    table.attach(vbar_vp, 0, 1, 1, 2, 0, gtk.FILL)
    table.attach(matrix_vp, 1, 2, 1, 2, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
    table.attach(vsb, 2, 3, 1, 2, 0, gtk.FILL)
    table.attach(hsb, 1, 2, 2, 3, gtk.FILL, 0)

    win.add(table)

    win.show_all()

    gtk.main()
