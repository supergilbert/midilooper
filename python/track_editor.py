#!/usr/bin/python

import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

from widget import MsqHBarTimeWidget, MsqVBarNoteWidget, MsqNoteGridWidget, DEFAULT_XPADSZ, DEFAULT_FONT_NAME

def redraw_grid_vp(grid_vp):
    grid = grid_vp.get_child()
    hadj = grid_vp.get_hadjustment()
    vadj = grid_vp.get_vadjustment()
    area = gtk.gdk.Rectangle(int(hadj.get_value()),
                             int(vadj.get_value()),
                             int(hadj.get_page_size()),
                             int(vadj.get_page_size()))
    grid.draw_all(area)

class TrackEditor(object):
    def __init__(self, track, track_len, ppq, xpadsz=DEFAULT_XPADSZ, font_name=DEFAULT_FONT_NAME):
        width = 320
        height = 240

        def set_adjustment(widget, event, hadj, vadj, xpadsz, ypadsz):
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

        noteb = gtk.Notebook()
        #noteb.set_tab_pos(gtk.POS_LEFT)

        def reset_track(button, grid_vp):
            grid = grid_vp.get_child()
            grid.track = []
            redraw_grid_vp(grid_vp)

        def grid_type_changed(combobox, grid_vp):
            val = combobox.get_model()[combobox.get_active()][0]
            grid = grid_vp.get_child()
            if val == "Bar":
                grid.set_draw_bars_mode()
            else:
                grid.set_draw_points_mode()
            redraw_grid_vp(grid_vp)

        def debug_grid(button, grid_vp):
            redraw_grid_vp(grid_vp)

        for channel, events in track.items():
            table = gtk.Table(3, 3)

            hbar = MsqHBarTimeWidget(track_len, xpadsz=xpadsz)
            xpadsz = hbar.xpadsz
            hbar_vp = gtk.Viewport()
            hbar_vp.set_size_request(width, -1)
            hbar_vp.add(hbar)
            hbar_vp.set_shadow_type(gtk.SHADOW_NONE)
            hadj = hbar_vp.get_hadjustment()
            hbar_vp.connect("scroll_event", set_adjustment, hadj, hadj, xpadsz, xpadsz)

            vbar = MsqVBarNoteWidget(font_name=font_name)
            ypadsz = vbar.ypadsz
            vbar_vp = gtk.Viewport()
            vbar_vp.set_size_request(-1, height)
            vbar_vp.add(vbar)
            vbar_vp.set_shadow_type(gtk.SHADOW_NONE)
            vadj = vbar_vp.get_vadjustment()
            vbar_vp.connect("scroll_event", set_adjustment, hadj, vadj, xpadsz, ypadsz)

            grid = MsqNoteGridWidget(events, track_len, ppq=ppq, xpadsz=xpadsz, ypadsz=ypadsz)
            grid_vp = gtk.Viewport()
            grid_vp.set_size_request(width, height)
            grid_vp.add(grid)
            grid_vp.connect("scroll_event", set_adjustment, hadj, vadj, xpadsz, ypadsz)
            grid_vp.set_shadow_type(gtk.SHADOW_NONE)
            grid_vp.set_hadjustment(hadj)
            grid_vp.set_vadjustment(vadj)

            vsb = gtk.VScrollbar(vadj)
            hsb = gtk.HScrollbar(hadj)

            table.attach(hbar_vp, 1, 2, 0, 1, gtk.FILL, 0)
            table.attach(vbar_vp, 0, 1, 1, 2, 0, gtk.FILL)
            table.attach(grid_vp, 1, 2, 1, 2, gtk.EXPAND|gtk.FILL, gtk.EXPAND|gtk.FILL)
            table.attach(vsb, 2, 3, 1, 2, 0, gtk.FILL)
            table.attach(hsb, 1, 2, 2, 3, gtk.FILL, 0)

            hbox = gtk.HBox()
            button_clear = gtk.Button("Reset")
            button_clear.connect("clicked", reset_track, grid_vp)
            hbox.pack_start(button_clear)

            button_misc = gtk.Button("Refresh")
            button_misc.connect("clicked", debug_grid, grid_vp)
            hbox.pack_end(button_misc)

            grid_type_select = gtk.combo_box_new_text()
            grid_type_select.append_text("Bar")
            grid_type_select.append_text("Point")
            grid_type_select.set_active(0)

            grid_type_select.connect("changed", grid_type_changed, grid_vp)

            grid_disp_frame = gtk.Frame("Note grid display setting")
            grid_disp_frame.add(grid_type_select)

            vbox = gtk.VBox()
            vbox.pack_start(table)
            vbox.pack_end(hbox, expand=False)
            vbox.pack_end(grid_disp_frame, expand=False)

            noteb.append_page(vbox, gtk.Label("Channel %i" % channel))


        win = gtk.Window()
        win.set_title('Track editor')
        win.connect('delete_event', gtk.main_quit)

        win.add(noteb)

        self.win = win


def get_track_dict(trackev_list):

    def append_cev(event_list, tick, event):
        for idx, tickev in enumerate(event_list):
            if tick > tickev[0]:
                continue
            elif tick == tickev[0]:
                tickev[1].append(event)
                return
            else:
                event_list.insert(idx, (tick, [event]))
                return
        event_list.append((tick, [event]))

    track_dict = {}
    for tickev in trackev_list:
        tick = tickev[0]
        for event in tickev[1]:
            if not track_dict.has_key(event[1]):
                track_dict[event[1]] = [(tick, [event])]
            else:
                append_cev(track_dict[event[1]], tick, event)
    return track_dict



if __name__ == '__main__':
    # import sys
    # sys.path.append('./build/lib.linux-x86_64-2.7')
    # import midiseq
    # midif = midiseq.midifile("../../file/midi/Highway_to_Hell.mid")
    # track = midif.getmergedtrack()
    # events = track.getevents()
    # track_dict =  get_track_dict(events[2])
    # ppq = midif.getppq()
    # min = events[0] / ppq
    # max = events[1] / ppq
    # len = max - min
    # track = TrackEditor(track_dict, max - min, ppq)
    ppq = 48
    len = 4 * 4 * 10
    track = TrackEditor({1: []}, len, ppq)
    track.win.show_all()
    gtk.main()
