#!/usr/bin/env python

import Params, Environment, UnitTest, Utils, Runner, Object
import os
import sys

Params.g_autoconfig = 1
srcdir = '.'
blddir = '_build_'
waf_tools = 'waf_tools'
sys.path.append(waf_tools)

def build(bld):
  # overcome chdir problems for jump-to-error
  if not Params.g_commands['clean']:
    print "waf: Entering directory `%s'" % \
        (os.path.join(Params.g_build.m_srcnode.abspath(), blddir))
  bld.add_subdirs('libndspp')
  if bld.env()['WITH_BUNJALLOO']:
    bld.add_subdirs('bunjalloo')
  if bld.env()['WITH_CHAOS']:
    bld.add_subdirs('ChaosDS')

def set_options(opt):
  opt.tool_options('g++')
  opt.tool_options('compiler_cxx')
  opt.tool_options('compiler_cc')
  opt.tool_options('arm', waf_tools)
  opt.add_option('--delete-scons', action='store_true',
          help='Delete the leftovers from SCons builds (use with "build").',
          default=False, dest='delete_scons')
  opt.add_option('--install-to', action='store',
          help='Set the install directory (use with "install").',
          default='bunjalloo-dist', dest='install_to')
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
  if not Params.g_options.without_chaos and not conf.env['HAVE_GRIT_SHARED']:
    Params.warning("""Required version of 'grit' is v0.8 (devkitpro r22) at least.""")
    without_chaos = True

  if not Params.g_options.without_chaos:
    conf.check_tool('sox', waf_tools)
    if not conf.env['SOX'] or not conf.env['PADBIN']:
      without_chaos = True

  conf.check_tool('ndstool', waf_tools)
  conf.check_tool('dlditool', waf_tools)
  conf.check_tool('objcopy', waf_tools)
  if not Params.g_options.without_chaos and without_chaos:
    Params.warning("Chaos will not be compiled due to missing requirements")
    Params.g_options.without_chaos = without_chaos
  conf.env['WITH_CHAOS'] = not Params.g_options.without_chaos
  conf.env['WITH_BUNJALLOO'] = not Params.g_options.without_bunjalloo

  # see if we have ctags
  ctags = conf.find_program('ctags', var='CTAGS')
  conf.env['CTAGS'] = ctags

def sdl_tool_check(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('compiler_cc')
  conf.check_tool('grit', waf_tools)
  if not Params.g_options.without_chaos:
    conf.check_tool('sox', waf_tools)
  if not Params.g_options.without_cppunit:
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
  e.uselib = 'ARM9'
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
  e.uselib = 'ARM7'
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
  pkgconfig.uselib = 'HOST'
  for i in 'libpng sdl'.split():
    pkgconfig.name = i
    pkgconfig.run()

  conf.env['HAVE_CPPUNIT'] = 0
  if not (Params.g_options.without_bunjalloo or Params.g_options.without_cppunit):
    pkgconfig.uselib = 'TEST'
    pkgconfig.name = 'cppunit'
    pkgconfig.run()
    if not conf.env['LIB_TEST']:
      Params.g_options.without_cppunit = True
      Params.pprint('RED', 'Unit tests will not be built')
    else:
      conf.env['HAVE_CPPUNIT'] = 1

  e = conf.create_library_configurator()
  e.mandatory = 1
  e.uselib = 'HOST'
  e.name = 'GL'
  e.run()
  lib_check_common(e)


def lib_check_arm7(conf):
  e = conf.create_library_configurator()
  e.nosystem = 1
  e.mandatory = 1
  e.uselib = 'ARM7'
  e.name = 'dswifi7'
  e.run()

def lib_check_arm9(conf):
  e = conf.create_library_configurator()
  e.nosystem = 1
  e.mandatory = 1
  e.uselib = 'ARM9'
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
  if Params.g_options.with_dldi:
    conf.env['DLDIFLAGS'] = Params.g_options.with_dldi

  conf.env['WITH_SDL'] = True
  if Params.g_options.without_sdl:
    Params.pprint('BLUE','SDL version not configured')
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
  import UnitTest
  ut = UnitTest.unit_test()
  ut.change_to_testfile_dir = 1
  ut.run_if_waf_does = 'check'
  if Params.g_verbose:
    ut.want_to_see_test_output = True
    ut.want_to_see_test_error = True
  ut.run()
  ut.print_results()

  ctags = Params.g_build.env()['CTAGS']
  if Params.g_commands['build'] and ctags:
    # create ctags if we recompiled any program, or force with --tags option
    for obj in Object.g_allobjs:
      if (getattr(obj,'m_type','') == 'program'
          and obj.link_task
          and getattr(obj.link_task,"m_executed",0) != 0):
        Params.g_options.tags = True
        break
    if Params.g_options.tags:
      import commands
      commands.getoutput('%s -R --c++-kinds=+p --fields=+iaS --extra=+q .'%ctags)
      Params.pprint('BLUE','Created tags')
