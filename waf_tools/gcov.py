"""
Usage for code coverage:

configure --with-gcov
Compile the code and run the tests or run the program
Generate the coverage files:

    $ cd _build_
    $ gcov -o default/path/to/src SourceFile_N  # object file less the .o
    $ cp SourceFile.cpp.gcov ../path/to/src
"""
import Options

def set_options(opt):
    opt.add_option(
            '--with-gcov',
            action='store_true',
            help='Use gcov for profiling',
            default=False)

def detect(conf):
    conf.find_program('gcov', mandatory=Options.options.with_gcov)
    if Options.options.with_gcov:
        gcov_flags = '-fprofile-arcs -ftest-coverage'.split()
        conf.env['CXXFLAGS'] = conf.env['CFLAGS'] = \
                conf.env['LINKFLAGS'] = gcov_flags
