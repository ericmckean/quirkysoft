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
  BUILD_DIRS = {
      'WITH_BUNJALLOO': 'bunjalloo',
      'WITH_CHAOS': 'ChaosDS'
      }
  for e in BUILD_DIRS:
    if bld.env[e]:
      bld.add_subdirs(BUILD_DIRS[e])

def set_options(opt):
  #opt.tool_options('g++')
  opt.tool_options('compiler_cxx')
  opt.tool_options('compiler_cc')
  opt.tool_options('arm', waf_tools)
  OPTIONS = {
      '--with-dldi': {
        'action':  'store',
        'help':    'Set the DLDI patch to use (use with "build").',
        'default': None,
        },
      '--without-chaos': {
        'action':  'store_true',
        'help':    'Do not build ChaosDS (use with "configure")',
        'default': False,
        },
      '--without-bunjalloo': {
        'action':  'store_true',
        'help':    'Do not build bunjalloo (use with "configure")',
        'default': False
        },
      '--without-sdl': {
        'action':  'store_true',
        'help':    'Do not build the SDL port',
        'default': False
        },
      '--without-cppunit': {
        'action':  'store_true',
        'help':    'Do not build the unit tests',
        'default': False},
      '--with-tcmalloc': {
        'action':  'store_true',
        'help':    'Use Google heap profiler tcmalloc (http://code.google.com/p/google-perftools/)',
        'default': False
        },
      '--with-profiler': {
        'action':  'store_true',
        'help':    'Use Google CPU profiler (http://code.google.com/p/google-perftools/)',
        'default': False
        },
      '--with-gcov': {
        'action':  'store_true',
        'help':    'Build with gcov flags',
        'default': False
        },
      '--tags': {
        'action':  'store_true',
        'help':    'Force creation of tags file using ctags (use with "build")',
        'default': False
        }
      }
  ao = opt.add_option
  for op in OPTIONS:
    kwargs = OPTIONS[op]
    ao(op, **kwargs)

# Tool checking
def arm_tool_check(conf):
  conf.check_tool('misc')

  # cannot check compiler_cc for devkitArm as it needs LINKFLAGS, which waf
  # stips off in the check.
  ARM_TOOLS = '''
    arm
    grit
    ndstool
    dlditool
    objcopy
  '''.split()
  for tool in ARM_TOOLS:
    conf.check_tool(tool, waf_tools)

  without_chaos = False
  if not Options.options.without_chaos and not conf.env['HAVE_GRIT_SHARED']:
    Logs.warn("""Required version of 'grit' is v0.8 (devkitpro r22) at least.""")
    without_chaos = True

  if not Options.options.without_chaos:
    conf.check_tool('sox', waf_tools)
    if not conf.env['SOX'] or not conf.env['PADBIN']:
      without_chaos = True

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
  conf.check_tool('objcopy', waf_tools)
  if not Options.options.without_chaos:
    conf.check_tool('sox', waf_tools)
  if not Options.options.without_cppunit:
    conf.check_tool('unit_test', waf_tools)

# Header checking

def common_header_checks(configurator):
  for header in ('png.h', 'zlib.h', 'gif_lib.h', 'matrixSsl.h'):
    configurator(header_name=header, uselib_store='HOST')

def arm_header_check(conf):
  # check headers
  common_header_checks(conf.arm_check)
  conf.arm_check(header_name='nds/jtypes.h dswifi9.h')
  conf.arm_check(header_name='stdio.h jpeglib.h')
  conf.arm_check(header_name='nds/jtypes.h dswifi7.h', uselib_store='ARM7')

def sdl_header_check(conf):
  common_header_checks(conf.check)
  conf.sdl_check(header_name='SDL/SDL.h', mandatory=1)
  conf.sdl_check(header_name='stdio.h jpeglib.h')

# Library checking

def lib_check_common(configurator):
  for l in 'png z gif jpeg matrixsslstatic unzip'.split():
    configurator(lib=l)

from Configure import conf

