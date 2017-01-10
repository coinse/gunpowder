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
int CAVM_TRACE_IDX = 0;

int tracelog(int stmtid, int result, double trueDistance, double falseDistance) {
    CAVM_TRACE.push_back(traceItem{stmtid, result, trueDistance, falseDistance});
}

extern "C" {
traceItem getTrace() {
    if (CAVM_TRACE_IDX<CAVM_TRACE.size()) {
        return CAVM_TRACE[CAVM_TRACE_IDX++];
    } else {
        return traceItem{-1,-1,-1,-1};
    }
}

int getTraceSize() {
    return CAVM_TRACE.size();
}
}
