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
# print "/".join(sys.argv[0].split("/")[:-1])
pym_path = os.path.dirname(sys.argv[0])

mdsq_libc_path = "%s/../lib_midiseq/" % pym_path

module1 = Extension("midiseq",
                    include_dirs = [mdsq_libc_path],
                    library_dirs = [mdsq_libc_path],
                    libraries = ["midiseq", "asound"],
                    # extra_link_args = "-fPIC",
                    extra_compile_args = ["-Werror"],
                    sources = ["%s/pym_midiseq_class.c" % pym_path,
                               "%s/pym_midiseq_tools.c" % pym_path,
                               "%s/pym_midiseq_track.c" % pym_path,
                               "%s/pym_midiseq_aport.c" % pym_path,
                               "%s/pym_midiseq_file.c" % pym_path,
                               "%s/pym_midiseq_evwr.c" % pym_path,
                               "%s/pym_midiseq.c" % pym_path])

setup (name = "midiseq",
       description = "sequencer midi",
       ext_modules = [module1])
