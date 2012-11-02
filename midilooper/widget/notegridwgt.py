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

class MsqNGWHandleEvent:
    def handle_button_release(self, widget, event):
        if event.button == 3:
            self.window.set_cursor(self.cursor_arrow)
            self.button3down = False
        elif event.button == 1:
            if self.paste_motion:
                self.paste_note(event.x, event.y)
                self.paste_motion = False
                self.paste_area = None
                return
            if self.to_move:
                self.paste_move_selection(event.x, event.y)
                self.to_move = None
                self.paste_area = None
                return
            if self.inc_note_right:
                self.inc_right_note_selection(event.x)
                self.draw_all(self.inc_note_right[2])
                self.inc_note_right = None
                return
            if self.inc_note_left:
                self.inc_left_note_selection(event.x)
                self.inc_note_left = None
                return
            if self.button3down:
                self.add_note(event.x, event.y)
            else:
                if self.rect_select: # clear rectangle selection
                    self.draw_all(self.rect_select) # temporary change draw_all to reversible effect
                    if event.state & gtk.gdk.CONTROL_MASK and self.ctrl_click:
                        self.selection.extend(self.select_note(self.rect_select))
                    else:
                        self.selection = self.select_note(self.rect_select)
                    self.ctrl_click = False
                    if len(self.selection): # render new selection
                        selarea = self.get_notelist_area(self.selection)
                        selarea.x = selarea.x - 2
                        selarea.y = selarea.y - 2
                        selarea.width = selarea.width + 4
                        selarea.height = selarea.height + 4
                        self.draw_all(selarea)
                    self.rect_select = None
                self.rect_select_start = None

    def get_selection_minsize(self):
        if self.selection == None:
            return None
        def get_note_on_off_ticksz(note_list_node):
            note_on = note_list_node[0].get_event()
            note_off = note_list_node[1].get_event()
            return note_off[0] - note_on[0]
        return min(map(get_note_on_off_ticksz, self.selection))

    def handle_button_press(self, widget, event):
        self.grab_focus()
        if self.paste_motion:
            return
        if event.button == 3:
            self.window.set_cursor(self.cursor_pencil)
            self.button3down = True
        elif event.button == 1 and not self.button3down:
            selarea = self.get_notelist_area(self.selection)
            if event.state & gtk.gdk.CONTROL_MASK:
                self.ctrl_click = True
            else:
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
                if ev_on_off_tick:
                    if event.state & gtk.gdk.SHIFT_MASK:
                        len1 = ev_on_off_tick[2] - ev_on_off_tick[0][0]
                        len2 = ev_on_off_tick[1][0] - ev_on_off_tick[2]
                        tick_diff = int(self.get_selection_minsize() / self.note_param["quant"]) * self.note_param["quant"]
                        if len1 < len2:
                            self.inc_note_left = (ev_on_off_tick[0][0],
                                                  ev_on_off_tick[0][0] + tick_diff,
                                                  selarea,
                                                  map(lambda note_on_off: (note_on_off[0].get_event(), note_on_off[1].get_event()),
                                                      self.selection))
                            self.delete_selection()
                            return
                        else:
                            self.inc_note_right = (ev_on_off_tick[1][0],
                                                   ev_on_off_tick[1][0] - tick_diff,
                                                   selarea,
                                                  map(lambda note_on_off: (note_on_off[0].get_event(), note_on_off[1].get_event()),
                                                      self.selection))
                            self.delete_selection()
                            return
                    else:
                        self.to_move = {"tick": int(ev_on_off_tick[0][0] / self.note_param["quant"]) * self.note_param["quant"],
                                        "note": ev_on_off_tick[0][3],
                                        "selection": map(lambda x: (x[0].get_event(), x[1].get_event()),
                                                         self.selection)}
                        self.delete_selection()
                        return
                else:
                    self.selection = []
            if selarea:    # clear previous selection
                selarea.x = selarea.x - 2
                selarea.y = selarea.y - 2
                selarea.width = selarea.width + 4
                selarea.height = selarea.height + 4
                self.draw_all(selarea)
            self.rect_select_start = (event.x, event.y)

    def diff_note(self, note, tick_diff, note_diff):
        note = (note[0] + tick_diff,
                note[1],
                note[2],
                note[3] + note_diff,
                note[4])
        return note

    def paste_note_selection(self, xpos, ypos, tick_seed, note_seed, selection):
        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]

        note_diff = note - note_seed
        tick_diff = tick - tick_seed

        note_min = note_seed + note_diff
        tick_max = tick_seed + tick_diff

        new_note_selection = []
        for note_on, note_off in selection:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            self.track.add_note_event(*diff_note_on)
            self.track.add_note_event(*diff_note_off)
            # temp hack
            mnoteon = self.get_match_note(diff_note_on[0], diff_note_on[1], MIDI_NOTEON_EVENT, diff_note_on[3])[0]
            mnoteoff = self.get_match_note(diff_note_off[0], diff_note_off[1], MIDI_NOTEOFF_EVENT, diff_note_off[3])[0]
            # end of temp hack
            new_note_selection.append((mnoteon, mnoteoff))
        return new_note_selection


    def paste_note(self, xpos, ypos):
        note_max = max(self.to_paste, key=lambda x: x[0][3])[0][3]
        tick_min = min(self.to_paste, key=lambda x: x[0][0])[0][0]
        self.paste_note_selection(xpos, ypos, tick_min, note_max, self.to_paste)
        if self.paste_area:
            self.draw_all(self.paste_area)

    def paste_move_selection(self, xpos, ypos):
        self.selection = self.paste_note_selection(xpos, ypos,
                                                   self.to_move["tick"], self.to_move["note"],
                                                   self.to_move["selection"])
        if self.paste_area:
            self.draw_all(self.paste_area)

    def inc_left_note_selection(self, xpos):
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]
        if tick != self.inc_note_left[0]:
            inc_size = 0
            if tick < self.inc_note_left[1]:
                inc_size = self.inc_note_left[0] - tick
            new_note_selection = []
            for note_on, note_off in self.inc_note_left[3]:
                note_on = (note_on[0] - inc_size,
                           note_on[1],
                           note_on[2],
                           note_on[3],
                           note_on[4])
                self.track.add_note_event(*note_on)
                self.track.add_note_event(*note_off)
                mnoteon = self.get_match_note(note_on[0], note_on[1], MIDI_NOTEON_EVENT, note_on[3])[0]
                mnoteoff = self.get_match_note(note_off[0], note_off[1], MIDI_NOTEOFF_EVENT, note_off[3])[0]
                new_note_selection.append((mnoteon, mnoteoff))
            self.selection = new_note_selection
        if self.paste_area:
            self.draw_all(self.paste_area)

    def inc_right_note_selection(self, xpos):
        tick = self.xpos2tick(xpos)
        tick = (int(tick / self.note_param["quant"]) + 1) * self.note_param["quant"]
        if tick != self.inc_note_right[0]:
            inc_size = 0
            if self.inc_note_right[1] < tick:
                inc_size = tick - self.inc_note_right[0]
            new_note_selection = []
            for note_on, note_off in self.inc_note_right[3]:
                note_off = (note_off[0] + inc_size,
                            note_off[1],
                            note_off[2],
                            note_off[3],
                            note_off[4])
                self.track.add_note_event(*note_on)
                self.track.add_note_event(*note_off)
                mnoteon = self.get_match_note(note_on[0], note_on[1], MIDI_NOTEON_EVENT, note_on[3])[0]
                mnoteoff = self.get_match_note(note_off[0], note_off[1], MIDI_NOTEOFF_EVENT, note_off[3])[0]
                new_note_selection.append((mnoteon, mnoteoff))
            self.selection = new_note_selection
        if self.paste_area:
            self.draw_all(self.paste_area)

    def draw_selection_motion(self, xpos, ypos, tick_seed, note_seed, selection, selected=False):
        note = self.ypos2noteval(int(ypos))
        tick = self.xpos2tick(xpos)

        note_diff = note - note_seed
        tick_diff = tick - tick_seed
        tick_diff = int(tick_diff / self.note_param["quant"]) * self.note_param["quant"]

        tick_min = tick_seed + tick_diff
        tick_max = tick_seed + tick_diff
        note_min = note_seed + note_diff
        note_max = note_seed + note_diff

        note_list = []
        for note_on, note_off in selection:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            note_list.append((diff_note_on, diff_note_off))
        self.draw_note_list(note_list, selected=selected)

    def draw_paste(self, xpos, ypos):
        if self.paste_area:
            self.draw_all(self.paste_area)
        note_max = max(self.to_paste, key=lambda x: x[0][3])[0][3]
        tick_min = min(self.to_paste, key=lambda x: x[0][0])[0][0]
        self.draw_selection_motion(xpos, ypos, tick_min, note_max, self.to_paste)

    def draw_move(self, xpos, ypos):
        if self.paste_area:
            self.draw_all(self.paste_area)
        self.draw_selection_motion(xpos, ypos,
                                   self.to_move["tick"], self.to_move["note"], self.to_move["selection"],
                                   selected=True)


    def draw_inc_left(self, xpos):
        if self.paste_area:
            self.draw_all(self.paste_area)
        tick = self.xpos2tick(xpos)
        tick = int(tick / self.note_param["quant"]) * self.note_param["quant"]
        inc_size = 0
        if tick < self.inc_note_left[1]:
            inc_size = self.inc_note_left[0] - tick
        else:
            return
        inc_note_list = []
        for note_on, note_off in self.inc_note_left[3]:
            note_on = (note_on[0] - inc_size,
                       note_on[1],
                       note_on[2],
                       note_on[3],
                       note_on[4])
            inc_note_list.append((note_on, note_off))
        self.draw_note_list(inc_note_list, selected=True)

    def draw_inc_right(self, xpos):
        if self.paste_area:
            self.draw_all(self.paste_area)
        tick = self.xpos2tick(xpos)
        tick = (int(tick / self.note_param["quant"]) + 1) * self.note_param["quant"]
        inc_size = 0
        if self.inc_note_right[1] < tick:
            inc_size = tick - self.inc_note_right[0]
        else:
            return
        inc_note_list = []
        for note_on, note_off in self.inc_note_right[3]:
            note_off = (note_off[0] + inc_size,
                        note_off[1],
                        note_off[2],
                        note_off[3],
                        note_off[4])
            inc_note_list.append((note_on, note_off))
        self.draw_note_list(inc_note_list, selected=True)
    def handle_motion(self, widget, event):
        if event.is_hint and self.rect_select_start:
            xmin = None
            xmax = None
            ymin = None
            ymax = None
            if event.x > self.rect_select_start[0]:
                xmax = event.x
                xmin = self.rect_select_start[0]
            else:
                xmax = self.rect_select_start[0]
                xmin = event.x
            if event.y > self.rect_select_start[1]:
                ymax = event.y
                ymin = self.rect_select_start[1]
            else:
                ymax = self.rect_select_start[1]
                ymin = event.y
            if self.rect_select:
                self.draw_all(self.rect_select)
            cr = self.window.cairo_create()
            # import cairo
            # cr.set_operator(cairo.OPERATOR_DEST_IN)
            # cr.set_operator(cairo.OPERATOR_DIFFERENCE)
            # cr.set_source_color(gtk.gdk.Color(255, 1, 1))
            self.rect_select = gtk.gdk.Rectangle(int(xmin), int(ymin), int(xmax - xmin), int(ymax - ymin))
            cr.set_source_rgba(0, 0, 0, 0.5)
            cr.rectangle(int(xmin), int(ymin), int(xmax - xmin), int(ymax - ymin))
            cr.fill()
        else:
            if self.paste_motion == True:
                self.draw_paste(event.x, event.y)
            elif self.to_move != None:
                self.draw_move(event.x, event.y)
            elif self.inc_note_left != None:
                self.draw_inc_left(event.x)
            elif self.inc_note_right != None:
                self.draw_inc_right(event.x)
            else:
                self.refresh_cursor(event.x, event.y, event.state)

    def handle_key_press(self, widget, event):
        self.grab_focus()
        keyname = gtk.gdk.keyval_name(event.keyval)
        # print "Key %s (%d) was pressed" % (keyname, event.keyval)
        if event.state & gtk.gdk.CONTROL_MASK:
            if self.paste_motion == False:
                if keyname == "v" and len(self.to_paste) > 0:
                    self.paste_motion = True
                if keyname == "c":
                    self.to_paste = []
                    for note_on, note_off in self.selection:
                        self.to_paste.append((note_on.get_event(), note_off.get_event()))
                if keyname == "x":
                    self.to_paste = []
                    for note_on, note_off in self.selection:
                        self.to_paste.append((note_on.get_event(), note_off.get_event()))
                    self.delete_selection()
        if event.keyval == gtk.keysyms.Delete or event.keyval == gtk.keysyms.BackSpace:
            self.delete_selection()

