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


cursor_pencil  = gtk.gdk.Cursor(gtk.gdk.PENCIL)
cursor_inc     = gtk.gdk.Cursor(gtk.gdk.SB_H_DOUBLE_ARROW)
cursor_inc_l   = gtk.gdk.Cursor(gtk.gdk.SB_LEFT_ARROW)
cursor_inc_r   = gtk.gdk.Cursor(gtk.gdk.SB_RIGHT_ARROW)
cursor_move    = gtk.gdk.Cursor(gtk.gdk.FLEUR)
current_cursor = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)

MIDI_NOTEOFF_EVENT = 0x8
MIDI_NOTEON_EVENT  = 0x9
MIDI_CTRL_EVENT    = 0xB
MIDI_PITCH_EVENT   = 0xE

class DrawAllArea(object):
    def draw_all(self):
        if self.window:
            xsz, ysz = self.window.get_size()
            self.draw_area(gtk.gdk.Rectangle(0, 0, xsz, ysz))

class Xpos2Tick(DrawAllArea):
    def xpos2tick(self, xpos):
        xadj_pos = xpos + self.xadj
        tick = int(xadj_pos * self.setting.getppq() / self.setting.qnxsz)
        return tick if tick >= 0 else 0

    def tick2xpos(self, tick):
        return (tick * self.setting.qnxsz / self.setting.getppq()) - self.xadj

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
        return ((127 - note) * self.setting.noteysz) - self.yadj

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
