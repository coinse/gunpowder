#include <stdio.h>

int tracelog(int stmtid, int result, int branchdistance) {
    FILE *f = fopen("trace", "a");
    fprintf(f, "%d %d %d\n", stmtid, result, branchdistance);
    fclose(f);
}
