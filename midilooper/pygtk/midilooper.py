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
        if self.tracklist_win.get_mapped():
            self.tracklist_win.unmap()
        else:
            self.tracklist_win.show_all()
            self.tracklist_win.map()

    def show_portlist(self, menuitem):
        if self.portlist_win.get_mapped():
            self.portlist_win.unmap()
        else:
            self.portlist_win.show_all()
            self.portlist_win.map()

    def file_save(self, menuitem):
        fchooser = gtk.FileChooserDialog(action=gtk.FILE_CHOOSER_ACTION_SAVE,
                                         buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                                  gtk.STOCK_SAVE, gtk.RESPONSE_OK))
        if fchooser.run() == gtk.RESPONSE_OK:
            filename = fchooser.get_filename()
            self.msq.save(filename)
        fchooser.destroy()

    def __init__(self, msq=None):
        gtk.Window.__init__(self)
        self.set_resizable(False)
        hbox = gtk.HBox()
        button_start =  gtk.Button("Start")
        button_stop =  gtk.Button("Stop")
        if msq == None:
            self.msq = midiseq.midiseq("MidiLooper")
        else:
            self.msq = msq
        self.msq.settickpos(0)

        self.portlist_win = PortList(self.msq)
        self.tracklist_win = TrackList(self.msq, self.portlist_win.liststore)

        self.portlist_win.tracklist = self.tracklist_win.liststore

        button_start =  gtk.Button(stock=gtk.STOCK_MEDIA_PLAY)
        button_start.connect("clicked", self.start_msq)
        button_stop =  gtk.Button(stock=gtk.STOCK_MEDIA_STOP)
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
        show_mi = gtk.MenuItem("Show")
        show_mi.set_submenu(show_menu)

        save_mi = gtk.MenuItem("Save")
        save_mi.connect("activate", self.file_save)

        file_menu = gtk.Menu()
        file_menu.append(save_mi)
        file_mi = gtk.MenuItem("File")
        file_mi.set_submenu(file_menu)

        menubar = gtk.MenuBar()
        menubar.append(file_mi)
        menubar.append(show_mi)

        vbox = gtk.VBox()
        vbox.pack_start(menubar)
        vbox.pack_start(hbox)

        self.add(vbox)
        self.connect('delete_event', gtk.main_quit)

        self.show_all()
        self.tracklist_win.show_all()

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
# gtk.rc_parse("/tmp/Clearlooks-DarkOrange/gtk-2.0/gtkrc")

if __name__ == "__main__":
    import sys

    msq = midiseq.midiseq("MidiLooper")
    if len(sys.argv) == 2:
        mfile = midiseq.midifile(sys.argv[1])
        msq.copy_midifile(mfile)
    mlooper = MidiLooper(msq)
    gtk.main()
