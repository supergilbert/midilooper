#!/usr/bin/python

#import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

from msqwidget import MsqHBarTimeWidget, MsqVBarNoteWidget, MsqNoteGridWidget, DEFAULT_PPQXSZ, DEFAULT_FONT_NAME#, DEFAULT_NOTEON_VAL

class LengthSettingHBox(gtk.HBox):
    def numerator_cb(self, combobox):
        val = combobox.get_model()[combobox.get_active()][0]
        self.length_numerator = int(val)
        self.cb_func(self.length_numerator,
                     self.length_denominator)

    def denominator_cb(self, combobox):
        val = combobox.get_model()[combobox.get_active()][0]
        self.length_denominator = int(val)
        self.cb_func(self.length_numerator,
                     self.length_denominator)

    def __init__(self, cb_func, *cb_args):
        gtk.HBox.__init__(self)
        self.length_numerator = 1
        self.length_denominator = 4
        self.cb_func = cb_func
        self.cb_args = cb_args

        num_cbbox = gtk.combo_box_new_text()
        for numerator in [1, 2, 3, 4, 6, 8, 12, 16]:
            num_cbbox.append_text(str(numerator))
        num_cbbox.set_active(0)
        num_cbbox.connect("changed", self.numerator_cb)

        slash_str = gtk.Label("/")

        den_cbbox = gtk.combo_box_new_text()
        for denominator in [1, 2, 4, 8, 16, 32, 3, 6, 12, 24]:
            den_cbbox.append_text(str(denominator))
        den_cbbox.set_active(2)
        den_cbbox.connect("changed", self.denominator_cb)

        self.pack_start(num_cbbox)
        self.pack_start(slash_str)
        self.pack_start(den_cbbox)


