#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdbool.h>

#define PLURAL(n) (n), ((n) == 1 ? "" : "s")
#define BOOL(b) ((b) ? "true" : "false")
#define PRI_BYTE_PERCENTAGE "%zu of %zu byte%s (%.4f%%)"
#define BYTE_PERCENTAGE(used, maximum) (used), PLURAL(maximum), \
                                       (100 * (double) (used) / (maximum))

#define MAXIMUM(X, Y) ((X) >= (Y) ? (X) : (Y))
#define MINIMUM(X, Y) ((X) <= (Y) ? (X) : (Y))

size_t str_len(char *str);

bool str_startswith(char *str, char *start);
bool str_equal(char *str, char *ztr);

#endif
