""" Set up Devkit ARM toolkit """
import os
import Options
import Utils
import Logs
import ccroot
from Configure import conftest, conf

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

def find_arm_eabi_program(conf, prog_name, var):
  full_name = 'arm-eabi-'+prog_name
  path = os.path.join(Options.options.devkitarm,'bin')
  conf.env[var]  = conf.require_program(full_name, path_list=[path])

@conftest
def find_arm_eabi_gxx(conf):
  find_arm_eabi_program(conf, 'g++', 'CXX');
  conf.env['CXX_NAME'] = 'gcc'
  ccroot.get_cc_version(conf, conf.env['CXX'], 'CXX_VERSION')

@conftest
def find_arm_eabi_gcc(conf):
  find_arm_eabi_program(conf, 'gcc', 'CC');
  conf.env['CC_NAME'] = 'gcc'
  ccroot.get_cc_version(conf, conf.env['CC'], 'CC_VERSION')

@conftest
def find_arm_eabi_ar(conf):
  find_arm_eabi_program(conf, 'ar', 'AR');
  conf.env['ARFLAGS'] = 'rcs'

@conftest
def find_arm_eabi_cpp(conf):
  find_arm_eabi_program(conf, 'cpp', 'CPP');

@conftest
def find_arm_eabi_ranlib(conf):
  find_arm_eabi_program(conf, 'ranlib', 'RANLIB');

@conf
def arm_check(self, *k, **kw):
  # check for arm features
  # if no cpp_path, use default
  if 'uselib_store' not in kw:
    kw['uselib_store'] = 'HOST'

  if 'cpp_path' not in kw:
    kw['cpp_path'] = 'CPPPATH_'+kw['uselib_store']

  envc = self.env.copy()
  # really check for libs, in lieu of proper waf 1.5 fix
  if 'lib' in kw:
    envc['LIB'] = [kw['lib']]
    kw['libpath'] = ['%s/libnds/lib' % (Options.options.devkitpro)]
  envc['CPPPATH'] = self.env[kw['cpp_path']]
  kw['env'] = envc
  kw['mandatory'] = 1

  self.check(*k, **kw)

def gxx_modifier_per_proc(conf, proc):
  env = conf.env
  uselib_store = {9: 'HOST', 7: 'ARM7'}[proc]

  cxx_flags = 'CXXFLAGS_%s' % (uselib_store)
  cc_flags = 'CCFLAGS_%s' % (uselib_store)
  cpp_flags = 'CPPFLAGS_%s'% (uselib_store)
  cpp_path = 'CPPPATH_%s'% (uselib_store)
  link_flags = 'LINKFLAGS_%s'% (uselib_store)
  lib_flags = 'LIB_%s'% (uselib_store)

  arch = '-mthumb -mthumb-interwork'
  env[cc_flags] = (arch + ' -ffast-math -O2 -Wall ').split()
  if env['CC_VERSION'] == '4.3.0':
    env[cc_flags].append('-Wno-array-bounds')
  env[cc_flags].extend(
      {9:' -march=armv5te -mtune=arm946e-s -DARM9',
       7:' -mcpu=arm7tdmi -mtune=arm7tdmi -DARM7 '}[proc].split()
      )
  env[cxx_flags] = ['-g',
                    '-fno-rtti',
                    '-fno-exceptions']
  env[cxx_flags].extend(env[cc_flags])
  env[cpp_path] = [
      '%s/libnds/include' % (Options.options.devkitpro)
      ]
  env[link_flags] = [
      '-specs=ds_arm%d.specs' % (proc),
      '-g',
      '-mno-fpu',
      '-Wl,-Map,map%d.map' % (proc),
      '-Wl,-gc-sections'] + arch.split()

  # check for libnds
  # cpp_path is optional, will use arm9 path if none given
  if proc == 9:
    conf.arm_check(lib='fat',
        header_name='fat.h')
  conf.arm_check(lib='nds%d'%proc, cpp_path=cpp_path, uselib_store=uselib_store)
  # swap the last 2 (nds9/fat) over so linking works.
  env[cpp_flags].extend(env[cc_flags])
  env['CXXDEFINES'] = env['CCDEFINES']

@conftest
def gxx_modifier_arm_eabi(conf):
  for proc in (7,9):
    gxx_modifier_per_proc(conf, proc)

@conftest
def check_devkitpro(conf):
  """ Set up the C and C++ flags for ARM compilation """
  if not Options.options.devkitpro:
    conf.fatal('''No DEVKITPRO variable set. Set the path to devkit pro or use the --with-devkitpro option''')
  if not Options.options.devkitarm:
    Options.options.devkitarm = os.path.join(Options.options.devkitpro, 'devkitARM')
    Logs.warn('''No DEVKITARM variable set. Using %s''' % (Options.options.devkitarm))

# The variable name 'detect' is magic.  These rules are evaluated in the
# ConfigurationContext.check_tool method, calling each function by reflection
detect = """
    check_devkitpro
    find_arm_eabi_gxx
    find_arm_eabi_gcc
    find_arm_eabi_ar
    find_arm_eabi_cpp
    find_arm_eabi_ranlib
    gxx_common_flags
    gcc_common_flags
    cc_load_tools
    cxx_load_tools
    gxx_modifier_arm_eabi
    """
