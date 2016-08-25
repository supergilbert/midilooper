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

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

from msqwidget import MIDI_NOTEON_EVENT, MIDI_NOTEOFF_EVENT
from track_editor import TrackEditor
from tools import prompt_gettext, prompt_keybinding, prompt_notebinding, MsqListMenu, prompt_get_loop, prompt_get_output

import time

# xpm_button_add = ["8 8 2 1",
#                   "  c None",
#                   "x c #000000",
#                   "        ",
#                   "   xx   ",
#                   "   xx   ",
#                   " xxxxxx ",
#                   " xxxxxx ",
#                   "   xx   ",
#                   "   xx   ",
#                   "        "]

# pxb_button_add = gtk.gdk.pixbuf_new_from_xpm_data(xpm_button_add)
# img_button_add = gtk.image_new_from_pixbuf(pxb_button_add)


# xpm_mute = ["8 8 2 1",
#             "  c None",
#             "x c #000000",
#             "        ",
#             " xx  xx ",
#             " xxxxxx ",
#             " x xx x ",
#             " x    x ",
#             " x    x ",
#             " x    x ",
#             "        "]

# pxb_mute = gtk.gdk.pixbuf_new_from_xpm_data(xpm_mute)
# img_mute = gtk.image_new_from_pixbuf(pxb_mute)


