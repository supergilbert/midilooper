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
