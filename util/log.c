#include <stdio.h>

int tracelog(int stmtid, int result, double branchdistance) {
    FILE *f = fopen("trace", "a");
    fprintf(f, "%d %d %f\n", stmtid, result, branchdistance);
    fclose(f);
}
