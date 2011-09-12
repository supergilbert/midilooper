import os
import sys

sys.path.append('/home/gilbr/dvpt/midi_dev/sequencer_gilbr/src/python/build/lib.linux-x86_64-2.6')
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
    print "Reading midifile '/home/gilbr/dvpt/midi_dev/sequencer_gilbr/file/midi/gnossie5.mid'"
    mdsq = read_midifile("/home/gilbr/dvpt/midi_dev/sequencer_gilbr/file/midi/gnossie5.mid")
    mdsq.setbpm(80)
    # mdsq.setms(750000)
    mdsq.start()
    mdsq.wait()
