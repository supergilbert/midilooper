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

#import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required"
    raise SystemExit

from msqwidget import MsqHBarTimeWidget, MsqVBarNoteWidget, MsqNoteGridWidget, MIN_QNOTE_XSZ, DEFAULT_QNOTE_XSZ, MAX_QNOTE_XSZ
from msqwidget.midivaluewgt import MsqValueWidget
from msqwidget.wgttools import MIDI_CTRL_EVENT

MAX_LENGTH = 240

class TrackSettingTable(gtk.Table):
    def port_changed(self, combobox):
        portlist = combobox.get_model()
        port_idx = combobox.get_active()
        if port_idx >= 0:
            port = portlist[port_idx][0]
            self.chaned.setting.track.set_output(port)

    def set_track_len(self, widget):
        self.chaned.set_len(int(widget.get_value()))

    def set_track_start(self, widget):
        self.chaned.set_start(int(widget.get_value()))

    def __init__(self, chaned, portlist):
        gtk.Table.__init__(self, 6, 1)
        self.chaned = chaned

        label = gtk.Label(" Loop Start: ")
        spinadj = gtk.Adjustment(self.chaned.setting.getstart() / self.chaned.setting.getppq(),
                                 0,
                                 MAX_LENGTH - 1,
                                 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinadj.connect("value-changed", self.set_track_start)
        spinbut.set_tooltip_text("Set the track start")
        self.attach(label, 0, 1, 0, 1)
        self.attach(spinbut, 1, 2, 0, 1)

        label = gtk.Label(" Loop length: ")
        spinadj = gtk.Adjustment(self.chaned.setting.getlen() / self.chaned.setting.getppq(),
                                 1,
                                 MAX_LENGTH,
                                 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinadj.connect("value-changed", self.set_track_len)
        spinbut.set_tooltip_text("Set the track length")
        self.attach(label, 2, 3, 0, 1)
        self.attach(spinbut, 3, 4, 0, 1)

        label = gtk.Label("  Output Port: ")
        portlist_cbbox = gtk.ComboBox(portlist)
        cell = gtk.CellRendererText()
        portlist_cbbox.pack_start(cell, True)
        portlist_cbbox.add_attribute(cell, 'text', 1)
        portlist_cbbox.connect("changed", self.port_changed)
        for idx, model in enumerate(portlist):
            if self.chaned.setting.track.has_output(model[0]):
                portlist_cbbox.set_active(idx)
                break
        self.attach(label, 4, 5, 0, 1)
        self.attach(portlist_cbbox, 5, 6, 0, 1)




class GridSettingTable(gtk.Table):

    def set_note_value_cb(self, widget):
        self.chaned.setting.note_val_on = int(widget.get_value())

    def chan_changed_cb(self, widget):
        chan_list = widget.get_model()
        chan_num = chan_list[widget.get_active()][0]
        self.chaned.setting.chan_num = chan_num

        # Refreshing channel combobox
        track_info = get_track_info(self.chaned.setting.track)
        citer = chan_list.get_iter_root()
        while citer:
            channel = chan_list.get_value(citer, 0)
            if channel in track_info[2]:
                chan_list.set(citer, 1, "%i *" % channel)
            else:
                chan_list.set(citer, 1, "%i" % channel)
            citer = chan_list.iter_next(citer)

        # Refreshing midivalue viewer combobox
        chan_key = "%i" % self.chaned.setting.chan_num
        if track_info[3].has_key(chan_key):
            ctrl_list = track_info[3][chan_key]
        else:
            ctrl_list = []
        viter = self.chaned.val_list.get_iter("2")
        while viter:
            ctrl_num = self.chaned.val_list.get_value(viter, 0)
            if ctrl_num in ctrl_list:
                self.chaned.val_list.set(viter, 1, "Ctrl %i *" % ctrl_num)
            else:
                self.chaned.val_list.set(viter, 1, "Ctrl %i" % ctrl_num)
            viter = self.chaned.val_list.iter_next(viter)

        self.chaned.redraw()

    def res_changed(self, cbbox):
        val_int = cbbox.get_model()[cbbox.get_active()][0]
        self.chaned.grid.setting.tick_res = val_int
        self.chaned.redraw()

    def __init__(self, chaned, chan_list):
        self.chaned = chaned
        gtk.Table.__init__(self, 8, 1)

        label = gtk.Label("   Resolution: ")
        res_list = gtk.ListStore(int, str)
        ppq = self.chaned.setting.sequencer.getppq()
        for val in [1, 2, 4, 8, 16, 32, 64, 3, 6, 12, 24, 48]:
            if not ppq % val:
                res_list.append((ppq/val, "1 / %s (%sp)" % (val, ppq/val)))
        combo_res = gtk.ComboBox(res_list)
        combo_res.set_active(2)
        cell = gtk.CellRendererText()
        combo_res.pack_start(cell, True)
        combo_res.add_attribute(cell, 'text', 1)
        combo_res.connect("changed", self.res_changed)

        self.attach(label, 2, 3, 0, 1)
        self.attach(combo_res, 3, 4, 0, 1)

        label = gtk.Label("   Value: ")
        spinadj = gtk.Adjustment(self.chaned.setting.note_val_on, 0, 127, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinbut.set_numeric(True)
        spinadj.connect("value-changed", self.set_note_value_cb)
        self.attach(label, 4, 5, 0, 1)
        self.attach(spinbut, 5, 6, 0, 1)

        label = gtk.Label("  Channel: ")
        chan_liststore = gtk.ListStore(int, str)
        for idx in range(0, 15):
            if idx in chan_list:
                chan_liststore.append([idx, "* %d" % idx])
            else:
                chan_liststore.append([idx, "%d" % idx])
        chan_cbbox = gtk.ComboBox(chan_liststore)
        cell = gtk.CellRendererText()
        chan_cbbox.pack_start(cell, True)
        chan_cbbox.add_attribute(cell, 'text', 1)
        chan_cbbox.set_active(self.chaned.setting.chan_num)
        chan_cbbox.connect("changed", self.chan_changed_cb)
        self.attach(label, 6, 7, 0, 1)
        self.attach(chan_cbbox, 7, 8, 0, 1)


def get_track_info(track):
    track_min = 0
    track_max = 0
    channel_list = []
    channel_ctrl = {}
    for event in track.getall_event():
        # event = evwr.get_event()
        if event[0] < track_min:
            track_min = event[0]
        if event[0] > track_max:
            track_max = event[0]
        chan_key = "%d" % event[1]
        if not (event[1] in channel_list):
            channel_list.append(event[1])
            channel_ctrl[chan_key] = []
        if event[2] == MIDI_CTRL_EVENT and not (event[3] in channel_ctrl[chan_key]):
            channel_ctrl[chan_key].append(event[3])
    channel_list.sort()
    for key in channel_ctrl.keys():
        channel_ctrl[key].sort()
    return (track_min, track_max, channel_list, channel_ctrl)

def is_mask_to_bypass(evstate):
    if (evstate & gtk.gdk.CONTROL_MASK or
        evstate & gtk.gdk.MOD1_MASK or
        evstate & gtk.gdk.MOD3_MASK or
        evstate & gtk.gdk.MOD4_MASK or
        evstate & gtk.gdk.MOD5_MASK):
           return True

def dec_adj(adj, pad):
    value = adj.get_value()
    new_val = value - pad
    adj.set_value(new_val if new_val >= adj.lower else adj.lower)

def inc_adj(adj, pad):
    value = adj.get_value()
    new_val = value + pad
    max_val = adj.upper - adj.page_size
    adj.set_value(new_val if new_val <= max_val else max_val)


class ChannelEditor(gtk.VBox):
    def resize_all(self):
        self.hbar.resize_wgt()
        self.grid.resize_wgt()
        self.value_wgt.resize_wgt()

    def set_len(self, track_len):
        self.setting.track.set_len(track_len * self.setting.getppq())
        self.resize_all()

    def set_start(self, start):
        self.setting.track.set_start(start * self.setting.getppq())
        self.resize_all()

    def set_chaned_vadj(self, widget, event, vadj, noteysz):
        if is_mask_to_bypass(event.state):
            return
        yinc = noteysz * 5
        if event.direction == gdk.SCROLL_DOWN:
            inc_adj(vadj, yinc)
        elif event.direction == gdk.SCROLL_UP:
            dec_adj(vadj, yinc)

    def set_chaned_hadj(self, widget, event, hadj, qnxsz):
        if is_mask_to_bypass(event.state):
            return
        xinc = qnxsz * 5
        if event.direction == gdk.SCROLL_RIGHT:
            inc_adj(hadj, xinc)
        elif event.direction == gdk.SCROLL_LEFT:
            dec_adj(hadj, xinc)

    def set_chaned_adj(self, widget, event, hadj, vadj, qnxsz, noteysz):
        if is_mask_to_bypass(event.state):
           return
        xinc = qnxsz * 5
        yinc = noteysz * 5

        if event.direction == gdk.SCROLL_DOWN:
            if event.state & gtk.gdk.SHIFT_MASK:
                xinc = qnxsz * 5
                inc_adj(hadj, xinc)
            else:
                yinc = noteysz * 5
                inc_adj(vadj, yinc)

        elif event.direction == gdk.SCROLL_UP:
            if event.state & gtk.gdk.SHIFT_MASK:
                xinc = qnxsz * 5
                dec_adj(hadj, xinc)
            else:
                yinc = noteysz * 5
                dec_adj(vadj, yinc)

        elif event.direction == gdk.SCROLL_RIGHT:
            xinc = qnxsz * 5
            inc_adj(hadj, xinc)

        elif event.direction == gdk.SCROLL_LEFT:
            xinc = qnxsz * 5
            dec_adj(hadj, xinc)

    @staticmethod
    def get_vp_area(vp):
        hadj = vp.get_hadjustment()
        vadj = vp.get_vadjustment()
        return gtk.gdk.Rectangle(int(hadj.get_value()),
                                 int(vadj.get_value()),
                                 int(hadj.get_page_size()),
                                 int(vadj.get_page_size()))

    def redraw(self):
        self.grid.draw_area(self.get_vp_area(self.grid_vp))
        self.hbar.draw_area(self.get_vp_area(self.hbar_vp))
        self.value_wgt.draw_area(self.get_vp_area(self.value_vp))

    def debug_grid1(self, button, track):
        self.redraw()

    def debug_grid2(self, button, track):
        track._dump()

    def handle_motion(self, widget, event, hbar, vbar):
        # tick = self.grid.xpos2tick(event.x)
        note = self.grid.ypos2note(int(event.y))
        # hbar.show_tick(tick)
        vbar.show_note(note)

    def handle_leave_notify(self, widget, event, hbar, vbar):
        vbar.clear_note()

    def handle_zoom_x(self, adj):
        val = adj.get_value()
        if val <= 16.0:
            self.setting.qnxsz = int(MIN_QNOTE_XSZ + ((DEFAULT_QNOTE_XSZ - MIN_QNOTE_XSZ) * val / 16.0))
        else:
            self.setting.qnxsz = int(DEFAULT_QNOTE_XSZ + ((MAX_QNOTE_XSZ - DEFAULT_QNOTE_XSZ) * (val - 16) / 16.0))
        self.resize_all()
        self.redraw()

    def valuetype_changed(self, combobox):
        value_list = combobox.get_model()
        list_idx   = combobox.get_active()
        if list_idx >= 0:
            value = value_list[list_idx][0]
            if list_idx == 0:
                self.value_wgt.set_note_mode()
            elif list_idx == 1:
                self.value_wgt.set_pitch_mode()
            else:
                if list_idx > 1:
                    self.value_wgt.set_ctrl_mode(list_idx - 2)
            self.value_wgt.draw_area(self.get_vp_area(self.value_vp))


    def __init__(self, track, sequencer):
        gtk.VBox.__init__(self)

        self.grid = MsqNoteGridWidget(track, sequencer)
        self.setting = self.grid.setting

        track_info = get_track_info(track)
        self.setting.chan_num = track_info[2][0] if len(track_info[2]) else 0

        self.hbar = MsqHBarTimeWidget(self.setting)
        self.hbar_vp = gtk.Viewport()
        self.hbar_vp.set_size_request(self.setting.min_width, -1)
        self.hbar_vp.add(self.hbar)
        self.hbar_vp.set_shadow_type(gtk.SHADOW_NONE)
        hadj = self.hbar_vp.get_hadjustment()
        self.hbar_vp.connect("scroll_event", self.set_chaned_hadj, hadj, self.setting.qnxsz)

        vbar = MsqVBarNoteWidget(self.setting)
        vbar_vp = gtk.Viewport()
        vbar_vp.set_size_request(-1, self.setting.min_height)
        vbar_vp.add(vbar)
        vbar_vp.set_shadow_type(gtk.SHADOW_NONE)
        vadj = vbar_vp.get_vadjustment()
        vbar_vp.connect("scroll_event", self.set_chaned_vadj, vadj, self.setting.noteysz)

        self.grid.connect("motion_notify_event", self.handle_motion, self.hbar, vbar)
        self.grid.vadj = vadj
        self.grid_vp = gtk.Viewport()
        self.grid_vp.set_size_request(self.setting.min_width, self.setting.min_height)
        self.grid_vp.add(self.grid)
        self.grid_vp.connect("scroll_event", self.set_chaned_adj, hadj, vadj, self.setting.qnxsz, self.setting.noteysz)
        self.grid_vp.set_shadow_type(gtk.SHADOW_NONE)
        self.grid_vp.set_hadjustment(hadj)
        self.grid_vp.set_vadjustment(vadj)
        self.grid_vp.set_tooltip_text("""\
* Left button to select notes
* Right button to enter in edit mode
  (then in edit mode press left button to write notes)
* Middle button to change note size
* Suppr to delete selected notes""")
        evbox_grid = gtk.EventBox()
        evbox_grid.add(self.grid_vp)
        evbox_grid.connect("leave-notify-event", self.handle_leave_notify, self.hbar, vbar)

        vbar_vp.connect("button_press_event", vbar.handle_button_press, self.grid)
        vbar_vp.connect("button_release_event", vbar.handle_button_release, self.grid)
        vbar_vp.connect("motion_notify_event", vbar.handle_motion, self.grid)

        hsb = gtk.HScrollbar(hadj)
        vsb = gtk.VScrollbar(vadj)

        table = gtk.Table(3, 2)
        table.attach(self.hbar_vp, 1, 2, 0, 1, gtk.FILL, 0)
        table.attach(vbar_vp,      0, 1, 1, 2, 0, gtk.FILL)
        table.attach(evbox_grid,   1, 2, 1, 2, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
        table.attach(vsb,          2, 3, 1, 2, 0, gtk.FILL)

        self.val_list = gtk.ListStore(int, str)
        chan_key = "%i" % self.setting.chan_num
        if track_info[3].has_key(chan_key):
            ctrl_list = track_info[3][chan_key]
        else:
            ctrl_list = []
        self.val_list.append([9, "Note on"])
        self.val_list.append([14, "Pitch Bend"])
        for idx in range(128):
            self.val_list.append([idx,
                                  "Ctrl %i *" % idx if idx in ctrl_list else "Ctrl %i" % idx])
        valuetype_cbbox = gtk.ComboBox(self.val_list)
        valuetype_cbbox.set_active(0)
        cell = gtk.CellRendererText()
        valuetype_cbbox.pack_start(cell, True)
        valuetype_cbbox.add_attribute(cell, 'text', 1)
        valuetype_cbbox.connect("changed", self.valuetype_changed)
        valuetype_cbbox.set_size_request(vbar.width, -1)

        self.value_wgt = MsqValueWidget(self.setting)
        self.value_vp = gtk.Viewport()
        self.value_vp.set_size_request(self.setting.min_width, -1)
        self.value_vp.connect("scroll_event", self.set_chaned_hadj, hadj, self.setting.qnxsz)
        self.value_vp.set_shadow_type(gtk.SHADOW_NONE)
        self.value_vp.set_hadjustment(hadj)
        # self.value_vp.set_vadjustment(vadj)
        self.value_vp.add(self.value_wgt)
        evbox_value = gtk.EventBox()
        evbox_value.add(self.value_vp)
        # evbox_value.connect("leave-notify-event", self.handle_leave_notify, self.hbar, vbar)

        self.grid.value_wgt = self.value_wgt
        self.value_wgt.grid = self.grid

        zx_adj = gtk.Adjustment(15.0, 0.0, 32.0, 1.0, 1.0, 0.0)
        zx_adj.connect("value_changed", self.handle_zoom_x)
        zoom_x = gtk.HScale(zx_adj)
        zoom_x.set_draw_value(False)
        zoom_x.set_update_policy(gtk.UPDATE_DISCONTINUOUS)

        table2 = gtk.Table(3, 2)
        table2.attach(zoom_x, 0, 1, 1, 2, gtk.FILL, 0)
        table2.attach(hsb,    1, 2, 1, 2, gtk.FILL, 0)
        table2.attach(valuetype_cbbox, 0, 1, 0, 1, gtk.FILL, gtk.EXPAND|gtk.FILL)
        table2.attach(evbox_value, 1, 2, 0, 1, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)

        paned_tables = gtk.VPaned()
        paned_tables.pack1(table, resize=False, shrink=False)
        paned_tables.pack2(table2, resize=False, shrink=False)

        chan_list = track_info[2]
        grid_setting_frame = gtk.Frame("Grid setting")
        grid_setting_frame.add(GridSettingTable(self, chan_list))

        setting_box = gtk.HBox()
        setting_box.pack_start(grid_setting_frame, expand=False)

        self.pack_start(setting_box, expand=False)
        self.pack_start(paned_tables)

        debug_hbox = gtk.HBox()

        button_misc = gtk.Button("Redraw")
        button_misc.connect("clicked", self.debug_grid1, self.setting.track)
        debug_hbox.pack_start(button_misc)

        button_misc = gtk.Button("Dump track")
        button_misc.connect("clicked", self.debug_grid2, self.setting.track)
        debug_hbox.pack_start(button_misc)

        debug_frame = gtk.Frame("Note grid debug")
        debug_frame.add(debug_hbox)
        self.pack_end(debug_frame, expand=False)

        self.resize_all()


class TrackEditor(gtk.Window):
    def set_name(self, name):
        if name:
            self.set_title("Track %s" % name)
            self.track.set_name(name)

    def update_pos(self, tickpos):
        self.chaned.grid.update_pos(tickpos)

    def clear_progress(self):
        self.chaned.grid.clear_progress()

    def __init__(self, track, sequencer, portlist=None):
        gtk.Window.__init__(self)
        self.track = track

        self.chaned = ChannelEditor(self.track, sequencer)

        self.set_title("Track %s" % self.track.get_name())
        def hide_tracked(win, event):
            win.hide()
            return True
        self.connect('delete-event', hide_tracked)

        track_setting_frame = gtk.Frame("Track setting")
        track_setting_frame.add(TrackSettingTable(self.chaned, portlist))

        hbox = gtk.HBox()
        hbox.pack_start(track_setting_frame, expand=False)

        vbox = gtk.VBox()
        vbox.pack_start(hbox, expand=False)
        vbox.pack_end(self.chaned)
        self.vbox = vbox
        self.add(vbox)

        self.set_focus_child(self.chaned.grid)
        # self.set_focus_chain((xxx, yyy, zzz))
        # self.set_default(self.chaned.grid)
        # self.chaned.grid.grab_default()
