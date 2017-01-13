#include <math.h>
#include "log.cpp"

double const K = 1.0;

typedef struct _BDist {
    int result;
    double trueDistance;
    double falseDistance;
    _BDist(int r, int t, int f){
        result = r;
        trueDistance = t;
        falseDistance = f;
    }
} BDist;

template <typename T>
double GT(T a, T b){
    return b - a < 0 ? 0.0 : b - a + K;
}

template <typename T>
double GTE(T a, T b){
    return b - a <= 0 ? 0.0 : b - a + K;
}

template <typename T>
double LT(T a, T b){
    return a - b < 0 ? 0.0 : a - b + K;
}

template <typename T>
double LTE(T a, T b){
    return a - b <= 0 ? 0.0 : a - b + K;
}

template <typename T>
double EQ(T a, T b){
    return a == b ? 0.0 : fabs(a - b) + K;
}

template <typename T>
double NEQ(T a, T b){
    return a == b ? K : 0.0;
}

template <typename T>
BDist isGreater(T a, T b){
    return BDist(a > b, GT(a, b), LTE(a, b));
}

template <typename T>
BDist isEqGreater(T a, T b){
    return BDist(a >= b, GTE(a, b), LT(a, b));
}

template <typename T>
BDist isLess(T a, T b){
    return BDist(a < b, LT(a, b), GTE(a, b));
}

template <typename T>
BDist isEqLess(T a, T b){
    return BDist(a <= b, LTE(a, b), GT(a, b));
}

template <typename T>
BDist isEqual(T a, T b){
    return BDist(a == b, EQ(a, b), NEQ(a, b));
}

template <typename T>
BDist isNotEqual(T a, T b){
    return BDist(a != b, NEQ(a, b), EQ(a, b));
}


// Basid predicate wrapper: this allows us to record the final branch distance
// for a composite predicate, at the end.
int inst(int stmtid, BDist distance){
    tracelog(stmtid, distance.result, distance.trueDistance, distance.falseDistance);
    return distance.result;
}

BDist l_and(BDist lhs, BDist rhs){
    if(lhs.result && rhs.result ) return BDist(true, 0.0, fmin(lhs.falseDistance, rhs.falseDistance));
    else return BDist(false, fmin(lhs.trueDistance, rhs.trueDistance), 0.0);
}

BDist l_or(BDist lhs, BDist rhs){
    if(lhs.result || rhs.result) return BDist(true, 0.0, fmin(lhs.falseDistance, rhs.falseDistance));
    else return BDist(false, fmin(lhs.trueDistance, rhs.trueDistance), 0.0);
}

BDist l_not(BDist subExpr){
    return BDist(!subExpr.result, subExpr.falseDistance, subExpr.trueDistance);
}

