#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

from tool import prompt_gettext, MsqListMenu

class PortListMenu(MsqListMenu):
    def rename_port(self, menuitem):
        if self.path:
            iter = self.portlist.liststore.get_iter(self.path[0])
            seqport = self.portlist.liststore.get_value(iter, 0)
            name = prompt_gettext("Rename port")
            if name:
                seqport.set_name(name)
                self.portlist.liststore.set_value(iter, 1, repr(seqport))
            self.path = None

    def del_port(self, menuitem):
        if self.path:
            iter = self.portlist.liststore.get_iter(self.path[0])
            seqport = self.portlist.liststore.get_value(iter, 0)
            self.portlist.seq.delport(seqport)
            self.portlist.liststore.remove(iter)
            self.path = None

    def __init__(self, portlist):
        MsqListMenu.__init__(self)
        self.portlist = portlist

        self.mlm_add_item("Rename track", self.rename_port)
        separator = gtk.SeparatorMenuItem()
        separator.show()
        self.append(separator)
        self.mlm_add_item("Del track", self.del_port)



class PortList(gtk.Window):
    def tvbutton_press_event(self, treeview, event):
        if event.button == 3:
            path = treeview.get_path_at_pos(int(event.x), int(event.y))
            if path:
                self.menu.path = path
                self.menu.popup(None, None, None, event.button, event.time)

    def button_add_port(self, button):
        name = prompt_gettext("Enter new port name")
        if name:
            seqport = self.seq.newoutput(name)
            self.liststore.append([seqport, repr(seqport)])

    def __init__(self, midiseq):
        gtk.Window.__init__(self)
        self.set_resizable(False)
        self.seq = midiseq
        self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str)
        for seqport in self.seq.getports():
            self.liststore.append([seqport, repr(seqport)])
        treev = gtk.TreeView(self.liststore)
        treev.set_enable_search(False)

        tvcolumn = gtk.TreeViewColumn('Sequencer Port')
        cell = gtk.CellRendererText()
        tvcolumn.pack_start(cell, True)
        tvcolumn.add_attribute(cell, 'text', 1)
        treev.append_column(tvcolumn)
        self.menu = PortListMenu(self)

        treev.connect('button-press-event', self.tvbutton_press_event)

        button_add = gtk.Button(stock=gtk.STOCK_ADD)
        button_add.connect("clicked", self.button_add_port)

        vbox = gtk.VBox()
        vbox.pack_start(treev)
        vbox.pack_start(button_add)

        self.add(vbox)

        def hide_portlist(win, event):
            win.hide()
            return True
        self.connect('delete_event', hide_portlist)
