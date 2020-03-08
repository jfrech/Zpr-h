#ifndef RULE_H
#define RULE_H

#include "Zprh.h"

bool parse_rules(FILE *src, char *src_filename, stats_t *pstats, rule_t *rules,
    char *rule_text, size_t *n_rule_text_required);

#endif
