#include "rule.h"

static bool _parse_rules(char *initial_rule_text, FILE *src,
    char *root_realpath, rule_t *initial_rules, stats_t *pstats,
    rule_t **prules, char **prule_text);
static bool _parse_rules_from_filename(char *initial_rule_text, FILE *src,
    char *src_filename, rule_t *initial_rules, stats_t *pstats,
    rule_t **prules, char **prule_text);

bool parse_rules(
        FILE *src, char *src_filename, stats_t *pstats, rule_t *rules,
        char *rule_text, size_t *n_rule_text_required) {

    /* initialization */
    char *initial_rule_text = rule_text;
    pstats->n_rules = 0;
    *rule_text++ = '\0';
    pstats->rule_text_memory_peak = 1;

    bool parsing_success = _parse_rules_from_filename(initial_rule_text, src
                         , src_filename, rules, pstats, &rules, &rule_text);
    *n_rule_text_required = rule_text - initial_rule_text + 1;

    DEBUG("? initial_rule_text = %p, rules[0].pat = %p\n"
         , (void *) initial_rule_text, (void *) rules[0].pat);
    DEBUG("Hex-dumping initial_rule_text up to but not including rule_text:\n");
    char *text = initial_rule_text;
    size_t block_width = 16, n_hex_dumped_bytes = 0;
    while (text < rule_text) {
        _DEBUG("%p-%p: ", (void *) text, (void *) (MINIMUM(text + block_width
              , rule_text) - 1));
        for (size_t j = 0; j < block_width; j++) {
            if (text + j < rule_text) {
                _DEBUG("%02X ", text[j] & 0xFF);
                n_hex_dumped_bytes++; }
            else {
                _DEBUG("   "); } }
        _DEBUG("| ");
        for (size_t j = 0; j < block_width; j++) {
            if (text + j < rule_text) {
                if (' ' <= text[j] && text[j] <= '~') {
                    _DEBUG("%c", text[j]); }
                else {
                    _DEBUG(ANSI_COLOR_NOT_PRINTABLE "?" ANSI_RESET); } } }
        _DEBUG("\n");
        text += block_width; }
    DEBUG("Hex-dumped %zu byte%s (should == %zu-1 bytes).\n"
         , PLURAL(n_hex_dumped_bytes), *n_rule_text_required);

    return parsing_success; }

static bool _parse_rules_from_filename(
        char *initial_rule_text, FILE *src, char *src_filename,
        rule_t *initial_rules, stats_t *pstats, rule_t **prules,
        char **prule_text) {

    char *root_realpath = realpath(src_filename, NULL);

    /* remove trailing file */
    char *rrp = root_realpath;
    while (*rrp)
        rrp++;
    rrp--;
    while (*rrp && *rrp != '/')
        rrp--;
    *rrp = '\0';

    bool parsing_success = _parse_rules(initial_rule_text, src, root_realpath
                         , initial_rules, pstats, prules, prule_text);
    free(root_realpath);
    return parsing_success; }

static bool new_rule(
        char *initial_rule_text, rule_t **prules, char **prule_text,
        bool *pparsing_body, size_t *pline, char **pbdy, char **ppat,
        stats_t **ppstats) {

    rule_t *rules = *prules; char *rule_text = *prule_text;
    bool parsing_body = *pparsing_body; size_t line = *pline; char *bdy = *pbdy;
    char *pat = *ppat; stats_t *pstats = *ppstats;

    if (rule_text[0] != '\n') {
        PARSE_ERROR(line, "Can only create new rule after newline, not "
                          "'\\x%02X'.\n", rule_text[0]); }

    /* if parsing has not yet begun, do not do anything */
    if (!pat && !bdy && !parsing_body) {
        return true; }

    if (!parsing_body)
        PARSE_ERROR(line, "Break whilst not parsing body.\n");
    rule_text--;
    while (*rule_text == ' ')
        rule_text--;
    bdy = rule_text;
    *++rule_text = '\0';
    while (*bdy)
        bdy--;
    bdy++;

    if (pat == NULL || bdy == NULL)
        PARSE_ERROR(line, "Could not parse.\n");

    PARSE_DEBUG(line, "Checking if rule " PRIrule_t " is already known ...\n"
               , pat, bdy);
    bool rule_known = false;
    for (size_t j = 0; j < pstats->n_rules; j++) {
        if (str_equal(rules[j].pat, pat)) {
            if (!str_equal(rules[j].bdy, bdy)) {
                PARSE_ERROR(line, "Pattern " PRIrule_t " is being redefined as "
                            PRIrule_t ".\n", RULE_T(rules[j]), pat, bdy); }
            rule_known = true;
            break; } }
    PARSE_DEBUG(line, "    ~> rule_known = %s\n", BOOL(rule_known));

    if (!rule_known) {
        if (pstats->n_rules < RULE_MEMORY)
            rules[(pstats->n_rules)++] = (rule_t) { .pat = pat, .bdy = bdy };
        else {
            PARSE_ERROR(line, "Could not add rule (#%zu) " PRIrule_t " because "
                       "of memory shortage.\n", pstats->n_rules, pat, bdy); }

        PARSE_DEBUG(line, "New rule (#%zu) " PRIrule_t ".\n"
                   , pstats->n_rules-1, rules[pstats->n_rules-1].pat
                   , rules[pstats->n_rules-1].bdy);
        pstats->rule_text_memory_peak += str_len(pat) + 1 + str_len(bdy) + 1;

        pat = bdy = NULL;
        parsing_body = false; }
    else {
        rule_text = pat;
        pat = bdy = NULL;
        parsing_body = false;
        rule_text--; }

    *prules = rules; *prule_text = rule_text; *pparsing_body = parsing_body;
    *pline = line; *pbdy = bdy; *ppat = pat; *ppstats = pstats;
    return true; }

