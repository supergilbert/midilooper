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

class FocusOutDialog(gtk.Dialog):
    "Cancel on focus out gtk.Dialog"
    def __init__(self):
        def emit_cancel(dialog, evt):
            dialog.response(gtk.RESPONSE_CANCEL)
        gtk.Dialog.__init__(self, flags=gtk.DIALOG_MODAL)
        self.connect("focus-out-event", emit_cancel)

def prompt_gettext(label, prev_text=None):

    def emit_resp(entry, dialog):
        dialog.response(gtk.RESPONSE_OK)


    dialog = FocusOutDialog()
    dialog.set_decorated(False)
    dialog.set_position(gtk.WIN_POS_MOUSE)
    label = gtk.Label(label)
    entry = gtk.Entry()
    if prev_text:
        entry.set_text(prev_text)
    entry.connect("activate", emit_resp, dialog)
    dialog.vbox.pack_start(label, True, True, 0)
    dialog.vbox.pack_end(entry, True, True, 0)
    dialog.show_all()
    resp = dialog.run()
    text = None
    if resp == gtk.RESPONSE_OK:
        text = entry.get_text()
    dialog.destroy()
    return text

def prompt_keybinding(name):
    key_binding = None
    label = gtk.Label("""\
Getting key binding for %s.
Press any alpha-numeric key (timeout in 5 sec)""" % name)

    def emit_resp(widget, event, dialog, key_press):
        key_press[0] = gtk.gdk.keyval_name(event.keyval)
        dialog.response(gtk.RESPONSE_OK)

    def timeout_func(dialog):
        dialog.response(gtk.RESPONSE_CANCEL)

    dialog = FocusOutDialog()
    dialog.set_decorated(False)
    dialog.set_position(gtk.WIN_POS_MOUSE)
    dialog.vbox.pack_start(label, True, True, 0)
    key_press = [None]
    dialog.connect("key_press_event", emit_resp, dialog, key_press)
    timeoutid = gobject.timeout_add(5000, timeout_func, dialog)

    dialog.show_all()
    resp = dialog.run()
    if resp == gtk.RESPONSE_OK:
        gobject.source_remove(timeoutid)
        key_binding = key_press[0]
    elif resp == gtk.RESPONSE_DELETE_EVENT:
        gobject.source_remove(timeoutid)
    dialog.destroy()
    return key_binding

def prompt_notebinding(seq, name):
    label = gtk.Label("""\
Getting midi binding for %s.
Press any note (timeout in 5 sec)""" % name)

    check_period = 100

    def timeout_func(dialog, seq, timeout_data, check_period):
        note = seq.get_remote_note()
        if note != 255:
            timeout_data[0] = note
            dialog.response(gtk.RESPONSE_OK)
            return False
        if timeout_data[1] >= 5000:
            dialog.response(gtk.RESPONSE_OK)
            return False
        timeout_data[1] += check_period
        return True

    dialog = FocusOutDialog()
    dialog.set_decorated(False)
    dialog.set_position(gtk.WIN_POS_MOUSE)
    dialog.vbox.pack_start(label, True, True, 0)
    timeout_data = [None, 0]    # [midinote, timeout time]
    seq.clean_remote_note()
    timeoutid = gobject.timeout_add(check_period,
                                    timeout_func,
                                    dialog, seq, timeout_data, check_period)

    dialog.show_all()
    resp = dialog.run()
    if resp == gtk.RESPONSE_DELETE_EVENT:
        gobject.source_remove(timeoutid)
    dialog.destroy()
    return timeout_data[0]

TRACK_MAX_LENGTH = 9999

