# Written in 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Written in 2017 by Byeong Hyeon You <byou@kaist.ac.kr>
"""CAVM
  $ cavm file [options]
"""

import sys
import argparse
from subprocess import run
from os import path
from cffi import FFI

from cavm.clang import Parser
import cavm.avm
from cavm.evaluation import ObjFunc


def get_dep_map(dep_list):
    """get dependecy map from list"""
    dep_map = {}
    for i in dep_list:
        # i is (target_branch_id, parent_bid, condition)
        dep_map[i[0]] = [i[1], i[2]]
    return dep_map


def unroll_input(c_type, parser, inputs, decls):
    """flatten the input"""
    if c_type == 'void':
        raise NotImplementedError
    elif c_type in ['char', 'signed char']:
        inputs.append('char')
    elif c_type in ['unsigned char']:
        inputs.append('unsigned char')
    elif c_type in ['short', 'signed short', 'short int', 'signed short int']:
        inputs.append('short')
    elif c_type in ['unsigned short', 'unsigned short int']:
        inputs.append('unsigned short')
    elif c_type in ['int', 'signed', 'signed int']:
        inputs.append('int')
    elif c_type in ['unsigned', 'unsigned int']:
        inputs.append('unsigned int')
    elif c_type in ['long', 'signed long', 'long int', 'signed long int']:
        inputs.append('long')
    elif c_type in ['unsigned long', 'unsigned long int']:
        inputs.append('unsigned long')
    elif c_type in [
            'long long', 'signed long long', 'long long int',
            'signed long long int'
    ]:
        inputs.append('long long')
    elif c_type in ['unsigned long long', 'unsigned long long int']:
        inputs.append('unsigned long long')
    elif c_type == 'float':
        inputs.append('float')
    elif c_type == 'double':
        inputs.append('double')
    elif c_type == 'long double':
        inputs.append('long double')
    elif c_type == '_Bool':
        inputs.append('_Bool')
    elif c_type[-1:] == '*':
        unroll_input(c_type[:-1].strip(), parser, inputs, decls)
    elif c_type[:6] == 'struct':
        decl, fields = parser.get_decl(c_type[6:].strip())
        decls[c_type] = (decl, fields)
        for field in fields:
            unroll_input(field, parser, inputs, decls)
    else:
        raise NotImplementedError


def unroll_inputs(params, parser):
    """flatten the list of inputs"""
    ret = []
    decls = {}
    for parameter in params:
        unroll_input(parameter, parser, ret, decls)
    return (ret, decls)


def main():
    """
    A simple method that runs a CAVM.
    """
    parser = argparse.ArgumentParser(
        description='Do AVM search over a given c function')
    parser.add_argument(
        'target',
        metavar='<target file>',
        type=str,
        help='location of target c file')
    parser.add_argument(
        '-f',
        '--function',
        metavar='<target function>',
        type=str,
        help='name of function',
        required=False)
    parser.add_argument(
        '-b',
        '--branch',
        metavar='<target branch>',
        type=str,
        help='name of branch',
        required=False)
    parser.add_argument(
        '--min',
        metavar='<minimum random value>',
        type=int,
        help='random minimum',
        default=-100,
        required=False)
    parser.add_argument(
        '--max',
        metavar='<maximum random value>',
        type=int,
        help='random maximum',
        default=100,
        required=False)
    parser.add_argument(
        '--termination',
        metavar='<search iteration bound>',
        type=int,
        help='iteration bound',
        default=1000,
        required=False)
    parser.add_argument(
        '--prec',
        metavar='<precision>',
        type=int,
        help='precision of floating numbers',
        default=1,
        required=False)
    args = parser.parse_args()
    parser = Parser(args.target)

    if args.function:
        name, _ = path.splitext(args.target)
        dlib = name + '.so'
        cfg = get_dep_map(parser.instrument(args.function))

        proc = run([
            'g++', '-fPIC', '-shared', '-o', dlib, name + '.inst.cpp',
            '-std=c++11'
        ])
        if proc.returncode != 0:
            sys.exit(proc.returncode)

        decl, params = parser.get_decl(args.function)
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

        node_num = len(cfg.keys())
        if args.branch != None:
            predicate = args.branch[-1] == "T" or args.branch[-1] == "t"
            targetbranch = [int(args.branch[:-1]), predicate]
            branchlist = [targetbranch]
        else:
            branchlist = [
                branch
                for node in range(node_num)
                for branch in ([node, False], [node, True])
            ]

        unrolled_input, decls = unroll_inputs(params, parser)
        for decl in decls:
            ffi.cdef(decls[decl][0])

        obj = ObjFunc(args.function, dlib, ffi, cfg, params, decls)
        print(
            cavm.avm.search(obj, unrolled_input, branchlist, args.min,
                            args.max, args.termination, args.prec))

    else:
        print('Specify the target function using -f option.')
        print('Functions in %s:' % args.target)
        parser.get_functions()


if __name__ == '__main__':
    main()
