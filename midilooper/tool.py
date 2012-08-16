#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

def prompt_gettext(label):
    # dialog = gtk.MessageDialog(None,
    #                            gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
    #                            gtk.MESSAGE_QUESTION,
    #                            gtk.BUTTONS_OK,
    #                            None)
    def emit_resp(entry, dialog):
        dialog.response(gtk.RESPONSE_OK)
    dialog = gtk.Dialog(flags=gtk.DIALOG_MODAL)
    dialog.set_decorated(False)
    label = gtk.Label(label)
    # dialog.set_markup(label)
    entry = gtk.Entry()
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
