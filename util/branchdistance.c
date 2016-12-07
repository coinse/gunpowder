int isGreater(int a, int b, int stmtid){

    int result = (a > b);
    log(stmtid, result, branchdistance);
    return result;
}
int isEqGreater(int a, int b, int stmtid){

    int result = (a >= b);
    log(stmtid, result, branchdistance);
    return result;
}
int isLess(int a, int b, int stmtid){

    int result = (a < b);
    log(stmtid, result, branchdistance);
    return result;
}
int isEqLess(int a, int b, int stmtid){

    int result = (a <= b);
    log(stmtid, result, branchdistance);
    return result;
}
int isEqual(int a, int b, int stmtid){

    int result = (a == b);
    log(stmtid, result, branchdistance);
    return result;
}
int isnotEqual(int a, int b, int stmtid){

    int result = (a != b);
    log(stmtid, result, branchdistance);
    return result;
}
