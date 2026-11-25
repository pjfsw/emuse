#include "types.h"
#include "kstring.h"

size_t strlen(const char *s) {
    const char *p = s;
    while (*p != '\0') {
        p++;
    }
    return p - s;
}

size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t len = strlen(src);
    if (size > 0) {
        size_t n = (len < size) ? len : size - 1;
        for (size_t i = 0; i < n; i++) {
            dst[i] = src[i];
        }
        dst[n] = '\0';
    }
    return len;
}
