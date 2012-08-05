#!/usr/bin/python

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
            new_trackname = prompt_gettext("Rename track")
            if new_trackname:
                tv_iter = self.tracklist.liststore.get_iter(self.path[0])
                tedit = self.tracklist.liststore.get_value(tv_iter, 0)
                tedit.set_name(new_trackname)
                self.tracklist.liststore.set_value(tv_iter, 1, repr(tedit.track))
            self.path = None

    def __init__(self, tracklist):
        MsqListMenu.__init__(self)
        self.tracklist = tracklist

        self.mlm_add_item("Show track", self.show_track)
        self.mlm_add_item("Rename track", self.rename_track)
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
            # else:
            #     tv_iter = self.liststore.get_iter(path[0])
            #     tedit = self.liststore.get_value(tv_iter, 0)
            #     if tedit.get_mapped():
            #         tedit.unmap()
            #     else:
            #         tedit.show_all()
            #         tedit.map()

    def add_track(self):
        track_name = prompt_gettext("Enter new track name")
        if track_name:
            track = self.seq.newtrack(track_name);
            tedit = TrackEditor(track, self.seq, self.portlist)
            self.liststore.append([tedit, repr(track), 0, 0])
            tedit.show_all()

    def button_add_track(self, button):
        self.add_track()

    def toggle_mute(self, cell, path, model):
        model[path][0].track.toggle_mute()
        model[path][3] = not model[path][3]

    def __init__(self, seq, portlist):
        gtk.Frame.__init__(self, "Track list")
        # self.set_resizable(False)
        self.seq = seq
        self.portlist = portlist
        self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str, int, bool)
        # self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str, int, gobject.TYPE_PYOBJECT)
        for track in seq.gettracks():
            tedit = TrackEditor(track, self.seq, self.portlist)
            tedit.unmap()
            self.liststore.append([tedit, repr(track), 0, track.get_mute_state()])
        self.treev = gtk.TreeView(self.liststore)
        self.treev.set_enable_search(False)
        self.menu = TrackListMenu(self)

        cell = gtk.CellRendererProgress()
        tvcolumn = gtk.TreeViewColumn('Track', cell, text=1, value=2)
        tvcolumn.set_expand(True)
        self.treev.append_column(tvcolumn)
        self.treev.connect('button-press-event', self.tvbutton_press_event)

        cell = gtk.CellRendererToggle()
        cell.set_property('activatable', True)
        cell.connect('toggled', self.toggle_mute, self.liststore)
        tvcolumn = gtk.TreeViewColumn('M', cell, active=3)
        tvcolumn.set_expand(False)
        self.treev.append_column(tvcolumn)

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
