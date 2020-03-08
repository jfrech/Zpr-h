#ifndef MATCH_H
#define MATCH_H

#include "Zprh.h"

bool is_sep(char c);

bool attempt_match(char *hook_buf, char *pat, char *bdy, char *exec_buffer, char *exec_temp);
bool match(stats_t *pstats, rule_t *rules, size_t n_rules, char *exec_buffer, char *exec_temp);

void info_on_rule_usage(stats_t stats, rule_t *rules);

#endif
