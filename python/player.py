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

# from threading import Thread
# import time

sys.path.append('./module')
import midiseq

# gobject.threads_init()

from track_editor import TrackEditor

# class tracked_updater(Thread):
#     def update_all_tracked(self):
#         for chan_num, channed in self.tracked.chaned_dict.items():
#             print "in chan num", chan_num
#             channed.grid.update_pos(self.midi_seq.gettickpos())
#     def run(self):
#         print "hoho"
#         print "msq", self.midi_seq
#         print "tracked", self.tracked
#         time_in_s = 0.1
#         print "time_in_s", time_in_s
#         while not self.quit:
#             # for chan_num, channed in self.tracked.chaned_dict.items():
#             #     print "in chan num", chan_num
#             #     channed.grid.update_pos(self.midi_seq.gettickpos())
#             if self.midi_seq.isrunning():
#                 print "midi_seq.isrunning"
#             #     gobject.idle_add(self.update_all_tracked)
#             print "time_in_s", time_in_s
#             time.sleep(time_in_s)
#         print "exiting loop_on_update"
#         # return True

#     def __init__(self, msq, tracked):
#         super(tracked_updater, self).__init__()
#         # Thread.__init__(self, target=self.loop_on_update)
#         self.midi_seq = msq
#         self.tracked = tracked
#         self.quit = False


def start_msq(button, msq, track, tracked):
    if msq.isrunning():
        print "start_msq: sequencer already running"
        return True
    msq.settrack(track)
    msq.settickpos(0)
    msq.start()
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

win = gtk.Window()
hbox = gtk.HBox()
button_start =  gtk.Button("Start")
button_start.connect("clicked", start_msq, msq, track, tracked)
button_stop =  gtk.Button("Stop")
button_stop.connect("clicked", stop_msq, msq)

hbox.pack_start(button_start)
hbox.pack_start(button_stop)

win.add(hbox)
win.connect('delete_event', gtk.main_quit)

win.show_all()
tracked.show_all()

# tup = tracked_updater(msq, tracked)
# def start_updater_cb(tup):
#     print "tup.isAlive()", tup.isAlive()
#     if tup.isAlive():
#         print "updater is alive"
#         return
#     tup.start()
# start_updater_cb(tup)
# time.sleep(30)

gtk.main()
# tup.quit = True
# time.sleep(1)
