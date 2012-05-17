#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

import sys

import midiseq
from portlist import PortList
from tracklist import TrackList

class MidiLooper(gtk.Window):
    def run_progression(self):
        if self.msq.isrunning():
            tickpos = self.msq.gettickpos()
            self.tracklist_win.update_pos(tickpos)
            return True
        else:
            self.tracklist_win.clear_progress()
            return False
    def start_msq(self, button):
        if self.msq.isrunning():
            print "start_msq: sequencer already running"
            return True
        self.msq.settickpos(0)
        self.msq.start()
        gobject.timeout_add(50, self.run_progression)

    def stop_msq(self, button):
        self.msq.stop()
        self.msq.settickpos(0)
        return True

    def show_tracklist(self, menuitem):
        self.tracklist_win.show_all()

    def show_portlist(self, menuitem):
        self.portlist_win.show_all()

    def __init__(self):
        gtk.Window.__init__(self)
        self.set_resizable(False)
        hbox = gtk.HBox()
        button_start =  gtk.Button("Start")
        button_stop =  gtk.Button("Stop")
        self.msq = midiseq.midiseq("MidiLooper")
        self.msq.settickpos(0)

        self.portlist_win = PortList(self.msq)
        self.tracklist_win = TrackList(self.msq, self.portlist_win.liststore)

        button_start =  gtk.Button("Start")
        button_start.connect("clicked", self.start_msq)
        button_stop =  gtk.Button("Stop")
        button_stop.connect("clicked", self.stop_msq)

        def spincb(widget, msq):
            value = widget.get_value()
            msq.setbpm(int(value))
        spinadj = gtk.Adjustment(120, 40, 300, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinbut.set_numeric(True)
        spinadj.connect("value-changed", spincb, self.msq)

        hbox = gtk.HBox()
        hbox.pack_start(button_start)
        hbox.pack_start(button_stop)
        hbox.pack_start(spinbut)

        tl_mi = gtk.MenuItem("Track list")
        tl_mi.connect("activate", self.show_tracklist)
        pl_mi = gtk.MenuItem("Port list")
        pl_mi.connect("activate", self.show_portlist)
        show_menu = gtk.Menu()
        show_menu.append(tl_mi)
        show_menu.append(pl_mi)
        showmi = gtk.MenuItem("Show")
        showmi.set_submenu(show_menu)
        menubar = gtk.MenuBar()
        menubar.append(showmi)

        vbox = gtk.VBox()
        vbox.pack_start(menubar)
        vbox.pack_start(hbox)

        # hbox = gtk.HBox()
        # hbox.pack_start(button_show_tracklist)
        # hbox.pack_start(button_show_portlist)

        # vbox.pack_start(hbox)

        self.add(vbox)
        self.connect('delete_event', gtk.main_quit)


# gtk.rc_parse_string("""
# style "midiseq_default_style"
# {
#         bg[NORMAL] = "black"
#         fg[NORMAL] = "white"
#         base[NORMAL] = "black"
#         text[NORMAL] = "dark green"
# }
# widget "*" style "midiseq_default_style"
# """)

mlooper = MidiLooper()
mlooper.show_all()

gtk.main()
