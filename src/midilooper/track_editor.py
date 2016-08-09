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
from collections import namedtuple


class TrackSettingTable(gtk.HBox):

    def set_loop(self, loop_start, loop_len):
        ppq = self.chaned.setting.getppq()
        self.chaned.setting.track.set_start(loop_start * ppq)
        self.chaned.setting.track.set_len(loop_len * ppq)
        self.loop_button.set_label(self.loop_str % (self.chaned.setting.getstart() / ppq,
                                                    self.chaned.setting.getlen()   / ppq))
        upper_pos = (loop_start + loop_len + 1) * self.chaned.setting.qnxsz
        self.chaned.setting.hadj.set_upper(upper_pos)
        self.chaned.draw_all()
        self.tracklist.update_all_info()

    def loop_button_cb(self, button):
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
            self.output_button.set_label(self.output_str % output_port)
        else:
            self.output_button.set_label(self.output_str % "None")
        self.tracklist.update_all_info()

    def output_button_cb(self, button):
        port_idx = 0
        for idx, model in enumerate(self.portlist):
            if self.chaned.setting.track.has_output(model[0]):
                port_idx = idx
                break;
        output_res = prompt_get_output(self.portlist, port_idx)
        if output_res[0]:
            self.set_output(output_res[1])

    def toggle_rec(self, button):
        self.tracklist.set_trackrec(self.chaned.setting.track,
                                    button.get_active())

    def rec_button_set_active(self, active=True):
        self.rec_button.handler_block(self.rec_button_hdlid)
        self.rec_button.set_active(active)
        self.rec_button.handler_unblock(self.rec_button_hdlid)

    def __init__(self, chaned, tracklist):
        gtk.HBox.__init__(self)
        self.set_border_width(10)
        self.set_spacing(5)
        self.chaned = chaned
        self.loop_str = "Loop start:%d length:%d "
        self.output_str = "  Output Port: %s "
        self.portlist = tracklist.portlist
        self.tracklist = tracklist

        self.loop_button = gtk.Button(self.loop_str % (self.chaned.setting.getstart() / self.chaned.setting.getppq(),
                                                       self.chaned.setting.getlen()   / self.chaned.setting.getppq()))
        self.loop_button.connect("clicked", self.loop_button_cb)
        self.loop_button.set_tooltip_text("Press to configure")
        self.pack_start(self.loop_button)

        self.output_button = gtk.Button()
        self.output_button.connect("clicked", self.output_button_cb)
        self.output_button.set_tooltip_text("Press to configure")
        output_port = None
        for idx, model in enumerate(self.portlist):
            if self.chaned.setting.track.has_output(model[0]):
                output_port = model[1]
                break
        if output_port:
            self.output_button.set_label(self.output_str % output_port)
        else:
            self.output_button.set_label(self.output_str % "None")
        self.chaned.setting.setting_table = self
        self.pack_start(self.output_button)

        self.rec_button = gtk.ToggleButton()
        self.rec_button.set_image(gtk.image_new_from_stock(gtk.STOCK_MEDIA_RECORD,  gtk.ICON_SIZE_BUTTON))
        self.rec_button_hdlid = self.rec_button.connect("toggled", self.toggle_rec)
        self.pack_start(self.rec_button)

def update_value_list(value_list, track_info, chan_num):
    chan_key = "%i" % chan_num

    viter = value_list.get_iter("0")
    if track_info.note_chan.has_key(chan_key):
        value_list.set(viter, 1, "Note on *")
    else:
        value_list.set(viter, 1, "Note on")

    viter = value_list.get_iter("1")
    if track_info.pitch_chan.has_key(chan_key):
        value_list.set(viter, 1, "Pitch Bend *")
    else:
        value_list.set(viter, 1, "Pitch Bend")

    if track_info.ctrl_chan.has_key(chan_key):
        ctrl_list = track_info.ctrl_chan[chan_key]
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

    trackinfo = namedtuple('MidiTrackInfo',
                           ['tick_min',
                            'tick_max',
                            'tick_len',
                            'channels',
                            'note_chan',
                            'pitch_chan',
                            'ctrl_chan'],
                           verbose=False)

    return trackinfo(tick_min=track_min,
                     tick_max=track_max,
                     tick_len=ev_len,
                     channels=channel_list,
                     note_chan=channel_note,
                     pitch_chan=channel_pitch,
                     ctrl_chan=channel_ctrl)


