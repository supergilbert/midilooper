#!/usr/bin/python

import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")
import gtk

import sys
sys.path.append('./module')

import midiseq

from portlist import PortList
from tracklist import TrackList

msq =  midiseq.midiseq("test treeview")
msq.newoutput("output 1")
msq.newoutput("output 2")
msq.newtrack("track 1")
msq.newtrack("track 2")

portlist = PortList(msq)
portlist.show_all()

tracklist = TrackList(msq)
tracklist.show_all()

gtk.main()
