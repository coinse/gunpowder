import sys
import argparse
from subprocess import run, PIPE
from cffi import FFI
from os import path

import cavm
import avm
from evaluation import ObjFunc


def get_dep_map(dep_list):
    map = {}
    for d in dep_list:
      map[d[0]] = [d[1], d[2]]
    return map

def unroll_input(t, parser, inputs, decls):
    if t in ['unsigned int', 'int', 'long', 'float', 'double']:
      inputs.append(t)
    elif t[-1:] == '*':
      raise NotImplementedError
    elif t[:6] == 'struct':
      decl, fields = parser.get_decl(t[6:].strip())
      decls[t] = (decl, fields);
      for f in fields:
        unroll_input(f, parser, inputs, decls)
    else:
      raise NotImplementedError

def unroll_inputs(params, parser):
    ret = []
    decls = {}
    for t in params:
      unroll_input(t, parser, ret, decls)
    return (ret, decls)

def main():
  parser = argparse.ArgumentParser(description='Do AVM search over a given c function')
  parser.add_argument('target', metavar='<target file>', type=str, help='location of target c file')
  parser.add_argument('-f', '--function', metavar='<target function>', type=str, help='name of function', required=False)
  parser.add_argument('-b', '--branch', metavar='<target branch>', type=str, help='name of branch', required=False)
  parser.add_argument('--min', metavar='<minimum random value>', type=int, help='random minimum', default=-100, required=False)
  parser.add_argument('--max', metavar='<maximum random value>', type=int, help='random maximum', default=100, required=False)
  parser.add_argument('--termination', metavar='<search iteration bound>', type=int, help='iteration bound', default=1000, required=False)
  parser.add_argument('--prec', metavar='<precision>', type=int, help='precision of floating numbers', default=1, required=False)
  args = parser.parse_args()

  if args.function:
    name, ext = path.splitext(args.target)
    dlib = name+'.so'
    p = cavm.Parser(args.target)
    cfg = get_dep_map(p.instrument(args.function))

    proc = run(['g++', '-fPIC', '-shared', '-o', dlib, name+'.inst.cpp', '-std=c++11'])
    if proc.returncode != 0:
      sys.exit(proc.returncode)

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

    NODENUM = len(cfg.keys())
    if args.branch != None:
        predicate = args.branch[-1] == "T" or args.branch[-1] == "t" 
        targetbranch = [int(args.branch[:-1]), predicate]
        branchlist = [targetbranch]
    else:
        branchlist = [branch for node in range(NODENUM) for branch in ([node, False], [node, True])]

    unrolled_input, decls = unroll_inputs(params, p)
    for d in decls:
      ffi.cdef(decls[d][0])

    obj = ObjFunc(args.function, dlib, ffi, cfg, params, decls)
    avm.search(obj, unrolled_input, branchlist, args.min, args.max, args.termination, args.prec)

  else:
    # TODO: print out the list of functions in target code
    print('list of functions')

if __name__ == '__main__':
  main()
