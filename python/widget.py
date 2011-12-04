#!/usr/bin/python

import gobject
import pygtk
pygtk.require('2.0')
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

import sys

class MsqMatrixTrackWidget(gtk.Widget):
    def __init__(self, mlen=4, xpadsz=10, ypadsz=5):
        gtk.Widget.__init__(self)
        self.mlen = mlen        # 4/4 or 3/3
        self.xpadsz = xpadsz
        self.ypadsz = ypadsz
        self.pattern = None

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
        self.fg_gc = self.style.fg_gc[gtk.STATE_NORMAL]
        self.bg_gc = self.style.bg_gc[gtk.STATE_NORMAL]
        self.mlen_gc = self.style.dark_gc[gtk.STATE_NORMAL]

    def do_unrealize(self):
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        requisition.width  = 400
        requisition.height = 300

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*self.allocation)

    def draw_matrix(self):
        xsize, ysize = self.window.get_size()
        pos = 0
        xpos = 0
        ypos = 0
        self.window.draw_rectangle(self.bg_gc, True, 0, 0, xsize, ysize)
        while xpos < xsize:
            if (pos % self.mlen) == 0:
                self.window.draw_line(self.fg_gc, xpos, 1, xpos, ysize - 1)
            else:
                self.window.draw_line(self.mlen_gc, xpos, 1, xpos, ysize - 1)
            pos += 1
            xpos += self.xpadsz
        while ypos < ysize:
            self.window.draw_line(self.fg_gc, 1, ypos, xsize - 1, ypos)
            ypos += self.ypadsz
        self.window.draw_rectangle(self.fg_gc, False, 0, 0, xsize - 1, ysize - 1)

    def do_expose_event(self, event):
        self.draw_matrix()

gobject.type_register(MsqMatrixTrackWidget)

if __name__ == '__main__':
    win = gtk.Window()
    win.set_border_width(5)
    win.set_title('test')
    win.connect('delete-event', gtk.main_quit)

    w = MsqMatrixTrackWidget(xpadsz=40, ypadsz=20)
#    w = MsqMatrixTrackWidget()
    win.add(w)
    win.show_all()

    gtk.main()

