#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

import sys
sys.path.append('./module')

from track_editor import TrackEditor

class TrackList(gtk.Window):
    def update_pos(self, tickpos):
        def update_tedit(tvmodel, path, tv_iter, tickpos):
            tedit = tvmodel.get_value(tv_iter, 0)
            tedit.update_pos(tickpos)
        tvmodel = self.treev.get_model()
        tvmodel.foreach(update_tedit, tickpos)

    def clear_progress(self):
        def clear_tedit_progress(tvmodel, path, tv_iter):
            tedit = tvmodel.get_value(tv_iter, 0)
            tedit.clear_progress()
        tvmodel = self.treev.get_model()
        tvmodel.foreach(clear_tedit_progress)

    def tvbutton_press_event(self, treeview, event):
        tvdata = treeview.get_path_at_pos(int(event.x), int(event.y))
        if tvdata:
            tvmodel = self.treev.get_model()
            tv_iter = tvmodel.get_iter(tvdata[0])
            tedit = tvmodel.get_value(tv_iter, 0)
            if tedit.get_mapped():
                tedit.unmap()
            else:
                tedit.show_all()
                tedit.map()

    # TODO
    # def add_track(self, button):

    def __init__(self, seq):
         gtk.Window.__init__(self)
         self.seq = seq
         self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str)
         for track in seq.gettracks():
             tedit = TrackEditor(track, seq.getppq(), sequencer=self.seq)
             tedit.unmap()
             self.liststore.append([tedit, repr(track)])
         treev = gtk.TreeView(self.liststore)

         tvcolumn = gtk.TreeViewColumn('Track name')
         cell = gtk.CellRendererText()
         tvcolumn.pack_start(cell, True)
         tvcolumn.add_attribute(cell, 'text', 1)
         treev.append_column(tvcolumn)

         treev.connect('button-press-event', self.tvbutton_press_event)
         self.treev = treev

         # button_add = gtk.Button("Add")
         # button_add.connect("clicked", self.add_track)

         vbox = gtk.VBox()
         vbox.pack_start(self.treev)
         # vbox.pack_start(button_add)

         self.add(vbox)
         self.connect('delete_event', gtk.main_quit)
