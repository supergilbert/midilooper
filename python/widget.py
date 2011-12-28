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

NG_XPADSZ = 20
VBN_FONT_NAME = "-misc-fixed-medium-r-normal--10-70-100-100-c-60-iso8859-1"

NOTE_MAX = 127

MIDI_NOTEOFF_EVENT = 0x8
MIDI_NOTEON_EVENT  = 0x9

NG_YPADSZ = gdk.Font(VBN_FONT_NAME).string_height("C -1")

MIDI_SEQ_TEST = ((0,   ((MIDI_NOTEON_EVENT, 0, 0, 127),)),
                 (48,  ((MIDI_NOTEOFF_EVENT, 0, 0, 127),
                        (MIDI_NOTEON_EVENT, 0, 2, 127))),
                 (96,  ((MIDI_NOTEON_EVENT, 0, 5, 127),
                        (MIDI_NOTEOFF_EVENT, 0, 2, 127))),
                 (144, ((MIDI_NOTEOFF_EVENT, 0, 5, 127),)),
                 (192, ((MIDI_NOTEON_EVENT, 0, 64, 127),)),
                 (192, ((MIDI_NOTEON_EVENT, 0, 127, 127),)),
                 (240, ((MIDI_NOTEOFF_EVENT, 0, 127, 127),)),
                 (288, ((MIDI_NOTEON_EVENT, 0, 5, 127),)),
                 (336, ((MIDI_NOTEOFF_EVENT, 0, 5, 127),)),
                 (676, ((MIDI_NOTEOFF_EVENT, 0, 64, 127),)),
                 (2880, ((MIDI_NOTEON_EVENT, 0, 5, 127),)),
                 (2928, ((MIDI_NOTEOFF_EVENT, 0, 5, 127),)))
# MIDI_SEQ_TEST = ((192, ((MIDI_NOTEON_EVENT, 0, 64, 127),)),
#                  (676, ((MIDI_NOTEOFF_EVENT, 0, 64, 127),)))


class MsqVBarNoteWidget(gtk.Widget):
    def __init__(self, font_name=VBN_FONT_NAME):
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


class MsqNoteGridWidget(gtk.Widget):
    def set_pad_size(self, xpadsz, ypadsz):
        self.xpadsz = xpadsz
        self.ypadsz = ypadsz
        self.max_height = (NOTE_MAX + 1) * self.ypadsz + 1

    def __init__(self, mlen=4, ppq=48, xpadsz=NG_XPADSZ, ypadsz=NG_YPADSZ):
        gtk.Widget.__init__(self)
        self.mlen = mlen # 4/4 or 3/4
        self.ppq = ppq
        self.pattern = None
        self.set_pad_size(xpadsz, ypadsz)

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

        self.light = self.window.new_gc()
        self.light.set_foreground(self.style.dark_gc[gtk.STATE_NORMAL].foreground)

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = self.mlen * 4 * self.xpadsz * 8
        requisition.height = self.max_height

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def draw_grid(self, area, mlen):
        xmax = area.x + area.width
        ymax = area.y + area.height
        if ymax > self.max_height:
            ymax = self.max_height
        xpos = area.x - (area.x % self.xpadsz)
        ypos = area.y - (area.y % self.ypadsz)
        time_pos = xpos / self.xpadsz
        note_pos = ypos / self.ypadsz

        self.window.draw_rectangle(self.bg_gc, True, area.x, area.y, area.width, area.height)

        while ypos < ymax:
            gc = self.dark if ((128 - note_pos) % 12) == 0 else self.light
            self.window.draw_line(gc, area.x, ypos, xmax, ypos)
            note_pos += 1
            ypos += self.ypadsz

        while xpos <= xmax:
            gc = self.dark if (time_pos % mlen) == 0 else self.light
            self.window.draw_line(gc, xpos, area.y, xpos, ymax)
            time_pos += 1
            xpos += self.xpadsz

    def draw_notes(self, area):
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
        for tick, midiev_list in MIDI_SEQ_TEST:
            xpos = self.xpadsz * tick / self.ppq
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

    def do_expose_event(self, event):
        self.draw_grid(event.area, self.mlen)
        self.draw_notes(event.area)

gobject.type_register(MsqVBarNoteWidget)
gobject.type_register(MsqNoteGridWidget)

if __name__ == '__main__':
    win = gtk.Window()
    win.set_title('test')
    win.connect('delete-event', gtk.main_quit)

    xpadsz = NG_XPADSZ

    vbar = MsqVBarNoteWidget()
    ypadsz = vbar.ypadsz
    matrix = MsqNoteGridWidget(ypadsz=ypadsz)

    vp1 = gtk.Viewport()
    vp1.set_size_request(320, 240)
    vp1.add(matrix)

    vadj = vp1.get_vadjustment()
    hadj = vp1.get_hadjustment()

    vp2 = gtk.Viewport()
    vp2.set_vadjustment(vadj)
    vp2.set_size_request(-1, 240)
    vp2.add(vbar)

    vsb = gtk.VScrollbar(vadj)
    hsb = gtk.HScrollbar(hadj)

    vp1.set_shadow_type(gtk.SHADOW_NONE)
    vp2.set_shadow_type(gtk.SHADOW_NONE)

    table = gtk.Table(3, 3)

    def set_adjustment(widget, event, vadj, hadj, xpadsz, ypadsz):
        value = vadj.get_value()
        xinc = xpadsz * 3
        yinc = ypadsz * 3
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

    vp1.connect("scroll_event", set_adjustment, vadj, hadj, xpadsz, ypadsz)
    vp2.connect("scroll_event", set_adjustment, vadj, hadj, xpadsz, ypadsz)

    table.attach(vp2, 0, 1, 0, 1, 0, gtk.FILL)
    table.attach(vp1, 1, 2, 0, 1, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
    table.attach(vsb, 2, 3, 0, 1, 0, gtk.FILL)
    table.attach(hsb, 1, 2, 1, 2, gtk.FILL, 0)

    win.add(table)

    win.show_all()

    gtk.main()