class TrackSettingTable(gtk.Table):
    def port_changed(self, combobox):
        portlist = combobox.get_model()
        port_idx = combobox.get_active()
        if port_idx >= 0:
            port = portlist[port_idx][0]
            self.track.set_port(port)

    def set_track_len(self, widget):
        self.tedit.set_len(int(widget.get_value()))

    def __init__(self, tedit, ppq, portlist):
        gtk.Table.__init__(self, 4, 1)
        self.track = tedit.track
        self.tedit = tedit
        self.ppq = ppq

        label = gtk.Label(" Track length: ")
        spinadj = gtk.Adjustment(self.track.get_len() / self.ppq, 1, 240, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinadj.connect("value-changed", self.set_track_len)
        self.attach(label, 0, 1, 0, 1)
        self.attach(spinbut, 1, 2, 0, 1)

        label = gtk.Label("  Output Port: ")
        portlist_cbbox = gtk.ComboBox(portlist)
        cell = gtk.CellRendererText()
        portlist_cbbox.pack_start(cell, True)
        portlist_cbbox.add_attribute(cell, 'text', 1)
        portlist_cbbox.connect("changed", self.port_changed)
        for idx, model in enumerate(portlist):
            if self.track.has_port(model[0]):
                portlist_cbbox.set_active(idx)
                break
        self.attach(label, 2, 3, 0, 1)
        self.attach(portlist_cbbox, 3, 4, 0, 1)




class NoteSettingTable(gtk.Table):

    def set_note_res(self, numerator, denominator):
        self.chaned.grid.note_resolution = self.chaned.grid.ppq * numerator / denominator
        self.chaned.redraw_grid_vp()

    def set_note_value_cb(self, widget):
        self.chaned.grid.note_val_on = int(widget.get_value())

    def chan_changed(self, widget):
        chan_num = widget.get_model()[widget.get_active()][0]
        self.chaned.grid.chan_num = chan_num
        self.chaned.redraw_grid_vp()

    def __init__(self, chaned, chan_list):
        self.chaned = chaned
        gtk.Table.__init__(self, 8, 1)

        label = gtk.Label("   Resolution: ")
        note_pos_box = LengthSettingHBox(self.set_note_res, chaned.grid)
        self.attach(label, 2, 3, 0, 1)
        self.attach(note_pos_box, 3, 4, 0, 1)

        label = gtk.Label("   Value: ")
        spinadj = gtk.Adjustment(chaned.grid.note_val_on, 0, 127, 1)
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
        chan_cbbox.set_active(chaned.grid.chan_num)
        chan_cbbox.connect("changed", self.chan_changed)
        self.attach(label, 6, 7, 0, 1)
        self.attach(chan_cbbox, 7, 8, 0, 1)


class ChannelEditor(gtk.VBox):
    def set_len(self, track_len):
        self.hbar.set_len(track_len)
        self.grid.resize_grid()

    def set_chaned_adj(self, widget, event, hadj, vadj, ppqxsz, noteysz):
        if (event.state & gtk.gdk.CONTROL_MASK or
            event.state & gtk.gdk.MOD1_MASK or
            event.state & gtk.gdk.MOD3_MASK or
            event.state & gtk.gdk.MOD4_MASK or
            event.state & gtk.gdk.MOD5_MASK):
           return
        value = vadj.get_value()
        xinc = ppqxsz * 5
        yinc = noteysz * 5
        if event.direction == gdk.SCROLL_DOWN:
            if event.state & gtk.gdk.SHIFT_MASK:
                new_val = value + xinc
                hupper = hadj.upper - hadj.page_size
                hadj.set_value(new_val if new_val <= hupper else hupper)
            else:
                new_val = value + yinc
                vupper = vadj.upper - vadj.page_size
                vadj.set_value(new_val if new_val <= vupper else vupper)

        elif event.direction == gdk.SCROLL_UP:
            if event.state & gtk.gdk.SHIFT_MASK:
                new_val = value - xinc
                hadj.set_value(new_val if new_val >= hadj.lower else hadj.lower)
            else:
                new_val = value - yinc
                vadj.set_value(new_val if new_val >= vadj.lower else vadj.lower)

        elif event.direction == gdk.SCROLL_RIGHT:
            new_val = value + xinc
            hupper = hadj.upper - hadj.page_size
            hadj.set_value(new_val if new_val <= hupper else hupper)

        elif event.direction == gdk.SCROLL_LEFT:
            new_val = value - xinc
            hadj.set_value(new_val if new_val >= hadj.lower else hadj.lower)

    def redraw_grid_vp(self):
        hadj = self.grid_vp.get_hadjustment()
        vadj = self.grid_vp.get_vadjustment()
        area = gtk.gdk.Rectangle(int(hadj.get_value()),
                                 int(vadj.get_value()),
                                 int(hadj.get_page_size()),
                                 int(vadj.get_page_size()))
        self.grid.draw_area(area)

    def redraw_hbar_vp(self):
        hadj = self.hbar_vp.get_hadjustment()
        vadj = self.hbar_vp.get_vadjustment()
        area = gtk.gdk.Rectangle(int(hadj.get_value()),
                                 0,
                                 int(hadj.get_page_size()),
                                 self.hbar.height)
        self.hbar.draw_area(area)

    def debug_grid1(self, button, track):
        self.redraw_grid_vp()

    def debug_grid2(self, button, track):
        track._dump()

    def handle_motion(self, widget, event, hbar, vbar):
        # tick = self.grid.xpos2tick(event.x)
        note = self.grid.ypos2noteval(int(event.y))
        # hbar.show_tick(tick)
        vbar.show_note(note)

    def handle_leave_notify(self, widget, event, hbar, vbar):
        vbar.clear_note()

    def handle_zoom_x(self, adj):
        self.grid.ppqxsz = int(self.grid.ppqxsz_seed * adj.get_value() / 8)
        self.hbar.ppqxsz = int(self.hbar.ppqxsz_seed * adj.get_value() / 8)
        self.grid.resize_grid()
        self.hbar.resize_hbar()
        self.redraw_grid_vp()
        self.redraw_hbar_vp()

    def __init__(self, tracked, chan_list):
        gtk.VBox.__init__(self)
        self.tracked = tracked

        track_len = self.tracked.track.get_len() / self.tracked.sequencer.getppq()

        self.hbar = MsqHBarTimeWidget(track_len, ppqxsz=self.tracked.ppqxsz)
        ppqxsz = self.hbar.ppqxsz
        self.hbar_vp = gtk.Viewport()
        self.hbar_vp.set_size_request(self.tracked.min_width, -1)
        self.hbar_vp.add(self.hbar)
        self.hbar_vp.set_shadow_type(gtk.SHADOW_NONE)
        hadj = self.hbar_vp.get_hadjustment()
        self.hbar_vp.connect("scroll_event", self.set_chaned_adj, hadj, hadj, ppqxsz, ppqxsz)

        vbar = MsqVBarNoteWidget(self.tracked)
        noteysz = vbar.noteysz
        vbar_vp = gtk.Viewport()
        vbar_vp.set_size_request(-1, self.tracked.min_height)
        vbar_vp.add(vbar)
        vbar_vp.set_shadow_type(gtk.SHADOW_NONE)
        vadj = vbar_vp.get_vadjustment()
        vbar_vp.connect("scroll_event", self.set_chaned_adj, hadj, vadj, ppqxsz, noteysz)

        self.grid = MsqNoteGridWidget(chan_list[0] if len(chan_list) > 0 else 0,
                                      self.tracked.track,
                                      ppq=self.tracked.sequencer.getppq(),
                                      ppqxsz=ppqxsz,
                                      noteysz=noteysz)

        self.grid.connect("motion_notify_event", self.handle_motion, self.hbar, vbar)
        self.grid.vadj = vadj
        self.grid_vp = gtk.Viewport()
        self.grid_vp.set_size_request(self.tracked.min_width, self.tracked.min_height)
        self.grid_vp.add(self.grid)
        self.grid_vp.connect("scroll_event", self.set_chaned_adj, hadj, vadj, ppqxsz, noteysz)
        self.grid_vp.set_shadow_type(gtk.SHADOW_NONE)
        self.grid_vp.set_hadjustment(hadj)
        self.grid_vp.set_vadjustment(vadj)
        evbox_grid = gtk.EventBox()
        evbox_grid.add(self.grid_vp)
        evbox_grid.connect("leave-notify-event", self.handle_leave_notify, self.hbar, vbar)

        vbar_vp.connect("button_press_event", vbar.handle_button_press, self.grid)
        vbar_vp.connect("button_release_event", vbar.handle_button_release, self.grid)
        vbar_vp.connect("motion_notify_event", vbar.handle_motion, self.grid)

        hsb = gtk.HScrollbar(hadj)
        vsb = gtk.VScrollbar(vadj)

        zx_adj = gtk.Adjustment(8.0, 1.0, 32.0, 1.0, 1.0, 0.0)
        zx_adj.connect("value_changed", self.handle_zoom_x)
        zoom_x = gtk.HScale(zx_adj)
        zoom_x.set_draw_value(False)
        zoom_x.set_update_policy(gtk.UPDATE_DISCONTINUOUS)

        table = gtk.Table(3, 3)
        table.attach(self.hbar_vp, 1, 2, 0, 1, gtk.FILL, 0)
        table.attach(vbar_vp, 0, 1, 1, 2, 0, gtk.FILL)
        table.attach(evbox_grid, 1, 2, 1, 2, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
        table.attach(vsb, 2, 3, 1, 2, 0, gtk.FILL)
        table.attach(hsb, 1, 2, 2, 3, gtk.FILL, 0)
        table.attach(zoom_x, 0, 1, 2, 3, gtk.FILL, 0)

        note_setting_frame = gtk.Frame("Note setting")
        note_setting_frame.add(NoteSettingTable(self, chan_list))

        setting_hbox = gtk.HBox()
        setting_hbox.pack_start(note_setting_frame, expand=False)

        self.pack_start(setting_hbox, expand=False)
        self.pack_start(table)

        debug_hbox = gtk.HBox()

        button_misc = gtk.Button("Refresh widget")
        button_misc.connect("clicked", self.debug_grid1, self.tracked.track)
        debug_hbox.pack_start(button_misc)

        button_misc = gtk.Button("Dump track")
        button_misc.connect("clicked", self.debug_grid2, self.tracked.track)
        debug_hbox.pack_start(button_misc)

        debug_frame = gtk.Frame("Note grid debug")
        debug_frame.add(debug_hbox)
        self.pack_end(debug_frame, expand=False)

        self.set_focus_child(table)


def get_track_info(track):
    track_min = 0
    track_max = 0
    channel_list = []
    for evwr in track:
        event = evwr.get_event()
        if event[0] < track_min:
            track_min = event[0]
        if event[0] < track_max:
            track_max = event[0]
        if not (event[1] in channel_list):
            channel_list.append(event[1])
    channel_list.sort()
    return (track_min, track_max, channel_list)

class TrackEditor(gtk.Window):
    def set_len(self, track_len):
        self.track.set_len(track_len * self.sequencer.getppq())
        self.chaned.set_len(track_len)

    def set_name(self, name):
        if name:
            self.set_title("Track %s" % name)
            self.track.set_name(name)

    def update_pos(self, tickpos):
        self.chaned.grid.update_pos(tickpos)

    def clear_progress(self):
        self.chaned.grid.clear_progress()

    def __init__(self, track, sequencer, portlist=None, ppqxsz=DEFAULT_PPQXSZ, font_name=DEFAULT_FONT_NAME):
        gtk.Window.__init__(self)

        self.sequencer = sequencer
        self.track = track

        track_min, track_max, channel_list = get_track_info(track)

        self.min_width = 320
        self.min_height = 240

        self.ppqxsz = ppqxsz
        self.font_name = font_name

        self.chaned = ChannelEditor(self, channel_list)

        self.set_title("Track %s" % self.track.get_name())
        def hide_tracked(win, event):
            win.hide()
            return True
        self.connect('delete-event', hide_tracked)

        track_setting_frame = gtk.Frame("Track setting")
        track_setting_frame.add(TrackSettingTable(self, self.sequencer.getppq(), portlist))

        hbox = gtk.HBox()
        hbox.pack_start(track_setting_frame, expand=False)

        vbox = gtk.VBox()
        vbox.pack_start(hbox, expand=False)
        vbox.pack_end(self.chaned)
        self.vbox = vbox
        self.add(vbox)

        self.set_default(self.chaned.grid)
