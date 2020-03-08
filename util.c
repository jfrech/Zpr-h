#include "util.h"

size_t str_len(char *str) {
    return *str ? 1 + str_len(++str) : 0; }

bool str_startswith(char *str, char *start) {
    return *str && *start ? *str == *start && str_startswith(++str, ++start)
                          : !*start; }

bool str_equal(char *str, char *ztr) {
    return *str == *ztr && (!*str || str_equal(++str, ++ztr)); }
