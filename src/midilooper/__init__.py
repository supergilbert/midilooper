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
from outputlist import OutputList
from tracklist import TrackList
from msqwidget import MIDI_NOTEON_EVENT, MIDI_NOTEOFF_EVENT
from msqwidget.wgttools import note_collision
from tool import TRACK_MAX_LENGTH

class MidiLooper(gtk.Window):
    def handle_record(self):
        rec_list = self.msq.getrecbuf()
        track = self.msq.getrectrack()
        if not track:
            self.rec_noteon_list = []
            return
        note_to_add = []
        ev_to_add = []
        for ev in rec_list:
            # print ev
            if ev[2] == MIDI_NOTEON_EVENT:
                tick  = track.get_loop_pos(ev[0])
                ev_on = (tick, ev[1], ev[2], ev[3], ev[4])
                self.rec_noteon_list.append(ev_on)
            elif ev[2] == MIDI_NOTEOFF_EVENT:
                for ev_on in self.rec_noteon_list:
                    if ev_on[3] == ev[3]:
                        tick_off = track.get_loop_pos(ev[0])
                        if ev_on[0] < tick_off:
                            ev_off = (tick_off, ev[1], ev[2], ev[3], ev[4])
                            note_to_add.append((ev_on, ev_off))
                            self.rec_noteon_list.remove(ev_on)
                            break
                        else:
                            self.rec_noteon_list.remove(ev_on)
            else:
                tick = track.get_loop_pos(ev[0])
                new_ev = (tick, ev[1], ev[2], ev[3], ev[4])
                ev_to_add.append(new_ev)
        if len(note_to_add):
            for note_on, note_off in note_to_add:
                note_col_list = track.sel_noteonoff_evwr(note_on[1],
                                                         note_on[0],
                                                         note_off[0],
                                                         note_on[3],
                                                         note_on[3])
                if len(note_col_list) > 0:
                    print "Deleting previous notes"
                    notes_to_del = []
                    for col_note_on, col_note_off in note_col_list:
                        notes_to_del.append(col_note_on)
                        notes_to_del.append(col_note_off)
                    track._delete_evwr_list(notes_to_del)
                ev_to_add.append(note_on)
                ev_to_add.append(note_off)
                # if note_collision(note_on[0],
                #                   note_off[0],
                #                   note_on[1],
                #                   note_on[3],
                #                   track.getall_noteonoff(note_on[1])):
                #     print "Can not rec note", note_on, note_off
                # else:
                #     print "Adding note", note_on, note_off
                #     ev_to_add.append(note_on)
                #     ev_to_add.append(note_off)
            # print "note on at least %d\n" % len(self.rec_noteon_list)
        if len(ev_to_add):
            track.add_evrepr_list(ev_to_add)

    def run_progression(self):
        if self.msq.isrunning():
            self.handle_record()
            tickpos = self.msq.gettickpos()
            self.tracklist_frame.update_pos(tickpos)
            self.progress_running = True
            return True
        else:
            self.tracklist_frame.clear_progress()
            self.progress_running = False
            return False


    def check_if_running(self):
        if self.progress_running == False:
            if self.msq.isrunning():
                gobject.timeout_add(25, self.run_progression)
        if self.msq.mute_state_changed():
            self.tracklist_frame.refresh_mute_state()
        return True

    def key_press(self, widget, event):
        keyval = gtk.gdk.keyval_name(event.keyval)
        if keyval == "space":
            if self.msq.isrunning():
                self.msq.stop()
            else:
                self.msq.start()
        else:
            self.msq.call_keypress(keyval)
        if not self.spinbut.has_focus():
            return True

    def start_msq(self, button):
        if self.msq.isrunning():
            print "start_msq: sequencer already running"
            return True
        self.msq.start()

    def stop_msq(self, button):
        self.msq.stop()
        return True

    def record_msq(self, button):
        if button.get_active():
            self.msq.setrecmode()
            print "Rec mode set"
        else:
            self.msq.unsetrecmode()
            print "Rec mode unset"
        return True

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

    def __init__(self, seq_name="MidiLooper", filename=None, engine_type=0):
        gtk.Window.__init__(self)
        self.set_resizable(False)
        hbox = gtk.HBox()
        button_start =  gtk.Button("Start")
        button_stop =  gtk.Button("Stop")
        self.progress_running = False
        try:
            self.msq = midiseq.midiseq(seq_name, engine_type)
        except:
            print "Error initialising midi sequencer", "alsa" if engine_type == 0 else "jack"
            sys.exit(-1)
        gobject.timeout_add(200, self.check_if_running)

        if filename:
            self.filename = filename
            try:
                mfile = midiseq.midifile(self.filename)
            except:
                print "Unable to find file \"%s\"" % self.filename
                sys.exit()
            self.msq.read_msqfile(mfile)
        else:
            self.filename = None
            self.msq.settempo(60000000/120)

        self.outputlist_frame = OutputList(self.msq)
        self.tracklist_frame = TrackList(self.msq, self.outputlist_frame.liststore)

        self.outputlist_frame.tracklist = self.tracklist_frame.liststore

        button_start = gtk.Button()
        button_start.set_image(gtk.image_new_from_stock(gtk.STOCK_MEDIA_PLAY,  gtk.ICON_SIZE_BUTTON))
        button_start.connect("clicked", self.start_msq)

        button_stop = gtk.Button()
        button_stop.set_image(gtk.image_new_from_stock(gtk.STOCK_MEDIA_STOP,  gtk.ICON_SIZE_BUTTON))
        button_stop.connect("clicked", self.stop_msq)

        button_record = gtk.ToggleButton()
        button_record.set_image(gtk.image_new_from_stock(gtk.STOCK_MEDIA_RECORD,  gtk.ICON_SIZE_BUTTON))
        button_record.connect("toggled", self.record_msq)
        self.rec_noteon_list = []

        def spincb(widget, msq):
            value = widget.get_value()
            msq.settempo(int(60000000/value))
        bpm = int(60000000 / self.msq.gettempo())
        spinadj = gtk.Adjustment(bpm, 40, 208, 1)
        self.spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        self.spinbut.set_numeric(True)
        spinadj.connect("value-changed", spincb, self.msq)

        hbox = gtk.HBox()
        hbox.pack_start(button_start)
        hbox.pack_start(button_stop)
        hbox.pack_start(button_record)
        hbox.pack_start(self.spinbut)

        save_mi = gtk.MenuItem("Save (Experimental)")
        save_mi.connect("activate", self.file_save)
        save_as_mi = gtk.MenuItem("Save as (Experimental)")
        save_as_mi.connect("activate", self.file_save_as)
        menu = gtk.Menu()
        menu.append(save_mi)
        menu.append(save_as_mi)
        mi = gtk.MenuItem("Menu")
        mi.set_submenu(menu)
        menubar = gtk.MenuBar()
        menubar.append(mi)

        vbox = gtk.VBox()
        vbox.pack_start(menubar)
        vbox.pack_start(hbox)
        vbox.pack_start(self.outputlist_frame)
        vbox.pack_start(self.tracklist_frame)
        self.add(vbox)
        self.connect("key_press_event", self.key_press)
        self.connect('delete_event', gtk.main_quit)
        self.show_all()
