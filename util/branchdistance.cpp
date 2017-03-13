// Copyright 2017 COINSE Lab.
#include <math.h>
#include "log.cpp"

double const K = 1.0;

typedef struct _BDist {
  int result;
  double trueDistance;
  double falseDistance;
  _BDist(int r, double t, double f) {
    result = r;
    trueDistance = t;
    falseDistance = f;
  }
} BDist;

template <typename T1, typename T2>
double GT(T1 a, T2 b) {
  return b - a < 0 ? 0.0 : b - a + K;
}

template <typename T1, typename T2>
double GTE(T1 a, T2 b) {
  return b - a <= 0 ? 0.0 : b - a + K;
}

template <typename T1, typename T2>
double LT(T1 a, T2 b) {
  return a - b < 0 ? 0.0 : a - b + K;
}

template <typename T1, typename T2>
double LTE(T1 a, T2 b) {
  return a - b <= 0 ? 0.0 : a - b + K;
}

template <typename T1, typename T2>
double EQ(T1 a, T2 b) {
  return a == b ? 0.0 : fabs(a - b) + K;
}
template <typename T1, typename T2>
double EQ(T1 *a, T2 b) {
  return a ? 0.0 : 100.0;
}

template <typename T1, typename T2>
double NEQ(T1 a, T2 b) {
  return a == b ? K : 0.0;
}
template <typename T1, typename T2>
double NEQ(T1 *a, T2 b) {
  return a ? 100.0 : 0.0;
}

template <typename T1, typename T2>
BDist isGreater(T1 a, T2 b) {
  return BDist(a > b, GT(a, b), LTE(a, b));
}

template <typename T1, typename T2>
BDist isEqGreater(T1 a, T2 b) {
  return BDist(a >= b, GTE(a, b), LT(a, b));
}

template <typename T1, typename T2>
BDist isLess(T1 a, T2 b) {
  return BDist(a < b, LT(a, b), GTE(a, b));
}

template <typename T1, typename T2>
BDist isEqLess(T1 a, T2 b) {
  return BDist(a <= b, LTE(a, b), GT(a, b));
}

template <typename T1, typename T2>
BDist isEqual(T1 a, T2 b) {
  return BDist(a == b, EQ(a, b), NEQ(a, b));
}
template <typename T1, typename T2>
BDist isEqual(T1 *a, T2 b) {
  return BDist(a == NULL, EQ(a, b), NEQ(a, b));
}

template <typename T1, typename T2>
BDist isNotEqual(T1 a, T2 b) {
  return BDist(a != b, NEQ(a, b), EQ(a, b));
}

// Basid predicate wrapper: this allows us to record the final branch distance
// for a composite predicate, at the end.
int inst(int stmtid, BDist distance) {
  tracelog(stmtid, distance.result, distance.trueDistance,
           distance.falseDistance);
  return distance.result;
}

BDist l_and(BDist lhs, BDist rhs) {
  if (lhs.result && rhs.result)
    return BDist(true, 0.0, fmin(lhs.falseDistance, rhs.falseDistance));
  else
    return BDist(false, lhs.trueDistance + rhs.trueDistance, 0.0);
}

BDist l_or(BDist lhs, BDist rhs) {
  if (lhs.result || rhs.result)
    return BDist(true, 0.0, lhs.falseDistance + rhs.falseDistance);
  else
    return BDist(false, fmin(lhs.trueDistance, rhs.trueDistance), 0.0);
}

BDist l_not(BDist subExpr) {
  return BDist(!subExpr.result, subExpr.falseDistance, subExpr.trueDistance);
}
