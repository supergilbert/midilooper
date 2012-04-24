#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

import sys
sys.path.append('../module')
sys.path.append('./')

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

    def show_tracklist(self, button):
        self.tracklist_win.show_all()

    def show_portlist(self, button):
        self.portlist_win.show_all()

    def __init__(self):
        gtk.Window.__init__(self)
        hbox = gtk.HBox()
        button_start =  gtk.Button("Start")
        button_stop =  gtk.Button("Stop")
        bpm = 120
        ppq = 240
        self.msq = midiseq.midiseq("MidiLooper")
        self.msq.setppq(ppq)
        self.msq.setbpm(bpm)
        self.msq.settickpos(0)

        output1 = self.msq.newoutput("output 1")
        output2 = self.msq.newoutput("output 2")
        track1 = self.msq.newtrack("track 1")
        track2 = self.msq.newtrack("track 2")

        track1.set_port(output1)
        track2.set_port(output2)

        self.portlist_win = PortList(self.msq)
        self.tracklist_win = TrackList(self.msq)

        button_start =  gtk.Button("Start")
        button_start.connect("clicked", self.start_msq)
        button_stop =  gtk.Button("Stop")
        button_stop.connect("clicked", self.stop_msq)

        def spincb(widget, msq):
            value = widget.get_value()
            msq.setbpm(int(value))
        spinadj = gtk.Adjustment(bpm, 40, 300, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinbut.set_numeric(True)
        spinadj.connect("value-changed", spincb, self.msq)

        hbox = gtk.HBox()
        hbox.pack_start(button_start)
        hbox.pack_start(button_stop)
        hbox.pack_start(spinbut)

        button_show_tracklist = gtk.Button("Track list")
        button_show_tracklist.connect("clicked", self.show_tracklist)

        button_show_portlist =  gtk.Button("Port list")
        button_show_portlist.connect("clicked", self.show_portlist)

        vbox = gtk.VBox()
        vbox.pack_start(hbox)

        hbox = gtk.HBox()
        hbox.pack_start(button_show_tracklist)
        hbox.pack_start(button_show_portlist)
        vbox.pack_start(hbox)

        self.add(vbox)
        self.connect('delete_event', gtk.main_quit)




mlooper = MidiLooper()
mlooper.show_all()

gtk.main()
