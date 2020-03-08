#include "match.h"

bool match(
        stats_t *pstats, rule_t *rules, size_t n_rules, char *program_text,
        char *tmp_program_text) {

    /* The order of the following two lines is important. */
    for (char prev_hook = '\n', *hook_program_text = program_text;
            *hook_program_text; prev_hook = *hook_program_text++)
        for (size_t j = 0; j < n_rules; j++)
            if (is_sep(prev_hook))
                if (attempt_match(hook_program_text, rules[j].pat
                                 , rules[j].bdy, program_text
                                 , tmp_program_text)
                    ) {
                        pstats->rule_usage[j]++;
                        return true; }
    return false; }

bool is_sep(char c) {
    return c == '\0' || c == '\n' || c == '(' || c == ' ' || c == ')'; }

void info_on_rule_usage(stats_t stats, rule_t *rules) {
    size_t j_rule_min = ~0, j_rule_max = ~0;
    size_t sum = 0;
    for (size_t j = 0; j < stats.n_rules; j++) {
        size_t u = stats.rule_usage[j];
        if (j_rule_min == ~0 || u < stats.rule_usage[j_rule_min])
            j_rule_min = j;
        if (j_rule_max == ~0 || u > stats.rule_usage[j_rule_max])
            j_rule_max = j;
        DEBUG("%zu use%s of rule " PRIrule_t ".\n", PLURAL(u)
             , RULE_T(rules[j]));
        sum += u; }

    size_t n_rules_min_usage = 0, n_rules_max_usage = 0;
    size_t number_of_never_used_rules_shown = 0;
    for (size_t j = 0; j < stats.n_rules; j++) {
        if (stats.rule_usage[j] == stats.rule_usage[j_rule_min])
            n_rules_min_usage++;
        if (stats.rule_usage[j] == stats.rule_usage[j_rule_max])
            n_rules_max_usage++;

        if (
                !! (stats.rule_usage[j] == 0)
                && (number_of_never_used_rules_shown
                   < MAXIMUM_NUMBER_OF_NEVER_USED_RULES_SHOWN)
            ) {
                INFO("Never used rule " PRIrule_t ".\n", RULE_T(rules[j]));
                number_of_never_used_rules_shown++; } }
    if (stats.rule_usage[j_rule_min] == 0) {
        if (number_of_never_used_rules_shown < n_rules_min_usage) {
            INFO("... never used %zu further rule%s.\n", PLURAL(
                    n_rules_min_usage - number_of_never_used_rules_shown)); } }

    if (stats.n_rules == 0) {
        INFO("No rules were defined.\n"); }
    else if (stats.rule_usage[j_rule_min] == 0) {
        double coverage = 1.0 - (double) n_rules_min_usage / stats.n_rules;
        INFO("Never used %zu rule%s out of a total of %zu rule%s "
             "(%.4f%% coverage).\n", PLURAL(n_rules_min_usage)
            , PLURAL(stats.n_rules), coverage * 100.0); }
    else {
        INFO("Every rule was used at least once (stats.rule_usage[j_rule_min] "
             "= %zu, n_rules_min_usage = %zu).\n"
            , stats.rule_usage[j_rule_min], n_rules_min_usage); } }

bool attempt_match(
        char *hook_program_text, char *pat, char *bdy, char *program_text,
        char *tmp_program_text) {

    char *txt = hook_program_text;
    char *points_start = tmp_program_text;
    char *points = points_start;
    for (; *pat; ) {
        if (*pat == *txt) {
            pat++; txt++; }
        else if (*pat == '.' && pat[1]) {
            pat++;
            while (!is_sep(*pat)) {
                *points++ = *pat++; }
            *points++ = '\0';

            int parens = 0;
            for (; *txt; txt++) {
                *points++ = *txt;
                if (*txt == '(') parens++;
                if (*txt == ')') parens--;
                if (parens < 0)
                    break;
                if (
                        !! (!is_sep(*txt) && is_sep(txt[1]) && parens == 0)
                        || (is_sep(*txt) && parens == 0)
                    ) { ++txt; break; } }
            *points++ = '\0'; }
        else return false; }
    if (!is_sep(*txt))
        return false;
    *points++ = '\0';

    if (!*pat) {
        char *tmp_program_text_after_points = points;
        /* copy bytes in program_text after matched pattern into
           tmp_program_text to later retrieve */
        {
            char *t, *b;
            for (t = tmp_program_text_after_points, b = txt; *b; ) {
                *t++ = *b++; }
            *t = '\0';
        }

        /* integrate the rule's body */
        txt = hook_program_text;
        #define REPL { \
            bool is_point = true, do_replace = false; \
            for (char *point = points_start; *point; \
                point++, is_point = !is_point) { \
                if (!do_replace && is_point) { \
                    if (txt >= program_text + str_len(point)) { \
                        char *pre_txt = txt - str_len(point); \
                        if ( \
                                !! is_sep(pre_txt[-1]) \
                                && str_startswith(pre_txt, point) \
                            ) { \
                                txt = pre_txt; do_replace = true; } } } \
                for (; *point; point++) { \
                    if (!is_point && do_replace) { \
                        *txt++ = *point; } } \
                if (!is_point && do_replace) { \
                    break; } } }
        for (char *y = bdy; *y; ) {
            *txt = *y;
            if (is_sep(*txt)) {
                *txt = '\0'; REPL; *txt = *y; }
            txt++; y++; }
        *txt = '\0'; REPL; *txt = '\0';
        #undef REPL

        /* retrieve previously stored program_text tail */
        for (char *t = tmp_program_text_after_points; *t; ) {
            *txt++ = *t++; }
        *txt++ = '\0';
        return true; }
    return false; }
