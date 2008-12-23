#! /usr/bin/env python
# encoding: utf-8

import os
import Task
import Utils
import TaskGen
import Options

def detect(conf):
  env = conf.env
  dka_bin = '%s/bin' % (Options.options.devkitarm)
  arm_eabi = 'arm-eabi-objcopy'
  objcopy = conf.find_program(arm_eabi, path_list=[dka_bin], var='OBJCOPY')
  if not objcopy:
    conf.fatal('objcopy was not found')
  env['OBJCOPY'] = objcopy
  copy = conf.find_program('cp', var='COPY')
  env['COPY'] = copy

def build_h(task):
  """ build h file from input file names """
  env = task.env
  filename = task.outputs[0].abspath(env)
  hfile  = open(filename, 'w')
  target = os.path.basename(filename).replace('.', '_')
  hfile.write("""#ifndef %s_seen
#define %s_seen
#ifdef __cplusplus
extern "C" {
#endif\n"""%(target, target))
  for i in task.inputs:
    symbol = os.path.basename(i.srcpath(env)).replace('.', '_')
    hfile.write("""
	extern const u16 _binary_%s_bin_end[];
	extern const u16 _binary_%s_bin_start[];
	extern const u32 _binary_%s_bin_size[];
"""%(symbol, symbol, symbol))
  hfile.write("""
#ifdef __cplusplus
};
#endif
#endif
""")
  return 0

class name2h_taskgen(TaskGen.task_gen):
  def apply(self):
    find_resource = self.path.find_resource
    task = self.create_task('name2h', self.env)
    task.set_inputs([find_resource(Utils.split_path(filename))
                    for filename in self.to_list(self.source)])
    task.set_outputs(self.path.find_or_declare(Utils.split_path(self.target)))

def setup(bld):
  # declare the Task Generators
  # objcopy
  # copies file.ext to file.ext.bin, then file.ext.bin to file.ext.o
  bin2o_str = 'cd ${SRC[0].bld_dir(env)} && ${OBJCOPY} ${OBJCOPYFLAGS} ${SRC[0].name} ${TGT[0].name}'
  copy_str = '${COPY} ${SRC} ${TGT}'

  # pal -> pal.bin
  for i in """
        .img
        .pal
        .map
        .snd
      """.split():
    TaskGen.declare_chain(
        name='in2bin',
        action=copy_str,
        ext_in=i,
        ext_out=i+'.bin')
  # pal.bin -> pal.bin.o
  TaskGen.declare_chain(
      name='bin2o',
      action=bin2o_str,
      ext_in='.bin',
      ext_out='.o',
      after="in2bin",
      reentrant=0)

  # bin2o
  # copies file.elf to file.arm
  objcopy_str = '${OBJCOPY} -O binary ${SRC} ${TGT}'
  TaskGen.declare_chain(
      name='objcopy',
      action=objcopy_str,
      ext_in='.elf',
      ext_out='.arm',
      before='ndstool_9 ndstool_9_b',
      after='cc_link cxx_link',
      reentrant=0)

  # name2h
  Task.task_type_from_func('name2h', vars=[], func=build_h, before='cc cxx')
