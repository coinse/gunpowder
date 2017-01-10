import cavm
import argparse
from subprocess import Popen, PIPE
from cffi import FFI

parser = argparse.ArgumentParser(description='Instrument and execute given c file')
parser.add_argument('target', metavar='<target file>', type=str, help='location of target c file')
parser.add_argument('-f', '--function', metavar='<target function>', type=str, help='name of function', required=False)
args = parser.parse_args()

if args.function:
  p = cavm.Parser(args.target)
  p.instrument(args.function)

  with Popen(['g++', '-fPIC', '-shared', '-o', args.target[:-2]+'.so', args.target[:-2]+'.inst.c'], stdout=PIPE) as proc:
      print(proc.stdout.read())

  ffi = FFI()
  ffi.cdef(p.get_decl(args.function))
  C = ffi.dlopen(args.target[:-2]+'.so')
  f = getattr(C, args.function)
  print(f(1,2,3))

else:
  # TODO: print out the list of functions in target code
  print('list of functions')
