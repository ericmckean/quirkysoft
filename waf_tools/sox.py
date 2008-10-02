
from os.path import basename
import TaskGen
import Options
import Logs
import subprocess

def detect(conf):
  sox = conf.find_program('sox', var='SOX')
  conf.env['SOX'] = sox
  if sox:
    flags = " -r 16000 -s -b -c 1 "
    if not conf.env['SOXFLAGS']:
      conf.env['SOXFLAGS'] = flags.split()
    # check sox SUPPORTED FILE FORMATS has wav
    # else: sudo apt-get install libsox-fmt-all
    process_pipe = subprocess.Popen([sox, '-h'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    process_pipe.wait()
    have_wav = False
    v = process_pipe.stdout.readlines()
    v.extend(process_pipe.stderr.readlines())
    for l in v:
      if l.upper().startswith('SUPPORTED FILE FORMATS'):
        if 'wav' in l.split(':')[1].split():
          have_wav = True
          break
    conf.check_message(basename(sox), 'has wav file format', have_wav)

  if not have_wav:
    Logs.warn("""sox does not have wav. run `sudo apt-get install libsox-fmt-all'?""")
    conf.env['SOX'] = None

  dka_bin = '%s/bin' % (Options.options.devkitarm)
  padbin = conf.find_program('padbin', path_list=[dka_bin], var='PADBIN')
  conf.env['PADBIN'] = padbin
  if padbin:
    flags = " 4 "
    if not conf.env['PADBINFLAGS']:
      conf.env['PADBINFLAGS'] = flags.split()

def setup(bld):
  sox_str = "${SOX} ${SRC} ${SOXFLAGS} ${TGT}"
  TaskGen.declare_chain(
      name = 'sox',
      action = sox_str,
      ext_in = '.wav',
      ext_out = '.raw',
      color='GREEN',
      reentrant=1
  )

  # need a declare_chain that accepts a fn
  padbin_str = "${COPY} ${SRC} ${TGT} && ${PADBIN} ${PADBINFLAGS} ${TGT}"
  TaskGen.declare_chain(
      name = 'padbin',
      action = padbin_str,
      ext_in = '.raw',
      ext_out = '.snd',
      color='YELLOW',
      before='objcopy name2h',
      reentrant=0
  )

