#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

class PortList(gtk.Window):
    def __init__(self, midiseq):
         gtk.Window.__init__(self)
         self.midiseq = midiseq
         liststore = gtk.ListStore(str)
         for seqport in midiseq.getports():
             liststore.append([repr(seqport)])
         treev = gtk.TreeView(liststore)

         tvcolumn = gtk.TreeViewColumn('Sequencer Port')
         cell = gtk.CellRendererText()
         tvcolumn.pack_start(cell, True)
         tvcolumn.add_attribute(cell, 'text', 0)
         treev.append_column(tvcolumn)
         # self.treeview.set_search_column(0)
         # self.tvcolumn.set_sort_column_id(0)
         # self.treeview.set_reorderable(True)

         self.add(treev)
         self.connect('delete_event', gtk.main_quit)