class GridSettingTable(gtk.HBox):
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
            if channel in track_info.channels:
                chan_list.set(citer, 1, "%i *" % channel)
            else:
                chan_list.set(citer, 1, "%i" % channel)
            citer = chan_list.iter_next(citer)

        # Refreshing midivalue viewer combobox
        update_value_list(self.chaned.val_list, track_info, self.chaned.setting.chan_num)

        self.chaned.draw_all()

    def res_changed(self, cbbox):
        val_int = cbbox.get_model()[cbbox.get_active()][0]
        self.chaned.grid.setting.tick_res = val_int
        self.chaned.draw_all()

    def scale_changed(self, cbbox):
        val_int = cbbox.get_model()[cbbox.get_active()][0]
        self.chaned.grid.set_scale(val_int)
        self.chaned.grid.draw_all()

    def __init__(self, chaned, chan_list):
        gtk.HBox.__init__(self)
        self.set_border_width(10)
        self.chaned = chaned

        label = gtk.Label("Scale: ")
        scale_list = gtk.ListStore(int, str)
        scale_str_list = ("C",
                          "C#",
                          "D",
                          "D#",
                          "E",
                          "F",
                          "F#",
                          "G",
                          "G#",
                          "A",
                          "A#",
                          "B")
        for idx, note in enumerate(scale_str_list):
            scale_list.append((idx, note))
        combo_scale = gtk.ComboBox(scale_list)
        combo_scale.set_active(0)
        cell = gtk.CellRendererText()
        combo_scale.pack_start(cell, True)
        combo_scale.add_attribute(cell, 'text', 1)
        combo_scale.connect("changed", self.scale_changed)
        self.pack_start(label)
        self.pack_start(combo_scale)

        label = gtk.Label(" Resolution: ")
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
        self.pack_start(label)
        self.pack_start(combo_res)

        label = gtk.Label(" Note on vel.: ")
        spinbut = gtk.SpinButton(adjustment=self.chaned.setting.note_valadj, climb_rate=1)
        spinbut.set_numeric(True)
        self.chaned.setting.note_valadj.connect("value-changed", self.set_note_value_cb)
        self.pack_start(label)
        self.pack_start(spinbut)

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
        self.pack_start(label)
        self.pack_start(chan_cbbox)

        label = gtk.Label("  Default bar value: ")
        spinbut = gtk.SpinButton(climb_rate=1)
        spinbut.set_numeric(True)
        self.chaned.value_wgt.spinbut = spinbut
        self.pack_start(label)
        self.pack_start(spinbut)
        self.chaned.value_wgt.set_note_mode()


def is_mask_to_bypass(evstate):
    if (evstate & gtk.gdk.MOD1_MASK or
        evstate & gtk.gdk.MOD3_MASK or
        evstate & gtk.gdk.MOD4_MASK or
        evstate & gtk.gdk.MOD5_MASK):
           return True

def dec_adj(adj, pad):
    value = adj.get_value() - pad
    valmin = adj.get_lower()
    adj.set_value(value if value >= valmin else valmin)


def inc_adj(adj, pad):
    value = adj.get_value() + pad
    valmax = adj.get_upper() - adj.get_page_size()
    adj.set_value(value if value <= valmax else valmax)


