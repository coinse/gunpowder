#include <stdio.h>

int tracelog(int stmtid, int result, double trueDistance, double falseDistance) {
    FILE *f = fopen("trace", "a");
    fprintf(f, "%d %d %f %f\n", stmtid, result, trueDistance, falseDistance);
    fclose(f);
}
