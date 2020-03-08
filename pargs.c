#include "pargs.h"

pargs_t parse_args(int argc, char **argv) {
    pargs_t pargs = default_pargs_t();

    #define _SET_ATTRIBUTE(attr) { \
        pargs.attr = true; }
    #define FLAG_HELP _SET_ATTRIBUTE(print_help)
    #define FLAG_INFO _SET_ATTRIBUTE(info)
    #define FLAG_DEBUG _SET_ATTRIBUTE(debug)
    #define FLAG_RULE_INFO { \
        FLAG_INFO \
        _SET_ATTRIBUTE(rule_info) }
    #define FLAG_DE_PEANO _SET_ATTRIBUTE(de_peano)
    #define FLAG_WATCH _SET_ATTRIBUTE(watch)
    #define FLAG_STATS { \
        FLAG_WATCH \
        _SET_ATTRIBUTE(stats) }
    #define FLAG_WATCH_COMPLETE { \
        FLAG_WATCH \
        _SET_ATTRIBUTE(watch_complete) }
    #define EXIT { pargs.valid = false; return pargs; }

    bool file_only_parse = false;
    for (size_t j = 1; j < argc; j++) {
        char *arg = argv[j];

        if (file_only_parse) {
            goto PARSE_FILENAME; }

        if (arg[0] == '-') {
            if (arg[1] == '-') {
                if (arg[2] == '\0') {
                    file_only_parse = true;
                    continue; }

                /**/ if (str_equal(arg, "--help")) FLAG_HELP
                else if (str_equal(arg, "--info")) FLAG_INFO
                else if (str_equal(arg, "--debug")) FLAG_DEBUG
                else if (str_equal(arg, "--rule-info")) FLAG_RULE_INFO
                else if (str_equal(arg, "--de-peano")) FLAG_DE_PEANO
                else if (str_equal(arg, "--watch")) FLAG_WATCH
                else if (str_equal(arg, "--stats")) FLAG_STATS
                else if (str_equal(arg, "--watch-complete")) FLAG_WATCH_COMPLETE
                else {
                    ERROR("Unrecognized flag \"%s\".\n", arg);
                    EXIT; } }
            else {
                for (char *a = arg + 1; *a; a++) {
                    /**/ if (*a == 'h') FLAG_HELP
                    else if (*a == 'w') FLAG_WATCH
                    else {
                        ERROR("Unrecognized one-letter flag '-%c' in \"%s\".\n"
                             , *a, arg);
                        EXIT; } } } }
        else {
            PARSE_FILENAME:
            if (pargs.source) {
                ERROR("Attempt at redefining source file thru \"%s\"; "
                      "already defined as \"%s\".\n", arg
                     , pargs.source);
                EXIT; }
            pargs.source = arg; } }

    #undef _SET_ATTRIBUTE
    #undef FLAG_HELP
    #undef FLAG_INFO
    #undef FLAG_DEBUG
    #undef FLAG_DE_PEANO
    #undef FLAG_WATCH
    #undef FLAG_STATS
    #undef FLAG_WATCH_COMPLETE
    #undef EXIT

    return pargs; }
