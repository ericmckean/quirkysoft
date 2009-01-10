""" Set up Devkit ARM toolkit """
import os
import Options
import Utils
import Logs
import ccroot

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

  dkpo.add_option('--with-devkitpro',
                  type='string',
                  help='Path to devkitpro. Will use env DEVKITPRO by default.',
                  default=dkp,
                  dest='devkitpro')

  dkpo.add_option('--with-devkitarm',
                  type='string',
                  help='Path to devkitarm. Will use env DEVKITARM by default.',
                  default=dka,
                  dest='devkitarm')

def detect(conf):
  """ Set up the C and C++ flags for ARM compilation """
  env = conf.env
  if not Options.options.devkitpro:
    conf.fatal('''No DEVKITPRO variable set. Set the path to devkit pro or use the --with-devkitpro option''')
  env[DEVKITPRO] = Options.options.devkitpro
  if not Options.options.devkitarm:
    Options.options.devkitarm = os.path.join(Options.options.devkitpro, 'devkitARM')
    Logs.warn('''No DEVKITARM variable set. Using %s''' % (Options.options.devkitarm))
  env[DEVKITARM] = Options.options.devkitarm

  ccflags = """-ffast-math -O2 -Wall -mthumb -mthumb-interwork -march=armv5te
               -mtune=arm946e-s -Wno-array-bounds""".split()
  cxxflags = ccflags + '-g -fno-rtti -fno-exceptions'.split()
  ccdefines = ['ARM9']
  linkflags = """-specs=ds_arm9.specs -g -mno-fpu -Wl,-Map,map9.map
                 -Wl,-gc-sections -mthumb -mthumb-interwork""".split()
  arm_eabi = os.path.join(Options.options.devkitarm, 'bin', 'arm-eabi-')
  env.table.update({
      'CC':  arm_eabi+'gcc',
      'CXX': arm_eabi+'g++',
      'CPP': arm_eabi+'cpp',
      'AR':  arm_eabi+'ar',
      'RANLIB': arm_eabi+'ranlib',
      'CXXFLAGS': cxxflags,
      'CCFLAGS': ccflags,
      'CCDEFINES': ccdefines,
      'CXXDEFINES': ccdefines,
      'CPPPATH': ['%s/libnds/include' % (Options.options.devkitpro)],
      'LIBPATH': ['%s/libnds/lib' % (Options.options.devkitpro)],
      'LINKFLAGS': linkflags
      })
  conf.check_tool('compiler_cc')
  conf.check_tool('compiler_cxx')
  # need -lfat -lnds9 in that order, but fat requires nds9...
  conf.check(lib='nds9', mandatory=1, uselib_store='HOST')
  if conf.check(lib='fat', mandatory=1, uselib='HOST'):
    env.prepend_value('LIB_HOST', 'fat')
