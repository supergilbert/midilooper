from distutils.core import setup, Extension

mdsq_libc_path = '../../C/'

module1 = Extension('midiseq',
                    include_dirs = [mdsq_libc_path],
                    library_dirs = [mdsq_libc_path],
                    libraries = ['midiseq', 'asound'],
                    # extra_link_args = "-fPIC",
                    sources = ['./pym_midiseq_class.c',
                               './pym_midiseq_track.c',
                               './pym_midiseq_aport.c',
                               './pym_midiseq_file.c',
                               './pym_midiseq_evwr.c',
                               './engine.c',
                               './engine_midisave.c',
                               './pym_midiseq.c'])

setup (name = 'midiseq',
       description = 'sequencer midi',
       ext_modules = [module1])
