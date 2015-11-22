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

import pygtk
pygtk.require("2.0")
import gtk

if gtk.pygtk_version < (2, 8):
    print "PyGtk 2.8 or later required"
    raise SystemExit

from wgttools import *

NO_MODE     = 0
EDIT_MODE   = 1
SELECT_MODE = 2
PASTE_MODE  = 3
INC_MODE    = 4

INC_LEFT  = 0
INC_RIGHT = 1

ADD_NOTE = 0
DEL_NOTE = 1

# other mode ???
NO_LOGMODE   = 0
UNDO_LOGMODE = 1
HISTORY_MARK = 4242

# Size of "note off" tick decrementation
NOTEOFF_DEC = 1

DEFAULT_NOTEON_VAL  = 64
DEFAULT_NOTEOFF_VAL = 0


class MsqNGWEventHdl(Xpos2Tick, Ypos2Note):
    def set_history_mark(self):
        self.history_list.append(HISTORY_MARK)


    def __init__(self):
        self.wgt_mode    = NO_MODE
        self.paste_cache = None
        self.note_clipboard = None
        self.tmp_note_area  = None # Changer le nom de cette variable
        self.start_coo   = None
        self.select_area = None
        self.ctrl_click  = False
        self.data_cache    = None
        self.set_flags(gtk.CAN_DEFAULT)
        self.history_list = []


    def get_notes_evwr(self, rectangle):
        tick_min = self.xpos2tick(rectangle.x)
        tick_max = self.xpos2tick(rectangle.x + rectangle.width)
        note_max = self.ypos2note(rectangle.y)
        note_min = self.ypos2note(rectangle.y + rectangle.height)
        if tick_min < 0:
            tick_min = 0
        return self.setting.track.sel_noteonoff_evwr(self.setting.chan_num,
                                                     tick_min,
                                                     tick_max,
                                                     note_min,
                                                     note_max)


    def get_notelist_area(self, note_list):
        if len(note_list) == 0:
            return None
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
        xmax = self.tick2xpos(max_tick)
        xmin = self.tick2xpos(min_tick)
        ymax = self.note2ypos(min_note)
        ymin = self.note2ypos(max_note)
        return gtk.gdk.Rectangle(xmin - 1, ymin - 1, xmax - xmin + 2, ymax - ymin + self.setting.noteysz + 2)


    def unset_selection(self):
        if self.selection:
            tmp_notelist = evwr_to_repr_list(self.selection)
            self.selection = None
            self.draw_notelist_area(tmp_notelist)

    def redraw_selection(self):
        if self.selection:
            tmp_notelist = evwr_to_repr_list(self.selection)
            self.draw_notelist_area(tmp_notelist)

    def _getminlen_noteonoff(self):
        repr_list = evwr_to_repr_list(self.selection)
        min_len = repr_list[0][1][0] - repr_list[0][0][0]
        minev_on_off = repr_list[0]
        for note_on, note_off in repr_list:
            note_len = note_off[0] - note_on[0]
            if note_len < min_len:
                minev_on_off = (note_on, note_off)
                min_len = note_len
        return min_len, minev_on_off


    def leftinc_getdata(self, ev_on_off_tick):
        min_len, minev_on_off = self._getminlen_noteonoff()
        evon_maxtick = self.setting.quantify_tick(minev_on_off[1][0])
        if evon_maxtick != minev_on_off[1][0]:
            maxtick = ev_on_off_tick[0][0] + (evon_maxtick - minev_on_off[0][0])
        else:
            maxtick = ev_on_off_tick[0][0] + self.setting.tick_res
        return (INC_LEFT, ev_on_off_tick[0][0], maxtick)


    def rightinc_getdata(self, ev_on_off_tick):
        min_len, minev_on_off = self._getminlen_noteonoff()
        evoff_mintick = self.setting.quantify_tick(minev_on_off[0][0] + self.setting.tick_res)
        if evoff_mintick != minev_on_off[0][0]:
            mintick = ev_on_off_tick[1][0] + (evoff_mintick - minev_on_off[1][0])
        else:
            mintick = minev_on_off[1][0] - self.setting.tick_res
        return (INC_RIGHT, ev_on_off_tick[1][0], mintick)


    def gen_note_at_pos(self, xpos, ypos):
        noteon_tick = self.setting.quantify_tick(self.xpos2tick(xpos))
        noteoff_tick = noteon_tick + self.setting.tick_res - NOTEOFF_DEC
        note = self.ypos2note(int(ypos))

        if note_collision(noteon_tick,
                          noteoff_tick,
                          self.setting.chan_num,
                          note,
                          self.setting.track.getall_noteonoff(self.setting.chan_num)):
            print "Can not add notes at this position"
            return None

        note_on = (noteon_tick,
                   self.setting.chan_num,
                   MIDI_NOTEON_EVENT,
                   note,
                   int(self.setting.note_valadj.get_value()))
        note_off = (noteoff_tick,
                    self.setting.chan_num,
                    MIDI_NOTEOFF_EVENT,
                    note,
                    DEFAULT_NOTEOFF_VAL)
        return [(note_on, note_off)]


    def handle_button_press(self, widget, event):
        self.grab_focus()

        if self.wgt_mode == EDIT_MODE and event.button == 1:
            self.data_cache = self.gen_note_at_pos(event.x, event.y)
            if self.data_cache:
                self.draw_notelist(self.data_cache, True)
            else:
                self.wgt_mode = NO_MODE

        elif self.wgt_mode == NO_MODE and event.button == 1:
            if self.selection:
                ev_on_off_tick = self.coo_under_notelist(event.x,
                                                         event.y,
                                                         evwr_to_repr_list(self.selection))
                if ev_on_off_tick:
                    self.start_coo = (event.x, event.y)
                    if event.state & gtk.gdk.CONTROL_MASK:
                        self.ctrl_click = True
                        self.wgt_mode = SELECT_MODE
                    else:
                        self.wgt_mode = PASTE_MODE
                        self.paste_cache = evwr_to_repr_list(self.selection)
                        self.data_cache = (self.setting.quantify_tick(ev_on_off_tick[2]), ev_on_off_tick[0][3])
                else:
                    if event.state & gtk.gdk.CONTROL_MASK:
                        self.ctrl_click = True
                    else:
                        self.clear_selection()
                    self.start_coo = (event.x, event.y)
                    self.wgt_mode = SELECT_MODE
            else:
                if event.state & gtk.gdk.CONTROL_MASK:
                    self.ctrl_click = True
                self.start_coo = (event.x, event.y)
                self.wgt_mode = SELECT_MODE

        elif self.wgt_mode == NO_MODE and event.button == 3:
            self.window.set_cursor(cursor_pencil)
            self.wgt_mode = EDIT_MODE

        elif self.wgt_mode == INC_MODE or (self.wgt_mode == NO_MODE and event.button == 2):
            self.wgt_mode = INC_MODE
            ev_on_off_tick = None
            if self.selection:
                ev_on_off_tick = self.coo_under_notelist(event.x,
                                                         event.y,
                                                         evwr_to_repr_list(self.selection))
                if ev_on_off_tick == None: # case of a click other note from selection
                    note_list = evwr_to_repr_list(self.selection)
                    self.selection = self.get_notes_evwr(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                    self.draw_notelist(note_list, False)
                    if self.selection:
                        ev_on_off_tick = self.coo_under_notelist(event.x,
                                                                 event.y,
                                                                 evwr_to_repr_list(self.selection))
            else:
                self.selection = self.get_notes_evwr(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))
                if self.selection:
                    ev_on_off_tick = self.coo_under_notelist(event.x,
                                                             event.y,
                                                             evwr_to_repr_list(self.selection))

            if ev_on_off_tick:
                if self._is_note_left_inc(ev_on_off_tick):
                    self.window.set_cursor(cursor_inc_l)
                    self.data_cache = self.leftinc_getdata(ev_on_off_tick)
                else:
                    self.window.set_cursor(cursor_inc_r)
                    self.data_cache = self.rightinc_getdata(ev_on_off_tick)
            else:
                self.window.set_cursor(current_cursor)
                self.wgt_mode = NO_MODE

        elif self.wgt_mode == NO_MODE and event.button == 2:
            self.wgt_mode = INC_MODE
            # if not self.selection:
            #     self.selection = self.get_notes_evwr(gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1))

        else:
            pass


    def notelist_collision(self, notelist, excl_list=None):
        track_notes = self.setting.track.getall_noteonoff(self.setting.chan_num)
        for note_on, note_off in notelist:
            if note_on[0] < 0 or note_off[0] < 0:
                return True
            if note_collision(note_on[0],
                              note_off[0],
                              note_on[1],
                              note_on[3],
                              track_notes,
                              excl_list):
                return True
        return False


    def get_noteonoff_from_list(self, evwrlist):
        # Based on the fact that note was append ordered
        ret_list = []
        idx = 0
        idx_end = len(evwrlist)
        while (idx + 1 < idx_end):
            ret_list.append((evwrlist[idx], evwrlist[idx + 1]))
            idx += 2
        return ret_list

    def add_note_on_off_list(self, notelist, logtype=UNDO_LOGMODE):
        note_to_add = []
        for note_on, note_off in notelist:
            note_to_add.append(note_on)
            note_to_add.append(note_off)
        evwr_list = self.setting.track.add_evrepr_list(note_to_add)

        ret_list = self.get_noteonoff_from_list(evwr_list)

        if logtype == UNDO_LOGMODE:
            self.history_list.append((ADD_NOTE, notelist))
        if len(ret_list):
            return ret_list
        else:
            return None


    def _get_diff_paste_note(self, xpos, ypos):
        # TODO (optimisation)
        tick_diff = self.setting.quantify_tick(self.xpos2tick(xpos)) - self.data_cache[0]
        note_diff = self.ypos2note(int(ypos)) - self.data_cache[1]
        return tick_diff, note_diff


    def gen_notelist_at(self, tick_diff, note_diff, note_list):
        for note_on, note_off in note_list:
            if note_on[0] + tick_diff < 0:
                tick_diff = 0 - note_on[0]
            if note_off[0] + tick_diff < 0:
                tick_diff = 0 - note_off[0]
            if note_on[3] + note_diff > 127:
                note_diff = 127 - note_on[3]
            elif note_on[3] + note_diff < 0:
                note_diff = 0 - note_on[3]
        tick_min = note_list[0][0][0] + tick_diff
        tick_max = note_list[0][0][0] + tick_diff
        note_min = note_list[0][0][3] + note_diff
        note_max = note_list[0][0][3] + note_diff
        diff_note_list = []
        for note_on, note_off in note_list:
            diff_note_on = self.copy_note_at(note_on, tick_diff, note_diff)
            diff_note_off = self.copy_note_at(note_off, tick_diff, note_diff)
            diff_note_list.append((diff_note_on, diff_note_off))
        return diff_note_list


    def _paste_notes_at(self, xpos, ypos, note_list):
        tick_diff, note_diff = self._get_diff_paste_note(xpos, ypos)
        new_list = self.gen_notelist_at(tick_diff, note_diff, note_list)
        if self.notelist_collision(new_list):
            return None
        else:
            return self.add_note_on_off_list(new_list)


    def delete_notes(self, notelist, logtype=UNDO_LOGMODE):
        notelist_repr = []
        notes_to_del = []
        for note_on, note_off in notelist:
            notes_to_del.append(note_on)
            notes_to_del.append(note_off)
            notelist_repr.append((note_on.get_event(),
                                  note_off.get_event()))
        selarea = self.get_notelist_area(notelist_repr)
        self.setting.track._delete_evwr_list(notes_to_del)
        if selarea:
            selarea.x = selarea.x - 2
            selarea.y = selarea.y - 2
            selarea.width = selarea.width + 4
            selarea.height = selarea.height + 4
            self.draw_area(selarea)

        if logtype == UNDO_LOGMODE:
            self.history_list.append((DEL_NOTE, notelist_repr))

    def delete_selection(self):
        notelist = self.selection
        self.selection = None
        self.delete_notes(notelist)


    def draw_notelist_area(self, notelist):
        selarea = self.get_notelist_area(notelist)
        selarea.x = selarea.x - 2
        selarea.y = selarea.y - 2
        selarea.width = selarea.width + 4
        selarea.height = selarea.height + 4
        self.draw_area(selarea)


    def handle_button_release(self, widget, event):
        if event.button == 3:
            self.window.set_cursor(current_cursor)
            self.wgt_mode = NO_MODE

        elif event.button == 1 and self.wgt_mode != INC_MODE:

            # Notes write
            if self.wgt_mode == EDIT_MODE and self.data_cache:
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
                    new_selection = self.add_note_on_off_list(notelist)
                    self.set_history_mark()
                    if new_selection:
                        if self.selection:
                            tmp_notelist = evwr_to_repr_list(self.selection)
                            self.selection = new_selection
                            self.draw_notelist_area(tmp_notelist)
                        else:
                            self.selection = new_selection
                        tmp_notelist = evwr_to_repr_list(self.selection)
                        self.draw_notelist_area(tmp_notelist)
                    self.data_cache = None

            elif self.wgt_mode == SELECT_MODE:
                if self.select_area:
                    self.buffer_refresh_area(self.select_area)
                else:
                    self.select_area = gtk.gdk.Rectangle(int(event.x), int(event.y), 1, 1)
                if self.ctrl_click:
                    new_sel = self.get_notes_evwr(self.select_area)
                    if new_sel and self.selection:
                        final_sel = []
                        for noteonoff in new_sel:
                            noteon, noteoff = evwr_to_repr_list([noteonoff])[0]
                            if not is_in_notelist(noteon, noteoff, evwr_to_repr_list(self.selection)):
                                final_sel.append(noteonoff)
                        for noteonoff in self.selection:
                            noteon, noteoff = evwr_to_repr_list([noteonoff])[0]
                            if not is_in_notelist(noteon, noteoff, evwr_to_repr_list(new_sel)):
                                final_sel.append(noteonoff)
                        self.selection = final_sel
                    else:
                        self.selection = new_sel
                    self.draw_notelist_area(evwr_to_repr_list(new_sel))
                else:
                    self.selection = self.get_notes_evwr(self.select_area)
                self.ctrl_click  = False
                self.select_area = None
                self.start_coo   = None
                if self.selection: # render new selection
                    self.draw_notelist_area(evwr_to_repr_list(self.selection))
                self.wgt_mode = NO_MODE
                self.setting.value_widget.unset_selection()

            elif self.wgt_mode == PASTE_MODE:
                if  self.tmp_note_area:
                    self.draw_area(self.tmp_note_area)

                    if self.paste_cache:
                        if self.selection: self.delete_selection()
                        new_notes = self._paste_notes_at(event.x, event.y, self.paste_cache)
                        self.set_history_mark()
                        if new_notes:
                            self.selection = new_notes
                            self.draw_notelist_area(evwr_to_repr_list(new_notes))
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
            self.window.set_cursor(current_cursor)
            if self.data_cache:
                repr_list = evwr_to_repr_list(self.selection)
                if self.data_cache[0] == INC_LEFT:
                    tickdiff = self.get_inc_left_tickdiff(self.xpos2tick(event.x),
                                                          self.data_cache[1],
                                                          self.data_cache[2])
                    notelist = self.get_inc_left_notelist(tickdiff, repr_list)
                else:
                    tickdiff = self.get_inc_right_tickdiff(self.xpos2tick(event.x),
                                                           self.data_cache[1],
                                                           self.data_cache[2])
                    notelist = self.get_inc_right_notelist(tickdiff, repr_list)

                if self.notelist_collision(notelist, repr_list):
                    print "Can not increment note at this position"
                else:
                    self.delete_selection()
                    self.selection = self.add_note_on_off_list(notelist)
                    self.set_history_mark()
                    self.draw_notelist_area(notelist)
                self.data_cache = None
                self.wgt_mode = NO_MODE


    def coo_under_notelist(self, xpos, ypos, notelist):
        note = self.ypos2note(ypos)
        tick = self.xpos2tick(xpos)
        for ev_on, ev_off in notelist:
            note_on = ev_on[3]
            if note == note_on:
                tick_on = ev_on[0]
                tick_off = ev_off[0]
                if tick >= tick_on and tick <= tick_off:
                    return ev_on, ev_off, tick
        return None


    def copy_note_at(self, note, tick_diff, note_diff):
        note = (note[0] + tick_diff,
                self.setting.chan_num,
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
            self.draw_note(self.window, note_on, note_off, selected)
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
                                 ymax - ymin + self.setting.noteysz + 4)


    def draw_paste_at(self, xpos, ypos, note_list, selected, excl_list=None):
        tick_diff, note_diff = self._get_diff_paste_note(xpos, ypos)

        if self.tmp_note_area:
            self.buffer_refresh_area(self.tmp_note_area)
            pass

        diff_note_list = self.gen_notelist_at(tick_diff, note_diff, note_list)
        if self.notelist_collision(diff_note_list, excl_list):
             self.tmp_note_area = None
             return

        self.tmp_note_area = self.draw_notelist(diff_note_list)


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
        qtick = self.setting.quantify_tick(tick)
        if maxtick <= qtick:
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
        qtick = self.setting.quantify_tick(tick + self.setting.tick_res) - NOTEOFF_DEC
        if mintick >= qtick:
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
        if self.selection:
            ev_on_off_tick = self.coo_under_notelist(xpos, ypos, evwr_to_repr_list(self.selection))
        else:
            ev_on_off_tick = None
        if ev_on_off_tick:
            if self._is_note_left_inc(ev_on_off_tick):
                self.window.set_cursor(cursor_inc_l)
            else:
                self.window.set_cursor(cursor_inc_r)
        else:
            self.window.set_cursor(cursor_inc)


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
                self.buffer_refresh_area(self.select_area)
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
                self.buffer_refresh_area(self.tmp_note_area)
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
            self.draw_paste_at(event.x, event.y, self.paste_cache, True, self.paste_cache)

        elif self.wgt_mode == INC_MODE and self.selection:
            if self.data_cache:
                if self.tmp_note_area:
                    self.buffer_refresh_area(self.tmp_note_area)
                    self.tmp_note_area = None
                tick = self.xpos2tick(event.x)
                note_list = evwr_to_repr_list(self.selection)
                if self.data_cache[0] == INC_LEFT:
                    self.tmp_note_area = self.draw_inc_left(tick,
                                                            note_list,
                                                            self.data_cache[1],
                                                            self.data_cache[2],
                                                            note_list)
                else:
                    self.tmp_note_area = self.draw_inc_right(tick,
                                                             note_list,
                                                             self.data_cache[1],
                                                             self.data_cache[2],
                                                             note_list)
            else:
                self.set_inc_cursor(event.x, event.y)

        elif self.wgt_mode == NO_MODE and self.selection:
            ev_on_off_tick = self.coo_under_notelist(event.x,
                                                     event.y,
                                                     evwr_to_repr_list(self.selection))
            if ev_on_off_tick:
                self.window.set_cursor(cursor_move)
            else:
                self.window.set_cursor(current_cursor)


    def handle_key_press(self, widget, event):
        if event.keyval == gtk.keysyms.Alt_L or event.keyval == gtk.keysyms.Alt_R:
            self.wgt_mode = EDIT_MODE
            self.window.set_cursor(cursor_pencil)
        elif event.keyval == gtk.keysyms.Shift_L or event.keyval == gtk.keysyms.Shift_R:
            self.wgt_mode = INC_MODE
            xpos, ypos, mod = self.window.get_pointer()
            self.set_inc_cursor(xpos, ypos)
        elif event.keyval == gtk.keysyms.space:
            if self.setting.sequencer.isrunning():
                self.setting.sequencer.stop()
            else:
                self.setting.sequencer.start()


    def get_paste_data(self, notelist):
        tick_min = min(notelist, key=lambda x: x[0][0])[0][0]
        note_max = max(notelist, key=lambda x: x[0][3])[0][3]
        return (tick_min, note_max)

    def clear_selection(self):
        if self.selection:
            note_list = evwr_to_repr_list(self.selection)
            self.selection = None
            self.draw_notelist_area(note_list)

    def handle_key_release(self, widget, event):
        if self.wgt_mode == PASTE_MODE:
            pass
        elif self.wgt_mode == INC_MODE:
            if not self.data_cache:
                self.wgt_mode = NO_MODE
                self.window.set_cursor(current_cursor)
        else:
            self.wgt_mode = NO_MODE
            self.window.set_cursor(current_cursor)
            if event.state & gtk.gdk.CONTROL_MASK:
                keyname = gtk.gdk.keyval_name(event.keyval)
                if self.selection:
                    if keyname == "x":
                        self.note_clipboard = evwr_to_repr_list(self.selection)
                        self.delete_selection()
                        self.set_history_mark()
                    elif keyname == "c":
                        self.note_clipboard = evwr_to_repr_list(self.selection)
                if keyname == "v":
                    if self.note_clipboard:
                        self.paste_cache = self.note_clipboard
                        self.data_cache = self.get_paste_data(self.paste_cache)
                        self.clear_selection() # clear last selection
                        pointer = self.window.get_pointer()
                        self.draw_paste_at(float(pointer[0]),
                                           float(pointer[1]),
                                           self.paste_cache,
                                           True)
                        self.wgt_mode = PASTE_MODE
                if keyname == "z":
                    self.undo()
                if keyname == "a":
                    self.clear_selection() # clear last selection
                    self.selection = self.setting.track.getall_noteonoff_evwr(self.setting.chan_num)
                    self.draw_notelist_area(evwr_to_repr_list(self.selection))
            else:
                if self.selection and (event.keyval == gtk.keysyms.Delete or
                                                    event.keyval == gtk.keysyms.BackSpace):
                    self.delete_selection()
                    self.set_history_mark()


    def _delete_noterepr_list(self, noterepr_list):
        evrepr_list = []
        for noteon, noteoff in noterepr_list:
            evrepr_list.append(noteon)
            evrepr_list.append(noteoff)
        evwr_list = self.setting.track._get_evwr_list(evrepr_list)
        self.setting.track._delete_evwr_list(evwr_list)
        self.draw_notelist_area(noterepr_list)


    def undo(self):
        if len(self.history_list):
            if HISTORY_MARK == self.history_list.pop():
                ev = True
                while len(self.history_list) and self.history_list[-1] != HISTORY_MARK:
                    ev = self.history_list.pop()
                    self.clear_selection()
                    if ev[0] == ADD_NOTE:
                        self._delete_noterepr_list(ev[1])
                    elif ev[0] == DEL_NOTE:
                        self.selection = self.add_note_on_off_list(ev[1], NO_LOGMODE)
                        self.draw_notelist_area(ev[1])
        else:
            print "No more event in history"



    def realize_noteonoff_handler(self):
        self.connect("button_press_event", self.handle_button_press)
        self.connect("button_release_event", self.handle_button_release)
        self.connect("key_press_event", self.handle_key_press)
        self.connect("key_release_event", self.handle_key_release)
        self.connect("motion_notify_event", self.handle_motion)
        cursor_pencil = gtk.gdk.Cursor(gtk.gdk.PENCIL)
        cursor_inc = gtk.gdk.Cursor(gtk.gdk.SB_H_DOUBLE_ARROW)
        cursor_inc_l = gtk.gdk.Cursor(gtk.gdk.SB_LEFT_ARROW)
        cursor_inc_r = gtk.gdk.Cursor(gtk.gdk.SB_RIGHT_ARROW)
        cursor_move = gtk.gdk.Cursor(gtk.gdk.FLEUR)
        current_cursor = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)
        self.window.set_cursor(current_cursor)
        self.set_can_focus(True)
