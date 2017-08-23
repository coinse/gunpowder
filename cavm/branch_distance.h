/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 * Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
 * Copyright (C) 2017 by Shin Yoo <shin.yoo@kaist.ac.kr>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#ifndef __CAVM__UTILITY_H__
#define __CAVM__UTILITY_H__

#include <math.h>
#include <stdlib.h>

typedef struct _BDist {
  int result;
  double true_distance;
  double false_distance;
} BDist;

typedef struct _trace {
  int stmtid;
  int result;
  double true_distance;
  double false_distance;
  struct _trace *next;
} trace;
void log_trace(int stmtid, int result, double true_distance, double false_distance);
trace get_trace(); 

#include <stdio.h>
double const K = 1.0;
trace *TAIL = NULL;
trace *HEAD = NULL;

#define GT(a, b) b - a < 0 ? 0.0 : b - a + K
#define GTE(a, b) b - a <= 0 ? 0.0 : b - a + K
#define LT(a, b) a - b < 0 ? 0.0 : a - b + K
#define LTE(a, b) a - b <= 0 ? 0.0 : a - b + K
#define EQ(a, b) a == b ? 0.0 : fabs(a - b) + K
#define NEQ(a, b) a == b ? K : 0.0

// https://gcc.gnu.org/onlinedocs/gcc/Typeof.html
#define isGreater(a, b) ({ \
  __auto_type _a = a; \
  __auto_type _b = b; \
  (BDist){_a > _b, GT(_a, _b), LTE(_a, _b)}; \
})
#define isEqGreater(a, b) ({ \
  __auto_type _a = a; \
  __auto_type _b = b; \
  (BDist){_a >= _b, GTE(_a, _b), LT(_a, _b)}; \
})
#define isLess(a, b) ({ \
  __auto_type _a = a; \
  __auto_type _b = b; \
  (BDist){_a < _b, LT(_a, _b), GTE(_a, _b)}; \
})
#define isEqLess(a, b) ({ \
  __auto_type _a = a; \
  __auto_type _b = b; \
  (BDist){_a <= _b, LTE(_a, _b), GT(_a, _b)}; \
})
#define isEqual(a, b) ({ \
  __auto_type _a = a; \
  __auto_type _b = b; \
  (BDist){_a == _b, EQ(_a, _b), NEQ(_a, _b)}; \
})
#define isNotEqual(a, b) ({ \
  __auto_type _a = a; \
  __auto_type _b = b; \
  (BDist){_a != _b, NEQ(_a, _b), EQ(_a, _b)}; \
})

void log_trace(int stmtid, int result, double true_distance, double false_distance) {
  if (TAIL) {
    TAIL->next = (trace *)malloc(sizeof(trace));
    TAIL = TAIL->next;
  } else {
    HEAD = (trace *)malloc(sizeof(trace));
    TAIL = HEAD;
  }
  TAIL->stmtid = stmtid;
  TAIL->result = result;
  TAIL->true_distance = true_distance;
  TAIL->false_distance = false_distance;
  TAIL->next = NULL;
}

trace get_trace() {
  if (HEAD == NULL) {
    TAIL = NULL;
    return (trace){-1};
  }
  trace t = {
    HEAD->stmtid,
    HEAD->result,
    HEAD->true_distance,
    HEAD->false_distance
  };
  trace *tmp = HEAD;
  HEAD = HEAD->next;
  free(tmp);
  return t;
}

int inst(int stmtid, BDist distance) {
  log_trace(stmtid, distance.result, distance.true_distance,
           distance.false_distance);
  return distance.result;
}

BDist l_and(BDist lhs, BDist rhs) {
  if (lhs.result && rhs.result) {
    BDist d = {1, 0.0, fmin(lhs.false_distance, rhs.false_distance)};
    return d;
  } else {
    BDist d ={0, lhs.true_distance + rhs.true_distance, 0.0};
    return d;
  }
}

BDist l_or(BDist lhs, BDist rhs) {
  if (lhs.result || rhs.result)
    return (BDist){1, 0.0, lhs.false_distance + rhs.false_distance};
  else
    return (BDist){0, fmin(lhs.true_distance, rhs.true_distance), 0.0};
}

BDist l_not(BDist subExpr) {
  return (BDist){!subExpr.result, subExpr.false_distance, subExpr.true_distance};
}

#endif // __CAVM__UTILITY_H__
