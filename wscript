#!/usr/bin/env python

import Build
import Configure
import Constants
import Environment
import Logs
import Options
import os
import Runner
import sys
import Task
import TaskGen
import UnitTest
import Utils

Task.algotype = Constants.JOBCONTROL
Configure.autoconfig = True
srcdir = '.'
blddir = '_build_'
waf_tools = 'waf_tools'
sys.path.append(waf_tools)

def build(bld):
  Task.g_shuffle = False
  # overcome chdir problems for jump-to-error
  if not Options.commands['clean']:
    print "waf: Entering directory `%s'" % \
        (os.path.join(Build.bld.srcnode.abspath(), blddir))
  bld.add_subdirs('libndspp')
  if bld.env['WITH_BUNJALLOO']:
    bld.add_subdirs('bunjalloo')
  if bld.env['WITH_CHAOS']:
    bld.add_subdirs('ChaosDS')

def set_options(opt):
  #opt.tool_options('g++')
  opt.tool_options('compiler_cxx')
  opt.tool_options('compiler_cc')
  opt.tool_options('arm', waf_tools)
  opt.add_option('--with-dldi', action='store',
          help='Set the DLDI patch to use (use with "build").',
          default='', dest='with_dldi')
  opt.add_option('--without-chaos', action='store_true',
          help='Do not build ChaosDS (use with "configure")',
          default=False, dest='without_chaos')
  opt.add_option('--without-bunjalloo', action='store_true',
          help='Do not build bunjalloo (use with "configure")',
          default=False, dest='without_bunjalloo')
  opt.add_option('--without-sdl', action='store_true',
          help='Do not build the SDL port',
          default=False, dest='without_sdl')

  opt.add_option('--without-cppunit', action='store_true',
          help='Do not build the unit tests',
          default=False, dest='without_cppunit')
  opt.add_option('--with-tcmalloc', action='store_true',
          help='Use Google heap profiler tcmalloc (http://code.google.com/p/google-perftools/)',
          default=False, dest='with_tcmalloc')
  opt.add_option('--with-profiler', action='store_true',
          help='Use Google CPU profiler (http://code.google.com/p/google-perftools/)',
          default=False, dest='with_profiler')

  opt.add_option('--tags', action='store_true',
          help='Force creation of tags file using ctags (use with "build")',
          default=False, dest='tags')

# Tool checking

def arm_tool_check(conf):
  conf.check_tool('misc')
  conf.check_tool('g++', funs='cxx_load_tools gxx_common_flags')
  conf.check_tool('gcc', funs='cc_load_tools gcc_common_flags')

  # cannot check compiler_cc for devkitArm as it needs LINKFLAGS, which waf
  # stips off in the check.
  conf.check_tool('arm7', waf_tools)
  conf.check_tool('arm9', waf_tools)
  conf.check_tool('grit', waf_tools)
  without_chaos = False
  if not Options.options.without_chaos and not conf.env['HAVE_GRIT_SHARED']:
    Logs.warn("""Required version of 'grit' is v0.8 (devkitpro r22) at least.""")
    without_chaos = True

  if not Options.options.without_chaos:
    conf.check_tool('sox', waf_tools)
    if not conf.env['SOX'] or not conf.env['PADBIN']:
      without_chaos = True

  conf.check_tool('ndstool', waf_tools)
  conf.check_tool('dlditool', waf_tools)
  conf.check_tool('objcopy', waf_tools)
  if not Options.options.without_chaos and without_chaos:
    Logs.warn("Chaos will not be compiled due to missing requirements")
    Options.options.without_chaos = without_chaos
  conf.env['WITH_CHAOS'] = not Options.options.without_chaos
  conf.env['WITH_BUNJALLOO'] = not Options.options.without_bunjalloo

  # see if we have ctags
  ctags = conf.find_program('ctags', var='CTAGS')
  conf.env['CTAGS'] = ctags

