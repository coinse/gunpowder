# Written in 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Written in 2017 by Byeonghyeon You <byou@kaist.ac.kr>
"""CAVM
  $ cavm file [options]
"""

import sys
import argparse
import shutil
import subprocess
from os import path
from cffi import FFI

from cavm import avm
from cavm import clang
from cavm import commands
from cavm import ctype
from cavm import evaluation

CAVM_HEADER = path.dirname(__file__) + '/branch_distance.h'


def get_dep_map(dep_list):
    """get dependecy map from list"""
    dep_map = {}
    for i in dep_list:
        # i is (target_branch_id, parent_bid, condition)
        dep_map[i[0]] = [i[1], i[2]]
    return dep_map


def _get_decl(type_name, parser, decl_dict):
    if not type_name in decl_dict:
        if type_name[-1:] == '*':
            return _get_decl(type_name[:-1].strip(), parser, decl_dict)
        elif type_name[:6] == 'struct':
            decl, members = parser.get_decl(type_name[6:].strip())
            decl_dict[type_name] = (decl, members)
            for member in members:
                _get_decl(member, parser, decl_dict)


def get_decl_dict(parameters, parser):
    decl_dict = {}
    for param in parameters:
        _get_decl(param, parser, decl_dict)
    return decl_dict


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
    usage = [
        "",
        "Type 'cavm <command> -h' for help on a specific command.",
        "",
        "Available commands:",
        "\tinstrument",
        "\tsearch",
        "\trun",
    ]
    command_parser = argparse.ArgumentParser(
        usage='\n'.join(usage), add_help=False)
    command_parser.add_argument(
        'command', type=str, choices=['instrument', 'search', 'run'])

    opts, args = command_parser.parse_known_args()

    if opts.command == 'instrument':
        instrument(args)
    elif opts.command == 'search':
        search(args)
    elif opts.command == 'run':
        run(args)


def instrument(argv):
    cmd = commands.Instrument()
    parser = argparse.ArgumentParser(description=cmd.description)
    cmd.add_args(parser)
    args = parser.parse_args(argv)
    parser = clang.Parser(args.target, args.flags)
    if args.function:
        parser.instrument(args.function)
        shutil.copy(CAVM_HEADER, path.dirname(args.target))
    else:
        print('Specify the target function using -f option.')
        print('Functions in %s:' % args.target)
        parser.print_functions()
    return


def search(argv):
    cmd = commands.Search()
    parser = argparse.ArgumentParser(description=cmd.description)
    cmd.add_args(parser)
    args = parser.parse_args(argv)

    parser = clang.Parser(args.target, args.flags)

    if args.function:
        name, _ = path.splitext(args.target)
        dlib = args.binary
        cfg = get_dep_map(parser.instrument(args.function))

        decl, params = parser.get_decl(args.function)
        ffi = FFI()
        ffi.cdef(decl)

        with open(CAVM_HEADER, 'r') as f:
            lines = f.readlines()
            ffi.cdef(''.join(lines[13:22]))

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

        decls = get_decl_dict(params, parser)
        c_input = []
        for parameter in params:
            c_type = ctype.make_CType(parameter, decls)
            c_input.append(c_type)

        for decl in decls:
            ffi.cdef(decls[decl][0])

        obj = evaluation.ObjFunc(args.function, dlib, ffi, cfg, params, decls)
        set_search_params(c_input, args.min, args.max, args.prec)
        print(avm.search(obj, c_input, branchlist, args.termination))

    else:
        print('Specify the target function using -f option.')
        print('Functions in %s:' % args.target)
        parser.print_functions()


def run(argv):
    cmd = commands.Run()
    arg_parser = argparse.ArgumentParser(description='Run CAVM without own build chain')
    cmd.add_args(arg_parser)
    args = arg_parser.parse_args(argv)

    parser = clang.Parser(args.target, args.flags)

    if args.function:
        name, _ = path.splitext(args.target)
        dlib = name + '.so'
        cfg = get_dep_map(parser.instrument(args.function))
        shutil.copy(CAVM_HEADER, path.dirname(args.target))
        # End of Instrumentation

        proc = subprocess.run([
            'gcc', '-fPIC', '-shared', '-o', dlib, name + '.inst.c',
        ])
        if proc.returncode != 0:
            sys.exit(proc.returncode)

        decl, params = parser.get_decl(args.function)
        ffi = FFI()
        ffi.cdef(decl)

        with open(CAVM_HEADER, 'r') as f:
          lines = f.readlines()
          ffi.cdef(''.join(lines[13:22]))

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

        decls = get_decl_dict(params, parser)
        c_input = []
        for parameter in params:
            c_type = ctype.make_CType(parameter, decls)
            c_input.append(c_type)

        for decl in decls:
            ffi.cdef(decls[decl][0])

        obj = evaluation.ObjFunc(args.function, dlib, ffi, cfg, params, decls)
        set_search_params(c_input, args.min, args.max, args.prec)
        print(avm.search(obj, c_input, branchlist, args.termination))

    else:
        print('Specify the target function using -f option.')
        print('Functions in %s:' % args.target)
        parser.print_functions()

if __name__ == '__main__':
    main()
