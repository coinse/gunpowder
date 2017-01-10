#include <cstdio>
#include <vector>
#include <tuple>

typedef struct {
  int stmtid;
  int result;
  double trueDistance;
  double falseDistance;
} traceItem;

typedef std::vector<traceItem> Trace;

Trace CAVM_TRACE;

int tracelog(int stmtid, int result, double trueDistance, double falseDistance) {
    CAVM_TRACE.push_back(traceItem{stmtid, result, trueDistance, falseDistance});
}

extern "C" {
traceItem getTrace(int idx) {
    return CAVM_TRACE[idx];
}

int getTraceSize() {
    return CAVM_TRACE.size();
}
}
