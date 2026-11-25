#ifndef _KSTRING_H
#define _KSTRING_H

#include "types.h"

/* Length of a zero-terminated string, not counting the terminator */
size_t strlen(const char *s);

/*
 * Copy at most size-1 characters from src to dst, and always
 * zero-terminate dst (if size > 0). Returns strlen(src), so
 * truncation happened if the result is >= size.
 */
size_t strlcpy(char *dst, const char *src, size_t size);

#endif
