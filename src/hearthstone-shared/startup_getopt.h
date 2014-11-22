/***
 * Demonstrike Core
 */

#pragma once

/* getopt() wrapper */
#define startup_no_argument            0
#define startup_required_argument      1
#define startup_optional_argument      2
struct startup_option
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
};
extern char startup_optarg[514];
int startup_getopt_long_only (int ___argc, char *const *___argv, const char *__shortopts, const struct startup_option *__longopts, int *__longind);
