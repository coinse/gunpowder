#include <string.h>
double charDistance(char a, char b) {
    return 1.0 - 1.0 / (1.0 + abs(a - b));
}
double strcmp2(const char *p1, const char *p2)
{
    char *s1 = (char *) p1;
    char *s2 = (char *) p2;
    int lengthDiff = 0;
    double distance = 0;
    for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
        distance += charDistance(*s1, *s2);
    }
    lengthDiff = abs((int)(strlen(p1) - strlen(p2)));
    distance += lengthDiff;
    return distance;
}
