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

from tool import prompt_gettext, MsqListMenu


class OutputListMenu(MsqListMenu):
    def rename_output(self, menuitem):
        if self.outputlist.seq.isrunning():
            print "Can not rename output while running"
            return
        if self.path:
            iter = self.outputlist.listfilter.get_iter(self.path[0])
            seqoutput = self.outputlist.listfilter.get_value(iter, 0)
            name = prompt_gettext("Rename output", seqoutput.get_name())
            if name:
                seqoutput.set_name(name)
                child_iter = self.outputlist.listfilter.convert_iter_to_child_iter(iter)
                self.outputlist.liststore.set_value(child_iter, 1, seqoutput.get_name())
            self.path = None

    def del_output(self, menuitem):
        if self.path:
            iter = self.outputlist.listfilter.get_iter(self.path[0])
            seqoutput = self.outputlist.listfilter.get_value(iter, 0)
            def check_output(model, path, iter):
                tedit = model.get_value(iter, 0)
                if tedit.track.has_output(seqoutput):
                    tedit.track.set_output(None)
            self.outputlist.tracklist.foreach(check_output)

            self.outputlist.seq.deloutput(seqoutput)
            child_iter = self.outputlist.listfilter.convert_iter_to_child_iter(iter)
            self.outputlist.liststore.remove(child_iter)
            self.path = None

    def __init__(self, outputlist):
        MsqListMenu.__init__(self)
        self.outputlist = outputlist

        self.mlm_add_root_item("Rename output", self.rename_output)
        separator = gtk.SeparatorMenuItem()
        separator.show()
        self.append(separator)
        self.mlm_add_root_item("Delete output", self.del_output)


class OutputList(gtk.Frame):
    def tvbutton_press_event(self, treeview, event):
        if event.button == 3:
            path = treeview.get_path_at_pos(int(event.x), int(event.y))
            if path:
                self.menu.path = path
                self.menu.popup(None, None, None, event.button, event.time)

    def add_output(self, name):
        if name:
            seqoutput = self.seq.newoutput(name)
            self.liststore.append([seqoutput, seqoutput.get_name()])
            return seqoutput

    def button_add_output(self, button):
        name = prompt_gettext("Enter new output name")
        self.add_output(name)

    def drag_data_get_data(self, treeview, context, selection, target_id, etime):
        if self.seq.isrunning():
            return
        treeselection = treeview.get_selection()
        model, iter = treeselection.get_selected()
        self.dnd_output = model.get_value(iter, 0)
        model.remove(iter)

    def drag_data_received_data(self, treeview, context, x, y, selection, info, etime):
        if self.seq.isrunning():
            return
        model = treeview.get_model()
        drop_info = treeview.get_dest_row_at_pos(x, y)
        if drop_info:
            path, position = drop_info
            iter = model.get_iter(path)
            output = model.get_value(iter, 0)
            if (position == gtk.TREE_VIEW_DROP_BEFORE or position == gtk.TREE_VIEW_DROP_INTO_OR_BEFORE):
                self.seq.move_output_before(output, self.dnd_output)
                model.insert_before(iter, [self.dnd_output, self.dnd_output.get_name()])
            else:
                self.seq.move_output_after(output, self.dnd_output)
                model.insert_after(iter, [self.dnd_output, self.dnd_output.get_name()])
        else:
            nchild = model.iter_n_children(None)
            if nchild > 1:
                iterator = model.get_iter("%d" % (nchild - 1))
                output = model.get_value(iterator, 0)
                self.seq.move_output_after(output, self.dnd_output)
                model.insert_after(iterator, [self.dnd_output,
                                              self.dnd_output.get_name()])


    def __init__(self, midiseq):
        gtk.Frame.__init__(self)
        self.seq = midiseq
        self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, str)
        self.liststore.append([None, "No output"])
        for seqoutput in self.seq.getoutputs():
            self.liststore.append([seqoutput, seqoutput.get_name()])

        self.listfilter = self.liststore.filter_new()
        def visible_output_func(model, iter):
            output = model.get_value(iter, 0)
            if output:
                return True
            else:
                return False
        self.listfilter.set_visible_func(visible_output_func)

        treev = gtk.TreeView(self.listfilter)
        treev.set_enable_search(False)

        tvcolumn = gtk.TreeViewColumn('Sequencer Output')
        cell = gtk.CellRendererText()
        # import pango
        # cell.set_property("alignment", pango.ALIGN_CENTER)
        tvcolumn.pack_start(cell, True)
        tvcolumn.add_attribute(cell, 'text', 1)
        treev.append_column(tvcolumn)
        self.menu = OutputListMenu(self)

        treev.connect('button-press-event', self.tvbutton_press_event)

        # Temporarily unavailable
        # treev.enable_model_drag_source(gtk.gdk.BUTTON1_MASK,
        #                                [("MIDILOOPER_OUTPUT_LIST",
        #                                  gtk.TARGET_SAME_WIDGET,
        #                                  0)],
        #                                gtk.gdk.ACTION_DEFAULT|gtk.gdk.ACTION_MOVE)
        # treev.enable_model_drag_dest([("MIDILOOPER_OUTPUT_LIST",
        #                                gtk.TARGET_SAME_WIDGET,
        #                                0)],
        #                              gtk.gdk.ACTION_DEFAULT)
        # treev.connect("drag_data_get", self.drag_data_get_data)
        # treev.connect("drag_data_received", self.drag_data_received_data)


        button_add = gtk.Button(stock=gtk.STOCK_ADD)
        button_add.connect("clicked", self.button_add_output)

        vbox = gtk.VBox()
        vbox.pack_start(treev)
        vbox.pack_start(button_add)

        self.add(vbox)

        self.tracklist = None
