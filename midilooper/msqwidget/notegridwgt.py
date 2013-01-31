#!/usr/bin/python

import gobject
import pygtk
pygtk.require("2.0")
import gtk
from gtk import gdk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required for this example"
    raise SystemExit

MIDI_NOTEOFF_EVENT = 0x8
MIDI_NOTEON_EVENT  = 0x9
DEFAULT_NOTEON_VAL = 100

NO_MODE     = 0
EDIT_MODE   = 1
SELECT_MODE = 2
PASTE_MODE  = 3
INC_MODE    = 4

class MsqNGWEventHdl(object):
    def __init__(self, ppq):
        self.note_param  = {"channel": 0,
                            "val_on":  DEFAULT_NOTEON_VAL,
                            "val_off": 0,
                            "len":     ppq / 4,
                            "quant":   ppq / 4}
        self.wgt_mode    = NO_MODE
        self.paste_cache = None
        self.paste_area  = None
        self.start_coo   = None
        self.select_area = None
        self.ctrl_click  = False
        # self.selection = None
        # self.clipboard = None

        # self.to_paste = []
        # self.to_move = None
        # self.inc_note_left  = None
        # self.inc_note_right = None
        # self.rect_select_start = None
        # self.paste_area = None
        # self.paste_motion = False

    def get_notelist_area(self, note_list):
        if len(note_list) == 0:
            return None
        self.track.lock()
        ev_noteon, ev_noteoff = note_list[0]
        min_tick = ev_noteon[0]
        max_tick = ev_noteoff[0]
        min_note = ev_noteon[3]
        max_note = ev_noteon[3]
        for ev_noteon, ev_noteoff in note_list:
            if ev_noteon[0] < min_tick:
                min_tick = ev_noteon[0]
            if ev_noteoff[0] > max_tick:
                max_tick = ev_noteoff[0]
            if ev_noteon[3] < min_note:
                min_note = ev_noteon[3]
            if ev_noteon[3] > max_note:
                max_note = ev_noteon[3]
        self.track.unlock()
        xmax = max_tick * self.xpadsz / self.ppq
        xmin = min_tick * self.xpadsz / self.ppq
        ymax = ((127 - min_note) * self.ypadsz)
        ymin = ((127 - max_note) * self.ypadsz)
        return gtk.gdk.Rectangle(xmin - 1, ymin - 1, xmax - xmin + 2, ymax - ymin + self.ypadsz + 2)

    def handle_button_press(self, widget, event):
        self.grab_focus()

        if event.button == 3:
            self.window.set_cursor(self.cursor_pencil)
            self.wgt_mode = EDIT_MODE
        elif event.button == 1 and self.wgt_mode == NO_MODE:
            ev_on_off_tick = self.coo_under_notelist(event.x, event.y) if self.selection else None
            if ev_on_off_tick:
                self.wgt_mode = PASTE_MODE
                self.paste_cache = self.selection
                self.start_coo = (event.x, event.y)
            else:
                if event.state & gtk.gdk.CONTROL_MASK:
                    self.ctrl_click = True
                self.start_coo = (event.x, event.y)
                self.wgt_mode = SELECT_MODE
                if (not self.ctrl_click) and self.selection:
                    selarea = self.get_notelist_area(self.selection)
                    self.selection = None
                    if selarea:    # clear previous selection
                        selarea.x = selarea.x - 2
                        selarea.y = selarea.y - 2
                        selarea.width = selarea.width + 4
                        selarea.height = selarea.height + 4
                        self.draw_all(selarea)

    def xpos2tick(self, xpos):
        return int(xpos * self.ppq / self.xpadsz)

    def ypos2noteval(self, ypos):
        return int(127 - (ypos / self.ypadsz))

    def quantify_tick(self, tick):
        return int(tick / self.note_param["quant"]) * self.note_param["quant"]

    def add_note(self, xpos, ypos):
        tick = self.quantify_tick(self.xpos2tick(xpos))
        note = self.ypos2noteval(int(ypos))

        self.track.lock()
        tick_max = tick + self.note_param["len"] -1
        for evwr in self.track:
            ev = evwr.get_event()
            if ev[3] == note:
                if tick <= ev[0] and ev[0] <= tick_max:
                    self.track.unlock()
                    print "Can't add note at this position found other one", ev
                    return

        note_on = (tick,
                   self.note_param["channel"],
                   MIDI_NOTEON_EVENT,
                   note,
                   self.note_param["val_on"])
        note_off = (tick_max,
                    self.note_param["channel"],
                    MIDI_NOTEOFF_EVENT,
                    note,
                    self.note_param["val_off"])
        self.track.add_note_event(*note_on)
        self.track.add_note_event(*note_off)
        self.track.unlock()

        self.draw_note(note_on, note_off)



    def get_diff_from_start_coo(self, xpos, ypos):
        tick_diff = self.quantify_tick(self.xpos2tick(xpos - self.start_coo[0])
                                       + (self.note_param["quant"]/2))
        note_diff = int(- (ypos - (self.start_coo[1] + (self.ypadsz / 2)))
                          / self.ypadsz)
        return tick_diff, note_diff


    def _paste_notes_at(self, xpos, ypos, note_list):
        tick_diff, note_diff = self.get_diff_from_start_coo(xpos, ypos)
        new_list = []
        self.track.lock()
        for note_on, note_off in self.paste_cache:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            new_list.append((diff_note_on, diff_note_off))
            self.track.add_note_event(*diff_note_on)
            self.track.add_note_event(*diff_note_off)
        self.track.unlock()
        return new_list


    def delete_notes(self, note_list):
        self.track.lock()
        if self.track.is_handled():
            # temporary hack for trash API (with the deleted attribut of nodes)
            try:
                for evwr in self.track:
                    event = evwr.get_event()
                    while event and self.is_selected(event, note_list):
                        self.track.event2trash(evwr)
                        evwr.next()
            except StopIteration: # catching exception raised by the next function
                pass
        else:
            for evwr in self.track:
                event = evwr.get_event()
                while event and self.is_selected(event, note_list):
                    evwr._del_event()
                    event = evwr.get_event()
        self.track.unlock()

        selarea = self.get_notelist_area(note_list)
        if selarea:
            selarea.x = selarea.x - 2
            selarea.y = selarea.y - 2
            selarea.width = selarea.width + 4
            selarea.height = selarea.height + 4
            self.draw_all(selarea)


    def draw_notelist(self, notelist):
        selarea = self.get_notelist_area(notelist)
        selarea.x = selarea.x - 2
        selarea.y = selarea.y - 2
        selarea.width = selarea.width + 4
        selarea.height = selarea.height + 4
        self.draw_all(selarea)


    def handle_button_release(self, widget, event):
        def note_in_select(noteonoff, select):
            ev1 = noteonoff[0]
            for n_onoff in select:
                if ev1 == n_onoff[0]:
                    return True
            return False

        if event.button == 3:
            self.window.set_cursor(self.current_cursor)
            self.wgt_mode = NO_MODE

        elif event.button == 1:

            if self.wgt_mode == EDIT_MODE:
                self.add_note(event.x, event.y)

            elif self.wgt_mode == SELECT_MODE:
                if self.select_area:
                    self.draw_all(self.select_area) # temporary change draw_all to reversible effect
                    if self.ctrl_click:
                        new_sel = self.get_notes(self.select_area)
                        if self.selection:
                            for noteonoff in new_sel:
                                if not note_in_select(noteonoff, self.selection):
                                    self.selection.append(noteonoff)
                        else:
                            self.selection = new_sel
                    else:
                        self.selection = self.get_notes(self.select_area)
                else:
                    self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                self.ctrl_click  = False
                self.select_area = None
                self.start_coo   = None
                self.wgt_mode    = NO_MODE
                if self.selection and len(self.selection): # render new selection
                    self.draw_notelist(self.selection)
                self.wgt_mode = NO_MODE

            elif self.wgt_mode == PASTE_MODE:
                if  self.paste_area:
                    self.draw_all(self.paste_area)
                if self.paste_cache:
                    if self.selection: self.delete_notes(self.selection)
                    self.selection = self._paste_notes_at(event.x, event.y, self.paste_cache)
                    self.draw_all(self.get_notelist_area(self.selection))
                self.wgt_mode = NO_MODE


    def coo_under_notelist(self, xpos, ypos):
        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)
        for ev_on, ev_off in self.selection:
            note_on = ev_on[3]
            if note == note_on:
                tick_on = ev_on[0]
                tick_off = ev_off[0]
                if tick >= tick_on and tick <= tick_off:
                    return ev_on, ev_off, tick
        return None


    def diff_note(self, note, tick_diff, note_diff):
        note = (note[0] + tick_diff,
                note[1],
                note[2],
                note[3] + note_diff,
                note[4])
        return note


    def draw_notelist_at(self, tick_diff, note_diff, note_list):
        tick_min = note_list[0][0][0] + tick_diff
        tick_max = note_list[0][0][0] + tick_diff
        note_min = note_list[0][0][3] + note_diff
        note_max = note_list[0][0][3] + note_diff
        for note_on, note_off in note_list:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            self.draw_note(diff_note_on, diff_note_off, selected=False)
            if tick_max < diff_note_off[0]:
                tick_max = diff_note_off[0]
            if tick_min > diff_note_on[0]:
                tick_min = diff_note_on[0]
            if note_min > diff_note_on[3]:
                note_min = diff_note_on[3]
            if note_max < diff_note_on[3]:
                note_max = diff_note_on[3]
                note_max = diff_note_on[3]
        xmin = self.tick2xpos(tick_min)
        xmax = self.tick2xpos(tick_max)
        ymin = self.note2ypos(note_max)
        ymax = self.note2ypos(note_min)
        self.paste_area = gtk.gdk.Rectangle(xmin - 2,
                                            ymin - 2,
                                            xmax - xmin + 4,
                                            ymax - ymin + self.ypadsz + 4)


    def draw_paste_at(self, xpos, ypos, note_list):
        tick_diff, note_diff = self.get_diff_from_start_coo(xpos, ypos)

        if self.paste_area:
            self.draw_all(self.paste_area)
        self.draw_notelist_at(tick_diff, note_diff, note_list)


    def handle_motion(self, widget, event):
        if self.wgt_mode == SELECT_MODE and event.is_hint and self.start_coo:
            xmin = None
            xmax = None
            ymin = None
            ymax = None
            if event.x > self.start_coo[0]:
                xmax = event.x
                xmin = self.start_coo[0]
            else:
                xmax = self.start_coo[0]
                xmin = event.x
            if event.y > self.start_coo[1]:
                ymax = event.y
                ymin = self.start_coo[1]
            else:
                ymax = self.start_coo[1]
                ymin = event.y
            if self.select_area:
                self.draw_all(self.select_area)
            cr = self.window.cairo_create()
            self.select_area = gtk.gdk.Rectangle(int(xmin) - 2,
                                                 int(ymin) - 2,
                                                 int(xmax - xmin) + 4,
                                                 int(ymax - ymin) + 4)
            cr.set_source_rgb(0, 0, 0)
            cr.rectangle(int(xmin), int(ymin), int(xmax - xmin), int(ymax - ymin))
            cr.stroke()
        elif self.wgt_mode == NO_MODE and self.selection and len(self.selection) > 0:
            ev_on_off_tick = self.coo_under_notelist(event.x, event.y)
            if ev_on_off_tick:
                self.window.set_cursor(self.cursor_move)
            else:
                self.window.set_cursor(self.current_cursor)
        elif self.wgt_mode == PASTE_MODE:
            self.draw_paste_at(event.x, event.y, self.paste_cache)



    def handle_key_press(self, widget, event):
        self.grab_focus()
        if self.selection and (event.keyval == gtk.keysyms.Delete
                               or event.keyval == gtk.keysyms.BackSpace):
            if self.selection:
                self.delete_notes(self.selection)
                self.selection = None
        else:
            if event.keyval == gtk.keysyms.Alt_L or event.keyval == gtk.keysyms.Alt_R:
                self.wgt_mode = EDIT_MODE
                self.window.set_cursor(self.cursor_pencil)
            elif event.keyval == gtk.keysyms.Shift_L or event.keyval == gtk.keysyms.Shift_R:
                self.wgt_mode = INC_MODE
                self.window.set_cursor(self.cursor_inc)


    def get_upleft_from_notelist(self, notelist):
        note_max = max(self.paste_cache, key=lambda x: x[0][3])[0][3]
        tick_min = min(self.paste_cache, key=lambda x: x[0][0])[0][0]
        return (self.tick2xpos(tick_min), self.note2ypos(note_max))


    def handle_key_release(self, widget, event):
        if self.wgt_mode == PASTE_MODE:
            pass
        else:
            self.grab_focus()
            self.wgt_mode = NO_MODE
            self.window.set_cursor(self.current_cursor)
            if event.state & gtk.gdk.CONTROL_MASK:
                keyname = gtk.gdk.keyval_name(event.keyval)
                if self.selection:
                    if keyname == "x":
                        self.paste_cache = self.selection
                        self.delete_notes(self.selection)
                        self.selection = None
                    elif keyname == "c":
                        self.paste_cache = self.selection
                if keyname == "v":
                    if self.paste_cache:
                        self.start_coo = self.get_upleft_from_notelist(self.paste_cache)
                        if self.selection:
                            note_list = self.selection
                            self.selection = None
                            self.draw_notelist(note_list) # clear last selection
                        self.wgt_mode = PASTE_MODE


    def realize_event_handler(self):
        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("key_press_event", self.handle_key_press)
        self.connect("key_release_event", self.handle_key_release)
        self.connect("motion_notify_event", self.handle_motion)
        self.cursor_pencil = gtk.gdk.Cursor(gtk.gdk.PENCIL)
        self.cursor_inc = gtk.gdk.Cursor(gtk.gdk.SB_H_DOUBLE_ARROW)
        self.cursor_move = gtk.gdk.Cursor(gtk.gdk.FLEUR)
        self.current_cursor = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.window.set_cursor(self.current_cursor)
        self.set_can_focus(True)
