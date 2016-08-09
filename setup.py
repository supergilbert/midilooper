#!/usr/bin/python2.7
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

from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext
import sys, os

mdsq_libc_path = "./src/midiseq/"

macros = []
if os.getenv("OLDJACKAPI"):
    macros.append(("__MLP_OLD_JACK", "yes"))

midilooper_module = Extension("midilooper/midiseq",
                              include_dirs = [mdsq_libc_path],
                              library_dirs = [mdsq_libc_path],
                              libraries = ["asound", "jack"],
                              extra_compile_args = ["-Wall", "-Werror"],
                              sources = ["./src/midiseq_ext/pym_midiseq_class.c",
                                         "./src/midiseq_ext/pym_midiseq_tools.c",
                                         "./src/midiseq_ext/pym_midiseq_track.c",
                                         "./src/midiseq_ext/pym_midiseq_output.c",
                                         "./src/midiseq_ext/pym_midiseq_file.c",
                                         "./src/midiseq_ext/pym_midiseq_evwr.c",
                                         "./src/midiseq_ext/pym_midiseq.c",
                                         "./src/midiseq/asound/aseq.c",
                                         "./src/midiseq/asound/aseq_tool.c",
                                         "./src/midiseq/clock/clock.c",
                                         "./src/midiseq/debug_tool/debug_tool.c",
                                         "./src/midiseq/debug_tool/dump_trackst.c",
                                         "./src/midiseq/jack/jack_backend.c",
                                         "./src/midiseq/loop_engine/engine_binding.c",
                                         "./src/midiseq/loop_engine/engine.c",
                                         "./src/midiseq/loop_engine/engine_jack.c",
                                         "./src/midiseq/loop_engine/engine_midisave.c",
                                         "./src/midiseq/loop_engine/engine_nanosleep.c",
                                         "./src/midiseq/loop_engine/midi_ring_buffer.c",
                                         "./src/midiseq/loop_engine/output_req.c",
                                         "./src/midiseq/loop_engine/track_ctx.c",
                                         "./src/midiseq/midi/midi_channel_ev.c",
                                         "./src/midiseq/midi/midifile.c",
                                         "./src/midiseq/midi/midifile_get_varlen.c",
                                         "./src/midiseq/midi/midifile_set_varlen.c",
                                         "./src/midiseq/midi/midifile_tool.c",
                                         "./src/midiseq/midi/midi_meta_ev.c",
                                         "./src/midiseq/midi/midi_tool.c",
                                         "./src/midiseq/seqtool/ev_iterator.c",
                                         "./src/midiseq/seqtool/seqtool.c",
                                         "./src/midiseq/tool/tool.c"],
                              define_macros = macros)

setup(name = "midilooper",
      version = "0.01",
      description = "sequencer midi",
      ext_modules = [midilooper_module],
      packages=["midilooper",
                "midilooper/msqwidget"],
      package_dir={"midilooper": "src/midilooper",
                   "midilooper/msqwidget": "src/midilooper/msqwidget"},
      scripts=["src/midilooper/scripts/midilooper"])
# cmdclass={'build_ext': build_midilooper})
