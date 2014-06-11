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
            def check_port(model, path, iter):
                tedit = model.get_value(iter, 0)
                if tedit.track.has_port(seqport):
                    tedit.track.set_port(None)
            self.portlist.tracklist.foreach(check_port)

            self.portlist.seq.delport(seqport)
            self.portlist.liststore.remove(iter)
            self.path = None

    def __init__(self, portlist):
        MsqListMenu.__init__(self)
        self.portlist = portlist

        self.mlm_add_item("Rename port", self.rename_port)
        separator = gtk.SeparatorMenuItem()
        separator.show()
        self.append(separator)
        self.mlm_add_item("Delete port", self.del_port)


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

    def drag_data_get_data(self, treeview, context, selection, target_id, etime):
        if self.seq.isrunning():
            return
        treeselection = treeview.get_selection()
        model, iter = treeselection.get_selected()
        self.dnd_port = model.get_value(iter, 0)
        model.remove(iter)

    def drag_data_received_data(self, treeview, context, x, y, selection, info, etime):
        if self.seq.isrunning():
            return
        model = treeview.get_model()
        drop_info = treeview.get_dest_row_at_pos(x, y)
        if drop_info:
            path, position = drop_info
            iter = model.get_iter(path)
            port = model.get_value(iter, 0)
            if (position == gtk.TREE_VIEW_DROP_BEFORE or position == gtk.TREE_VIEW_DROP_INTO_OR_BEFORE):
                self.seq.move_port_before(port, self.dnd_port)
                model.insert_before(iter, [self.dnd_port, repr(self.dnd_port)])
            else:
                self.seq.move_port_after(port, self.dnd_port)
                model.insert_after(iter, [self.dnd_port, repr(self.dnd_port)])
        else:
            nchild = model.iter_n_children(None)
            if nchild > 1:
                iterator = model.get_iter("%d" % (nchild - 1))
                port = model.get_value(iterator, 0)
                self.seq.move_port_after(port, self.dnd_port)
                model.insert_after(iterator, [self.dnd_port,
                                              repr(self.dnd_port)])


    def __init__(self, midiseq):
        gtk.Window.__init__(self)
        self.set_resizable(False)
        self.seq = midiseq
        self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str)
        self.liststore.append([None, ""])
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

        treev.enable_model_drag_source(gtk.gdk.BUTTON1_MASK,
                                       [("MIDILOOPER_TRACK_LIST",
                                         gtk.TARGET_SAME_WIDGET,
                                         0)],
                                       gtk.gdk.ACTION_DEFAULT|gtk.gdk.ACTION_MOVE)
        treev.enable_model_drag_dest([("MIDILOOPER_TRACK_LIST",
                                       gtk.TARGET_SAME_WIDGET,
                                       0)],
                                     gtk.gdk.ACTION_DEFAULT)
        treev.connect("drag_data_get", self.drag_data_get_data)
        treev.connect("drag_data_received", self.drag_data_received_data)


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
        self.tracklist = None
