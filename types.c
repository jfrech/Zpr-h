#include "types.h"

pargs_t default_pargs_t() {
    return (pargs_t) { .source = NULL

                     , .print_help = false
                     , .info = false
                     , .debug = false
                     , .rule_info = false

                     , .de_peano = false
                     , .watch = false
                     , .stats = false
                     , .watch_complete = false

                     , .valid = true }; }

stats_t default_stats_t() {
    return (stats_t) { .n_iterations = 0
                     , .program_text_memory_peak = 0

                     , .n_rules = 0
                     , .rule_text_memory_peak = 0
                     , .rule_usage = NULL

                     , .seconds_elapsed = 0.0 }; }
