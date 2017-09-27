"""
Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
Copyright (C) 2017 by Gabin An <agb94@kaist.ac.kr>

Licensed under the MIT License:
See the LICENSE file at the top-level directory of this distribution.
"""


def get_dep_chain(dependency_map, exit_flow, targetbranch):
    """get dependency chain of target branch from dependency map"""
    dep_chain = [targetbranch]
    nodeid = targetbranch[0]
    while nodeid in dependency_map:
        parent = dependency_map[nodeid]
        for k, v in dependency_map.items():
            if k < nodeid and v == parent:
                for e in exit_flow[k]:
                    dep_chain.append(tuple(e[0], not e[1]))
        dep_chain.append(parent)
        nodeid = parent[0]

    return sorted(dep_chain, key=lambda x: x[0], reverse=True)


def _get_divergence_point(trace, dependency_chain):
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


def get_fitness(trace, dependency_chain):
    """get fitness score of input vector"""
    divpoint = _get_divergence_point(trace, dependency_chain)
    if divpoint is None:
        return [0, 0]

    if divpoint[0] == -1:
        app_lv = float("inf")
        branch_dist = float("inf")
    else:
        app_lv = divpoint[1]
        branch_dist = trace[divpoint[0]][3] if divpoint[2] == 0 else trace[
            divpoint[0]][2]
    fitness = [app_lv, branch_dist]
    return fitness

