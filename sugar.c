#include "sugar.h"

void de_peano(char *buf) {
    while (*buf) {
        if (*buf == '(') {
            char *_buf = buf + 1;
            size_t n = 0;
            while (_buf[0] == 'S' && _buf[1] == ' ' && _buf[2] == '(')
                n++, _buf += 3;
            size_t m = 0;
            while (m < n+1 && *_buf == ')')
                m++, _buf++;
            if (m == n+1) {
                buf += sprintf(buf, "%zu", n);
                char *b;
                for (b = buf; *_buf; )
                    *b++ = *_buf++;
                *b = '\0'; }
            else buf++; }
        else buf++; } }
