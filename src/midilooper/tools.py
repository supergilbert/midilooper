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
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk


class FocusOutDialog(Gtk.Dialog):
    "Cancel on focus out Gtk.Dialog"
    def __init__(self, parent_win):
        def emit_cancel(dialog, evt):
            dialog.response(Gtk.ResponseType.CANCEL)
        Gtk.Dialog.__init__(self, flags=Gtk.DialogFlags.MODAL, parent=parent_win)
        self.connect("focus-out-event", emit_cancel)


def prompt_gettext(parent_win, label, prev_text=None):

    def emit_resp(entry, dialog):
        dialog.response(Gtk.ResponseType.OK)

    dialog = FocusOutDialog(parent_win)
    dialog.set_decorated(False)
    dialog.set_position(Gtk.WindowPosition.MOUSE)
    label = Gtk.Label(label=label)
    entry = Gtk.Entry()
    if prev_text:
        entry.set_text(prev_text)
    entry.connect("activate", emit_resp, dialog)
    dialog.vbox.pack_start(label, True, True, 0)
    dialog.vbox.pack_end(entry, True, True, 0)
    dialog.show_all()
    resp = dialog.run()
    text = None
    if resp == Gtk.ResponseType.OK:
        text = entry.get_text()
    dialog.destroy()
    return text


def prompt_keybinding(parent_win, name):
    key_binding = None
    label = Gtk.Label(label="""\
Getting key binding for %s.
Press any alpha-numeric key (timeout in 5 sec)""" % name)

    def emit_resp(widget, event, dialog, key_press):
        key_press[0] = Gdk.keyval_name(event.keyval)
        dialog.response(Gtk.ResponseType.OK)

    def timeout_func(dialog):
        dialog.response(Gtk.ResponseType.CANCEL)

    dialog = FocusOutDialog(parent_win)
    dialog.set_decorated(False)
    dialog.set_position(Gtk.WindowPosition.MOUSE)
    dialog.vbox.pack_start(label, True, True, 0)
    key_press = [None]
    dialog.connect("key_press_event", emit_resp, dialog, key_press)
    timeoutid = GObject.timeout_add(5000, timeout_func, dialog)

    dialog.show_all()
    resp = dialog.run()
    if resp == Gtk.ResponseType.OK:
        GObject.source_remove(timeoutid)
        key_binding = key_press[0]
    elif resp == Gtk.ResponseType.DELETE_EVENT:
        GObject.source_remove(timeoutid)
    dialog.destroy()
    return key_binding


def prompt_notebinding(parent_win, seq, name):
    label = Gtk.Label(label="""\
Getting midi binding for %s.
Press any note (timeout in 5 sec)""" % name)

    check_period = 100

    def timeout_func(dialog, seq, timeout_data, check_period):
        note = seq.get_remote_note()
        if note != 255:
            timeout_data[0] = note
            dialog.response(Gtk.ResponseType.OK)
            return False
        if timeout_data[1] >= 5000:
            dialog.response(Gtk.ResponseType.OK)
            return False
        timeout_data[1] += check_period
        return True
    # For virtual keyboard debugging
    # dialog = Gtk.Dialog(flags=Gtk.DialogFlags.MODAL, parent=parent_win)
    dialog = FocusOutDialog(parent_win)
    dialog.set_decorated(False)
    dialog.set_position(Gtk.WindowPosition.MOUSE)
    dialog.vbox.pack_start(label, True, True, 0)
    timeout_data = [None, 0]    # [midinote, timeout time]
    seq.clean_remote_note()
    timeoutid = GObject.timeout_add(check_period,
                                    timeout_func,
                                    dialog, seq, timeout_data, check_period)

    dialog.show_all()
    resp = dialog.run()
    if resp == Gtk.ResponseType.DELETE_EVENT:
        GObject.source_remove(timeoutid)
    dialog.destroy()
    return timeout_data[0]


TRACK_MAX_LENGTH = 9999


