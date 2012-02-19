#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

import sys

sys.path.append('./module')
import midiseq

from track_editor import TrackEditor

class tracked_updater(object):
    def update_all_tracked(self):
        tickpos = self.midi_seq.gettickpos()
        for chan_num, channed in self.tracked.chaned_dict.items():
            channed.grid.update_pos(tickpos)
    def clear_all_tracked(self):
        for chan_num, channed in self.tracked.chaned_dict.items():
            channed.grid.clear_progressline()
    def run(self):
        if self.midi_seq.isrunning():
            self.update_all_tracked()
            return True
        else:
            self.clear_all_tracked()
            return False

    def __init__(self, msq, tracked):
        self.midi_seq = msq
        self.tracked = tracked

def start_msq(button, msq, track, tracked, tup):
    if msq.isrunning():
        print "start_msq: sequencer already running"
        return True
    msq.settrack(track)
    msq.settickpos(0)
    msq.start()
    gobject.timeout_add(50, tup.run)
    return True

def stop_msq(button, msq):
    msq.stop()
    msq.settickpos(0)
    return True

player_name = "player"

ppq = 240
track_len = 4 * 4 * 10
track = midiseq.track("%s test track" % player_name)
msq = midiseq.midiseq("%s test alsa port" % player_name)
msq.setppq(ppq)
msq.setbpm(120)

tracked = TrackEditor(track, ppq, track_len=track_len)
tup = tracked_updater(msq, tracked)

win = gtk.Window()
hbox = gtk.HBox()
button_start =  gtk.Button("Start")
button_start.connect("clicked", start_msq, msq, track, tracked, tup)
button_stop =  gtk.Button("Stop")
button_stop.connect("clicked", stop_msq, msq)

hbox.pack_start(button_start)
hbox.pack_start(button_stop)

win.add(hbox)
win.connect('delete_event', gtk.main_quit)

win.show_all()
tracked.show_all()
gtk.main()
