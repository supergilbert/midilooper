#!/usr/bin/python3
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

from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext
import sys, os

mdsq_libc_path = "midiseq/"

macros = []
if os.getenv("OLDJACKAPI"):
    macros.append(("__MLP_OLD_JACK", "yes"))

midilooper_module = Extension("midiseq",
                              include_dirs = [mdsq_libc_path],
                              library_dirs = [mdsq_libc_path],
                              libraries = ["asound", "jack"],
                              extra_compile_args = ["-Wall", "-Werror"],
                              sources = ["midiseq/asound/aseq.c",
                                         "midiseq/asound/aseq_tool.c",
                                         "midiseq/clock/clock.c",
                                         "midiseq/debug_tool/debug_tool.c",
                                         "midiseq/debug_tool/dump_trackst.c",
                                         "midiseq/jack/jack_backend.c",
                                         "midiseq/loop_engine/engine_binding.c",
                                         "midiseq/loop_engine/engine.c",
                                         "midiseq/loop_engine/engine_jack.c",
                                         "midiseq/loop_engine/engine_midisave.c",
                                         "midiseq/loop_engine/engine_nanosleep.c",
                                         "midiseq/loop_engine/output_req.c",
                                         "midiseq/loop_engine/midi_ring_buffer.c",
                                         "midiseq/loop_engine/track_ctx.c",
                                         "midiseq/midi/midifile.c",
                                         "midiseq/midi/midifile_get_varlen.c",
                                         "midiseq/midi/midifile_set_varlen.c",
                                         "midiseq/midi/midifile_tool.c",
                                         "midiseq/midi/midi_channel_ev.c",
                                         "midiseq/midi/midi_meta_ev.c",
                                         "midiseq/midi/midi_tool.c",
                                         "midiseq/seqtool/ev_iterator.c",
                                         "midiseq/seqtool/seqtool.c",
                                         "midiseq/tool/tool.c",
                                         "midiseq_ext/pym_midiseq.c",
                                         "midiseq_ext/pym_midiseq_class.c",
                                         "midiseq_ext/pym_midiseq_evwr.c",
                                         "midiseq_ext/pym_midiseq_file.c",
                                         "midiseq_ext/pym_midiseq_output.c",
                                         "midiseq_ext/pym_midiseq_tools.c",
                                         "midiseq_ext/pym_midiseq_track.c"],
                              define_macros = macros)

setup(name = "midilooper",
      version = "0.1",
      description = "Midi loop sequencer",
      long_description = "Midi loop sequencer that can be controlled via keyboard and (or) midi shortcut.",
      author = "Gilbert Romer",
      author_email = "gilbux@gmail.com",
      url = "https://github.com/supergilbert/midiseq",
      ext_modules = [midilooper_module],
      packages = ["midilooper",
                  "midilooper/msqwidget"],
      package_dir = {"midilooper": "midilooper",
                     "midilooper/msqwidget": "midilooper/msqwidget"},
      scripts = ["midilooper/scripts/midilooper"],
      data_files = [("share/man/man1/", ["midilooper.1"])],
      license = "GNU General Public License")