def prompt_get_loop(parent_win, loop_start, loop_len):

    def button_apply_cb(button, dialog, loop_pos, spinbut1, spinbut2):
        loop_pos[0] = int(spinbut1.get_value())
        loop_pos[1] = int(spinbut2.get_value())
        dialog.response(Gtk.ResponseType.OK)

    def button_cancel_cb(button, loop_pos):
        loop_pos[0] = None
        dialog.response(Gtk.ResponseType.CANCEL)

    hbox = Gtk.HBox()

    label = Gtk.Label(label=" Loop Start: ")
    spinadj = Gtk.Adjustment(value=loop_start,
                             lower=0,
                             upper=TRACK_MAX_LENGTH,
                             step_increment=1)
    spinbut1 = Gtk.SpinButton(adjustment=spinadj, climb_rate=1)
    hbox.pack_start(label,   True, True, 0)
    hbox.pack_start(spinbut1,True, True, 0)

    label = Gtk.Label(label=" Loop length: ")
    spinadj = Gtk.Adjustment(value=loop_len,
                             lower=1,
                             upper=TRACK_MAX_LENGTH,
                             step_increment=1)
    spinbut2 = Gtk.SpinButton(adjustment=spinadj, climb_rate=1)
    hbox.pack_start(label,   True, True, 0)
    hbox.pack_start(spinbut2, True, True, 0)

    dialog = FocusOutDialog(parent_win)
    dialog.set_position(Gtk.WindowPosition.MOUSE)
    dialog.vbox.pack_start(hbox, True, True, 0)

    hbox = Gtk.HBox()

    loop_pos = [None, None]

    button = Gtk.Button(stock=Gtk.STOCK_APPLY)
    button.connect("clicked",
                   button_apply_cb, dialog, loop_pos, spinbut1, spinbut2)
    hbox.pack_start(button, True, True, 0)

    button = Gtk.Button(stock=Gtk.STOCK_CANCEL)
    button.connect("clicked", button_cancel_cb, loop_pos)
    hbox.pack_start(button, True, True, 0)

    dialog.vbox.pack_start(hbox, True, True, 0)

    dialog.show_all()
    resp_type = dialog.run()
    dialog.destroy()
    if resp_type != Gtk.ResponseType.OK or loop_pos[0] == None:
        return None
    return loop_pos


def prompt_get_output(parent_win, portlist, idx=None):
    def set_output_path(tv_path, dialog, portlist, output_res):
        tv_iter = portlist.get_iter(tv_path[0])
        output = portlist.get_value(tv_iter, 0)
        output_res[0] = True
        output_res[1] = output
        dialog.response(Gtk.ResponseType.OK)

    def row_activated(tv, tv_path, col, dialog, portlist, output_res):
        set_output_path(tv_path, dialog, portlist, output_res)

    def button_apply_cb(button, dialog, treev, portlist, output_res):
        tv_path = treev.get_cursor()
        set_output_path(tv_path, dialog, portlist, output_res)

    def button_cancel_cb(button):
        dialog.response(Gtk.ResponseType.CANCEL)

    cell = Gtk.CellRendererText()
    tvcolumn = Gtk.TreeViewColumn('Output', cell, text=1)
    treev = Gtk.TreeView(portlist)
    treev.append_column(tvcolumn)
    treev.set_headers_visible(False)
    treev.set_cursor(idx)
    dialog = FocusOutDialog(parent_win)
    dialog.set_position(Gtk.WindowPosition.MOUSE)
    dialog.vbox.pack_start(treev, True, True, 0)

    output_res = [False, None]
    treev.connect('row-activated', row_activated, dialog, portlist, output_res)

    hbox = Gtk.HBox()
    button = Gtk.Button(stock=Gtk.STOCK_APPLY)
    button.connect("clicked", button_apply_cb, dialog, treev, portlist, output_res)
    hbox.pack_start(button, True, True, 0)
    button = Gtk.Button(stock=Gtk.STOCK_CANCEL)
    button.connect("clicked", button_cancel_cb)
    hbox.pack_start(button, True, True, 0)

    dialog.vbox.pack_start(hbox, True, True, 0)

    dialog.show_all()
    dialog.run()
    dialog.destroy()
    return output_res


class MsqListMenu(Gtk.Menu):
    @staticmethod
    def mlm_add_menu_item(menu, name, callback=None):
        item = Gtk.MenuItem(name)
        if callback:
            item.connect("activate", callback)
        item.show()
        menu.append(item)

    def mlm_add_root_item(self, name, callback=None):
        self.mlm_add_menu_item(self, name, callback)

    @staticmethod
    def mlm_add_menu_separator(menu):
        separator = Gtk.SeparatorMenuItem()
        separator.show()
        menu.append(separator)

    def mlm_add_root_separator(self):
        self.mlm_add_menu_separator(self)

    @staticmethod
    def mlm_add_submenu(menu, name):
        submenu = Gtk.Menu()
        mi = Gtk.MenuItem(name)
        mi.set_submenu(submenu)
        mi.show()
        menu.append(mi)
        return submenu

    def __init__(self):
        GObject.GObject.__init__(self)
        self.path = None
