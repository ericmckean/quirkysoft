""" Build test programs """
import os, sys
import Options
import TaskGen

def build(bld, with_vera=True, buildsdl=True):
  """
  Build a test program, optionally building the library and optionally
  building for SDL
  """
  app_name = os.path.basename(bld.path.srcpath(bld.env))

  arm9 = bld.new_task_gen('cxx', 'program')
  arm9.install_path = 0
  arm9.find_sources_in_dirs('arm9')
  arm9.includes += ' .'
  libs = 'bwt bunjalloo'
  if with_vera:
    libs += ' vera'
  arm9.uselib_local = libs.split()

  arm9.uselib = 'ARM9'
  arm9.target = app_name+'.elf'

  if buildsdl and bld.env['WITH_SDL']:
    sdl = arm9.clone('sdl')
    sdl.target = app_name

  arm9bin = TaskGen.task_gen()
  arm9bin.install_path = 0
  arm9bin.source = arm9.target
  arm9bin.target = app_name+'.arm'

  nds = bld.new_task_gen('ndstool')
  nds.source = app_name+'.arm'
  nds.target = app_name+'.nds'

def build_test(bld, uselibs='bunjalloo bwt'):
  """ Build a unit test program """
  if not bld.env['WITH_SDL']:
    return
  if not bld.env_of_name('sdl')['HAVE_CPPUNIT']:
    return
  uselibs = ' '.join([ '%s_default'%l for l in uselibs.split()])
  tst = bld.new_task_gen('cxx', 'program')
  tst.install_path = 0
  tst.env = bld.env_of_name('sdl').copy()
  tst.target = 'tester'
  tst.find_sources_in_dirs('.')
  tst.unit_test = 1
  tst.includes += ' .'
  tst.uselib_local = '%s cppunitmain'%(uselibs)
  tst.uselib = 'TEST HOST'

  # Cached unit tests.
  unit_test = bld.new_task_gen('unit_test')
  unit_test.env = bld.env_of_name('sdl').copy()
  unit_test.source = tst.target

def generate_banner(app, comment="", author=None):
  if author == None:
    author = '?'
    try:
      import pwd
      author = pwd.getpwnam(os.getlogin()).pw_gecos.split(',')[0]
    except:
      pass
  banner = '%s;%s;By %s'%(app.title(), comment, author)
  if sys.platform.startswith('win'):
    banner = banner.replace(' ', '_')
  return banner
