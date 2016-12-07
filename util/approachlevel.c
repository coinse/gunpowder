#include <stdio.h>
#include <stdlib.h>

struct branchdep {
    int branch;
    int parent;
};

struct branchdep* dependencies;

int getdependencies(void) {
    FILE *fp = fopen("controldep.txt", "r");
    int lines = 0;
    while (!feof(fp)) {
        char ch = fgetc(fp);
        if (ch == '\n')
            lines++;
    }

    dependencies = malloc(lines * sizeof(struct branchdep));

    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < lines; i++)
        fscanf(fp, "%d %d", &dependencies[i].branch, &dependencies[i].parent);

    for (int i = 0; i < lines; i++)
        printf("%d %d\n", dependencies[i].branch, dependencies[i].parent);

    return lines;
}
int approachlevel(int branchid) {
    int line = getdependencies();

    for (int i = 0; i < line; i++){
        printf("i%d i%d\n", dependencies[i].branch, dependencies[i].parent);
    }


    int apr_lv = 0;
    return apr_lv;
}

int main(){
    approachlevel(0);
}
