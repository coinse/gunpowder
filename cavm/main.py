import argparse
from subprocess import Popen, PIPE
from cffi import FFI

import cavm
import avm


def get_dep_map(dep_list):
    map = {}
    for d in dep_list:
      map[d[0]] = [d[1], d[2]]
    return map

def main():
  parser = argparse.ArgumentParser(description='Do AVM search over a given c function')
  parser.add_argument('target', metavar='<target file>', type=str, help='location of target c file')
  parser.add_argument('-f', '--function', metavar='<target function>', type=str, help='name of function', required=False)
  args = parser.parse_args()

  if args.function:
    dlib = args.target[:-2]+'.so'
    p = cavm.Parser(args.target)
    cfg = get_dep_map(p.instrument(args.function))

    with Popen(['g++', '-fPIC', '-shared', '-o', dlib, args.target[:-2]+'.inst.cpp', '-std=c++11'], stdout=PIPE) as proc:
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
      
      void resetTrace();
    """)

    avm.search(args.function, dlib, ffi, cfg)

  else:
    # TODO: print out the list of functions in target code
    print('list of functions')

if __name__ == '__main__':
  main()
