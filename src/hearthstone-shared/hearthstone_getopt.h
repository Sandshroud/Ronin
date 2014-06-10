/***
 * Demonstrike Core
 */

#pragma once

/* getopt() wrapper */
#define hearthstone_no_argument            0
#define hearthstone_required_argument      1
#define hearthstone_optional_argument      2
struct hearthstone_option
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
};
extern char hearthstone_optarg[514];
int hearthstone_getopt_long_only (int ___argc, char *const *___argv, const char *__shortopts, const struct hearthstone_option *__longopts, int *__longind);