@conf
def sdl_check(self, *k, **kw):
  # really check for libs, in lieu of proper waf 1.5 fix
  #if 'lib' in kw:
  #  envc = self.env.copy()
  #  envc['LIB'] = [kw['lib']]
  #  kw['env'] = envc
  kw['uselib_store'] = 'HOST'
  # returns 0 if all is well (WTF?)
  # returns None if failed!
  return self.check(*k, **kw) == 0

def lib_check_sdl(conf):
  conf.env['HAVE_CPPUNIT'] = False

  for i in 'libpng sdl'.split():
    conf.check_cfg(
        package=i,
        mandatory=1,
        args='--cflags --libs',
        uselib_store='HOST')

  if not (Options.options.without_bunjalloo or Options.options.without_cppunit):
    result = conf.check_cfg(
        package='cppunit',
        args='--cflags --libs',
        uselib_store='TEST')
    if 'LIB_TEST' not in conf.env:
      Options.options.without_cppunit = True
      Utils.pprint('RED', 'cppunit not found. Unit tests will not be built')
      Utils.pprint('RED', 'Install from http://cppunit.sourceforge.net')
      Utils.pprint('RED', 'or try "sudo apt-get install cppunit"')
    else:
      conf.env['HAVE_CPPUNIT'] = True

  perf_msg = 'Install this from http://code.google.com/p/google-perftools/'
  if Options.options.with_tcmalloc:
    if not conf.sdl_check(lib='tcmalloc'):
      Utils.pprint('RED', perf_msg)
  if Options.options.with_profiler:
    if not conf.sdl_check(lib='profiler'):
      Utils.pprint('RED', perf_msg)



def lib_check_arm7(conf):
  conf.arm_check(lib='dswifi7', uselib_store='ARM7')

def lib_check_arm9(conf):
  conf.arm_check(lib='dswifi9')
  lib_check_common(conf.arm_check)

def check_gcov(conf):
  if Options.options.with_gcov:
    if conf.sdl_check(lib='gcov'):
      flags = '-fprofile-arcs -ftest-coverage'.split()
      for i in ('CXXFLAGS', 'CCFLAGS'):
        conf.env[i].extend(flags)

def config_status(conf):
  conf.setenv('default')
  Utils.pprint('GREEN',"""Summary of configure options:""")

  Utils.pprint('BLUE',"""
      Install prefix:       %(PREFIX)s
      DLDI used:            %(DLDIFLAGS)s
      Will build SDL code:  %(WITH_SDL)s
      Will build Bunjalloo: %(WITH_BUNJALLOO)s
      Will build ChaosDS:   %(WITH_CHAOS)s"""%conf.env)
  conf.setenv('sdl')
  Utils.pprint('BLUE',"""\
      Cppunit found:        %(HAVE_CPPUNIT)s"""%conf.env)
  env = {
      'gcov': Options.options.with_gcov,
      'tcmalloc': Options.options.with_tcmalloc,
      'profiler': Options.options.with_profiler
      }
  Utils.pprint('BLUE',"""\
      Using gcov:           %(gcov)s
      Using tcmalloc:       %(tcmalloc)s
      Using profiler:       %(profiler)s
      """%env)

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
  else:
    conf.env['DLDIFLAGS'] = ''

  conf.env['WITH_SDL'] = True
  if Options.options.without_sdl:
    Utils.pprint('BLUE','SDL version not configured')
    conf.env['WITH_SDL'] = False
    conf.env['CXXDEFINES'].extend(['sprintf_platform=siprintf'])
    config_status(conf)
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
  try:
    lib_check_sdl(conf)
  except :
    pass
  conf.sdl_check(lib='GL')
  lib_check_common(conf.sdl_check)

  sdl['OBJCOPYFLAGS'] = ' -I binary -O elf32-i386 -B i386 '.split()
  # do not add -O2 FFS, it makes debugging impossible
  sdl['CCFLAGS'] = ['-Wall', '-g']
  sdl['CXXFLAGS'] = ['-Wall', '-g']
  sdl['OBJCOPY'] = 'objcopy'
  sdl_header_check(conf)

  check_gcov(conf)
  config_status(conf)

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
