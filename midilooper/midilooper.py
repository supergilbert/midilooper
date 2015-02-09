#!/usr/bin/python

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
            self.tracklist_frame.update_pos(tickpos)
            return True
        else:
            self.tracklist_frame.clear_progress()
            return False

    def start_msq(self, button):
        if self.msq.isrunning():
            print "start_msq: sequencer already running"
            return True
        self.msq.start()
        # gobject.timeout_add(50, self.run_progression)
        gobject.timeout_add(25, self.run_progression)

    def stop_msq(self, button):
        self.msq.stop()
        return True

    def show_portlist(self, menuitem):
        if self.portlist_win.get_mapped():
            self.portlist_win.unmap()
        else:
            self.portlist_win.show_all()
            self.portlist_win.map()

    def file_save_as(self, menuitem):
        fchooser = gtk.FileChooserDialog(action=gtk.FILE_CHOOSER_ACTION_SAVE,
                                         buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                                  gtk.STOCK_SAVE, gtk.RESPONSE_OK))
        if fchooser.run() == gtk.RESPONSE_OK:
            filename = fchooser.get_filename()
            self.msq.save(filename)
            self.filename = filename
        fchooser.destroy()

    def file_save(self, menuitem):
        if self.filename:
            self.msq.save(self.filename)
        else:
            self.file_save_as(menuitem)

    def __init__(self, seq_name="MidiLooper", filename=None):
        gtk.Window.__init__(self)
        self.set_resizable(False)
        hbox = gtk.HBox()
        button_start =  gtk.Button("Start")
        button_stop =  gtk.Button("Stop")
        self.msq = midiseq.midiseq(seq_name)

        if filename:
            self.filename = filename
            try:
                mfile = midiseq.midifile(self.filename)
            except:
                print "Unable to find file \"%s\"" % self.filename
                import sys; sys.exit()
            self.msq.read_msqfile(mfile)
        else:
            self.filename = None
            self.msq.settempo(60000000/120)

        self.portlist_win = PortList(self.msq)
        self.tracklist_frame = TrackList(self.msq, self.portlist_win.liststore)

        self.portlist_win.tracklist = self.tracklist_frame.liststore

        button_start =  gtk.Button(stock=gtk.STOCK_MEDIA_PLAY)
        button_start.connect("clicked", self.start_msq)
        button_stop =  gtk.Button(stock=gtk.STOCK_MEDIA_STOP)
        button_stop.connect("clicked", self.stop_msq)

        def spincb(widget, msq):
            value = widget.get_value()
            msq.settempo(int(60000000/value))
        bpm = int(60000000 / self.msq.gettempo())
        spinadj = gtk.Adjustment(bpm, 40, 208, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinbut.set_numeric(True)
        spinadj.connect("value-changed", spincb, self.msq)

        hbox = gtk.HBox()
        hbox.pack_start(button_start)
        hbox.pack_start(button_stop)
        hbox.pack_start(spinbut)

        pl_mi = gtk.MenuItem("Port list")
        pl_mi.connect("activate", self.show_portlist)
        save_mi = gtk.MenuItem("Save (Experimental)")
        save_mi.connect("activate", self.file_save)
        save_as_mi = gtk.MenuItem("Save as (Experimental)")
        save_as_mi.connect("activate", self.file_save_as)
        menu = gtk.Menu()
        menu.append(pl_mi)
        menu.append(save_mi)
        menu.append(save_as_mi)
        mi = gtk.MenuItem("Menu")
        mi.set_submenu(menu)
        menubar = gtk.MenuBar()
        menubar.append(mi)

        vbox = gtk.VBox()
        vbox.pack_start(menubar)
        vbox.pack_start(hbox)
        vbox.pack_start(self.tracklist_frame)
        self.add(vbox)
        self.connect('delete_event', gtk.main_quit)

        self.show_all()

if __name__ == "__main__":

#     gtk.rc_parse_string("""
# style "midiseq_default_style"
# {
#         bg[NORMAL]        = "black"
#         fg[NORMAL]        = "white"
#         base[NORMAL]      = "black"
#         text[NORMAL]      = "dark green"
#         bg[PRELIGHT]      = "black"
#         fg[PRELIGHT]      = "white"
#         base[PRELIGHT]    = "black"
#         text[PRELIGHT]    = "dark green"
#         bg[ACTIVE]        = "black"
#         fg[ACTIVE]        = "white"
#         base[ACTIVE]      = "black"
#         text[ACTIVE]      = "dark green"
#         bg[INSENSITIVE]   = "black"
#         fg[INSENSITIVE]   = "white"
#         base[INSENSITIVE] = "black"
#         text[INSENSITIVE] = "dark green"
#         bg[SELECTED]      = "black"
#         fg[SELECTED]      = "white"
#         base[SELECTED]    = "black"
#         text[SELECTED]    = "dark green"
# }
# widget "*" style "midiseq_default_style"
# """)
#    gtk.rc_parse("")

    mlooper = None
    if len(sys.argv) == 2:
        mlooper = MidiLooper(filename=sys.argv[1])
    else:
        mlooper = MidiLooper()
    gtk.main()