def sdl_tool_check(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('compiler_cc')
  conf.check_tool('grit', waf_tools)
  if not Options.options.without_chaos:
    conf.check_tool('sox', waf_tools)
  if not Options.options.without_cppunit:
    conf.check_tool('unit_test', waf_tools)
  conf.check_tool('objcopy', waf_tools)

# Header checking

def common_header_checks(configurator):
  for header in ('png.h', 'zlib.h', 'gif_lib.h', 'matrixSsl.h'):
    configurator.name = header
    configurator.run()

def arm_header_check(conf):
  # check headers
  e = conf.create_header_configurator()
  e.mandatory = 1
  e.nosystem = 1
  e.uselib_store = 'ARM9'
  e.name = 'fat.h'
  e.run()

  common_header_checks(e)

  e.name = 'dswifi9.h'
  e.header_code = '#include <nds/jtypes.h>'
  e.run()

  e.name = 'jpeglib.h'
  e.header_code = '#include <stdio.h>'
  e.run()

  e.name = 'dswifi7.h'
  e.uselib_store = 'ARM7'
  e.header_code = '#include <nds/jtypes.h>'
  e.run()

def sdl_header_check(conf):
  e = conf.create_header_configurator()
  e.mandatory = 1
  e.name = 'SDL/SDL.h'
  e.run()

  common_header_checks(e)

  e.name = 'jpeglib.h'
  e.header_code = '#include <stdio.h>'
  e.run()


# Library checking

def lib_check_common(configurator):
  for l in 'png z gif jpeg matrixsslstatic unzip'.split():
    configurator.name = l
    configurator.run()


def lib_check_sdl(conf):
  pkgconfig = conf.create_pkgconfig_configurator()
  pkgconfig.mandatory = 1
  pkgconfig.uselib_store = 'HOST'
  for i in 'libpng sdl'.split():
    pkgconfig.name = i
    pkgconfig.run()

  conf.env['HAVE_CPPUNIT'] = 0
  if not (Options.options.without_bunjalloo or Options.options.without_cppunit):
    pkgconfig.uselib_store = 'TEST'
    pkgconfig.name = 'cppunit'
    pkgconfig.run()
    if not conf.env['LIB_TEST']:
      Options.options.without_cppunit = True
      Utils.pprint('RED', 'Unit tests will not be built')
    else:
      conf.env['HAVE_CPPUNIT'] = 1

  e = conf.create_library_configurator()
  e.mandatory = 1
  e.uselib_store = 'HOST'
  e.name = 'GL'
  e.run()

  if Options.options.with_tcmalloc:
    e = conf.create_library_configurator()
    e.mandatory = 1
    e.uselib_store = 'HOST'
    e.name = 'tcmalloc'
    e.run()
  if Options.options.with_profiler:
    e = conf.create_library_configurator()
    e.mandatory = 1
    e.uselib_store = 'HOST'
    e.name = 'profiler'
    e.run()



  lib_check_common(e)


def lib_check_arm7(conf):
  e = conf.create_library_configurator()
  e.nosystem = 1
  e.mandatory = 1
  e.uselib_store = 'ARM7'
  e.name = 'dswifi7'
  e.run()

def lib_check_arm9(conf):
  e = conf.create_library_configurator()
  e.nosystem = 1
  e.mandatory = 1
  e.uselib_store = 'ARM9'
  e.name = 'dswifi9'
  e.run()
  lib_check_common(e)

# Main configuration entry point
def configure(conf):
  # check native compiler
  top = os.getcwd()

  arm_tool_check(conf)
  arm_header_check(conf)
  conf.env['OBJCOPYFLAGS'] = """ -I binary -O elf32-littlearm -B arm
      --rename-section .data=.rodata,readonly,contents """.split()
  lib_check_arm9(conf)
  lib_check_arm7(conf)
  GRITFLAGS='-pw 16 -gB 8 -m! -ftc -fh -q -pe64'
  DATADIR='data/bunjalloo'
  conf.env['CXXDEFINES'] = ['DATADIR=\\"%s\\"'%DATADIR]

  conf.env['GRITFLAGS'] = GRITFLAGS
  if Options.options.with_dldi:
    conf.env['DLDIFLAGS'] = Options.options.with_dldi

  conf.env['WITH_SDL'] = True
  if Options.options.without_sdl:
    Utils.pprint('BLUE','SDL version not configured')
    conf.env['WITH_SDL'] = False
    conf.env['CXXDEFINES'].extend(['sprintf_platform=siprintf'])
    return True

  sdl = Environment.Environment()

  conf.set_env_name('sdl', sdl)
  sdl.set_variant('sdl')

  for f in ('GRITFLAGS', 'CXXDEFINES', 'LIBPATH'):
    if type(conf.env[f]) == type([]):
      sdl[f] = [x for x in conf.env[f]]
    else:
      sdl[f] = conf.env[f]

  conf.env['CXXDEFINES'].extend(['sprintf_platform=siprintf'])
  sdl['CXXDEFINES'].extend(['sprintf_platform=sprintf'])
  conf.setenv('sdl')
  sdl_tool_check(conf)
  lib_check_sdl(conf)

  sdl['OBJCOPYFLAGS'] = ' -I binary -O elf32-i386 -B i386 '.split()
  # do not add -O2 FFS, it makes debugging impossible
  sdl['CCFLAGS'] = ['-Wall', '-g']
  sdl['CXXFLAGS'] = ['-Wall', '-g']
  sdl['OBJCOPY'] = 'objcopy'
  sdl_header_check(conf)

def shutdown():
  # force running unit tests
  ut = UnitTest.unit_test()
  ut.change_to_testfile_dir = 1
  ut.run_if_waf_does = 'check'
  if Logs.verbose:
    ut.want_to_see_test_output = True
    ut.want_to_see_test_error = True
  ut.run()
  ut.print_results()

  ctags = Build.bld.env['CTAGS']
  if Options.commands['build'] and ctags:
    # create ctags if we recompiled any program, or force with --tags option
    for obj in Build.bld.all_task_gen:
      if (getattr(obj,'type','') == 'program'
          and obj.link_task
          and getattr(obj.link_task,"executed",0) != 0):
        Options.options.tags = True
        break
    if Options.options.tags:
      import commands
      # create the tags file in the build directory, but include all the source files
      top_dir = Build.bld.srcnode.abspath()
      launch_dir = Options.launch_dir
      os.chdir(launch_dir)
      commands.getoutput('%s -R --c++-kinds=+p --fields=+iaS --extra=+q %s'%(ctags, top_dir))
      Utils.pprint('BLUE','Created tags')
