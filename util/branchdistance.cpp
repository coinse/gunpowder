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

double GT(int a, int b){
    return b - a < 0 ? 0.0 : b - a + K;
}
double GTE(int a, int b){
    return b - a <= 0 ? 0.0 : b - a + K;
}

double LT(int a, int b){
    return a - b < 0 ? 0.0 : a - b + K;
}

double LTE(int a, int b){
    return a - b <= 0 ? 0.0 : a - b + K;
}

double EQ(int a, int b){
    return a == b ? 0.0 : fabs(a - b) + K;
}

double NEQ(int a, int b){
    return a == b ? K : 0.0;
}

BDist isGreater(int a, int b){
    return BDist(a > b, GT(a, b), LTE(a, b));
}

BDist isEqGreater(int a, int b){
    int result = (a >= b);
    return BDist(a >= b, GTE(a, b), LT(a, b));
}

BDist isLess(int a, int b){
    return BDist(a < b, LT(a, b), GTE(a, b));
}

BDist isEqLess(int a, int b){
    return BDist(a <= b, LTE(a, b), GT(a, b));
}

BDist isEqual(int a, int b){
    return BDist(a == b, EQ(a, b), NEQ(a, b));
}

BDist isNotEqual(int a, int b){
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


