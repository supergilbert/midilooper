#!/usr/bin/python

#import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

from widget import MsqHBarTimeWidget, MsqVBarNoteWidget, MsqNoteGridWidget, DEFAULT_XPADSZ, DEFAULT_FONT_NAME#, DEFAULT_NOTEON_VAL

class LengthSettingHBox(gtk.HBox):
    def numerator_cb(self, combobox):
        val = combobox.get_model()[combobox.get_active()][0]
        self.length_numerator = int(val)
        self.cb_func(self.length_numerator,
                     self.length_denominator,
                     *self.cb_args)

    def denominator_cb(self, combobox):
        val = combobox.get_model()[combobox.get_active()][0]
        self.length_denominator = int(val)
        self.cb_func(self.length_numerator,
                     self.length_denominator,
                     *self.cb_args)

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
        # self.track.set_len(int(widget.get_value()) * self.ppq)
        self.tedit.set_len(int(widget.get_value()))

    def __init__(self, tedit, ppq, portlist):
        gtk.Table.__init__(self, 3, 3)
        track = tedit.track
        self.tedit = tedit
        self.ppq = ppq
        label = gtk.Label("Track length")
        spinadj = gtk.Adjustment(track.get_len() / self.ppq, 1, 240, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinadj.connect("value-changed", self.set_track_len)

        self.attach(label, 0, 2, 0, 1)
        self.attach(spinbut, 2, 3, 0, 1)

        label = gtk.Label("output Port")
        combobox = gtk.ComboBox(portlist)
        cell = gtk.CellRendererText()
        combobox.pack_start(cell, True)
        combobox.add_attribute(cell, 'text', 1)
        combobox.connect("changed", self.port_changed)

        self.attach(label, 0, 2, 1, 2)
        self.attach(combobox, 2, 3, 1, 2)


class NoteSettingTable(gtk.Table):
    def set_note_length(self, numerator, denominator, grid):
        grid.note_param["len"] = grid.ppq * numerator / denominator

    def set_note_quant(self, numerator, denominator, grid):
        grid.note_param["quant"] = grid.ppq * numerator / denominator

    def set_note_value_cb(self, widget, grid):
        grid.note_param["on"] = widget.get_value()

    def __init__(self, grid):
        gtk.Table.__init__(self, 3, 3)

        label = gtk.Label("Note lenght")
        note_length_box = LengthSettingHBox(self.set_note_length, grid)

        self.attach(label, 0, 2, 0, 1)
        self.attach(note_length_box, 2, 3, 0, 1)

        label = gtk.Label("Note position")
        note_pos_box = LengthSettingHBox(self.set_note_quant, grid)

        self.attach(label, 0, 2, 1, 2)
        self.attach(note_pos_box, 2, 3, 1, 2)

        label = gtk.Label("Noteon value")
        spinadj = gtk.Adjustment(grid.note_param["val_on"], 0, 127, 1)
        spinbut = gtk.SpinButton(adjustment=spinadj, climb_rate=1)
        spinbut.set_numeric(True)
        spinadj.connect("value-changed", self.set_note_value_cb, grid)
        self.attach(label, 0, 2, 2, 3)
        self.attach(spinbut, 2, 3, 2, 3)


class ChannelEditor(gtk.VBox):
    def set_len(self, track_len):
        self.hbar.set_len(track_len)
        self.grid.resize_grid()

    def set_chaned_adj(self, widget, event, hadj, vadj, xpadsz, ypadsz):
        value = vadj.get_value()
        xinc = xpadsz * 5
        yinc = ypadsz * 5
        if event.direction == gdk.SCROLL_DOWN:
            new_val = value + yinc
            vupper = vadj.upper - vadj.page_size
            vadj.set_value(new_val if new_val <= vupper else vupper)
        elif event.direction == gdk.SCROLL_UP:
            new_val = value - yinc
            vadj.set_value(new_val if new_val >= vadj.lower else vadj.lower)
        elif event.direction == gdk.SCROLL_RIGHT:
            new_val = value + xinc
            hupper = hadj.upper - hadj.page_size
            hadj.set_value(new_val if new_val <= hupper else hupper)
        elif event.direction == gdk.SCROLL_LEFT:
            new_val = value - xinc
            hadj.set_value(new_val if new_val >= hadj.lower else hadj.lower)

    def redraw_grid_vp(self, grid_vp):
        grid = grid_vp.get_child()
        hadj = grid_vp.get_hadjustment()
        vadj = grid_vp.get_vadjustment()
        area = gtk.gdk.Rectangle(int(hadj.get_value()),
                                 int(vadj.get_value()),
                                 int(hadj.get_page_size()),
                                 int(vadj.get_page_size()))
        grid.draw_all(area)

    def reset_track(self, button, grid_vp):
        if self.tracked.sequencer:
            if self.tracked.sequencer.isrunning():
                print "Can't reset track while running"
                return True
        grid = grid_vp.get_child()
        grid.track.clear()
        for chaned in self.tracked.chaned_dict.values():
            chaned.grid.track_events = []
        self.redraw_grid_vp(grid_vp)

    def grid_type_changed(self, combobox, grid_vp):
        val = combobox.get_model()[combobox.get_active()][0]
        grid = grid_vp.get_child()
        if val == "Bar":
            grid.set_draw_bars_mode()
        else:
            grid.set_draw_points_mode()
        self.redraw_grid_vp(grid_vp)

    def debug_grid(self, button, grid_vp, track):
        print "redraw"
        self.redraw_grid_vp(grid_vp)

    def __init__(self, tracked, chan_num):
        gtk.VBox.__init__(self)
        self.tracked = tracked

        track_len = self.tracked.track.get_len() / self.tracked.sequencer.getppq()

        self.hbar = MsqHBarTimeWidget(track_len, xpadsz=self.tracked.xpadsz)
        xpadsz = self.hbar.xpadsz
        hbar_vp = gtk.Viewport()
        hbar_vp.set_size_request(self.tracked.min_width, -1)
        hbar_vp.add(self.hbar)
        hbar_vp.set_shadow_type(gtk.SHADOW_NONE)
        hadj = hbar_vp.get_hadjustment()
        hbar_vp.connect("scroll_event", self.set_chaned_adj, hadj, hadj, xpadsz, xpadsz)
        self.hadj = hadj

        vbar = MsqVBarNoteWidget(font_name=self.tracked.font_name)
        ypadsz = vbar.ypadsz
        vbar_vp = gtk.Viewport()
        vbar_vp.set_size_request(-1, self.tracked.min_height)
        vbar_vp.add(vbar)
        vbar_vp.set_shadow_type(gtk.SHADOW_NONE)
        vadj = vbar_vp.get_vadjustment()
        vbar_vp.connect("scroll_event", self.set_chaned_adj, hadj, vadj, xpadsz, ypadsz)

        self.grid = MsqNoteGridWidget(chan_num,
                                      self.tracked.track,
                                      ppq=self.tracked.sequencer.getppq(),
                                      xpadsz=xpadsz,
                                      ypadsz=ypadsz)
        grid_vp = gtk.Viewport()
        grid_vp.set_size_request(self.tracked.min_width, self.tracked.min_height)
        grid_vp.add(self.grid)
        grid_vp.connect("scroll_event", self.set_chaned_adj, hadj, vadj, xpadsz, ypadsz)
        grid_vp.set_shadow_type(gtk.SHADOW_NONE)
        grid_vp.set_hadjustment(hadj)
        grid_vp.set_vadjustment(vadj)

        hsb = gtk.HScrollbar(hadj)
        vsb = gtk.VScrollbar(vadj)

        def_vertical_val = vadj.get_page_size() / 2
        vadj.set_value(def_vertical_val)

        table = gtk.Table(3, 3)
        table.attach(hbar_vp, 1, 2, 0, 1, gtk.FILL, 0)
        table.attach(vbar_vp, 0, 1, 1, 2, 0, gtk.FILL)
        table.attach(grid_vp, 1, 2, 1, 2, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
        table.attach(vsb, 2, 3, 1, 2, 0, gtk.FILL)
        table.attach(hsb, 1, 2, 2, 3, gtk.FILL, 0)

        # table.set_focus_child(grid_vp) # test

        note_setting_frame = gtk.Frame("Note setting")
        note_setting_frame.add(NoteSettingTable(self.grid))

        #track_setting_frame = gtk.Frame("Track setting")
        #track_setting_frame.add(TrackSettingTable(self.tracked.track, self.tracked.sequencer.getppq(), ))

        setting_hbox = gtk.HBox()
        setting_hbox.pack_start(note_setting_frame, expand=False)
        # setting_hbox.pack_start(track_setting_frame, expand=False)

        self.pack_start(setting_hbox, expand=False)
        self.pack_start(table)

        debug_hbox = gtk.HBox()
        button_clear = gtk.Button("Clear all")
        button_clear.connect("clicked", self.reset_track, grid_vp)
        debug_hbox.pack_start(button_clear)

        button_misc = gtk.Button("Refresh")
        button_misc.connect("clicked", self.debug_grid, grid_vp, self.tracked.track)
        debug_hbox.pack_start(button_misc)

        grid_type_select = gtk.combo_box_new_text()
        grid_type_select.append_text("Bar")
        grid_type_select.append_text("Point")
        grid_type_select.set_active(0)
        grid_type_select.connect("changed", self.grid_type_changed, grid_vp)
        debug_hbox.pack_end(grid_type_select)

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
        if not event[2] in channel_list:
            channel_list.append(event[2])
    channel_list.sort()
    return (track_min, track_max, channel_list)

class TrackEditor(gtk.Window):
    def set_len(self, track_len):
        self.track.set_len(track_len * self.sequencer.getppq())
        for chaned in self.chaned_dict.values():
            chaned.set_len(track_len)

    def set_name(self, name):
        if name:
            self.set_title("Track %s" % name)
            self.track.set_name(name)

    def update_pos(self, tickpos):
        for channed in self.chaned_dict.values():
            channed.grid.update_pos(tickpos)

    def clear_progress(self):
        for channed in self.chaned_dict.values():
            channed.grid.clear_progress()

    def __init__(self, track, sequencer, portlist=None, xpadsz=DEFAULT_XPADSZ, font_name=DEFAULT_FONT_NAME):
        gtk.Window.__init__(self)
        # temporary
        self.sequencer = sequencer
        self.track = track

        track_min, track_max, channel_list = get_track_info(track)

        self.min_width = 320
        self.min_height = 240

        self.xpadsz = xpadsz
        self.font_name = font_name

        noteb = gtk.Notebook()
        #noteb.set_tab_pos(gtk.POS_LEFT)

        self.chaned_dict = {}
        if len(channel_list) > 0:
            for channel_num in channel_list:
                chaned = ChannelEditor(self, channel_num)
                noteb.append_page(chaned, gtk.Label("Channel %i" % channel_num))
                self.chaned_dict[channel_num] = chaned
        else:
            channel_num = 0
            chaned = ChannelEditor(self, channel_num)
            noteb.append_page(chaned, gtk.Label("Channel %i" % channel_num))
            self.chaned_dict[channel_num] = chaned

        self.set_title("Track %s" % self.track.get_name())
        def hide_tracked(win, event):
            win.hide()
            return True
        self.connect('delete-event', hide_tracked)
        vbox = gtk.VBox()

        track_setting_frame = gtk.Frame("Track setting")
        track_setting_frame.add(TrackSettingTable(self, self.sequencer.getppq(), portlist))

        vbox.pack_start(track_setting_frame, expand=False)
        vbox.pack_end(noteb)
        self.add(vbox)
