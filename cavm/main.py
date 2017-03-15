# Written in 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Written in 2017 by Byeonghyeon You <byou@kaist.ac.kr>
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
from cavm import ctype


def get_dep_map(dep_list):
    """get dependecy map from list"""
    dep_map = {}
    for i in dep_list:
        # i is (target_branch_id, parent_bid, condition)
        dep_map[i[0]] = [i[1], i[2]]
    return dep_map


def unroll_input(c_type, parser, inputs, decls):
    """flatten the input"""
    if c_type[-1:] == '*':
        unroll_input(c_type[:-1].strip(), parser, inputs, decls)
    elif c_type[:6] == 'struct':
        decl, fields = parser.get_decl(c_type[6:].strip())
        decls[c_type] = (decl, fields)
        for field in fields:
            unroll_input(field, parser, inputs, decls)
    else:
        inputs.append(cavm.ctype.c_type_factory(c_type))


def make_CType(c_type, parser, stop_recursion=False):
    if c_type[-1:] == '*':
        if stop_recursion:
            return ctype.CPointer(c_type[:-1].strip())
        else:
            pointer = ctype.CPointer(c_type[:-1].strip())
            pointer.pointee = make_CType(c_type[:-1].strip(), parser)
            return pointer
    elif c_type[:6] == 'struct':
        struct = ctype.CStruct(c_type)
        decl, fields = parser.get_decl(c_type[6:].strip())
        struct.decl = (decl, fields)
        for field in fields:
            if field == c_type + ' *':
                struct.is_recursive = True
                struct.members.append(make_CType(field, parser, True))
            else:
                struct.members.append(make_CType(field, parser))
        return struct
    else:
        return ctype.c_type_factory(c_type)


def get_decl_dict(inputs):
    decl = {}
    for c_type in inputs:
        if isinstance(c_type, ctype.CStruct):
            decl[c_type.name] = c_type.decl
            decl.update(get_decl_dict(c_type.members))
        elif isinstance(c_type, ctype.CPointer):
            if c_type.pointee:
                decl.update(get_decl_dict([c_type.pointee]))
    return decl


def unroll_inputs(params, parser):
    """flatten the list of inputs"""
    ret = []
    decls = {}
    for parameter in params:
        unroll_input(parameter, parser, ret, decls)
    return (ret, decls)


def set_search_params(c_input, minimum, maximum, prec):
    for c_type in c_input:
        if isinstance(c_type, ctype.CType):
            c_type.set_min(minimum)
            c_type.set_max(maximum)
            if c_type.is_floating() and prec is not None:
                c_type.precision = prec
        elif isinstance(c_type, ctype.CStruct):
            set_search_params(c_type.members, minimum, maximum, prec)


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

        #unrolled_input, decls = unroll_inputs(params, parser)
        c_input = []
        for parameter in params:
            c_type = make_CType(parameter, parser)
            c_input.append(c_type)
        decls = get_decl_dict(c_input)
        for decl in decls:
            ffi.cdef(decls[decl][0])

        obj = ObjFunc(args.function, dlib, ffi, cfg, params, decls)
        set_search_params(c_input, args.min, args.max, args.prec)
        print(cavm.avm.search(obj, c_input, branchlist, args.termination))

    else:
        print('Specify the target function using -f option.')
        print('Functions in %s:' % args.target)
        parser.print_functions()


if __name__ == '__main__':
    main()
