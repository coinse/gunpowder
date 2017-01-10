import cavm
import argparse
from subprocess import Popen, PIPE
from cffi import FFI

def get_trace(dynamic_lib):
  trace = []
  for i in range(dynamic_lib.getTraceSize()):
    t = dynamic_lib.getTrace(i)
    trace.append((t.stmtid, t.result, t.trueDistance, t.falseDistance))
  return trace

parser = argparse.ArgumentParser(description='Instrument and execute given c file')
parser.add_argument('target', metavar='<target file>', type=str, help='location of target c file')
parser.add_argument('-f', '--function', metavar='<target function>', type=str, help='name of function', required=False)
args = parser.parse_args()

if args.function:
  p = cavm.Parser(args.target)
  p.instrument(args.function)

  with Popen(['g++', '-fPIC', '-shared', '-o', args.target[:-2]+'.so', args.target[:-2]+'.inst.cpp', '-std=c++11'], stdout=PIPE) as proc:
      print(proc.stdout.read())

  ffi = FFI()
  ffi.cdef(p.get_decl(args.function))
  ffi.cdef("""
    typedef struct {
      int stmtid;
      int result;
      double trueDistance;
      double falseDistance;
    } traceItem;

    traceItem getTrace(int idx);

    int getTraceSize();
  """)
  C = ffi.dlopen(args.target[:-2]+'.so')
  f = getattr(C, args.function)
  print(f(1,2,3))

  trace = get_trace(C)
  print(trace)

else:
  # TODO: print out the list of functions in target code
  print('list of functions')
