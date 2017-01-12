import sys
import argparse
from subprocess import run, PIPE
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
  parser.add_argument('--range', metavar='<initial vector random range>', type=int, help='random range', default=100, required=False)
  parser.add_argument('--termination', metavar='<search iteration bound>', type=int, help='iteration bound', default=1000, required=False)
  args = parser.parse_args()

  if args.function:
    dlib = args.target[:-2]+'.so'
    p = cavm.Parser(args.target)
    cfg = get_dep_map(p.instrument(args.function))

    proc = run(['g++', '-fPIC', '-shared', '-o', dlib, args.target[:-2]+'.inst.cpp', '-std=c++11'], stdout=PIPE, stderr=PIPE)
    if proc.returncode != 0:
      sys.exit(proc.stderr)

    decl, params = p.get_decl(args.function)
    ffi = FFI()
    ffi.cdef(decl)
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

    avm.search(args.function, dlib, ffi, cfg, params, args.range, args.termination)

  else:
    # TODO: print out the list of functions in target code
    print('list of functions')

if __name__ == '__main__':
  main()
