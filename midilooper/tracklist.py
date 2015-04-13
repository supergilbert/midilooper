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

from track_editor import TrackEditor
from tool import prompt_gettext, MsqListMenu



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
                self.tracklist.liststore.set_value(tv_iter, 1, repr(tedit.track))
            self.path = None

    def copy_track(self, menuitem):
        if self.path:
            tv_iter = self.tracklist.liststore.get_iter(self.path[0])
            tedit = self.tracklist.liststore.get_value(tv_iter, 0)
            new_track = self.tracklist.seq.copy_track(tedit.track)
            new_name = "%s (copy)" % new_track.get_name()
            new_tedit = TrackEditor(new_track,
                                    self.tracklist.seq,
                                    self.tracklist.portlist)
            new_tedit.set_name(new_name)
            self.tracklist.liststore.append([new_tedit,
                                             repr(new_track),
                                             0,
                                             new_tedit.track.get_mute_state()])
            new_tedit.show_all()

    def __init__(self, tracklist):
        MsqListMenu.__init__(self)
        self.tracklist = tracklist

        self.mlm_add_item("Show track", self.show_track)
        self.mlm_add_item("Rename track", self.rename_track)
        self.mlm_add_item("Copy track", self.copy_track)
        separator = gtk.SeparatorMenuItem()
        separator.show()
        self.append(separator)
        self.mlm_add_item("Delete track", self.del_track)


class TrackList(gtk.Frame):
    def update_pos(self, tickpos):
        def update_tedit(tvmodel, path, tv_iter, tickpos):
            tedit = tvmodel.get_value(tv_iter, 0)
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
            # elif self.track_col == path[1]:
            #     tv_iter = self.liststore.get_iter(path[0])
            #     tedit = self.liststore.get_value(tv_iter, 0)
            #     if tedit.get_mapped():
            #         tedit.unmap()
            #     else:
            #         tedit.show_all()
            #         tedit.map()

    def tvbutton_row_activated(self, treeview, path, view_column):
        tv_iter = self.liststore.get_iter(path[0])
        tedit = self.liststore.get_value(tv_iter, 0)
        if tedit.get_mapped():
            tedit.unmap()
        else:
            tedit.show_all()
            tedit.map()

    def add_track(self, track_name):
        track = self.seq.newtrack(track_name);
        tedit = TrackEditor(track, self.seq, self.portlist)
        self.liststore.append([tedit, repr(track), 0, tedit.track.get_mute_state()])
        tedit.show_all()

    def button_add_track(self, button):
        track_name = prompt_gettext("Enter new track name)")
        if track_name:
            self.add_track(track_name)

    def toggle_mute(self, cell, path, model):
        model[path][0].track.toggle_mute()
        model[path][3] = not model[path][3]

    def toggle_mute_all(self, tv):
        def _toggle_mute(tvmodel, path, tv_iter):
            tvmodel.get_value(tv_iter, 0).track.toggle_mute()
            mute_val = not tvmodel.get_value(tv_iter, 3)
            self.liststore.set_value(tv_iter, 3, mute_val)
        self.liststore.foreach(_toggle_mute)

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
                                               repr(self.dnd_tedit.track),
                                               0,
                                               self.dnd_tedit.track.get_mute_state()])
            else:
                self.seq.move_track_after(tedit.track, self.dnd_tedit.track)
                model.insert_after(iterator, [self.dnd_tedit,
                                              repr(self.dnd_tedit.track),
                                              0,
                                              self.dnd_tedit.track.get_mute_state()])
        else:
            nchild = model.iter_n_children(None)
            if nchild > 1:
                iterator = model.get_iter("%d" % (nchild - 1))
                tedit = model.get_value(iterator, 0)
                self.seq.move_track_after(tedit.track, self.dnd_tedit.track)
                model.insert_after(iterator, [self.dnd_tedit,
                                              repr(self.dnd_tedit.track),
                                              0,
                                              self.dnd_tedit.track.get_mute_state()])

    def __init__(self, seq, portlist):
        gtk.Frame.__init__(self, "Track list")
        self.seq = seq
        self.portlist = portlist
        self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str, int, bool)
        for track in seq.gettracks():
            tedit = TrackEditor(track, self.seq, self.portlist)
            tedit.unmap()
            self.liststore.append([tedit, repr(track), 0, track.get_mute_state()])
        self.treev = gtk.TreeView(self.liststore)
        self.treev.set_enable_search(False)
        self.menu = TrackListMenu(self)

        cell_rdrr = gtk.CellRendererProgress()
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
        tvcolumn.connect('clicked', self.toggle_mute_all)
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
