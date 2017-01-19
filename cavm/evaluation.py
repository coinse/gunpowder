import os
import subprocess
import warnings

def get_trace(dynamic_lib):
  trace = []
  for i in range(dynamic_lib.getTraceSize()):
    t = dynamic_lib.getTrace(i)
    trace.append((t.stmtid, t.result, t.trueDistance, t.falseDistance))
  dynamic_lib.resetTrace()
  return trace


def get_dep_chain(dependency_map, targetbranch):
    Dep_chain = [targetbranch]
    nodeid = targetbranch[0]
    while nodeid in dependency_map:
        parent = dependency_map[nodeid]
        Dep_chain.append(parent)
        nodeid = parent[0]
    return Dep_chain


def get_divergence_point(trace, dependency_chain):
    for depid in range(len(dependency_chain)):
        dependency_node = dependency_chain[depid]
        closest = []
        dist = float("inf")
        for traceid in range(len(trace)):
            executed = trace[traceid]
            if(executed[0] == dependency_node[0]):
                if(executed[1] == dependency_node[1]):
                    return None
                else:
                    thisdist = executed[2] if dependency_node[1] else executed[3]
                    if thisdist < dist:
                        dist = thisdist
                        closest = [traceid, depid, dependency_node[1]]
        else:
            if not closest == []:
                return closest
    warnings.warn("May result wrong output: trace does not contain any control dependent node of " + str(dependency_chain[0]))

class ObjFunc:
    def __init__(self, target_ftn, dlib, ffi, cfg, p, d):
        self.target_function = target_ftn
        self.dlib = dlib
        self.ffi = ffi
        self.params = p
        self.decls = d
        self.cfg = cfg
        self.counter = 0
        self.dictionary = {}

    def vector_to_input(self, vector, idx, params):
        i = []
        for t in params:
          if t in ['unsigned int', 'int', 'long', 'float', 'double']:
            i.append(vector[idx])
            idx += 1
          elif t[-1:] == '*':
            raise NotImplementedError
          elif t[:6] == 'struct':
            fields = self.decls[t][1]
            val, idx = self.vector_to_input(vector, idx, fields)
            # struct should be made using pointer and dereferenced
            i.append(self.ffi.new(t+"*", val)[0])
        return (i, idx)

    def set_target(self, branch_id):
        self.counter = 0
        self.dictionary = {}
        self.target_branch_id = branch_id
        self.dependency_chain = get_dep_chain(self.cfg, branch_id)

    def get_fitness(self, inputvector):
        self.counter += 1
        inputtuple = tuple(inputvector)
        if inputtuple in self.dictionary:
            return self.dictionary[inputtuple]
        else:
            C = self.ffi.dlopen(self.dlib)
            f = getattr(C, self.target_function)
            input, _ = self.vector_to_input(inputvector, 0, self.params)
            f(*input)
            trace = get_trace(C)
            divpoint = get_divergence_point(trace, self.dependency_chain)
            if divpoint == None:
                self.dictionary[inputtuple] = [0, 0]
                return [0, 0]

            app_lv = divpoint[1]
            branch_dist = trace[divpoint[0]][3] if divpoint[2] == 0 else trace[divpoint[0]][2]
#            fitness = app_lv + (1 - 1.001 ** (-branch_dist))
            fitness = [app_lv, branch_dist]
            self.dictionary[inputtuple] = fitness
            return fitness
    
