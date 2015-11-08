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
import sys

import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required"
    raise SystemExit

from msqwidget import MsqHBarTimeWidget, MsqVBarNoteWidget, MsqNoteGridWidget, MIN_QNOTE_XSZ, DEFAULT_QNOTE_XSZ, MAX_QNOTE_XSZ
from msqwidget.midivaluewgt import MsqValueWidget
from msqwidget.wgttools import MIDI_CTRL_EVENT, MIDI_NOTEOFF_EVENT, MIDI_NOTEON_EVENT, MIDI_PITCH_EVENT
from tool import prompt_get_loop, prompt_get_output


class TrackSettingTable(gtk.Table):

    def set_loop(self, loop_start, loop_len):
        self.chaned.setting.track.set_start(loop_start * self.chaned.setting.getppq())
        self.chaned.setting.track.set_len(loop_len * self.chaned.setting.getppq())
        self.chaned.resize_all()
        self.loop_label.set_text(self.loop_str % (self.chaned.setting.getstart() / self.chaned.setting.getppq(),
                                                  self.chaned.setting.getlen()   / self.chaned.setting.getppq()))

    def button_set_loop(self, button):
        loop_pos = prompt_get_loop(self.chaned.setting.getstart() / self.chaned.setting.getppq(),
                                   self.chaned.setting.getlen()   / self.chaned.setting.getppq())
        if loop_pos:
            self.set_loop(loop_pos[0], loop_pos[1])

    def set_output(self, output):
        self.chaned.setting.track.set_output(output)
        output_port = None
        for idx, model in enumerate(self.portlist):
            if self.chaned.setting.track.has_output(model[0]):
                output_port = model[1]
                break
        if output_port:
            self.output_label.set_text(self.output_str % output_port)
        else:
            self.output_label.set_text(self.output_str % "None")

    def button_set_output(self, button):
        port_idx = 0
        for idx, model in enumerate(self.portlist):
            if self.chaned.setting.track.has_output(model[0]):
                port_idx = idx
                break;
        output_res = prompt_get_output(self.portlist, port_idx)
        if output_res[0]:
            self.set_output(output_res[1])

    def __init__(self, chaned, portlist):
        gtk.Table.__init__(self, 4, 1)
        self.chaned = chaned
        self.loop_str = "Loop start:%d length:%d "
        self.output_str = "  Output Port: %s "
        self.portlist = portlist
        self.loop_label = gtk.Label(self.loop_str % (self.chaned.setting.getstart() / self.chaned.setting.getppq(),
                                                     self.chaned.setting.getlen()   / self.chaned.setting.getppq()))
        self.attach(self.loop_label, 0, 1, 0, 1)

        button = gtk.Button("Configure loop")
        button.connect("clicked", self.button_set_loop)

        self.attach(button, 1, 2, 0, 1)

        self.output_label = gtk.Label()
        output_port = None
        for idx, model in enumerate(self.portlist):
            if self.chaned.setting.track.has_output(model[0]):
                output_port = model[1]
                break
        if output_port:
            self.output_label.set_text(self.output_str % output_port)
        else:
            self.output_label.set_text(self.output_str % "None")

        button = gtk.Button("Configure output")
        button.connect("clicked", self.button_set_output)

        self.attach(self.output_label, 2, 3, 0, 1)
        self.attach(button, 3, 4, 0, 1)

def update_value_list(value_list, track_info, chan_num):
    chan_key = "%i" % chan_num

    viter = value_list.get_iter("0")
    if track_info['note_chan'].has_key(chan_key):
        value_list.set(viter, 1, "Note on *")
    else:
        value_list.set(viter, 1, "Note on")

    viter = value_list.get_iter("1")
    if track_info['pitch_chan'].has_key(chan_key):
        value_list.set(viter, 1, "Pitch Bend *")
    else:
        value_list.set(viter, 1, "Pitch Bend")

    if track_info['ctrl_chan'].has_key(chan_key):
        ctrl_list = track_info['ctrl_chan'][chan_key]
        viter = value_list.get_iter("2")
        while viter:
            ctrl_num = value_list.get_value(viter, 0)
            if ctrl_num in ctrl_list:
                value_list.set(viter, 1, "Ctrl %i *" % ctrl_num)
            else:
                value_list.set(viter, 1, "Ctrl %i" % ctrl_num)
            viter = value_list.iter_next(viter)
    else:
        while viter:
            ctrl_num = value_list.get_value(viter, 0)
            value_list.set(viter, 1, "Ctrl %i" % ctrl_num)
            viter = value_list.iter_next(viter)


