/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 * Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
 * Copyright (C) 2017 by Jongmin Park <qwaqwa93@kaist.ac.kr>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#include <string.h>
int charDistance(char a, char b) { return abs(a - b); }
double strcmp2(const char *p1, const char *p2) {
  char *s1 = (char *)p1;
  char *s2 = (char *)p2;
  int lengthDiff = 0;
  int charDiff = 0;
  double distance = 0;
  for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
    charDiff += charDistance(*s1, *s2);
  }
  lengthDiff = abs((int)(strlen(p1) - strlen(p2)));
  distance = lengthDiff + (1 - pow(1.001, -charDiff));
  int original_result = strcmp(p1, p2);
  int sign = (original_result > 0) - (original_result < 0);
  distance = sign * distance;
  return distance;
}