class ChannelEditor(gtk.VBox):
    def vadj_value_cb(self, adj):
        self.vbar.draw_all()
        self.grid.draw_all()

    def vadj_value_cb(self, adj):
        self.hbar.draw_all()
        self.grid.draw_all()
        self.value_wgt.draw_all()

    def scroll_adj_cb(self, widget, event, setting):
        if is_mask_to_bypass(event.state):
            return
        inc_mult = 4
        if event.direction == gdk.SCROLL_DOWN:
            if event.state & gtk.gdk.SHIFT_MASK:
                xinc = setting.qnxsz * inc_mult
                inc_adj(setting.hadj, xinc)
            elif event.state & gtk.gdk.CONTROL_MASK:
                step = self.zx_adj.get_step_increment()
                val = self.zx_adj.get_value() - step
                if val >= self.zx_adj.get_lower():
                    self.zx_adj.set_value(val)
            else:
                yinc = setting.noteysz * inc_mult
                inc_adj(setting.vadj, yinc)
        elif event.direction == gdk.SCROLL_UP:
            if event.state & gtk.gdk.SHIFT_MASK:
                xinc = setting.qnxsz * inc_mult
                dec_adj(setting.hadj, xinc)
            elif event.state & gtk.gdk.CONTROL_MASK:
                step = self.zx_adj.get_step_increment()
                val = self.zx_adj.get_value() + step
                if val <= self.zx_adj.get_upper():
                    self.zx_adj.set_value(val)
            else:
                yinc = setting.noteysz * inc_mult
                dec_adj(setting.vadj, yinc)
        elif event.direction == gdk.SCROLL_RIGHT:
            xinc = setting.qnxsz * inc_mult
            inc_adj(setting.hadj, xinc)

        elif event.direction == gdk.SCROLL_LEFT:
            xinc = setting.qnxsz * inc_mult
            dec_adj(setting.hadj, xinc)

    def draw_all(self):
        self.hbar.draw_all()
        self.grid.draw_all()
        self.value_wgt.draw_all()

    def debug_grid1(self, button, track):
        self.draw_all()

    def debug_grid2(self, button, track):
        track._dump()

    def handle_motion(self, widget, event, hbar, vbar):
        # tick = self.grid.xpos2tick(event.x)
        note = vbar.ypos2note(int(event.y))
        # hbar.show_tick(tick)
        vbar.show_note(note)

    def handle_leave_notify(self, widget, event, hbar, vbar):
        vbar.clear_note()

    def handle_zoom_x(self, adj):
        zval = adj.get_value()
        old_qnxsz = self.setting.qnxsz
        old_pos = self.setting.hadj.get_value()
        if zval <= 16.0:
            self.setting.qnxsz = int(MIN_QNOTE_XSZ + ((DEFAULT_QNOTE_XSZ - MIN_QNOTE_XSZ) * zval / 16.0))
        else:
            self.setting.qnxsz = int(DEFAULT_QNOTE_XSZ + ((MAX_QNOTE_XSZ - DEFAULT_QNOTE_XSZ) * (zval - 16) / 16.0))
        self.setting.hadj.set_upper(self.setting.getmaxwidth())
        pos = old_pos * self.setting.qnxsz / old_qnxsz
        self.setting.hadj.set_value(pos)
        self.draw_all()

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
            self.value_wgt.draw_all()

    def __init__(self, track, sequencer):
        gtk.VBox.__init__(self)

        self.grid = MsqNoteGridWidget(track, sequencer)
        self.setting = self.grid.setting

        track_info = get_track_info(track)
        self.setting.chan_num = track_info.channels[0] if len(track_info.channels) else 0

        self.hbar = MsqHBarTimeWidget(self.grid)

        self.vbar = MsqVBarNoteWidget(self.setting)
        self.setting.vadj.connect("value-changed", self.vadj_value_cb)

        self.grid.connect("motion_notify_event", self.handle_motion, self.hbar, self.vbar)
        self.grid.set_tooltip_text("""\
* Left button to select notes
* Right button to enter in edit mode
  then in edit mode press left button to write notes
  (do not release the button to increase size)
* Middle button to change note size
* Suppr to delete selected notes
* Ctrl-a Select all notes
* Ctrl-c Copy selected
* Ctrl-v Paste copied notes""")
        self.grid.get_settings().set_long_property("gtk-tooltip-timeout", 3000, "midilooper:gridvp")
        self.grid.connect("leave-notify-event", self.handle_leave_notify, self.hbar, self.vbar)

        hsb = gtk.HScrollbar(self.setting.hadj)
        vsb = gtk.VScrollbar(self.setting.vadj)

        table = gtk.Table(3, 2)
        table.attach(self.hbar, 1, 2, 0, 1, gtk.FILL,            0)
        table.attach(self.vbar, 0, 1, 1, 2, 0,                   gtk.FILL)
        table.attach(self.grid, 1, 2, 1, 2, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
        table.attach(vsb,       2, 3, 1, 2, 0,                   gtk.FILL)

        self.val_list = gtk.ListStore(int, str)
        chan_key = "%i" % self.setting.chan_num
        if track_info.ctrl_chan.has_key(chan_key):
            ctrl_list = track_info.ctrl_chan[chan_key]
        else:
            ctrl_list = []

        if track_info.note_chan.has_key(chan_key):
            self.val_list.append([9, "Note on *"])
        else:
            self.val_list.append([9, "Note on"])
        if track_info.pitch_chan.has_key(chan_key):
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

        value_adjwgt = gtk.VScale()
        value_adjwgt.set_inverted(True)
        value_adjwgt.set_draw_value(False)

        value_box = gtk.HBox()
        value_box.pack_start(valuetype_cbbox)
        value_box.pack_start(value_adjwgt, expand=False, fill=False)

        value_box.set_size_request(self.vbar.width, -1)

        self.value_wgt = MsqValueWidget(self.setting, value_adjwgt)
        self.value_wgt.connect("leave-notify-event", self.handle_leave_notify, self.hbar, self.vbar)

        self.grid.value_wgt = self.value_wgt
        self.value_wgt.grid = self.grid

        self.zx_adj = gtk.Adjustment(15.0, 1.0, 25.0, 1.0)
        self.zx_adj.connect("value_changed", self.handle_zoom_x)
        zoom_x = gtk.HScale(self.zx_adj)
        zoom_x.set_draw_value(False)
        zoom_x.set_update_policy(gtk.UPDATE_DISCONTINUOUS)

        table2 = gtk.Table(2, 2)
        table2.attach(value_box,      0, 1, 0, 1, gtk.FILL,            gtk.EXPAND|gtk.FILL)
        table2.attach(self.value_wgt, 1, 2, 0, 1, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
        table2.attach(zoom_x,         0, 1, 1, 2, gtk.FILL,            0)
        table2.attach(hsb,            1, 2, 1, 2, gtk.FILL,            0)

        self.hbar.connect("scroll-event", self.scroll_adj_cb, self.setting)
        self.vbar.connect("scroll-event", self.scroll_adj_cb, self.setting)
        self.grid.connect("scroll-event", self.scroll_adj_cb, self.setting)

        paned_tables = gtk.VPaned()
        paned_tables.pack1(table, resize=False, shrink=False)
        paned_tables.pack2(table2, resize=False, shrink=False)

        grid_setting_frame = gtk.Frame("Grid setting")
        grid_setting_frame.add(GridSettingTable(self, track_info.channels))

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

        self.draw_all()


class TrackEditor(gtk.Window):
    def set_name(self, name):
        if name:
            self.set_title("Track %s" % name)
            self.track.set_name(name)

    def update_pos(self, tickpos):
        self.chaned.grid.update_pos(tickpos)

    def clear_progress(self):
        self.chaned.grid.clear_progress()

    def __init__(self, track, tracklist):
        gtk.Window.__init__(self)
        self.track = track

        self.chaned = ChannelEditor(self.track, tracklist.seq)

        self.set_title("Track %s" % self.track.get_name())
        def hide_tracked(win, event):
            win.hide()
            return True
        self.connect('delete-event', hide_tracked)

        self.track_setting = TrackSettingTable(self.chaned, tracklist)
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