def prompt_get_loop(loop_start, loop_len):

    def button_apply_cb(button, dialog, loop_pos, spinbut1, spinbut2):
        loop_pos[0] = int(spinbut1.get_value())
        loop_pos[1] = int(spinbut2.get_value())
        dialog.response(gtk.RESPONSE_OK)

    def button_cancel_cb(button, loop_pos):
        loop_pos[0] = None
        dialog.response(gtk.RESPONSE_CANCEL)

    hbox = gtk.HBox()

    label = gtk.Label(" Loop Start: ")
    spinadj = gtk.Adjustment(loop_start,
                             0,
                             TRACK_MAX_LENGTH,
                             1)
    spinbut1 = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
    hbox.pack_start(label,   True, True, 0)
    hbox.pack_start(spinbut1, True, True, 0)

    label = gtk.Label(" Loop length: ")
    spinadj = gtk.Adjustment(loop_len,
                             1,
                             TRACK_MAX_LENGTH,
                             1)
    spinbut2 = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
    hbox.pack_start(label,   True, True, 0)
    hbox.pack_start(spinbut2, True, True, 0)

    dialog = FocusOutDialog()
    dialog.set_position(gtk.WIN_POS_MOUSE)
    dialog.vbox.pack_start(hbox, True, True, 0)

    hbox = gtk.HBox()

    loop_pos = [loop_start, loop_len]

    button = gtk.Button(stock=gtk.STOCK_APPLY)
    button.connect("clicked",
                   button_apply_cb, dialog, loop_pos, spinbut1, spinbut2)
    hbox.pack_start(button, True, True, 0)

    button = gtk.Button(stock=gtk.STOCK_CANCEL)
    button.connect("clicked", button_cancel_cb, loop_pos)
    hbox.pack_start(button, True, True, 0)

    dialog.vbox.pack_start(hbox, True, True, 0)

    dialog.show_all()
    dialog.run()
    dialog.destroy()
    if loop_pos[0] == None:
        return None
    return loop_pos

def prompt_get_output(portlist, idx=None):

    def button_apply_cb(button, portlist_cbbox, port_res):
        port_iter = portlist_cbbox.get_active_iter()
        model = portlist_cbbox.get_model()
        output = model.get_value(port_iter, 0)
        port_res[0] = True
        port_res[1] = output
        dialog.response(gtk.RESPONSE_OK)

    def button_cancel_cb(button, loop_ptr):
        dialog.response(gtk.RESPONSE_CANCEL)

    # Temp hack: can not use FocusOutDialog class because combox window focus-in
    # send focus-out signal to the class (and that create dependencies problem).
    # dialog = FocusOutDialog()
    dialog = gtk.Dialog()
    dialog.set_position(gtk.WIN_POS_MOUSE)

    portlist_cbbox = gtk.ComboBox(portlist)
    cell = gtk.CellRendererText()
    portlist_cbbox.pack_start(cell, True)
    portlist_cbbox.add_attribute(cell, 'text', 1)
    if idx != None:
        portlist_cbbox.set_active(idx)
    else:
        portlist_cbbox.set_active(0)

    dialog.vbox.pack_start(portlist_cbbox, True, True, 0)

    hbox = gtk.HBox()

    port_res = [False, None]
    button = gtk.Button(stock=gtk.STOCK_APPLY)
    button.connect("clicked", button_apply_cb, portlist_cbbox, port_res)
    hbox.pack_start(button, True, True, 0)

    button = gtk.Button(stock=gtk.STOCK_CANCEL)
    button.connect("clicked", button_cancel_cb, port_res)
    hbox.pack_start(button, True, True, 0)

    dialog.vbox.pack_start(hbox, True, True, 0)

    dialog.show_all()
    dialog.run()
    dialog.destroy()
    return port_res

class MsqListMenu(gtk.Menu):
    @staticmethod
    def mlm_add_menu_item(menu, name, callback=None):
        item = gtk.MenuItem(name)
        if callback:
            item.connect("activate", callback)
        item.show()
        menu.append(item)

    def mlm_add_root_item(self, name, callback=None):
        self.mlm_add_menu_item(self, name, callback)

    @staticmethod
    def mlm_add_menu_separator(menu):
        separator = gtk.SeparatorMenuItem()
        separator.show()
        menu.append(separator)

    def mlm_add_root_separator(self):
        self.mlm_add_menu_separator(self)

    @staticmethod
    def mlm_add_submenu(menu, name):
        submenu = gtk.Menu()
        mi = gtk.MenuItem(name)
        mi.set_submenu(submenu)
        mi.show()
        menu.append(mi)
        return submenu

    def __init__(self):
        gtk.Menu.__init__(self)
        self.path = None
