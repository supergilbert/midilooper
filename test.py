#!/usr/bin/python

import os
import re
import stat

import sys
sys.path.append('/home/gilbr/dvpt/midi_dev/sequencer_gilbr/src/python/build/lib.linux-x86_64-2.6')

import midiseq

def get_all_midi_file(dir_path):
    if re.search(".*/$", dir_path):
        dir_path = dir_path[:-1]
    tmp_file_list = os.listdir(dir_path)
    tmp_file_list = map(lambda file: "/".join([dir_path, file]), tmp_file_list)
    file_list = filter(lambda file: re.search(".*\.mid$|.*\.midi$", file, re.I),
                       tmp_file_list)
    dir_list = filter(lambda file: stat.S_ISDIR(os.stat(file).st_mode), tmp_file_list)
    for dir in dir_list:
        file_list.extend(get_all_midi_file(dir))
    return file_list


def test_readable_midi_file(dir_path):
    midi_file_list = get_all_midi_file(dir_path)
    number_of_file = len(midi_file_list)
    number_of_readable = 0
    for file in midi_file_list:
        try:
            midif = midiseq.midifile(file)
            midif = None
            number_of_readable += 1
        except:
            pass
    print "number of file:     %i" % number_of_file
    print "number of readable: %i" % number_of_file
    print "percentage:         %i\%" % (100 * number_of_readable / number_of_file)


# midif = midiseq.midifile("/home/gilbr/dvpt/midi_dev/sequencer_gilbr/file/midi/Highway_to_Hell.mid")
# track = midif.getmergedtrack()
# mdsq = midiseq.midiseq("bobo")
# mdsq.setppq(midif.getppq())
# mdsq.setms(midif.getms())
# mdsq.settrack(track)
# #import time
# #time.sleep(5)
# mdsq.start()
