#!/usr/bin/python

import pygtk
pygtk.require("2.0")
import gtk

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

INC_LEFT  = 0
INC_RIGHT = 1


class MsqNGWEventHdl(object):
    def __init__(self, ppq):
        self.note_param  = {"val_on":  DEFAULT_NOTEON_VAL,
                            "val_off": 0,
                            "len":     ppq / 4,
                            "quant":   ppq / 4}
        self.wgt_mode    = NO_MODE
        self.paste_cache = None
        self.note_clipboard = None
        self.tmp_note_area  = None # Changer le nom de cette variable
        self.start_coo   = None
        self.select_area = None
        self.ctrl_click  = False
        self.inc_data    = None


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


    def _getminlen_noteonoff(self):
        min_len = self.selection[0][1][0] - self.selection[0][0][0]
        minev_on_off = self.selection[0]
        for note_on, note_off in self.selection:
            note_len = note_off[0] - note_on[0]
            if note_len < min_len:
                min_len = note_len
        return min_len, minev_on_off


    def leftinc_getdata(self, ev_on_off_tick):
        min_len, minev_on_off = self._getminlen_noteonoff()
        evon_maxtick = self.quantify_tick(minev_on_off[1][0] - self.note_param["quant"])
        if evon_maxtick != minev_on_off[1][0]:
            maxtick = ev_on_off_tick[0][0] + (evon_maxtick - minev_on_off[0][0])
        else:
            maxtick = ev_on_off_tick[0][0] + self.note_param["quant"]
        return (INC_LEFT, ev_on_off_tick[0], maxtick)


    def rightinc_getdata(self, ev_on_off_tick):
        min_len, minev_on_off = self._getminlen_noteonoff()
        evoff_mintick = self.quantify_tick(minev_on_off[0][0] + self.note_param["quant"])
        if evoff_mintick != minev_on_off[0][0]:
            mintick = ev_on_off_tick[1][0] + (evoff_mintick - minev_on_off[1][0])
        else:
            mintick = minev_on_off[1][0] - self.note_param["quant"]
        return (INC_RIGHT, ev_on_off_tick[1], mintick)


    def handle_button_press(self, widget, event):
        self.grab_focus()

        if event.button == 2:
            self.wgt_mode = INC_MODE
            if not self.selection:
                self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))

        if event.button == 3:
            self.window.set_cursor(self.cursor_pencil)
            self.wgt_mode = EDIT_MODE

        elif event.button == 1 and self.wgt_mode == NO_MODE:
            if self.selection:
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
                if not ev_on_off_tick:
                    new_selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                    if new_selection:
                        ev_on_off_tick = self.coo_under_notelist(event.x, event.y, new_selection)
                        if ev_on_off_tick:
                            selarea = self.get_notelist_area(self.selection)
                            self.selection = new_selection
                            self.draw_all(selarea)
            else:
                self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection) if self.selection else None

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

        elif self.wgt_mode == INC_MODE:
            if self.selection:
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
                if ev_on_off_tick:
                    if self._is_note_left_inc(ev_on_off_tick):
                        self.window.set_cursor(self.cursor_inc_l)
                        self.inc_data = self.leftinc_getdata(ev_on_off_tick)
                    else:
                        self.window.set_cursor(self.cursor_inc_r)
                        self.inc_data = self.rightinc_getdata(ev_on_off_tick)
                else:
                    self.window.set_cursor(self.current_cursor)
                    self.wgt_mode = NO_MODE
        else:
            pass


    def xpos2tick(self, xpos):
        return int(xpos * self.ppq / self.xpadsz)


    def ypos2noteval(self, ypos):
        return int(127 - (ypos / self.ypadsz))


    def quantify_tick(self, tick):
        return int(tick / self.note_param["quant"]) * self.note_param["quant"]


    def add_note(self, xpos, ypos):
        noteon_tick = self.quantify_tick(self.xpos2tick(xpos))
        note = self.ypos2noteval(int(ypos))

        noteoff_tick = noteon_tick + self.note_param["len"]

        # Check if there is already a note at position
        self.track.lock()
        for evwr in self.track:
            ev = evwr.get_event()
            if ev[2] == MIDI_NOTEON_EVENT or ev[2] == MIDI_NOTEOFF_EVENT:
                if ev[1] == self.chan_num and ev[3] == note:
                    if noteon_tick <= ev[0] and ev[0] <= noteoff_tick:
                        self.track.unlock()
                        print "Can't add note at this position found other one", ev
                        return

        note_on = (noteon_tick,
                   self.chan_num,
                   MIDI_NOTEON_EVENT,
                   note,
                   self.note_param["val_on"])
        note_off = (noteoff_tick,
                    self.chan_num,
                    MIDI_NOTEOFF_EVENT,
                    note,
                    self.note_param["val_off"])
        self.track.add_note_event(*note_on)
        self.track.add_note_event(*note_off)
        self.track.unlock()

        self.draw_note(note_on, note_off)


    def _get_diff_from_start_coo(self, xpos, ypos):
        # TODO (optimisation)
        tick_diff = self.quantify_tick(self.xpos2tick(xpos)) - self.quantify_tick(self.xpos2tick(self.start_coo[0]))
        note_diff = self.ypos2noteval(ypos) - self.ypos2noteval(self.start_coo[1])
        return tick_diff, note_diff


    def _paste_notes_at(self, xpos, ypos, note_list):
        tick_diff, note_diff = self._get_diff_from_start_coo(xpos, ypos)
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
            # tmp hack: handle of trash API (usage of node's deleted attribut)
            try:
                for evwr in self.track:
                    event = evwr.get_event()
                    while event and self.is_selected(event, note_list):
                        self.track.event2trash(evwr)
                        evwr.next()
                        event = evwr.get_event()
            except StopIteration: # bis: catching exception raised by the next function
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


    def draw_notelist_area(self, notelist):
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

        elif event.button == 1 or event.button == 2:
            if self.wgt_mode == EDIT_MODE and event.button == 1:
                self.add_note(event.x, event.y)

            elif self.wgt_mode == SELECT_MODE and event.button == 1:
                if self.select_area:
                    self.draw_all(self.select_area) # tmp: change draw_all to reversible effect (or a faster redraw)
                    new_sel = self.get_notes(self.select_area)
                else:
                    self.select_area = gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1)

                if self.ctrl_click:
                    new_sel = self.get_notes(self.select_area)
                    if new_sel and self.selection:
                        for noteonoff in new_sel:
                            if not note_in_select(noteonoff, self.selection):
                                self.selection.append(noteonoff)
                    else:
                        self.selection = new_sel
                else:
                    self.selection = self.get_notes(self.select_area)
                self.ctrl_click  = False
                self.select_area = None
                self.start_coo   = None
                self.wgt_mode    = NO_MODE
                if self.selection: # render new selection
                    self.draw_notelist_area(self.selection)
                self.wgt_mode = NO_MODE

            elif self.wgt_mode == PASTE_MODE and event.button == 1:
                if  self.tmp_note_area:
                    self.draw_all(self.tmp_note_area)

                    if self.paste_cache:
                        if self.selection: self.delete_notes(self.selection)
                        self.selection = self._paste_notes_at(event.x, event.y, self.paste_cache)
                        self.draw_all(self.get_notelist_area(self.selection))
                self.wgt_mode = NO_MODE

            elif self.wgt_mode == INC_MODE:
                if self.tmp_note_area:
                    self.draw_all(self.tmp_note_area)
                    self.tmp_note_area = None
                self.window.set_cursor(self.current_cursor)
                self.wgt_mode = NO_MODE
                if self.inc_data:
                    if self.inc_data[0] == INC_LEFT:
                        notelist = self.get_inc_left_notelist(event.x)
                    else:
                        notelist = self.get_inc_right_notelist(event.x)
                    self.delete_notes(self.selection)
                    for note_on, note_off in notelist:
                        self.track.add_note_event(*note_on)
                        self.track.add_note_event(*note_off)
                    self.selection = notelist
                    self.draw_notelist_area(notelist)
                    self.inc_data = None


    def coo_under_notelist(self, xpos, ypos, notelist):
        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)
        for ev_on, ev_off in notelist:
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


    def draw_notelist(self, note_list, selected=True):
        tick_min = note_list[0][0][0]
        tick_max = note_list[0][0][0]
        note_min = note_list[0][0][3]
        note_max = note_list[0][0][3]
        for note_on, note_off in note_list:
            self.draw_note(note_on, note_off, selected)
            if tick_max < note_off[0]:
                tick_max = note_off[0]
            if tick_min > note_on[0]:
                tick_min = note_on[0]
            if note_min > note_on[3]:
                note_min = note_on[3]
            if note_max < note_on[3]:
                note_max = note_on[3]
                note_max = note_on[3]
        xmin = self.tick2xpos(tick_min)
        xmax = self.tick2xpos(tick_max)
        ymin = self.note2ypos(note_max)
        ymax = self.note2ypos(note_min)
        return gtk.gdk.Rectangle(xmin - 2,
                                 ymin - 2,
                                 xmax - xmin + 4,
                                 ymax - ymin + self.ypadsz + 4)


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
        return gtk.gdk.Rectangle(xmin - 2,
                                 ymin - 2,
                                 xmax - xmin + 4,
                                 ymax - ymin + self.ypadsz + 4)


    def draw_paste_at(self, xpos, ypos, note_list):
        tick_diff, note_diff = self._get_diff_from_start_coo(xpos, ypos)

        if self.tmp_note_area:
            self.draw_all(self.tmp_note_area)
        self.tmp_note_area = self.draw_notelist_at(tick_diff, note_diff, note_list)


    def _is_note_left_inc(self, ev_on_off_tick):
        len1 = ev_on_off_tick[2] - ev_on_off_tick[0][0]
        len2 = ev_on_off_tick[1][0] - ev_on_off_tick[2]
        if len1 < len2:
            return True
        else:
            return False


    def get_inc_left_notelist(self, xpos):
        tick = self.quantify_tick(self.xpos2tick(xpos))
        maxtick = self.inc_data[2]
        current_noteon = self.inc_data[1]
        if maxtick < tick:
            tick_diff = maxtick - current_noteon[0]
        else:
            tick_diff = tick - current_noteon[0]
        inc_note_list = []
        for note_on, note_off in self.selection:
            note_on = (note_on[0] + tick_diff,
                       note_on[1],
                       note_on[2],
                       note_on[3],
                       note_on[4])
            inc_note_list.append((note_on, note_off))
        return inc_note_list

    def draw_inc_left(self, xpos):
        inc_note_list = self.get_inc_left_notelist(xpos)
        return self.draw_notelist(inc_note_list, True)


    def get_inc_right_notelist(self, xpos):
        tick = self.quantify_tick(self.xpos2tick(xpos) + self.note_param["quant"])
        mintick = self.inc_data[2]
        current_noteoff = self.inc_data[1]
        if mintick > tick:
            tick_diff = mintick - current_noteoff[0]
        else:
            tick_diff = tick - current_noteoff[0]
        inc_note_list = []
        for note_on, note_off in self.selection:
            note_off = (note_off[0] + tick_diff,
                        note_off[1],
                        note_off[2],
                        note_off[3],
                        note_off[4])
            inc_note_list.append((note_on, note_off))
        return inc_note_list

    def draw_inc_right(self, xpos):
        inc_note_list = self.get_inc_right_notelist(xpos)
        return self.draw_notelist(inc_note_list, True)


    def set_inc_cursor(self, xpos, ypos):
        ev_on_off_tick = self.coo_under_notelist(xpos, ypos, self.selection)
        if ev_on_off_tick:
            if self._is_note_left_inc(ev_on_off_tick):
                self.window.set_cursor(self.cursor_inc_l)
            else:
                self.window.set_cursor(self.cursor_inc_r)
        else:
            self.window.set_cursor(self.cursor_inc)


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
            self.select_area = gtk.gdk.Rectangle(int(xmin) - 2,
                                                 int(ymin) - 2,
                                                 int(xmax - xmin) + 4,
                                                 int(ymax - ymin) + 4)
            cr = self.window.cairo_create()
            cr.set_source_rgb(0, 0, 0)
            cr.set_line_width(1)
            cr.rectangle(int(xmin) - 0.5, int(ymin) - 0.5, int(xmax - xmin), int(ymax - ymin))
            cr.stroke()
        elif self.wgt_mode == PASTE_MODE:
            self.draw_paste_at(event.x, event.y, self.paste_cache)
        elif self.wgt_mode == NO_MODE and self.selection:
            ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
            if ev_on_off_tick:
                self.window.set_cursor(self.cursor_move)
            else:
                self.window.set_cursor(self.current_cursor)
        elif self.wgt_mode == INC_MODE and self.selection:
            if self.inc_data:
                if self.tmp_note_area:
                    self.draw_all(self.tmp_note_area)
                    self.tmp_note_area = None
                if self.inc_data[0] == INC_LEFT:
                    self.tmp_note_area = self.draw_inc_left(event.x)
                else:
                    self.tmp_note_area = self.draw_inc_right(event.x)
            else:
                self.set_inc_cursor(event.x, event.y)


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
                xpos, ypos, mod = self.window.get_pointer()
                self.set_inc_cursor(xpos, ypos)


    def get_upleft_from_notelist(self, notelist):
        note_max = max(notelist, key=lambda x: x[0][3])[0][3]
        tick_min = min(notelist, key=lambda x: x[0][0])[0][0]
        return (self.tick2xpos(tick_min), self.note2ypos(note_max))


    def handle_key_release(self, widget, event):
        self.grab_focus()
        if self.wgt_mode == PASTE_MODE:
            pass
        elif self.wgt_mode == INC_MODE:
            if not self.inc_data:
                self.wgt_mode = NO_MODE
                self.window.set_cursor(self.current_cursor)
        else:
            self.wgt_mode = NO_MODE
            self.window.set_cursor(self.current_cursor)
            if event.state & gtk.gdk.CONTROL_MASK:
                keyname = gtk.gdk.keyval_name(event.keyval)
                if self.selection:
                    if keyname == "x":
                        self.note_clipboard = self.selection
                        self.delete_notes(self.selection)
                        self.selection = None
                    elif keyname == "c":
                        self.note_clipboard = self.selection
                if keyname == "v":
                    if self.note_clipboard:
                        self.paste_cache = self.note_clipboard
                        self.start_coo = self.get_upleft_from_notelist(self.paste_cache)
                        if self.selection:
                            note_list = self.selection
                            self.selection = None
                            self.draw_notelist_area(note_list) # clear last selection
                        self.wgt_mode = PASTE_MODE


    def realize_event_handler(self):
        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("key_press_event", self.handle_key_press)
        self.connect("key_release_event", self.handle_key_release)
        self.connect("motion_notify_event", self.handle_motion)
        self.cursor_pencil = gtk.gdk.Cursor(gtk.gdk.PENCIL)
        self.cursor_inc = gtk.gdk.Cursor(gtk.gdk.SB_H_DOUBLE_ARROW)
        self.cursor_inc_l = gtk.gdk.Cursor(gtk.gdk.SB_LEFT_ARROW)
        self.cursor_inc_r = gtk.gdk.Cursor(gtk.gdk.SB_RIGHT_ARROW)
        self.cursor_move = gtk.gdk.Cursor(gtk.gdk.FLEUR)
        self.current_cursor = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.window.set_cursor(self.current_cursor)
        self.set_can_focus(True)
