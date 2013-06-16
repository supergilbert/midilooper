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

ADD_NOTE = 0
DEL_NOTE = 1

NO_LOGMODE   = 0
UNDO_LOGMODE = 1

history_list = []

class MsqNGWEventHdl(object):
    def __init__(self):
        self.note_val_on     = DEFAULT_NOTEON_VAL
        self.note_val_off    = 0
        self.wgt_mode    = NO_MODE
        self.paste_cache = None
        self.note_clipboard = None
        self.tmp_note_area  = None # Changer le nom de cette variable
        self.start_coo   = None
        self.select_area = None
        self.ctrl_click  = False
        self.data_cache    = None
        self.set_flags(gtk.CAN_DEFAULT)


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
        xmax = max_tick * self.ppqxsz / self.ppq
        xmin = min_tick * self.ppqxsz / self.ppq
        ymax = ((127 - min_note) * self.noteysz)
        ymin = ((127 - max_note) * self.noteysz)
        return gtk.gdk.Rectangle(xmin - 1, ymin - 1, xmax - xmin + 2, ymax - ymin + self.noteysz + 2)


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
        evon_maxtick = self.quantify_tick(minev_on_off[1][0] - self.note_resolution)
        if evon_maxtick != minev_on_off[1][0]:
            maxtick = ev_on_off_tick[0][0] + (evon_maxtick - minev_on_off[0][0])
        else:
            maxtick = ev_on_off_tick[0][0] + self.note_resolution
        return (INC_LEFT, ev_on_off_tick[0][0], maxtick)


    def rightinc_getdata(self, ev_on_off_tick):
        min_len, minev_on_off = self._getminlen_noteonoff()
        evoff_mintick = self.quantify_tick(minev_on_off[0][0] + self.note_resolution)
        if evoff_mintick != minev_on_off[0][0]:
            mintick = ev_on_off_tick[1][0] + (evoff_mintick - minev_on_off[1][0])
        else:
            mintick = minev_on_off[1][0] - self.note_resolution
        return (INC_RIGHT, ev_on_off_tick[1][0], mintick)


    def handle_button_press(self, widget, event):
        self.grab_focus()

        if self.wgt_mode == EDIT_MODE and event.button == 1:
            self.data_cache = self.gen_note_at_pos(event.x, event.y)
            if self.selection:
                selarea = self.get_notelist_area(self.selection)
            if self.data_cache:
                self.draw_notelist(self.data_cache, True)
                if self.selection:
                    self.selection = self.data_cache
                    self.draw_area(selarea)
            else:
                self.wgt_mode = NO_MODE

        elif self.wgt_mode == NO_MODE and event.button == 1:
            if self.selection:
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
                if not ev_on_off_tick:
                    new_selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                    if new_selection:
                        ev_on_off_tick = self.coo_under_notelist(event.x, event.y, new_selection)
                        if ev_on_off_tick:
                            selarea = self.get_notelist_area(self.selection)
                            self.selection = new_selection
                            self.draw_area(selarea)
            else:
                self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection) if self.selection else None

            if ev_on_off_tick:
                self.draw_notelist(self.selection, True)
                self.wgt_mode = PASTE_MODE
                self.paste_cache = self.selection
                self.data_cache = (ev_on_off_tick[0][0], ev_on_off_tick[0][3])
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
                        self.draw_area(selarea)

        elif event.button == 3:
            self.window.set_cursor(self.cursor_pencil)
            self.wgt_mode = EDIT_MODE

        elif self.wgt_mode == INC_MODE or (self.wgt_mode == NO_MODE and event.button == 2):
            self.wgt_mode = INC_MODE
            if self.selection:
                ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
                if ev_on_off_tick == None:
                    note_list = self.selection
                    self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                    self.draw_notelist(note_list, False)
                    if self.selection:
                        ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
            else:
                self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                if self.selection:
                    ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
                else:
                    ev_on_off_tick = None

            if ev_on_off_tick:
                if self._is_note_left_inc(ev_on_off_tick):
                    self.window.set_cursor(self.cursor_inc_l)
                    self.data_cache = self.leftinc_getdata(ev_on_off_tick)
                else:
                    self.window.set_cursor(self.cursor_inc_r)
                    self.data_cache = self.rightinc_getdata(ev_on_off_tick)
            else:
                self.window.set_cursor(self.current_cursor)
                self.wgt_mode = NO_MODE

        elif event.button == 2:
            self.wgt_mode = INC_MODE
            # if not self.selection:
            #     self.selection = self.get_notes(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))

        else:
            pass


    def xpos2tick(self, xpos):
        return int(xpos * self.ppq / self.ppqxsz)


    def ypos2noteval(self, ypos):
        return int(127 - (ypos / self.noteysz))


    def quantify_tick(self, tick):
        return int(tick / self.note_resolution) * self.note_resolution


    def is_in_notelist(self, noteon, noteoff, notelist):
        def _is_the_same_list(list1, list2):
            if len(list1) != len(list2):
                return False
            maxidx = len(list1)
            idx = 0
            while (idx < maxidx):
                if list1[idx] != list2[idx]:
                    return False
                idx += 1
            return True

        if notelist:
            for x_noteon, x_noteoff in notelist:
                if _is_the_same_list(x_noteon, noteon) and _is_the_same_list(x_noteoff, noteoff):
                    return True
        return False


    def note_collision(self, tick_on, tick_off, note, notelist, excl_list=None):
        # /!\ LOCK
        # Check if there is already a note at position
        # TODO exclusion list
        if tick_on < 0:
            return True
        for note_on, note_off in notelist:
            if note_on[3] == note and not self.is_in_notelist(note_on, note_off, excl_list):
                if tick_on >= note_on[0] and tick_on < note_off[0]:
                    return True
                elif tick_off > note_on[0] and tick_off <= note_off[0]:
                    return True
                elif note_on[0] >= tick_on and note_on[0] < tick_off:
                    return True
                elif note_off[0] > tick_on and note_off[0] <= tick_off:
                    return True
        return False


    def notelist_collision(self, notelist, excl_list=None):
        # /!\ LOCK
        track_notes = self.get_all_notes()
        for note_on, note_off in notelist:
            if self.note_collision(note_on[0], note_off[0], note_on[3], track_notes, excl_list):
                return True
        return False


    def add_note_on_off_list(self, notelist, logtype=UNDO_LOGMODE):
        # /!\ LOCK
        self.track.lock()
        for note_on, note_off in notelist:
            self.track.add_note_event(*note_on)
            self.track.add_note_event(*note_off)
        self.track.unlock()
        self.draw_notelist_area(notelist)
        if logtype == UNDO_LOGMODE:
            history_list.append((ADD_NOTE, notelist))


    def gen_note_at_pos(self, xpos, ypos):
        noteon_tick = self.quantify_tick(self.xpos2tick(xpos))
        noteoff_tick = noteon_tick + self.note_resolution
        note = self.ypos2noteval(int(ypos))

        if self.note_collision(noteon_tick, noteoff_tick, note, self.get_all_notes()):
            print "Can not add notes at this position"
            return None

        note_on = (noteon_tick,
                   self.chan_num,
                   MIDI_NOTEON_EVENT,
                   note,
                   self.note_val_on)
        note_off = (noteoff_tick,
                    self.chan_num,
                    MIDI_NOTEOFF_EVENT,
                    note,
                    self.note_val_off)
        return [(note_on, note_off)]

    def _get_diff_paste_note(self, xpos, ypos):
        # TODO (optimisation)
        tick_diff = self.quantify_tick(self.xpos2tick(xpos)) - self.data_cache[0]
        note_diff = self.ypos2noteval(int(ypos)) - self.data_cache[1]
        return tick_diff, note_diff


    def _paste_notes_at(self, xpos, ypos, note_list):
        tick_diff, note_diff = self._get_diff_paste_note(xpos, ypos)
        new_list = []
        for note_on, note_off in self.paste_cache:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            new_list.append((diff_note_on, diff_note_off))
        if self.notelist_collision(new_list):
            return None
        else:
            self.add_note_on_off_list(new_list)
            return new_list


    def delete_notes(self, notelist, logtype=UNDO_LOGMODE):
        self.track.lock()
        if self.track.is_handled():
            # tmp hack: handle of trash API (usage of node's deleted attribut)
            try:
                for evwr in self.track:
                    event = evwr.get_event()
                    while event and self.is_selected(event, notelist):
                        self.track.event2trash(evwr)
                        evwr.next()
                        event = evwr.get_event()
            except StopIteration: # bis: catching exception raised by the next function
                pass
        else:
            for evwr in self.track:
                event = evwr.get_event()
                while event and self.is_selected(event, notelist):
                    evwr._del_event()
                    event = evwr.get_event()
        self.track.unlock()

        selarea = self.get_notelist_area(notelist)
        if selarea:
            selarea.x = selarea.x - 2
            selarea.y = selarea.y - 2
            selarea.width = selarea.width + 4
            selarea.height = selarea.height + 4
            self.draw_area(selarea)

        if logtype == UNDO_LOGMODE:
            history_list.append((DEL_NOTE, notelist))


    def draw_notelist_area(self, notelist):
        selarea = self.get_notelist_area(notelist)
        selarea.x = selarea.x - 2
        selarea.y = selarea.y - 2
        selarea.width = selarea.width + 4
        selarea.height = selarea.height + 4
        self.draw_area(selarea)


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

        elif event.button == 1 and not self.wgt_mode == INC_MODE:
            if self.wgt_mode == EDIT_MODE:
                tick = self.xpos2tick(event.x)
                if tick < self.data_cache[0][0][0]:
                    tickdiff = self.get_inc_left_tickdiff(self.xpos2tick(event.x),
                                                          self.data_cache[0][0][0],
                                                          self.data_cache[0][0][0])
                    notelist = self.get_inc_left_notelist(tickdiff, self.data_cache)
                elif tick > self.data_cache[0][1][0]:
                    tickdiff = self.get_inc_right_tickdiff(self.xpos2tick(event.x),
                                                           self.data_cache[0][1][0],
                                                           self.data_cache[0][1][0])
                    notelist = self.get_inc_right_notelist(tickdiff, self.data_cache)
                else:
                    notelist = self.data_cache
                if self.notelist_collision(notelist):
                    print "Can not add note at this position"
                else:
                    self.add_note_on_off_list(notelist)
                    if self.selection:
                        tmp_notelist = self.selection
                        self.selection = None
                        self.draw_notelist_area(tmp_notelist)
                    self.selection = notelist
                    self.draw_notelist_area(self.selection)
                    self.data_cache = None

            elif self.wgt_mode == SELECT_MODE:
                if self.select_area:
                    # tmp: change draw_area to reversible effect (or a faster redraw)
                    self.draw_area(self.select_area)
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

            elif self.wgt_mode == PASTE_MODE:
                if  self.tmp_note_area:
                    self.draw_area(self.tmp_note_area)

                    if self.paste_cache:
                        if self.selection: self.delete_notes(self.selection)
                        new_notes = self._paste_notes_at(event.x, event.y, self.paste_cache)
                        if new_notes:
                            self.selection = new_notes
                            self.draw_notelist_area(self.selection)
                        else:
                            print "Can not move note at this position"
                else:
                    print "Can not move note at this position"
                self.data_cache = None
                self.wgt_mode = NO_MODE

        elif self.wgt_mode == INC_MODE:
            if self.tmp_note_area:
                self.draw_area(self.tmp_note_area)
                self.tmp_note_area = None
            self.window.set_cursor(self.current_cursor)
            if self.data_cache:
                if self.data_cache[0] == INC_LEFT:
                    tickdiff = self.get_inc_left_tickdiff(self.xpos2tick(event.x),
                                                          self.data_cache[1],
                                                          self.data_cache[2])
                    notelist = self.get_inc_left_notelist(tickdiff, self.selection)
                else:
                    tickdiff = self.get_inc_right_tickdiff(self.xpos2tick(event.x),
                                                           self.data_cache[1],
                                                           self.data_cache[2])
                    notelist = self.get_inc_right_notelist(tickdiff, self.selection)

                if self.notelist_collision(notelist, self.selection):
                    print "Can not increment note at this position"
                else:
                    self.delete_notes(self.selection)
                    self.selection = notelist
                    self.add_note_on_off_list(notelist)
                self.data_cache = None
            self.wgt_mode = NO_MODE


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
                                 ymax - ymin + self.noteysz + 4)


    def gen_notelist_at(self, tick_diff, note_diff, note_list):
        tick_min = note_list[0][0][0] + tick_diff
        tick_max = note_list[0][0][0] + tick_diff
        note_min = note_list[0][0][3] + note_diff
        note_max = note_list[0][0][3] + note_diff
        diff_note_list = []
        for note_on, note_off in note_list:
            diff_note_on = self.diff_note(note_on, tick_diff, note_diff)
            diff_note_off = self.diff_note(note_off, tick_diff, note_diff)
            diff_note_list.append((diff_note_on, diff_note_off))
            # if tick_max < diff_note_off[0]:
            #     tick_max = diff_note_off[0]
            # if tick_min > diff_note_on[0]:
            #     tick_min = diff_note_on[0]
            # if note_min > diff_note_on[3]:
            #     note_min = diff_note_on[3]
            # if note_max < diff_note_on[3]:
            #     note_max = diff_note_on[3]
            #     note_max = diff_note_on[3]
        return diff_note_list

        # xmin = self.tick2xpos(tick_min)
        # xmax = self.tick2xpos(tick_max)
        # ymin = self.note2ypos(note_max)
        # ymax = self.note2ypos(note_min)
        # return gtk.gdk.Rectangle(xmin - 2,
        #                          ymin - 2,
        #                          xmax - xmin + 4,
        #                          ymax - ymin + self.noteysz + 4)


    def draw_paste_at(self, xpos, ypos, note_list, selected, excl_list=None):
        tick_diff, note_diff = self._get_diff_paste_note(xpos, ypos)

        if self.tmp_note_area:
            self.draw_area(self.tmp_note_area)
        diff_note_list = self.gen_notelist_at(tick_diff, note_diff, note_list)
        if self.notelist_collision(diff_note_list, excl_list):
             self.tmp_note_area = None
             return
        self.draw_notelist(diff_note_list)
        self.tmp_note_area = self.get_notelist_area(diff_note_list)
        # self.tmp_note_area = self.draw_notelist_at(tick_diff, note_diff, note_list, selected)


    def _is_note_left_inc(self, ev_on_off_tick):
        len1 = ev_on_off_tick[2] - ev_on_off_tick[0][0]
        len2 = ev_on_off_tick[1][0] - ev_on_off_tick[2]
        if len1 < len2:
            return True
        else:
            return False


    def get_inc_left_notelist(self, tick_diff, note_list):
        inc_note_list = []
        for note_on, note_off in note_list:
            note_on = (note_on[0] + tick_diff,
                       note_on[1],
                       note_on[2],
                       note_on[3],
                       note_on[4])
            inc_note_list.append((note_on, note_off))
        return inc_note_list


    def get_inc_left_tickdiff(self, tick, current_noteon_tick, maxtick):
        qtick = self.quantify_tick(tick)
        if maxtick < qtick:
            tick_diff = maxtick - current_noteon_tick
        else:
            tick_diff = qtick - current_noteon_tick
        return tick_diff


    def draw_inc_left(self, tick, note_list, current_noteon_tick, maxtick, excl_list=None):
        tick_diff = self.get_inc_left_tickdiff(tick, current_noteon_tick, maxtick)
        inc_note_list = self.get_inc_left_notelist(tick_diff, note_list)
        if self.notelist_collision(inc_note_list, excl_list):
            return None
        return self.draw_notelist(inc_note_list, True)


    def get_inc_right_notelist(self, tick_diff, note_list):
        inc_note_list = []
        for note_on, note_off in note_list:
            note_off = (note_off[0] + tick_diff,
                        note_off[1],
                        note_off[2],
                        note_off[3],
                        note_off[4])
            inc_note_list.append((note_on, note_off))
        return inc_note_list


    def get_inc_right_tickdiff(self, tick, current_noteoff_tick, mintick):
        qtick = self.quantify_tick(tick + self.note_resolution)
        if mintick > qtick:
            tick_diff = mintick - current_noteoff_tick
        else:
            tick_diff = qtick - current_noteoff_tick
        return tick_diff


    def draw_inc_right(self, tick, note_list, current_noteoff_tick, mintick, excl_list=None):
        tick_diff = self.get_inc_right_tickdiff(tick, current_noteoff_tick, mintick)
        inc_note_list = self.get_inc_right_notelist(tick_diff, note_list)
        if self.notelist_collision(inc_note_list, excl_list):
            return None
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
                self.draw_area(self.select_area)
            self.select_area = gtk.gdk.Rectangle(int(xmin) - 2,
                                                 int(ymin) - 2,
                                                 int(xmax - xmin) + 4,
                                                 int(ymax - ymin) + 4)
            cr = self.window.cairo_create()
            cr.set_source_rgb(0, 0, 0)
            cr.set_line_width(2)
            cr.rectangle(int(xmin), int(ymin), int(xmax - xmin), int(ymax - ymin))
            cr.stroke()

        elif self.wgt_mode == EDIT_MODE and self.data_cache:
            if self.tmp_note_area:
                self.draw_area(self.tmp_note_area)
                self.tmp_note_area = None
            tick = self.xpos2tick(event.x)
            if tick < self.data_cache[0][0][0]:
                self.tmp_note_area = self.draw_inc_left(tick,
                                                        self.data_cache,
                                                        self.data_cache[0][0][0],
                                                        self.data_cache[0][0][0])
            elif tick >= self.data_cache[0][1][0]:
                self.tmp_note_area = self.draw_inc_right(tick,
                                                         self.data_cache,
                                                         self.data_cache[0][1][0],
                                                         self.data_cache[0][1][0])
            else:
                self.draw_notelist(self.data_cache)

        elif self.wgt_mode == PASTE_MODE:
            self.draw_paste_at(event.x, event.y, self.paste_cache, True, self.selection)

        elif self.wgt_mode == INC_MODE and self.selection:
            if self.data_cache:
                if self.tmp_note_area:
                    self.draw_area(self.tmp_note_area)
                    self.tmp_note_area = None
                tick = self.xpos2tick(event.x)
                if self.data_cache[0] == INC_LEFT:
                    self.tmp_note_area = self.draw_inc_left(tick,
                                                            self.selection,
                                                            self.data_cache[1],
                                                            self.data_cache[2],
                                                            self.selection)
                else:
                    self.tmp_note_area = self.draw_inc_right(tick,
                                                             self.selection,
                                                             self.data_cache[1],
                                                             self.data_cache[2],
                                                             self.selection)
            else:
                self.set_inc_cursor(event.x, event.y)

        elif self.wgt_mode == NO_MODE and self.selection:
            ev_on_off_tick = self.coo_under_notelist(event.x, event.y, self.selection)
            if ev_on_off_tick:
                self.window.set_cursor(self.cursor_move)
            else:
                self.window.set_cursor(self.current_cursor)


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


    def get_paste_data(self, notelist):
        tick_min = min(notelist, key=lambda x: x[0][0])[0][0]
        note_max = max(notelist, key=lambda x: x[0][3])[0][3]
        return (tick_min, note_max)


    def handle_key_release(self, widget, event):
        self.grab_focus()
        if self.wgt_mode == PASTE_MODE:
            pass
        elif self.wgt_mode == INC_MODE:
            if not self.data_cache:
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
                        self.data_cache = self.get_paste_data(self.paste_cache)
                        if self.selection:
                            note_list = self.selection
                            self.selection = None
                            self.draw_notelist_area(note_list) # clear last selection
                        pointer = self.window.get_pointer()
                        self.draw_paste_at(float(pointer[0]), float(pointer[1]), self.paste_cache, True)
                        self.wgt_mode = PASTE_MODE
                if keyname == "z":
                    self.undo()


    def undo(self):
        if len(history_list):
            ev = history_list.pop()
            if ev[0] == ADD_NOTE:
                self.delete_notes(ev[1], NO_LOGMODE)
            elif ev[0] == DEL_NOTE:
                self.add_note_on_off_list(ev[1], NO_LOGMODE)
        else:
            print "No more event in history"



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
