""" Improved unit test module. """

import TaskGen
import Task

class unit_test_taskgen(TaskGen.task_gen):
  def apply(self):
    find = self.path.find_or_declare
    task = self.create_task('unit_test', self.env)
    task.run_from_srcdir = getattr(self, "run_from_srcdir", True)
    task.set_inputs([find(filename) for filename in self.to_list(self.source)])
    task.set_outputs(find('test.passed'))

def detect(conf):
  pass

def setup(bld):
  def run_unit_test(task):
    """ Execute unit tests, creating a test.passed cache file. Based on
     www.scons.org/wiki/UnitTest modified to run tests in the source dir """
    import os, subprocess
    import Options
    test_exe = task.inputs[0]
    app = test_exe.abspath(task.env)
    blddir = os.path.dirname(app)
    srcdir = os.path.dirname(test_exe.abspath())
    cwd = srcdir if task.run_from_srcdir else None
    target = os.path.join(blddir, task.outputs[0].name)
    process_pipe = subprocess.Popen(app, stdout=subprocess.PIPE, cwd=cwd)
    process_pipe.wait()
    returncode = process_pipe.returncode
    if Options.options.verbose or returncode:
      if task.run_from_srcdir:
        # this sorts out jump-to-errors in e.g. vim
        print "waf: Entering directory `%s'" % (srcdir)
      print process_pipe.stdout.read()
    out = 'FAILED'
    if returncode == 0:
      out = 'PASSED'
    open(str(target),'w').write(out+"\n")
    return returncode
  Task.task_type_from_func('unit_test', vars=[], func=run_unit_test, color='PINK', after="cxx_link")
