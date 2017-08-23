"""
Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
Copyright (C) 2017 by Gabin An <agb94@kaist.ac.kr>

Licensed under the MIT License:
See the LICENSE file at the top-level directory of this distribution.
"""

"""evaluation
  compute objective value
"""

from cavm import ctype
from multiprocessing import Process, Queue
import sys
import weakref

# Global dict is needed to keep all objects alive.
# https://cffi.readthedocs.io/en/latest/using.html#working-with-pointers-structures-and-arrays
global_weakkeydict = weakref.WeakKeyDictionary()


class GlobalOptima(Exception):
    pass


def get_covered_branches(trace):
    true_covered = set()
    false_covered = set()
    result = []
    for log in trace:
        if log[0] not in false_covered:
            if log[1] == 0 and log[3] == 0.0:
                false_covered.add(log[0])
                result.append([log[0], False])
        if log[0] not in true_covered:
            if log[1] == 1 and log[2] == 0.0:
                true_covered.add(log[0])
                result.append([log[0], True])
    return result


def get_trace(dynamic_lib):
    """get trace from dynamic library"""
    trace_list = []
    while True:
        trace = dynamic_lib.get_trace()
        if trace.stmtid == -1:
            break
        else:
            trace_list.append((trace.stmtid, trace.result, trace.true_distance,
                               trace.false_distance))
    return trace_list

def get_dep_chain(dependency_map, exit_flow, targetbranch):
    """get dependency chain of target branch from dependency map"""
    dep_chain = [targetbranch]
    nodeid = targetbranch[0]
    while nodeid in dependency_map:
        parent = dependency_map[nodeid]
        for k, v in dependency_map.items():
            if k < nodeid and v == parent:
                for e in exit_flow[k]:
                    dep_chain.append([e[0], not e[1]])
        dep_chain.append(parent)
        nodeid = parent[0]

    return sorted(dep_chain, key=lambda x: x[0], reverse=True)


def get_divergence_point(trace, dependency_chain):
    """get divergence point between trace and dependency chain"""
    for depid, _ in enumerate(dependency_chain):
        dependency_node = dependency_chain[depid]
        closest = []
        dist = float("inf")
        for traceid, _ in enumerate(trace):
            executed = trace[traceid]
            if executed[0] == dependency_node[0]:
                if executed[1] == dependency_node[1]:
                    return None
                else:
                    thisdist = executed[2] if dependency_node[1] else executed[
                        3]
                    if thisdist < dist:
                        dist = thisdist
                        closest = [traceid, depid, dependency_node[1]]
        if not closest == []:
            return closest
    return [-1, -1, False]


class ObjFunc:
    """Objective Function"""

    def __init__(self, target_ftn, dlib, ffi, cfg, wte, p, d, sb):
        self.target_function = target_ftn
        self.dlib = dlib
        self.ffi = ffi
        self.params = p
        self.decls = d
        self.cfg = cfg
        self.wte = wte
        self.counter = 0
        self.dictionary = {}
        self.target_branch_id = None
        self.dependency_chain = None
        self.covered = []
        self.sandbox = sb

    def _malloc(self, c_type, val):
        obj = self.ffi.new(c_type, val)
        size = self.ffi.sizeof(obj[0])
        c_lib = self.ffi.dlopen(self.dlib)
        p = c_lib.malloc(size)
        if p == self.ffi.NULL:
            print("Out of memory")
            sys.exit()
        p = self.ffi.cast(c_type, p)
        self.ffi.memmove(p, obj, size)
        global_weakkeydict[p] = val
        return p

    def make_cffi_input(self, c_input):
        params = []
        for c_type in c_input:
            if isinstance(c_type, ctype.CType):
                params.append(
                    c_type.value.to_bytes(1, 'big', signed=True)
                    if isinstance(c_type, ctype.CTypeChar) else c_type.value)
            elif isinstance(c_type, ctype.CStruct):
                members = self.make_cffi_input(c_type.members)
                params.append(self.ffi.new(c_type.name + '*', members)[0])
            elif isinstance(c_type, ctype.CPointer):
                if c_type.pointee:
                    if isinstance(c_type.pointee, ctype.CType):
                        val = c_type.pointee.value.to_bytes(
                            1, 'big', signed=True) if isinstance(
                                c_type.pointee,
                                ctype.CTypeChar) else c_type.pointee.value
                        p = self._malloc(c_type.underlying_type + '*', val)
                    elif isinstance(c_type.pointee, ctype.CStruct):
                        val = self.make_cffi_input(c_type.pointee.members)
                        p = self._malloc(c_type.underlying_type + '*', val)
                    elif isinstance(c_type.pointee, list):
                        if isinstance(c_type.pointee[0], ctype.CTypeChar):
                            val = b''
                            for x in c_type.pointee:
                                val = val + x.value.to_bytes(
                                    1, 'big', signed=True)
                        else:
                            val = [x.value for x in c_type.pointee]
                        p = self.ffi.new(c_type.underlying_type + '[]', val)
                    global_weakkeydict[p] = val
                    params.append(p)
                else:
                    params.append(self.ffi.NULL)
        return params

    def set_target(self, branch_id):
        """set ObjFunc object variables"""
        self.counter = 0
        self.dictionary = {}
        self.target_branch_id = branch_id
        self.dependency_chain = get_dep_chain(self.cfg, self.wte, branch_id)

    def execute(self, c_input, no_trace=False):
        """execute target function"""
        self.counter += 1

        # disable caching
        # inputtuple = tuple(inputvector)
        # if inputtuple in self.dictionary:
        #     return self.dictionary[inputtuple]
        # else:
        def sandbox(q, lib, target, inputs):
            c_function = getattr(lib, target)
            c_function(*inputs)
            if no_trace:
                q.put([])
            else:
                q.put(get_trace(lib))

        c_lib = self.ffi.dlopen(self.dlib)
        cffi_input = self.make_cffi_input(c_input)
        if self.sandbox:
            q = Queue()
            p = Process(
                target=sandbox, args=(q, c_lib, self.target_function, cffi_input))
            p.start()
            p.join()

            trace = q.get() if p.exitcode == 0 else []
        else:
            c_function = getattr(c_lib, self.target_function)
            c_function(*cffi_input)
            if no_trace:
                trace = []
            else:
                trace = get_trace(c_lib)

        return trace


    def get_fitness(self, c_input):
        """get fitness score of input vector"""
        trace = self.execute(c_input)
        divpoint = get_divergence_point(trace, self.dependency_chain)
        if divpoint is None:
            # self.dictionary[inputtuple] = [0, 0]
            self.covered.append(self.target_branch_id)
            raise GlobalOptima

        if divpoint[0] == -1:
            app_lv = float("inf")
            branch_dist = float("inf")
        else:
            app_lv = divpoint[1]
            branch_dist = trace[divpoint[0]][3] if divpoint[2] == 0 else trace[
                divpoint[0]][2]
        fitness = [app_lv, branch_dist]
        # self.dictionary[inputtuple] = fitness
        return fitness
