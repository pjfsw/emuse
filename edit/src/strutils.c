#include "strutils.h"

int stringLen(const char *str) {
    int n = 0;
    while (str[n] != 0) {
        n++;
    }
    return n;
}

char *stringCopy(char *destination, const char *source) {
    char *d = destination;
    const char *s = source;
    int n = 0;
    while (s[n] != 0) {
        d[n] = s[n];
        n++;
    }
    d[n] = s[n];
    return destination;
}