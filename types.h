#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char *pat;
    char *bdy;
} rule_t;

#define PRIrule_t "<\"%s\" |> \"%s\">"
#define RULE_T(rule) (rule).pat, (rule).bdy

typedef struct {
    char *source;

    bool print_help, info, debug, rule_info;
    bool de_peano, watch, stats, watch_complete;

    bool valid;
} pargs_t;
pargs_t default_pargs_t();

typedef struct {
    size_t n_iterations;
    size_t program_text_memory_peak;

    size_t n_rules;
    size_t rule_text_memory_peak;
    size_t *rule_usage;

    double seconds_elapsed;
} stats_t;
stats_t default_stats_t();

#endif
