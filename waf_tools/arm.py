""" Set up Devkit ARM toolkit """
import os
import Options
import Logs

DEVKITPRO = "DEVKITPRO"
DEVKITARM = "DEVKITARM"

def set_options(opt):
  """ Set waf option flags for devkitpro and devkitarm """
  dkp = ''
  dka = ''
  if DEVKITPRO in os.environ:
    dkp = os.environ[DEVKITPRO]

  if DEVKITARM in os.environ:
    dka = os.environ[DEVKITARM]
  elif dkp:
    dka = os.path.join(dkp, 'devkitARM')

  dkpo = opt.add_option_group("DevkitPro options")
  dkpo.add_option('--with-devkitpro', type='string',
          help='Path to devkitpro. Will use env DEVKITPRO by default.',
          default=dkp, dest='devkitpro')
  dkpo.add_option('--with-devkitarm', type='string',
          help='Path to devkitarm. Will use env DEVKITARM by default.',
          default=dka, dest='devkitarm')

def setup_tool(conf, proc):
  """ Set up the C and C++ flags for ARM compilation """
  if not Options.options.devkitpro:
    conf.fatal('''No DEVKITPRO variable set. Set the path to devkit pro or use the --with-devkitpro option''')
  if not Options.options.devkitarm:
    Options.options.devkitarm = os.path.join(Options.options.devkitpro, 'devkitARM')
    Logs.warn('''No DEVKITARM variable set. Using %s''' % (Options.options.devkitarm))

  env = conf.env
  arch = '-mthumb -mthumb-interwork'

  dka_bin = '%s/bin' % (Options.options.devkitarm)
  arm_eabi = 'arm-eabi-'

  # reset the tools
  for i in ( ('cpp','CPP'), ('gcc','CC'), ('g++','CXX'),
      ('ar','AR'), ('ranlib','RANLIB')):
    prog = conf.find_program(arm_eabi+i[0], path_list=[dka_bin])
    if not prog:
      conf.fatal(i[0]+' was not found')
    env[i[1]] = prog

  env['CC_NAME'] = 'gcc'
  env['CXX_NAME'] = 'gcc'
  env['LINK_CC'] = env['CC']
  env['LINK_CXX'] = env['CXX']
  import ccroot
  cxx_flags = 'CXXFLAGS_ARM%d' % (proc)
  cc_flags = 'CCFLAGS_ARM%d' % (proc)
  version = ccroot.get_cc_version(conf, env['CC'], 'CC_VERSION')
  env[cc_flags] = (arch + ' -ffast-math -O2 -Wall ').split()
  if version == '4.3.0':
    env[cc_flags].append('-Wno-array-bounds')
  env[cc_flags].extend(
      {9:' -march=armv5te -mtune=arm946e-s -DARM9',
       7:' -mcpu=arm7tdmi -mtune=arm7tdmi -DARM7 '}[proc].split())
  env[cxx_flags] = ' -g -fno-rtti -fno-exceptions '.split()
  env[cxx_flags].extend(env[cc_flags])

  path = ['%s/libnds/include' % (Options.options.devkitpro)]
  env['CPPPATH_ARM%d' % (proc)] = path
  env['LINKFLAGS_ARM%d' % (proc)] = ['-specs=ds_arm%d.specs' % (proc),
      '-g','-mno-fpu','-Wl,-Map,map%d.map' % (proc), '-Wl,-gc-sections'] \
          + arch.split()
  # check for libnds
  lib_conf = conf.create_library_configurator()
  lib_conf.nosystem = 1
  lib_conf.mandatory = 1
  lib_conf.uselib_store = 'ARM%d' % (proc)
  lib_conf.path = ['%s/libnds/lib' % (Options.options.devkitpro)]
  lib_conf.name = 'nds%d' % (proc)
  lib_conf.run()
  if proc == 9:
    lib_conf.name = 'fat'
    lib_conf.run()
  # swap the last 2 over so linking works.
  env['LIB_ARM%d' % (proc)].reverse()
  env['CPPFLAGS_ARM%d' % (proc)].extend(env['CCFLAGS_ARM%d' % (proc)])
  env['CXXDEFINES'] = env['CCDEFINES']
  env['ARFLAGS'] = 'rcs'

