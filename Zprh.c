/* Jonathan Frech, 3rd, 4th, 5th, 6th, 7th, 8th, 9th of March 2020 */
/* TODO :: match.c and match.h :: refactor (fancify) */

#include "Zprh.h"
bool SHOW_DEBUG = false;
bool SHOW_INFO = false;

void print_help() {
    fprintf(stderr,
        "Usage: Zprh [flags] <source> [flags]\n"
        "\n"
        "  --help, -h      : Print this message.\n"
        "  --info          : Display general info messages.\n"
        "  --debug         : Display more detailed debug messages.\n"
        "  --rule-info     : Display rule coverage (percentage of all defined "
            "rules that were used).\n                    Enables \"--info\".\n"
        "\n"
        "  --de-peano      : Replace Peano representations of natural numbers "
            "\n                    (e.g. \"(S (S (S ... (S ()) ...)))\") in "
            "the final program text.\n"
        "  --watch         : Periodically display program text.\n"
        "  --stats         : Also display current execution's statistics. "
            "Enables \"--watch\".\n"
        "  --watch-complete: Display every iteration's program text. Enables "
            "\"--watch\".\n" ); }

int main(int argc, char **argv) {
    pargs_t pargs = parse_args(argc, argv);

    if (!pargs.valid) {
        ERROR("Could not parse arguments. Try \"%s --help\" to display "
              "program usage.\n", *argv);
        return EXIT_FAILURE; }

    if (pargs.print_help) {
        print_help();
        return EXIT_SUCCESS; }

    if (!pargs.source) {
        ERROR("No source file specified.\n");
        return EXIT_FAILURE; }

    SHOW_DEBUG = pargs.debug;
    SHOW_INFO = pargs.info;

    stats_t stats = default_stats_t();
    #define EXIT(STATUS) { free(rules); free(rule_text); free(program_text); \
                            free(tmp_program_text); free(stats.rule_usage); \
                            return STATUS; }

    rule_t *rules = malloc(RULE_MEMORY * sizeof *rules);
    char *rule_text = malloc(RULE_TEXT_MEMORY * sizeof *rule_text);
    char *program_text = malloc(PROGRAM_TEXT_MEMORY * sizeof *program_text);
    char *tmp_program_text = malloc(PROGRAM_TEXT_MEMORY
                                   * sizeof *tmp_program_text);

    if (!rules || !rule_text || !program_text || !tmp_program_text) {
        ERROR("Memory allocation failed.\n");
        EXIT(EXIT_FAILURE); }

    FILE *src = fopen(pargs.source, "r");
    if (!src) {
        ERROR("Could not open source file \"%s\".\n", pargs.source);
        EXIT(EXIT_FAILURE); }

    size_t n_rules_required = 0, n_rule_text_required = 0;
    if (!parse_rules(
            src, pargs.source, &stats, rules, rule_text,
            &n_rule_text_required)
        ) {
            fclose(src);
            ERROR("Rule parsing failed.\n");
            EXIT(EXIT_FAILURE); }
    else {
        fclose(src); }
    n_rules_required = stats.n_rules;

    INFO("Parsed rules; using " PRI_BYTE_PERCENTAGE ".\n"
        , BYTE_PERCENTAGE(n_rules_required * sizeof *rules
                         , RULE_MEMORY * sizeof *rules));
    INFO("Parsed rule_text; using " PRI_BYTE_PERCENTAGE ".\n"
        , BYTE_PERCENTAGE(n_rule_text_required, RULE_TEXT_MEMORY));

    stats.rule_usage = malloc(stats.n_rules * sizeof stats.rule_usage);
    if (!stats.rule_usage) {
        ERROR("Memory allocation failed.\n");
        EXIT(EXIT_FAILURE); }
    for (size_t j = 0; j < stats.n_rules; j++) {
        stats.rule_usage[j] = 0;
        DEBUG("Acknowledged rule (#%zu) " PRIrule_t ".\n", j
             , RULE_T(rules[j])); }

    /* assures always being able to look one character back */
    program_text[0] = '\0';
    char *p1_program_text = program_text + 1;
    sprintf(p1_program_text, "main");
    stats.program_text_memory_peak = 1 + str_len(p1_program_text) + 1;

    DEBUG("Starting execution ...\n");
    DEBUG("stats.n_rules = %zu\n", stats.n_rules);

    /* starting execution */
    clock_t initial_time = clock();

    stats.n_iterations = 0;
    size_t last_n_iterations = 0;
    clock_t last_clock = initial_time;

    #define UPDATE_STATS(watch_ignore_time) { \
        size_t program_text_memory_used = 1 + str_len(p1_program_text) + 1;\
        stats.program_text_memory_peak = MAXIMUM( \
            stats.program_text_memory_peak, program_text_memory_used); \
        stats.seconds_elapsed = (double) \
            (clock() - initial_time) / CLOCKS_PER_SEC; \
        if (pargs.watch) { \
            if ( \
                    !! watch_ignore_time \
                    || pargs.watch_complete \
                    || clock() - last_clock >= WATCH_MINIMUM_ELAPSED_CLOCKS \
                ) { \
                    if ( \
                            !! (stats.n_iterations > last_n_iterations) \
                            || last_n_iterations == 0 \
                        ) { \
                            WATCH_STATS; } } } }
    #define WATCH_STATS { \
        WATCH(stats.n_iterations, "%.*s", WATCH_MAXIMUM_PROGRAM_TEXT \
             , p1_program_text); \
        last_n_iterations = stats.n_iterations; \
        last_clock = clock(); \
        if (str_len(p1_program_text) > WATCH_MAXIMUM_PROGRAM_TEXT) { \
            _WATCH(ANSI_COLOR_MORE "..." ANSI_RESET); } \
        _WATCH("\n"); \
        if (pargs.stats) { \
            WATCH(stats.n_iterations, "Statistics: "); \
            _WATCH("elapsed time: %.4f sec // ", stats.seconds_elapsed); \
            _WATCH("program_text_memory_peak = " PRI_BYTE_PERCENTAGE "// " \
                 , BYTE_PERCENTAGE(stats.program_text_memory_peak \
                 , PROGRAM_TEXT_MEMORY)); \
            _WATCH("program_text_memory_used = " PRI_BYTE_PERCENTAGE "" \
                 , BYTE_PERCENTAGE(program_text_memory_used \
                 , PROGRAM_TEXT_MEMORY)); \
            _WATCH("\n"); } }
    UPDATE_STATS(true);
    while (match(&stats, rules, stats.n_rules, p1_program_text
                , tmp_program_text)) {
            stats.n_iterations++;
            UPDATE_STATS(false); }
    UPDATE_STATS(true);
    #undef UPDATE_STATS
    #undef WATCH_STATS

    INFO("Completed matching; memory usage peaked at " PRI_BYTE_PERCENTAGE ".\n"
        , BYTE_PERCENTAGE(stats.program_text_memory_peak, PROGRAM_TEXT_MEMORY));

    if (pargs.de_peano)
        de_peano(p1_program_text);

    if (pargs.rule_info)
        info_on_rule_usage(stats, rules);

    stats.seconds_elapsed = (double) (clock() - initial_time) / CLOCKS_PER_SEC;
    INFO("Took %.4f seconds.\n", stats.seconds_elapsed);

    fprintf(stdout, "%s\n", p1_program_text);

    EXIT(EXIT_SUCCESS);
    #undef EXIT
}
