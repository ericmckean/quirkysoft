"""
Waf tool for generating a .nds file from 1 or 2 arm cores, optionally using a
banner.
"""
import TaskGen
import Task
import Options
from Logs import warn
import sys, os

class ndstool_taskgen(TaskGen.task_gen):
  def apply(self):
    find_build = self.path.find_or_declare
    input_nodes = [find_build(filename) for filename in
                                self.to_list(self.source)]
    taskname = 'ndstool'
    banner = getattr(self, 'banner', False)
    if banner:
      taskname += '_banner'
      self.env['NDSTOOL_BANNER'] = banner
      icon = getattr(self, 'icon', '')
      node = self.path.find_resource(icon)
      if not node:
        warn('ndstool: icon "%s" not found'%icon)
        sys.exit(1)
      input_nodes.append(node)
    os.environ['DEVKITPRO'] = self.env['DEVKITPRO']
    task = self.create_task(taskname, self.env)
    task.set_inputs(input_nodes)
    task.set_outputs(find_build(self.target))

def detect(conf):
  dka_bin = os.path.join(Options.options.devkitarm, 'bin')
  ndstool = 'ndstool'
  ndstool = conf.find_program(ndstool, path_list=[dka_bin], var='NDSTOOL')
  if not ndstool:
    conf.fatal('ndstool was not found')
  conf.env['NDSTOOL'] = ndstool

def setup(bld):
  outfile = ''
  if sys.platform.startswith('linux'):
    outfile = ' > /dev/null'
  ndstool_str = '${NDSTOOL} -c ${TGT} -9 ${SRC[0].bldpath(env)} %s '+outfile
  Task.simple_task_type('ndstool', ndstool_str%'', color='BLUE', after="objcopy", before="unit_test")
  Task.simple_task_type('ndstool_banner', ndstool_str%' -b ${SRC[1].srcpath(env)} ${NDSTOOL_ICON} \'${NDSTOOL_BANNER}\'',
      color='BLUE', after="objcopy", before="unit_test")
