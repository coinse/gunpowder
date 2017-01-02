#include <math.h>
#include "log.c"

double const K = 1.0;

int isGreater(int a, int b, int stmtid){

    int result = (a > b);
    double branchdistance = b - a < 0 ? 0.0 : b - a + K;
    tracelog(stmtid, result, branchdistance);
    return result;
}
int isEqGreater(int a, int b, int stmtid){

    int result = (a >= b);
    double branchdistance = b - a <= 0 ? 0.0 : b - a + K;
    tracelog(stmtid, result, branchdistance);
    return result;
}
int isLess(int a, int b, int stmtid){

    int result = (a < b);
    double branchdistance = a - b < 0 ? 0.0 : a - b + K;
    tracelog(stmtid, result, branchdistance);
    return result;
}
int isEqLess(int a, int b, int stmtid){

    int result = (a <= b);
    double branchdistance = a - b <= 0 ? 0.0 : a - b + K;
    tracelog(stmtid, result, branchdistance);
    return result;
}
int isEqual(int a, int b, int stmtid){

    int result = (a == b);
    double branchdistance = a == b ? 0.0 : abs(a - b) + K;
    tracelog(stmtid, result, branchdistance);
    return result;
}
int isnotEqual(int a, int b, int stmtid){

    int result = (a != b);
    double branchdistance = a == b ? K : 0.0;
    tracelog(stmtid, result, branchdistance);
    return result;
}
