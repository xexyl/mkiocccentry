/* jnamval_util - utility functions for JSON printer jnamval
 *
 * "Because specs w/o version numbers are forced to commit to their original design flaws." :-)
 *
 * This tool is being developed by:
 *
 *	@xexyl
 *	https://xexyl.net		Cody Boone Ferguson
 *	https://ioccc.xexyl.net
 *
 * The JSON parser was co-developed in 2022 by Cody and Landon.
 *
 * "Because sometimes even the IOCCC Judges need some help." :-)
 *
 * "Share and Enjoy!"
 *     --  Sirius Cybernetics Corporation Complaints Division, JSON spec department. :-)
 */


#if !defined(INCLUDE_JNAMVAL_UTIL_H)
#    define  INCLUDE_JNAMVAL_UTIL_H

#define _GNU_SOURCE /* feature test macro for strcasestr() */
#include <stdlib.h>
#include <unistd.h>
#include <regex.h> /* for -g and -G, regular expression matching */
#include <string.h>

/*
 * dbg - info, debug, warning, error, and usage message facility
 */
#include "../dbg/dbg.h"

/*
 * dyn_array - dynamic array facility
 */
#include "../dyn_array/dyn_array.h"

/*
 * util - entry common utility functions for the IOCCC toolkit
 */
#include "util.h"

/*
 * json_parse - JSON parser support code
 */
#include "json_parse.h"

/*
 * json_util - general JSON parser utility support functions
 */
#include "json_util.h"

/*
 * jparse - JSON parser
 */
#include "jparse.h"

/* defines */

/* -t types */
#define JNAMVAL_TYPE_NONE	    (0)
#define JNAMVAL_TYPE_INT	    (1)
#define JNAMVAL_TYPE_FLOAT	    (2)
#define JNAMVAL_TYPE_EXP	    (4)
#define JNAMVAL_TYPE_NUM	    (8)
#define JNAMVAL_TYPE_BOOL	    (16)
#define JNAMVAL_TYPE_STR	    (32)
#define JNAMVAL_TYPE_NULL	    (64)
#define JNAMVAL_TYPE_MEMBER	    (128)
#define JNAMVAL_TYPE_OBJECT	    (256)
#define JNAMVAL_TYPE_ARRAY	    (512)
#define JNAMVAL_TYPE_ANY	    (1023) /* bitwise OR of the above values */
/* JNAMVAL_TYPE_SIMPLE is bitwise OR of num, bool, str and null */
#define JNAMVAL_TYPE_SIMPLE  (JNAMVAL_TYPE_NUM|JNAMVAL_TYPE_BOOL|JNAMVAL_TYPE_STR|JNAMVAL_TYPE_NULL)
/* JNAMVAL_TYPE_COMPOUND is bitwise OR of object and array */
#define JNAMVAL_TYPE_COMPOUND (JNAMVAL_TYPE_MEMBER|JNAMVAL_TYPE_OBJECT|JNAMVAL_TYPE_ARRAY)

/* print types for -p option */
#define JNAMVAL_PRINT_NAME   (1)
#define JNAMVAL_PRINT_VALUE  (2)
#define JNAMVAL_PRINT_JSON   (4)
#define JNAMVAL_PRINT_BOTH   (JNAMVAL_PRINT_NAME | JNAMVAL_PRINT_VALUE)


#define JNAMVAL_CMP_OP_NONE (0)
#define JNAMVAL_CMP_EQ	    (1)
#define JNAMVAL_CMP_LT	    (2)
#define JNAMVAL_CMP_LE	    (3)
#define JNAMVAL_CMP_GT	    (4)
#define JNAMVAL_CMP_GE	    (5)

/* structs */

/* structs for various options */

/* for comparison of numbers / strings - options -n and -S */
struct jnamval_cmp_op
{
    struct json_number *number;	    /* for -n as signed number */
    struct json_string *string;	    /* for -S str */

    bool is_string;	    /* true if -S */
    bool is_number;	    /* true if -n */
    uintmax_t op;	    /* the operation - see JNAMVAL_CMP macros above */

    struct jnamval_cmp_op *next;    /* next in the list */
};

/* number ranges for the options -l, -n and -n */
struct jnamval_number_range
{
    intmax_t min;   /* min in range */
    intmax_t max;   /* max in range */

    bool less_than_equal;	/* true if number type must be <= min */
    bool greater_than_equal;	/* true if number type must be >= max */
    bool inclusive;		/* true if number type must be >= min && <= max */
};
struct jnamval_number
{
    /* exact number if >= 0 */
    intmax_t number;		/* exact number exact number (must be >= 0) */
    bool exact;			/* true if an exact match (number) must be found */

