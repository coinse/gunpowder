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
from cavm import report

CAVM_HEADER = path.dirname(__file__) + '/branch_distance.h'
STRCMP2 = path.dirname(__file__) + '/strcmp2.c'


def get_dep_map(dep_list):
    """get dependecy map from list"""
    dep_map = {}
    wte = {}
    for i in dep_list:
        # i is (target_branch_id, parent_bid, condition)
        if i[0] > 0:
            dep_map[i[0]] = [i[1], i[2]]
        elif i[0] == i[1]:
            wte[-i[0]] = [[-i[1], i[2]]]
    for b in reversed(sorted(dep_map)):
        if b not in wte:
            # initialize
            wte[b] = []
        dep_size = len(wte[b])
        while True:
            for k, v in dep_map.items():
                if v[0] == b or v[0] in list(map(lambda x: x[0], wte[b])):
                    wte[b] += filter(lambda x: x not in wte[b], wte[k])
            if len(wte[b]) > dep_size:
                dep_size = len(wte[b])
            else:
                break
    return dep_map, wte


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
        run_instrument(args)
    elif opts.command == 'search':
        run_search(args)
    elif opts.command == 'run':
        run(args)


def instrument(c_parser, target_file, target_function):
    if target_function:
        cfg = c_parser.instrument(target_function)
        shutil.copy(CAVM_HEADER, path.dirname(target_file))
        shutil.copy(STRCMP2, path.dirname(target_file))
    else:
        print('Specify the target function using -f option.')
        print('Functions in %s:' % target_file)
        c_parser.print_functions()
        sys.exit()
    return cfg


def search(c_parser, cfg, wte, target_function, dlib, args):

    decl, params = c_parser.get_decl(target_function)
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
            for node in reversed(range(1, node_num+1))
            for branch in ([node, False], [node, True])
        ]

    decls = get_decl_dict(params, c_parser)
    c_input = []
    for parameter in params:
        c_type = ctype.make_CType(parameter, decls)
        c_input.append(c_type)

    for decl in decls:
        ffi.cdef(decls[decl][0])

    obj = evaluation.ObjFunc(target_function, dlib, ffi, cfg, wte, params, decls, args.sandbox)
    set_search_params(c_input, args.min, args.max, args.prec)
    result = avm.search(obj, c_input, branchlist, args.termination)
    print(report.make_JSON(result))
    if args.coverage:
        report.coverage(ffi, args.coverage, args.function, result)
    report.make_csv(result, target_function)


def run_instrument(argv):
    cmd = commands.Instrument()
    parser = argparse.ArgumentParser(description=cmd.description)
    cmd.add_args(parser)
    args = parser.parse_args(argv)
    c_parser = clang.Parser(args.target, args.flags)
    instrument(c_parser, args.target, args.function)
    name, _ = path.splitext(args.target)
    print('%s.inst.c is created.' % name)


def run_search(argv):
    cmd = commands.Search()
    parser = argparse.ArgumentParser(description=cmd.description)
    cmd.add_args(parser)
    args = parser.parse_args(argv)

    c_parser = clang.Parser(args.target, args.flags)
    cfg, wte = get_dep_map(instrument(c_parser, args.target, args.function))

    search(c_parser, cfg, wte, args.function, args.binary, args)


def run(argv):
    cmd = commands.Run()
    parser = argparse.ArgumentParser(description=cmd.description)
    cmd.add_args(parser)
    args = parser.parse_args(argv)

    c_parser = clang.Parser(args.target, args.flags)
    cfg, wte = get_dep_map(instrument(c_parser, args.target, args.function))

    name, _ = path.splitext(args.target)
    dlib = name + '.so'
    proc = subprocess.run([
        'gcc',
        '-fPIC',
        '-shared',
        '-o',
        dlib,
        name + '.inst.c',
    ])
    if proc.returncode != 0:
        sys.exit(proc.returncode)
    args.coverage = name + '.gcov.so'
    proc = subprocess.run([
        'gcc',
        '-fPIC',
        '-shared',
        '--coverage',
        '-o',
        args.coverage,
        args.target,
    ])
    if proc.returncode != 0:
        sys.exit(proc.returncode)

    search(c_parser, cfg, wte, args.function, dlib, args)


if __name__ == '__main__':
    main()
