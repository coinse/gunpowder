# Written in 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Written in 2017 by Byeong Hyeon You <byou@kaist.ac.kr>
"""evaluation
  compute objective value
"""

from cavm import ctype

def get_trace(dynamic_lib):
    """get trace from dynamic library"""
    trace_list = []
    for i in range(dynamic_lib.getTraceSize()):
        trace = dynamic_lib.getTrace(i)
        trace_list.append((trace.stmtid, trace.result, trace.trueDistance,
                           trace.falseDistance))
    dynamic_lib.resetTrace()
    return trace_list


def get_dep_chain(dependency_map, targetbranch):
    """get dependency chain of target branch from dependency map"""
    dep_chain = [targetbranch]
    nodeid = targetbranch[0]
    while nodeid in dependency_map:
        parent = dependency_map[nodeid]
        dep_chain.append(parent)
        nodeid = parent[0]
    return dep_chain


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

    def __init__(self, target_ftn, dlib, ffi, cfg, p, d):
        self.target_function = target_ftn
        self.dlib = dlib
        self.ffi = ffi
        self.params = p
        self.decls = d
        self.cfg = cfg
        self.counter = 0
        self.dictionary = {}
        self.target_branch_id = None
        self.dependency_chain = None

    def vector_to_input(self, vector, idx, params):
        """convert python vector list to C input format"""
        i = []
        scalar_types = ['unsigned int', 'int', 'long', 'float', 'double']
        for c_type in params:
            if c_type in scalar_types:
                i.append(vector[idx])
                idx += 1
            elif c_type[-1:] == '*':
                underlying_type = c_type[:-1].strip()
                if underlying_type in scalar_types:
                    i.append(self.ffi.new(c_type, vector[idx]))
                elif underlying_type[:6] == 'struct':
                    fields = self.decls[underlying_type][1]
                    val, idx = self.vector_to_input(vector, idx, fields)
                    i.append(self.ffi.new(c_type, val))
            elif c_type[:6] == 'struct':
                fields = self.decls[c_type][1]
                val, idx = self.vector_to_input(vector, idx, fields)
                # struct should be made using pointer and dereferenced
                i.append(self.ffi.new(c_type + "*", val)[0])
        return (i, idx)

    def make_cffi_input(self, c_input):
        params = []
        for c_type in c_input:
            if isinstance(c_type, ctype.CType):
                params.append(c_type.value)
            elif isinstance(c_type, ctype.CStruct):
                members = self.make_cffi_input(c_type.members)
                params.append(self.ffi.new(c_type.name + '*', members)[0])
            elif isinstance(c_type, ctype.CPointer):
                params.append(self.ffi.new(c_type.underlying_type + '*'))
        return params

    def set_target(self, branch_id):
        """set ObjFunc object variables"""
        self.counter = 0
        self.dictionary = {}
        self.target_branch_id = branch_id
        self.dependency_chain = get_dep_chain(self.cfg, branch_id)

    def get_fitness(self, c_input):
        """get fitness score of input vector"""
        self.counter += 1
        # disable caching
        # inputtuple = tuple(inputvector)
        # if inputtuple in self.dictionary:
        #     return self.dictionary[inputtuple]
        # else:
        c_lib = self.ffi.dlopen(self.dlib)
        c_function = getattr(c_lib, self.target_function)
        cffi_input = self.make_cffi_input(c_input)
        c_function(*cffi_input)
        trace = get_trace(c_lib)
        divpoint = get_divergence_point(trace, self.dependency_chain)
        if divpoint is None:
            # self.dictionary[inputtuple] = [0, 0]
            return [0, 0]

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
