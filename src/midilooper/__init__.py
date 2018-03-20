# Copyright 2012-2016 Gilbert Romer

# This file is part of midilooper.

# midilooper is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# midilooper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU Gneneral Public License
# along with midilooper.  If not, see <http://www.gnu.org/licenses/>.

from gi.repository import GObject
GObject.threads_init()

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from gi.repository import Gdk

import sys

from midilooper import midiseq
from midilooper.outputlist import OutputList
from midilooper.tracklist import TrackList
from midilooper.msqwidget.wgttools import note_collision
from midilooper.tools import TRACK_MAX_LENGTH

class MidiLooper(Gtk.Window):
    def run_progression(self):
        if self.msq.isrunning():
            self.tracklist_frame.handle_record()
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
                GObject.timeout_add(25, self.run_progression)
        if self.msq.mute_state_changed():
            self.tracklist_frame.refresh_mute_state()
        if self.msq.rec_state_changed():
            self.tracklist_frame.refresh_rec_state()
        save_rq_ret = self.msq.get_save_request()
        if save_rq_ret:
            saverq, savepath = save_rq_ret
            self.filename = savepath
            if saverq == 1:     # save template
                self.msq.savetpl(savepath)
            if saverq == 2:     # save
                self.msq.save(savepath)
            if saverq == 3:     # save and auit
                self.msq.save(savepath)
                Gtk.main_quit()
        return True

    def key_press(self, widget, event):
        keyval = Gdk.keyval_name(event.keyval)
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
            print("start_msq: sequencer already running")
            return True
        self.msq.start()

    def stop_msq(self, button):
        self.msq.stop()
        return True

    def file_save_tpl(self, menuitem):
        fchooser = Gtk.FileChooserDialog(action=Gtk.FileChooserAction.SAVE,
                                         buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                                  Gtk.STOCK_SAVE, Gtk.ResponseType.OK))
        if fchooser.run() == Gtk.ResponseType.OK:
            filename = fchooser.get_filename()
            self.msq.savetpl(filename)
        fchooser.destroy()

    def file_save_as(self, menuitem):
        fchooser = Gtk.FileChooserDialog(action=Gtk.FileChooserAction.SAVE,
                                         buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                                  Gtk.STOCK_SAVE, Gtk.ResponseType.OK))
        if fchooser.run() == Gtk.ResponseType.OK:
            filename = fchooser.get_filename()
            self.msq.save(filename)
            self.filename = filename
        fchooser.destroy()

    def file_save(self, menuitem):
        if self.filename:
            self.msq.save(self.filename)
        else:
            self.file_save_as(menuitem)

    def close_check_save(self, wgt, ev):
        for idx, model in enumerate(self.tracklist_frame.liststore):
            tedit = model[0]
            if len(tedit.chaned.grid.history_list) > 0:
                msg_dialog = Gtk.MessageDialog(text="The session has been modified.\nDo you really want to exit.",
                                               flags=Gtk.DialogFlags.MODAL,
                                               buttons=Gtk.ButtonsType.OK_CANCEL,
                                               parent=self)
                if msg_dialog.run() == Gtk.ResponseType.OK:
                    break
                else:
                    msg_dialog.destroy()
                    return True
        Gtk.main_quit()

    def __init__(self, seq_name="MidiLooper", filename=None, engine_type=0, jacksessionid=""):
        Gtk.Window.__init__(self)
        self.set_resizable(False)
        hbox = Gtk.HBox()
        button_start = Gtk.Button("Start")
        button_stop = Gtk.Button("Stop")
        self.progress_running = False
        try:
            self.msq = midiseq.midilooper(seq_name, engine_type, jacksessionid)
        except Exception as e:
            print("Error initialising midi sequencer %s (%r)" % ("alsa" if engine_type == 0 else "jack", e))
            sys.exit(-1)
        GObject.timeout_add(200, self.check_if_running)

        if filename:
            self.filename = filename
            try:
                mfile = midiseq.midifile(self.filename)
            except:
                print("Unable to read file \"%s\"" % self.filename)
                sys.exit(1)
            self.msq.read_msqfile(mfile)
        else:
            self.filename = None
            self.msq.settempo(int(60000000/120))

        self.outputlist_frame = OutputList(self.msq)
        self.tracklist_frame = TrackList(self.msq, self.outputlist_frame.liststore)

        self.outputlist_frame.tracklist = self.tracklist_frame.liststore

        button_start = Gtk.Button()
        button_start.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_MEDIA_PLAY,  Gtk.IconSize.BUTTON))
        button_start.connect("clicked", self.start_msq)

        button_stop = Gtk.Button()
        button_stop.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_MEDIA_STOP,  Gtk.IconSize.BUTTON))
        button_stop.connect("clicked", self.stop_msq)

        def spincb(widget, msq):
            value = widget.get_value()
            msq.settempo(int(60000000/value))
        bpm = int(60000000 / self.msq.gettempo())
        spinadj = Gtk.Adjustment(value=bpm, lower=40, upper=208, step_increment=1)
        self.spinbut = Gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        self.spinbut.set_numeric(True)
        spinadj.connect("value-changed", spincb, self.msq)

        hbox = Gtk.HBox()
        hbox.pack_start(button_start, True, True, 0)
        hbox.pack_start(button_stop, True, True, 0)
        hbox.pack_start(self.spinbut, True, True, 0)

        save_mi = Gtk.MenuItem("Save")
        save_mi.connect("activate", self.file_save)
        save_as_mi = Gtk.MenuItem("Save as")
        save_as_mi.connect("activate", self.file_save_as)
        save_tpl_mi = Gtk.MenuItem("Save template")
        save_tpl_mi.connect("activate", self.file_save_tpl)
        menu = Gtk.Menu()
        menu.append(save_mi)
        menu.append(save_as_mi)
        menu.append(save_tpl_mi)
        mi = Gtk.MenuItem("Menu")
        mi.set_submenu(menu)
        menubar = Gtk.MenuBar()
        menubar.append(mi)

        vbox = Gtk.VBox()
        vbox.pack_start(menubar, True, True, 0)
        vbox.pack_start(hbox, True, True, 0)
        vbox.pack_start(self.outputlist_frame, True, True, 0)
        vbox.pack_start(self.tracklist_frame, True, True, 0)
        self.add(vbox)
        self.connect("key_press_event", self.key_press)
        self.connect('delete_event', self.close_check_save)
        self.show_all()
