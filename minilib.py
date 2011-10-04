#!/usr/bin/python

import os
import sys

#sys.path.append('/home/gilbr/dvpt/midi_dev/sequencer_gilbr/src/python/build/lib.linux-x86_64-2.6')
import midiseq


def read_midifile(filepath):
    "return midiseq class"
    if os.path.isfile(filepath):
        midifile = midiseq.midifile(filepath)
        track = midifile.getmergedtrack()
        mdsq = midiseq.midiseq("minilib")
        mdsq.setppq(midifile.getppq())
        mdsq.setms(midifile.getms())
        mdsq.settrack(track)
        return mdsq

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Need a midifile as argument"
    else:
        filepath = sys.argv[1]
        print "Reading midifile '%s'" % filepath
        mdsq = read_midifile(filepath)
        # mdsq.setbpm(80)
        # mdsq.setms(750000)
        if mdsq:
            print "Press enter to start sequence"
            sys.stdin.readline()
            mdsq.start()
            mdsq.wait()
        else:
            print "Probleme while reading '%s'" % filepath