class TrackListMenu(MsqListMenu):
    def show_track(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            if tedit.get_mapped():
                tedit.unmap()
            else:
                tedit.show_all()
                tedit.map()

    def del_track(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            track = tedit.track
            self.tracklist.liststore.remove(tv_iter)
            tedit.destroy()
            self.tracklist.seq.deltrack(track)
            self.path = None

    def rename_track(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            new_trackname = prompt_gettext("Rename track", tedit.track.get_name())
            if new_trackname:
                tedit.set_name(new_trackname)
                self.tracklist.liststore.set_value(tv_iter, 1, tedit.track.get_info())
            self.path = None


    def add_key_binding(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            key = prompt_keybinding(tedit.track.get_name())
            if key:
                self.tracklist.seq.add_keybinding(key, tedit.track)
                self.tracklist.liststore.set_value(tv_iter, 1, tedit.track.get_info())

    def add_note_binding(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            note = prompt_notebinding(self.tracklist.seq,
                                      tedit.track.get_name())
            if note:
                self.tracklist.seq.add_notebinding(note, tedit.track)
                self.tracklist.liststore.set_value(tv_iter, 1, tedit.track.get_info())

    def del_track_bindings(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            self.tracklist.seq.del_track_bindings(tedit.track)
            self.tracklist.liststore.set_value(tv_iter, 1, tedit.track.get_info())

    def clear_all_bindings(self, menuitem):
        self.tracklist.seq.clear_all_bindings()
        self.tracklist.update_all_info()

    def set_loop(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            loop_pos = prompt_get_loop(tedit.chaned.setting.getstart() / tedit.chaned.setting.getppq(),
                                       tedit.chaned.setting.getlen()   / tedit.chaned.setting.getppq())
            if loop_pos:
                tedit.track_setting.set_loop(loop_pos[0], loop_pos[1])

    def set_output(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            port_idx = 0
            for idx, model in enumerate(self.tracklist.portlist):
                if tedit.chaned.setting.track.has_output(model[0]):
                    port_idx = idx
                    break;
            output_res = prompt_get_output(self.tracklist.portlist, port_idx)
            if output_res and output_res[0]:
                tedit.track_setting.set_output(output_res[1])

    def copy_track(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            new_track = self.tracklist.seq.copy_track(tedit.track)
            new_name = "%s_bis" % new_track.get_name()
            new_tedit = TrackEditor(new_track,
                                    self.tracklist)
            new_tedit.set_name(new_name)
            self.tracklist.liststore.append([new_tedit,
                                             new_track.get_info(),
                                             0,
                                             new_tedit.track.get_mute_state(),
                                             False])
            new_tedit.show_all()

    def set_loop_all(self, loop_pos):
        def set_loop_all_cb(model, path, tv_iter, loop_pos):
            tedit = model.get_value(tv_iter, 0)
            tedit.track_setting.set_loop(loop_pos[0], loop_pos[1])
        self.tracklist.liststore.foreach(set_loop_all_cb, loop_pos)

    def menu_set_loop_all(self, menuitem):
        loop_pos = prompt_get_loop(0, 4)
        if loop_pos:
            self.set_loop_all(loop_pos)

    def set_output_all(self, output):
        def set_output_all_cb(model, path, tv_iter, output):
            tedit = model.get_value(tv_iter, 0)
            tedit.track_setting.set_output(output)
        self.tracklist.liststore.foreach(set_output_all_cb, output)

    def menu_set_output_all(self, menuitem):
        output_res = prompt_get_output(self.tracklist.portlist, 0)
        if output_res and output_res[0]:
            self.set_output_all(output_res[1])

    def __init__(self, tracklist):
        MsqListMenu.__init__(self)
        self.tracklist = tracklist

        self.mlm_add_root_item("Show track", self.show_track)
        self.mlm_add_root_item("Rename track", self.rename_track)
        self.mlm_add_root_item("Copy track", self.copy_track)
        self.mlm_add_root_item("Delete track", self.del_track)

        menutrack = self.mlm_add_submenu(self, "Configure track")
        self.mlm_add_menu_item(menutrack, "Loop", self.set_loop)
        self.mlm_add_menu_item(menutrack, "Output", self.set_output)

        self.mlm_add_menu_separator(menutrack)

        menualltracks = self.mlm_add_submenu(menutrack, "All tracks")
        self.mlm_add_menu_item(menualltracks, "Loop", self.menu_set_loop_all)
        self.mlm_add_menu_item(menualltracks, "Output", self.menu_set_output_all)

        self.mlm_add_root_separator()

        self.mlm_add_root_item("Add key binding", self.add_key_binding)
        self.mlm_add_root_item("Add note binding", self.add_note_binding)
        self.mlm_add_root_item("Del bindings", self.del_track_bindings)
        self.mlm_add_root_item("Clear all bindings", self.clear_all_bindings)


class TrackList(gtk.Frame):
    def handle_record(self):
        rec_list = self.seq.getrecbuf()
        track = self.seq.getrectrack()
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

    def refresh_mute_state(self):
        def update_mute_state(tvmodel, path, tv_iter):
            tedit = tvmodel.get_value(tv_iter, 0)
            state = tedit.track.get_mute_state()
            self.liststore.set_value(tv_iter, 3, state)
        tvmodel = self.treev.get_model()
        if tvmodel:
            tvmodel.foreach(update_mute_state)

    def refresh_rec_state(self):
        if self.seq.recmode():
            rectrack = self.seq.getrectrack()
            for ent in self.liststore:
                if ent[0].track.is_equal(rectrack):
                    ent[0].track_setting.rec_button_set_active(True)
                    ent[4] = True
                else:
                    ent[0].track_setting.rec_button_set_active(False)
                    ent[4] = False
        else:
            for ent in self.liststore:
                ent[0].track_setting.rec_button_set_active(False)
                ent[4] = False

    def update_pos(self, tickpos):
        def update_tedit(tvmodel, path, tv_iter, tickpos):
            tedit = tvmodel.get_value(tv_iter, 0)
            if tedit.track.has_changed():
                tedit.chaned.draw_all()
            tedit.update_pos(tickpos)
            def get_percent(tick, track_len):
                val = tick % track_len
                return val * 100 / track_len
            self.liststore.set_value(tv_iter, 2, get_percent(tickpos, tedit.track.get_len()))
        tvmodel = self.treev.get_model()
        if tvmodel:
            tvmodel.foreach(update_tedit, tickpos)

    def clear_progress(self):
        def clear_tedit_progress(tvmodel, path, tv_iter):
            tedit = tvmodel.get_value(tv_iter, 0)
            tedit.clear_progress()
            self.liststore.set_value(tv_iter, 2, 0)
        self.liststore.foreach(clear_tedit_progress)

    def tvbutton_press_event(self, treeview, event):
        path = treeview.get_path_at_pos(int(event.x), int(event.y))
        if path:
            if event.button == 3:
                self.menu.path = path
                self.menu.popup(None, None, None, event.button, event.time)

    def tvbutton_row_activated(self, treeview, path, view_column):
        tv_iter = self.liststore.get_iter(path[0])
        tedit = self.liststore.get_value(tv_iter, 0)
        if tedit.get_mapped():
            tedit.unmap()
        else:
            tedit.show_all()
            tedit.map()

    def _tracklist_info_it_func(self, track_liststore, path, tv_iter):
        if path:
            tv_iter = self.liststore.get_iter(path[0])
            tedit = self.liststore.get_value(tv_iter, 0)
            self.liststore.set_value(tv_iter, 1, tedit.track.get_info())

    def update_all_info(self):
        self.liststore.foreach(self._tracklist_info_it_func)

    def add_track(self, track_name):
        track = self.seq.newtrack(track_name);
        tedit = TrackEditor(track, self)
        self.liststore.append([tedit, tedit.track.get_info(), 0, tedit.track.get_mute_state(), False])
        tedit.show_all()

    def button_add_track(self, button):
        track_name = prompt_gettext("Enter new track name)")
        if track_name:
            self.add_track(track_name)

    def toggle_mute(self, cell, path, model):
        model[path][0].track.toggle_mute()
        model[path][3] = not model[path][3]

    def set_trackrec(self, track, record_request=True):
        if self.seq.recmode():
            self.seq.unsetrecmode()
            time.sleep(.5)
        track_ent = None
        for ent in self.liststore:
            if ent[0].track == track:
                track_ent = ent
            else:
                ent[4] = False
                ent[0].track_setting.rec_button_set_active(False)
        if record_request:
            self.seq.settrackrec(track)
            self.seq.setrecmode()
            track_ent[4] = True
        else:
            self.seq.unsetrecmode()
            track_ent[4] = False

    def set_trackrec_cb(self, cell, path, model):
        # beginning by unsetting all checkboxes
        if model[path][4]:
            self.set_trackrec(model[path][0].chaned.setting.track, False)
            model[path][0].track_setting.rec_button_set_active(False)
        else:
            self.set_trackrec(model[path][0].chaned.setting.track, True)
            model[path][0].track_setting.rec_button_set_active(True)

    def unset_trackrec_cb(self, tv):
        self.seq.unsettrackrec()
        for ent in self.liststore:
            if ent[4] == True:
                ent[0].track_setting.unset_rec()
            ent[4] = False

    def mute_all(self, tv):
        def _mute(tvmodel, path, tv_iter):
            tvmodel.get_value(tv_iter, 0).track.mute()
            mute_val = tvmodel.get_value(tv_iter, 0).track.get_mute_state()
            self.liststore.set_value(tv_iter, 3, mute_val)
        self.liststore.foreach(_mute)

    def drag_data_get_data(self, treeview, context, selection, target_id, etime):
        if self.seq.isrunning():
            return
        treeselection = treeview.get_selection()
        model, iterator = treeselection.get_selected()
        self.dnd_tedit = model.get_value(iterator, 0)
        model.remove(iterator)

    def drag_data_received_data(self, treeview, context, x, y, selection, info, etime):
        if self.seq.isrunning():
            return
        model = treeview.get_model()
        drop_info = treeview.get_dest_row_at_pos(x, y)
        if drop_info:
            path, position = drop_info
            iterator = model.get_iter(path)
            tedit = model.get_value(iterator, 0)
            if (position == gtk.TREE_VIEW_DROP_BEFORE or position == gtk.TREE_VIEW_DROP_INTO_OR_BEFORE):
                self.seq.move_track_before(tedit.track, self.dnd_tedit.track)
                model.insert_before(iterator, [self.dnd_tedit,
                                               self.dnd_tedit.track.get_info(),
                                               0,
                                               self.dnd_tedit.track.get_mute_state(),
                                               self.dnd_tedit.track.is_in_recmode()])
            else:
                self.seq.move_track_after(tedit.track, self.dnd_tedit.track)
                model.insert_after(iterator, [self.dnd_tedit,
                                              self.dnd_tedit.track.get_info(),
                                              0,
                                              self.dnd_tedit.track.get_mute_state(),
                                              self.dnd_tedit.track.is_in_recmode()])
        else:
            nchild = model.iter_n_children(None)
            if nchild > 1:
                iterator = model.get_iter("%d" % (nchild - 1))
                tedit = model.get_value(iterator, 0)
                self.seq.move_track_after(tedit.track, self.dnd_tedit.track)
                model.insert_after(iterator, [self.dnd_tedit,
                                              self.dnd_tedit.track.get_info(),
                                              0,
                                              self.dnd_tedit.track.get_mute_state(),
                                              self.dnd_tedit.track.is_in_recmode()])

    def __init__(self, seq, portlist):
        gtk.Frame.__init__(self, "Track list")
        self.seq = seq
        self.portlist = portlist
        self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str, int, bool, bool)
        for track in seq.gettracks():
            tedit = TrackEditor(track, self)
            tedit.unmap()
            self.liststore.append([tedit, tedit.track.get_info(), 0, track.get_mute_state(), False])
        self.treev = gtk.TreeView(self.liststore)
        self.treev.set_enable_search(False)
        self.menu = TrackListMenu(self)

        cell_rdrr = gtk.CellRendererToggle()
        # cell_rdrr.set_property('activatable', True)
        cell_rdrr.connect('toggled', self.set_trackrec_cb, self.liststore)
        tvcolumn = gtk.TreeViewColumn('R', cell_rdrr, active=4)
        tvcolumn.set_expand(False)
        tvcolumn.set_clickable(True)
        tvcolumn.connect('clicked', self.unset_trackrec_cb)
        self.treev.append_column(tvcolumn)
        self.rec_noteon_list = []

        cell_rdrr = gtk.CellRendererProgress()
        cell_rdrr.set_fixed_size(-1, cell_rdrr.get_size(self)[3] * 2)
        tvcolumn = gtk.TreeViewColumn('Track', cell_rdrr, text=1, value=2)
        tvcolumn.set_expand(True)
        self.track_col = tvcolumn
        self.treev.append_column(tvcolumn)
        self.treev.connect('button-press-event', self.tvbutton_press_event)
        self.treev.connect('row-activated', self.tvbutton_row_activated)

        cell_rdrr = gtk.CellRendererToggle()
        # cell_rdrr.set_property('activatable', True)
        cell_rdrr.connect('toggled', self.toggle_mute, self.liststore)
        tvcolumn = gtk.TreeViewColumn('M', cell_rdrr, active=3)
        tvcolumn.set_expand(False)
        tvcolumn.set_clickable(True)
        tvcolumn.connect('clicked', self.mute_all)
        self.treev.append_column(tvcolumn)

        self.treev.enable_model_drag_source(gtk.gdk.BUTTON1_MASK,
                                            [("MIDILOOPER_TRACK_LIST",
                                              gtk.TARGET_SAME_WIDGET,
                                              0)],
                                            gtk.gdk.ACTION_DEFAULT|gtk.gdk.ACTION_MOVE)
        self.treev.enable_model_drag_dest([("MIDILOOPER_TRACK_LIST",
                                            gtk.TARGET_SAME_WIDGET,
                                            0)],
                                          gtk.gdk.ACTION_DEFAULT)
        self.treev.connect("drag_data_get", self.drag_data_get_data)
        self.treev.connect("drag_data_received", self.drag_data_received_data)

        button_add = gtk.Button(stock=gtk.STOCK_ADD)
        # button_add.add(img_button_add)
        button_add.connect("clicked", self.button_add_track)

        vbox = gtk.VBox()
        vbox.pack_start(self.treev)
        hbox = gtk.HBox()
        hbox.pack_start(button_add)
        vbox.pack_start(hbox)

        self.add(vbox)

        def hide_tracklist(win, event):
            win.hide()
            return True
        self.connect('delete_event', hide_tracklist)
