import os
import subprocess

def get_trace():
    trace = []
    with open('trace') as tracefile:
        for line in tracefile:
            parts = line.split()
            trace.append([int(parts[0]), int(parts[1]), float(parts[2]), float(parts[3])])

    return trace


def get_depmaps():
    Depmaps = {}
    with open('controldep.txt') as depfile:
        for line in depfile:
            parts = line.split()
            if int(parts[1]) != -1:
                predicate = 1 if parts[2] == "true" else 0
                Depmaps[int(parts[0])] = [int(parts[1]), predicate]

    return Depmaps


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


def fitness(targetbranch, inputvector):
    if os.path.exists("trace"):
        os.remove("trace")
    run = subprocess.Popen(["./a.out", str(inputvector[0]), str(inputvector[1]), str(inputvector[2])])
    run.wait()
    trace = get_trace()
    dependency_map = get_depmaps()

    dependency_chain = get_dep_chain(dependency_map, targetbranch)
    divpoint = get_divergence_point(trace, dependency_chain)
    if divpoint == None:
        return 0

    app_lv = divpoint[1]
    branch_dist = trace[divpoint[0]][3] if divpoint[2] == 0 else trace[divpoint[0]][2]
    return app_lv + (1 - 1.001 ** (-branch_dist))
    