def get_track_info(track):
    track_min = sys.maxint
    track_max = 0
    channel_list = []
    channel_ctrl  = {}
    channel_note  = {}
    channel_pitch = {}
    ev_len = 0

    for event in track.getall_event():
        ev_len += 1
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
        elif event[2] == MIDI_NOTEOFF_EVENT or event[2] == MIDI_NOTEON_EVENT:
            channel_note[chan_key] = True
        elif event[2] == MIDI_PITCH_EVENT:
            channel_pitch[chan_key] = True

    channel_list.sort()
    for key in channel_ctrl.keys():
        channel_ctrl[key].sort()

    return {'min': track_min,
            'max': track_max,
            'len': ev_len,
            'channels': channel_list,
            'note_chan': channel_note,
            'pitch_chan': channel_pitch,
            'ctrl_chan': channel_ctrl}


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
            if channel in track_info['channels']:
                chan_list.set(citer, 1, "%i *" % channel)
            else:
                chan_list.set(citer, 1, "%i" % channel)
            citer = chan_list.iter_next(citer)

        # Refreshing midivalue viewer combobox
        update_value_list(self.chaned.val_list, track_info, self.chaned.setting.chan_num)

        self.chaned.redraw()

    def res_changed(self, cbbox):
        val_int = cbbox.get_model()[cbbox.get_active()][0]
        self.chaned.grid.setting.tick_res = val_int
        self.chaned.redraw()

    def __init__(self, chaned, chan_list):
        self.chaned = chaned
        gtk.Table.__init__(self, 10, 1)

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

        label = gtk.Label(" Note on vel.: ")
        spinbut = gtk.SpinButton(adjustment=self.chaned.setting.note_valadj, climb_rate=1)
        spinbut.set_numeric(True)
        self.chaned.setting.note_valadj.connect("value-changed", self.set_note_value_cb)
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

        label = gtk.Label("  Default bar value: ")
        spinbut = gtk.SpinButton(climb_rate=1)
        spinbut.set_numeric(True)
        self.chaned.value_wgt.spinbut = spinbut
        self.attach(label, 8, 9, 0, 1)
        self.attach(spinbut, 9, 10, 0, 1)
        self.chaned.value_wgt.set_note_mode()


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

        # Refreshing midivalue type list
        track_info = get_track_info(self.setting.track)
        update_value_list(value_list, track_info, self.setting.chan_num)

        # Updating "Value widget" mode and window
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
        self.setting.chan_num = track_info['channels'][0] if len(track_info['channels']) else 0

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
        self.grid_vp.get_settings().set_long_property("gtk-tooltip-timeout", 3000, "midilooper:gridvp")
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
        if track_info['ctrl_chan'].has_key(chan_key):
            ctrl_list = track_info['ctrl_chan'][chan_key]
        else:
            ctrl_list = []

        if track_info['note_chan'].has_key(chan_key):
            self.val_list.append([9, "Note on *"])
        else:
            self.val_list.append([9, "Note on"])
        if track_info['pitch_chan'].has_key(chan_key):
            self.val_list.append([14, "Pitch Bend *"])
        else:
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
        # valuetype_cbbox.set_size_request(vbar.width, -1)

        value_adjwgt = gtk.VScale()
        value_adjwgt.set_inverted(True)
        value_adjwgt.set_draw_value(False)

        value_box = gtk.HBox()
        value_box.pack_start(valuetype_cbbox)
        value_box.pack_start(value_adjwgt, expand=False, fill=False)

        value_box.set_size_request(vbar.width, -1)

        self.value_wgt = MsqValueWidget(self.setting, value_adjwgt)
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

        zx_adj = gtk.Adjustment(15.0, 1.0, 25.0, 1.0)
        zx_adj.connect("value_changed", self.handle_zoom_x)
        zoom_x = gtk.HScale(zx_adj)
        zoom_x.set_draw_value(False)
        zoom_x.set_update_policy(gtk.UPDATE_DISCONTINUOUS)

        table2 = gtk.Table(2, 2)
        table2.attach(value_box,   0, 1, 0, 1, gtk.FILL,            gtk.EXPAND|gtk.FILL)
        table2.attach(evbox_value, 1, 2, 0, 1, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
        table2.attach(zoom_x,      0, 1, 1, 2, gtk.FILL,            0)
        table2.attach(hsb,         1, 2, 1, 2, gtk.FILL,            0)

        paned_tables = gtk.VPaned()
        paned_tables.pack1(table, resize=False, shrink=False)
        paned_tables.pack2(table2, resize=False, shrink=False)

        grid_setting_frame = gtk.Frame("Grid setting")
        grid_setting_frame.add(GridSettingTable(self, track_info['channels']))

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

        self.track_setting = TrackSettingTable(self.chaned, portlist)
        track_setting_frame = gtk.Frame("Track setting")
        track_setting_frame.add(self.track_setting)

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
