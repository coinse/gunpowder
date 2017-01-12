import os
import subprocess

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
    for traceid in range(len(trace)):
        executed = trace[traceid]
        for depid in range(len(dependency_chain)):
            dependency_node = dependency_chain[depid]
            if(executed[0] == dependency_node[0]):
                if(executed[1] != dependency_node[1]):
                    return [traceid, depid, dependency_node[1]]

    return None

class ObjFunc:
    def __init__(self, target_ftn, target_bid, dlib, ffi, cfg):
        self.target_function = target_ftn
        self.target_branch_id = target_bid
        self.dlib = dlib
        self.ffi = ffi
        self.dependency_chain = get_dep_chain(cfg, target_bid)
        self.counter = 0
        self.dictionary = {}

    def get_fitness(self, inputvector):
        self.counter += 1
        inputtuple = tuple(inputvector)
        if inputtuple in self.dictionary:
            return self.dictionary[inputtuple]
        else:
            C = self.ffi.dlopen(self.dlib)
            f = getattr(C, self.target_function)
            f(*inputvector)
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
    
