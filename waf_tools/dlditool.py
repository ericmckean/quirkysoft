""" Waf tool for running the dlditool provided with DevkitARM """
import Options, Task, TaskGen, misc

def dldi_func(tsk):
  """
  Copy function callback so the patched nds is not recompiled every time
  """
  misc.copy_func(tsk)

  dlditsk = Task.TaskBase.classes['dlditool'](tsk.env)
  dlditsk.set_inputs(tsk.inputs)
  dlditsk.set_outputs(tsk.outputs)
  result = dlditsk.run()
  return result

after = 'ndstool_9 ndstool_9_b'
class dlditool_taskgen(TaskGen.task_gen):
  def apply(self):
    find = self.path.find_or_declare
    task = self.create_task('copy', self.env)
    task.after = after
    task.chmod = ''
    task.fun = dldi_func
    task.set_inputs([find(filename) for filename in self.to_list(self.source)])
    task.set_outputs(find(self.target))

def detect(conf):
  """ Detect dlditool in devkitarm path """
  dka_bin = '%s/bin' % (Options.options.devkitarm)
  dlditool = 'dlditool'
  dlditool = conf.find_program(dlditool, path_list=[dka_bin], var='DLDITOOL')
  if not dlditool:
    conf.fatal('dlditool was not found')
  conf.env['DLDITOOL'] = dlditool

def setup(bld):
  """ Create the dlditool Action """
  dlditool_str = '${DLDITOOL} ${DLDIFLAGS} ${TGT} > /dev/null'
  Task.simple_task_type('dlditool', dlditool_str, color='BLUE', after=after)
