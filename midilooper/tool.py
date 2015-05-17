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

def prompt_gettext(label, prev_text=None):
    # dialog = gtk.MessageDialog(None,
    #                            gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
    #                            gtk.MESSAGE_QUESTION,
    #                            gtk.BUTTONS_OK,
    #                            None)
    def emit_resp(entry, dialog):
        dialog.response(gtk.RESPONSE_OK)
    dialog = gtk.Dialog(flags=gtk.DIALOG_MODAL)
    dialog.set_decorated(False)
    dialog.set_position(gtk.WIN_POS_MOUSE)
    label = gtk.Label(label)
    # dialog.set_markup(label)
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

    dialog = gtk.Dialog(flags=gtk.DIALOG_MODAL)
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

    dialog = gtk.Dialog(flags=gtk.DIALOG_MODAL)
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


class MsqListMenu(gtk.Menu):
    def mlm_add_item(self, name, callback=None):
            item = gtk.MenuItem(name)
            if callback:
                item.connect("activate", callback)
            item.show()
            self.append(item)
    def __init__(self):
        gtk.Menu.__init__(self)
        self.path = None
