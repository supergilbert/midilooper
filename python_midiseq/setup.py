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
import sys, os

mdsq_libc_path = "../lib_midiseq/"

module1 = Extension("midiseq",
                    include_dirs = [mdsq_libc_path],
                    library_dirs = [mdsq_libc_path],
                    libraries = ["midiseq", "asound"],
                    # extra_link_args = "-fPIC",
                    extra_compile_args = ["-Werror"],
                    sources = ["./pym_midiseq_class.c",
                               "./pym_midiseq_tools.c",
                               "./pym_midiseq_track.c",
                               "./pym_midiseq_output.c",
                               "./pym_midiseq_file.c",
                               "./pym_midiseq_evwr.c",
                               "./pym_midiseq.c"])

setup (name = "midiseq",
       description = "sequencer midi",
       ext_modules = [module1])
