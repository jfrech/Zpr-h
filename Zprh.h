/* ========================================================================
   == If the below constant memory size PROGRAM_TEXT_MEMORY too small,   ==
   == a segmentation fault is a probable result. No checks will be made. ==
   ======================================================================== */

#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "types.h"
#include "util.h"
#include "pargs.h"

#include "rule.h"
#include "match.h"
#include "sugar.h"

#define MAXIMUM_NUMBER_OF_NEVER_USED_RULES_SHOWN (16)
#define WATCH_MAXIMUM_PROGRAM_TEXT (1000)
#define WATCH_MINIMUM_ELAPSED_CLOCKS (CLOCKS_PER_SEC / 2)

/* Number of rules that can be used. */
#ifndef RULE_MEMORY
#define RULE_MEMORY ((size_t) (1ul << 10))
#endif

/* Number of bytes all rules combined can harbor. */
#ifndef RULE_TEXT_MEMORY
#define RULE_TEXT_MEMORY ((size_t) (1ul << 20))
#endif

/* Number of bytes the program text can occupy. Is also used for the temporary
   program text. */
#ifndef PROGRAM_TEXT_MEMORY
#define PROGRAM_TEXT_MEMORY ((size_t) (1ul << 30))
#endif

#define RULE_TEXT_HAS_ROOM(initial_rule_text, rule_text, bytes) \
    ((RULE_TEXT_MEMORY) >= ((rule_text) - (initial_rule_text)) + (bytes))
#define REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, bytes) { \
    if (!RULE_TEXT_HAS_ROOM(initial_rule_text, rule_text, bytes)) { \
        PARSE_ERROR(line, "rule_text memory exhausted.\n"); } }

#ifdef MONOCHROME
#define ANSI_RESET ""
#define ANSI_COLOR_ERROR ""
#define ANSI_COLOR_DEBUG ""
#define ANSI_COLOR_INFO ""
#define ANSI_COLOR_WATCH ""
#define ANSI_COLOR_MORE ""
#define ANSI_COLOR_NOT_PRINTABLE ""
#else
#define ANSI_RESET "\33[0m"
#define ANSI_COLOR_ERROR "\33[91m"
#define ANSI_COLOR_DEBUG "\33[37m"
#define ANSI_COLOR_INFO "\33[32m"
#define ANSI_COLOR_WATCH "\33[93m"
#define ANSI_COLOR_MORE "\33[34m"
#define ANSI_COLOR_NOT_PRINTABLE "\33[35m"
#endif

#define _ERROR(...) { fprintf(stderr, __VA_ARGS__); }
#define _INFO(...) { if (SHOW_INFO) { fprintf(stderr, __VA_ARGS__); } }
#define _DEBUG(...) { if (SHOW_DEBUG) { fprintf(stderr, __VA_ARGS__); } }
#define _WATCH(...) { fprintf(stderr, __VA_ARGS__); }

#define ERROR(...) { _ERROR(ANSI_COLOR_ERROR "[error (%s)]" ANSI_RESET " " \
                           , __func__); _ERROR(__VA_ARGS__); }
#define INFO(...) { _INFO(ANSI_COLOR_INFO "[info (%s)]" ANSI_RESET " " \
                         , __func__); _INFO(__VA_ARGS__); }
#define DEBUG(...) { _DEBUG(ANSI_COLOR_DEBUG "[debug (%s)]" ANSI_RESET " " \
                           , __func__); _DEBUG(__VA_ARGS__); }
#define WATCH(iteration, ...) { _WATCH(ANSI_COLOR_WATCH "[watch %zu]" \
                                      ANSI_RESET " ", iteration); \
                                _WATCH(__VA_ARGS__); }

#define PARSE_ERROR(line, ...) { \
    ERROR("Line %zu: ", line); _ERROR(__VA_ARGS__); return false; }
#define PARSE_DEBUG(line, ...) { \
    DEBUG("Line %zu: ", line); _DEBUG(__VA_ARGS__); }


void print_help();

extern bool SHOW_INFO;
extern bool SHOW_DEBUG;

#endif
