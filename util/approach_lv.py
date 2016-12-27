def gettrace():
    trace = []
    with open('../trace') as tracefile:
        for line in tracefile:
            parts = line.split()
            trace.append(int(parts[0]))

    return trace


def getdepmaps():
    Depmaps = {}
    with open('../controldep.txt') as depfile:
        for line in depfile:
            parts = line.split()
            Depmaps[int(parts[0])] = int(parts[1])

    return Depmaps


def getlv(branch):
    trace = gettrace()
    Depmaps = getdepmaps()

    nodeid = int(branch[:-1])

    cdps = [nodeid]
    while nodeid in Depmaps:
        nodeid = Depmaps[nodeid]
        cdps.append(nodeid)

    app_lv = 0
    for node in cdps:
        if node not in trace:
            app_lv = app_lv + 1

    return app_lv


def main():
    approachlevel = getlv("5T")
    print(approachlevel)


if __name__ == '__main__':
    main()