static bool _parse_rules(
        char *initial_rule_text, FILE *src, char *root_realpath,
        rule_t *initial_rules, stats_t *pstats, rule_t **prules,
        char **prule_text) {

    char *rule_text = *prule_text;
    rule_t *rules = *prules;
    DEBUG("Parsing \"%s\" (pstats->n_rules = %zu)...\n", root_realpath
         , pstats->n_rules);

    char *pat = NULL, *bdy = NULL;
    bool parsing_body = false;

    size_t line = 0, total_characters = 0;
    line++;

    char character_before_EOF = '\0';
    for (int c; (c = getc(src)) != EOF; rule_text++) {
        *rule_text = c;

        /* one can look back a maximum of two characters */
        if (rule_text[-1] == '|' && rule_text[0] == '>') {
            if (parsing_body)
                PARSE_ERROR(line, "Derivation whilst parsing body.\n");
            rule_text -= 2;
            while (*rule_text == ' ')
                rule_text--;

            pat = rule_text;
            REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, 1);
            *++rule_text = '\0';
            while (*pat)
                pat--;
            pat++;

            parsing_body = true; }

        else if (rule_text[-1] == '<' && rule_text[0] == '|') {
            if (pat || parsing_body || bdy)
                PARSE_ERROR(line, "Improper inclusion.\n");

            PARSE_DEBUG(line, "Detected inclusion directive. (pstats->n_rules "
                       "= %zu)\n", pstats->n_rules);
            rule_text--;

            /* ignore leading spaces */
            c = getc(src);
            while (c == ' ')
                c = getc(src);

            char *fp = rule_text;
            REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text
                                   , str_len(root_realpath));
            /* copy root_realpath and connect */
            for (char *rrp = root_realpath; *rrp; rrp++)
                *rule_text++ = *rrp;
            REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, 1);
            *rule_text++ = '/';

            /* read until newline */
            while (c != '\n' && c != EOF) {
                REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, 1);
                *rule_text++ = c;
                c = getc(src);
            }

            /* remove trailing spaces and terminate string */
            rule_text--;
            while (*rule_text && *rule_text == ' ')
                rule_text--;
            rule_text++;
            REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, 1);
            *rule_text++ = '\0';

            PARSE_DEBUG(line, "Including \"%s\" ...\n", fp);

            FILE *f = fopen(fp, "r");
            if (!f)
                PARSE_ERROR(line, "Could not include file \"%s\".\n", fp);
            if (!_parse_rules_from_filename(
                    initial_rule_text, f, fp, initial_rules, pstats, &rules,
                    &rule_text)
            ) { PARSE_ERROR(line, "Inclusion failed.\n"); }
            fclose(f);

            rule_text--; }

        else if (
                !! (rule_text[-1] == ';' && rule_text[0] == '\n')
                || (rule_text[-1] == '\\' && rule_text[0] == '\n')
            ) { rule_text -= 2; }
        else if (
                !! (rule_text[-1] == ';')
                || (rule_text[-1] == ' ' && rule_text[0] == ' ')
                || (rule_text[-1] == '\0' && rule_text[0] == '\n')
                || (rule_text[-1] == '\0' && rule_text[0] == ' ')
            ) { rule_text--; }

        else if (rule_text[0] == '\n') {
            if (!new_rule(
                    initial_rule_text, &rules, &rule_text, &parsing_body,
                    &line, &bdy, &pat, &pstats)
                ) { return false; } }

        line += c == '\n';
        total_characters++;
        character_before_EOF = c;
        REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, 1); }

    REQUIRE_RULE_TEXT_BYTES(line, initial_rule_text, rule_text, 1);
    *rule_text = '\n';
    if (!new_rule(
            initial_rule_text, &rules, &rule_text, &parsing_body, &line, &bdy,
            &pat, &pstats)
        ) { return false; }

    if (character_before_EOF != '\n') {
        PARSE_DEBUG(line, "Source file did not end in a newline.\n"); }
    if (pat || bdy || parsing_body)
        PARSE_ERROR(line, "Unexpected state.\n");

    DEBUG("Parsed a total of %zu rule%s, %zu line%s and %zu character%s.\n"
         , PLURAL(pstats->n_rules), PLURAL(line), PLURAL(total_characters));

    *prules = rules;
    *prule_text = rule_text;

    PARSE_DEBUG(line, "Parsed \"%s\" (pstats->n_rules = %zu).\n", root_realpath
               , pstats->n_rules);
    return true; }
