# Copyright 2012-2016 Gilbert Romer

# This file is part of midilooper.

# midilooper is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# midilooper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU Gneneral Public License
# along with midilooper.  If not, see <http://www.gnu.org/licenses/>.

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk


cursor_pencil  = Gdk.Cursor.new(Gdk.CursorType.PENCIL)
cursor_inc     = Gdk.Cursor.new(Gdk.CursorType.SB_H_DOUBLE_ARROW)
cursor_inc_l   = Gdk.Cursor.new(Gdk.CursorType.SB_LEFT_ARROW)
cursor_inc_r   = Gdk.Cursor.new(Gdk.CursorType.SB_RIGHT_ARROW)
cursor_move    = Gdk.Cursor.new(Gdk.CursorType.FLEUR)
current_cursor = Gdk.Cursor.new(Gdk.CursorType.LEFT_PTR)

MIDI_NOTEOFF_EVENT = 0x8
MIDI_NOTEON_EVENT  = 0x9
MIDI_CTRL_EVENT    = 0xB
MIDI_PITCH_EVENT   = 0xE

class DrawAllArea(object):
    def draw_all(self):
        window = self.get_window()
        if window:
            self.do_draw(Gdk.cairo_create(window))

class Xpos2Tick(DrawAllArea):
    def xpos2tick(self, xpos):
        xadj_pos = xpos + self.xadj
        tick = int(xadj_pos * self.setting.getppq() / self.setting.qnxsz)
        return tick if tick >= 0 else 0

    def tick2xpos(self, tick):
        return int((tick * self.setting.qnxsz / self.setting.getppq()) - self.xadj)

    def hadj_val_cb(self, adj):
        self.xadj = int(self.setting.hadj.get_value())
        self.draw_all()

    def __init__(self):
        self.xadj = 0
        self.setting.hadj.connect("value-changed", self.hadj_val_cb)

class Ypos2Note(DrawAllArea):
    def ypos2note(self, ypos):
        yadj_pos = ypos + self.yadj
        note = 127 - int(yadj_pos / self.setting.noteysz)
        if note < 0:
            note = 0
        return note

    def note2ypos(self, note):
        return int(((127 - note) * self.setting.noteysz) - self.yadj)

    def vadj_val_cb(self, adj):
        self.yadj = int(self.setting.vadj.get_value())
        self.draw_all()

    def __init__(self):
        self.yadj = int(self.setting.vadj.get_value())
        self.setting.vadj.connect("value-changed", self.vadj_val_cb)

def evwr_to_repr_list(noteonoff_list):
    repr_list = []
    for noteon, noteoff in noteonoff_list:
        repr_list.append((noteon.get_event(), noteoff.get_event()))
    return repr_list


def is_the_same_list(list1, list2):
    if len(list1) != len(list2):
        return False
    maxidx = len(list1)
    idx = 0
    while (idx < maxidx):
        if list1[idx] != list2[idx]:
            return False
        idx += 1
    return True

def evrepr_is_in_notelist(evrepr, notelist):
    if notelist:
        for x_noteon, x_noteoff in notelist:
            if is_the_same_list(x_noteon, evrepr):
                return True
            if is_the_same_list(x_noteoff, evrepr):
                return True
    return False

def is_in_notelist(noteon, noteoff, notelist):
    if notelist:
        for x_noteon, x_noteoff in notelist:
            if is_the_same_list(x_noteon, noteon) and is_the_same_list(x_noteoff, noteoff):
                return True
    return False

def note_collision(tick_on, tick_off, channel, note, notelist, excl_list=None):
    for note_on, note_off in notelist:
        if note_on[1] == channel and note_on[3] == note and not is_in_notelist(note_on, note_off, excl_list):
                if tick_on >= note_on[0] and tick_on < note_off[0]:
                    return (note_on, note_off)
                elif tick_off > note_on[0] and tick_off <= note_off[0]:
                    return (note_on, note_off)
                elif note_on[0] >= tick_on and note_on[0] < tick_off:
                    return (note_on, note_off)
                elif note_off[0] > tick_on and note_off[0] <= tick_off:
                    return (note_on, note_off)
    return None

__all__ = ["Xpos2Tick", "Ypos2Note", "MIDI_NOTEOFF_EVENT", "MIDI_NOTEON_EVENT", "MIDI_CTRL_EVENT", "MIDI_PITCH_EVENT", "cursor_pencil", "cursor_inc_l", "cursor_inc_r", "cursor_move", "current_cursor", "cursor_inc", "evwr_to_repr_list", "evrepr_is_in_notelist", "is_in_notelist", "note_collision"]
