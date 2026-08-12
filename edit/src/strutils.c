#include "arch.h"
#include "strutils.h"

int stringLen(const char *str) {
    if (str == NULL) {
        return 0;
    }
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

char *stringNCopy(char *destination, const char *source, int count) {
    int n = 0;

    while (n < count && source[n] != 0) {
        destination[n] = source[n];
        n++;
    }

    while (n < count) {
        destination[n] = 0;
        n++;
    }

    return destination;
}