    /* for number ranges */
    struct jnamval_number_range range;	/* for ranges */
};

/*
 * jnamval - struct that holds most of the options, other settings and other data
 */
struct jnamval
{
    /* JSON file related */
    bool is_stdin;				/* reading from stdin */
    FILE *json_file;				/* FILE * to json file */
    char *file_contents;			/* file contents */

    /* out file related to -o */
    char *outfile_path;				/* -o file path */
    FILE *outfile;				/* FILE * of -o ofile */
    bool outfile_not_stdout;			/* -o used without stdout */

    /* string related options */
    bool encode_strings;			/* -e used */
    bool quote_strings;				/* -Q used */

    /* level constraints */
    bool levels_constrained;			/* -l specified */
    struct jnamval_number jnamval_levels;		/* -l level specified */

    /* printing related options */
    bool print_json_types_option;		/* -p explicitly used */
    uintmax_t print_json_types;			/* -p type specified */
    bool print_decoded;				/* -D used */
    bool print_json_levels;			/* -L specified */
    uintmax_t num_level_spaces;			/* number of spaces or tab for -L */
    bool print_level_tab;			/* -L tab option */
    bool invert_matches;			/* -i used */
    bool count_only;				/* -c used, only show count */
    bool count_and_show_values;			/* -C used, show count and values */

    /* search / matching related */
    bool json_types_specified;			/* -t used */
    uintmax_t json_types;			/* -t type */
    bool ignore_case;				/* true if -f, case-insensitive */
    bool match_decoded;				/* -d used - match decoded */
    bool arg_specified;				/* true if an arg was specified */
    bool match_substrings;			/* -s used, match substrings */
    bool use_regexps;				/* -g used, allow grep-like regexps */
    bool match_json_member_names;		/* -N used, match based on member names */
    bool match_hierarchies;			/* -H used, name hierarchies */
    uintmax_t total_matches;			/* for -c */

    bool string_cmp_used;			/* for -S */
    struct jnamval_cmp_op *string_cmp;		/* for -S str */
    bool num_cmp_used;				/* for -n */
    struct jnamval_cmp_op *num_cmp;			/* for -n num */
    uintmax_t max_depth;			/* max depth to traverse set by -m depth */
    struct json *json_tree;			/* json tree if valid merely as a convenience */
};


/* function prototypes */
struct jnamval *alloc_jnamval(void);

/* JSON types - -t option*/
uintmax_t jnamval_parse_types_option(char *optarg);
bool jnamval_match_none(uintmax_t types);
bool jnamval_match_any(uintmax_t types);
bool jnamval_match_int(uintmax_t types);
bool jnamval_match_float(uintmax_t types);
bool jnamval_match_exp(uintmax_t types);
bool jnamval_match_bool(uintmax_t types);
bool jnamval_match_num(uintmax_t types);
bool jnamval_match_string(uintmax_t types);
bool jnamval_match_null(uintmax_t types);
bool jnamval_match_simple(uintmax_t types);
bool jnamval_match_object(uintmax_t types);
bool jnamval_match_array(uintmax_t types);
bool jnamval_match_compound(uintmax_t types);
bool jnamval_match_member(uintmax_t types);

/* what to print - -p option */
uintmax_t jnamval_parse_print_option(char *optarg);
bool jnamval_print_name(uintmax_t types);
bool jnamval_print_value(uintmax_t types);
bool jnamval_print_both(uintmax_t types);
bool jnamval_print_json(uintmax_t types);


/* for number range option -l */
bool jnamval_parse_number_range(const char *option, char *optarg, bool allow_negative, struct jnamval_number *number);
bool jnamval_number_in_range(intmax_t number, intmax_t total_matches, struct jnamval_number *range);

/* for -S and -n */
struct jnamval_cmp_op *jnamval_parse_cmp_op(struct jnamval *jnamval, const char *option, char *optarg);

/* for -L option */
void jnamval_parse_st_level_option(char *optarg, uintmax_t *num_level_spaces, bool *print_level_tab);

/* functions to print matches */
bool jnamval_print_count(struct jnamval *jnamval);

/*
 * XXX - currently does nothing functionally - might or might not be needed to
 * process argv - XXX
 */
void parse_jnamval_args(struct jnamval *jnamval, char **argv);

/* free compare lists */
void free_jnamval_cmp_op_lists(struct jnamval *jnamval);
/* to free the entire struct jnamval */
void free_jnamval(struct jnamval **jnamval);


#endif /* !defined INCLUDE_JNAMVAL_UTIL_H */
