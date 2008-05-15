import Action, Object, Params

def detect(conf):
  sox = conf.find_program('sox', var='SOX')
  conf.env['SOX'] = sox
  if sox:
    flags = " -r 16000 -s -b -c 1 "
    if not conf.env['SOXFLAGS']:
      conf.env['SOXFLAGS'] = flags.split()

  dka_bin = '%s/bin' % (Params.g_options.devkitarm)
  padbin = conf.find_program('padbin', path_list=[dka_bin], var='PADBIN')
  conf.env['PADBIN'] = padbin
  if padbin:
    flags = " 4 "
    if not conf.env['PADBINFLAGS']:
      conf.env['PADBINFLAGS'] = flags.split()

def pad_things(task):
  # TODO: pad the node to 4 bytes instead of just cping
  import misc
  misc.copy_func(task)
  result = Action.g_actions['padbin'].run(task)
  return result

def setup(bld):
  sox_str = "${SOX} ${SRC} ${SOXFLAGS} ${TGT}"
  Action.simple_action('sox', sox_str, color='GREEN', prio=70)
  Action.Action('pad', vars=[], func=pad_things , color='YELLOW', prio=71)

  padbin_str = "${PADBIN} ${PADBINFLAGS} ${TGT}"
  Action.simple_action('padbin', padbin_str, color='YELLOW', prio=72)

from Object import extension, taskgen, after

class sox_taskgen(Object.task_gen):
  def clone(self, variant):
    import Utils
    obj = Params.g_build.create_obj('sox')
    obj.source = [a for a in Utils.to_list(self.source)]
    if variant != 'default':
      obj.env = Params.g_build.env(variant).copy()
    return obj

  def apply(self):
    """ Convert a wav file to a raw file """
    find_source = self.path.find_source
    for filename in self.to_list(self.source):
      node = find_source(filename)
      out_raw = node.change_ext('.raw')
      out_padded = node.change_ext('.snd')

      tsk = self.create_task('sox')
      tsk.set_inputs(node)
      tsk.set_outputs(out_raw)

      tsk = self.create_task('pad')
      tsk.chmod = ''
      tsk.set_inputs(out_raw)
      tsk.set_outputs(out_padded)
