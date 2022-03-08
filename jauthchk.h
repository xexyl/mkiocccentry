/*
 * jauthchk - IOCCC JSON .author.json checker and validator
 *
 * "Because sometimes even the IOCCC Judges need some help." :-)
 *
 * Written in 2022 by:
 *
 *	@xexyl
 *	https://xexyl.net		Cody Boone Ferguson
 *	https://ioccc.xexyl.net
 */


#if !defined(INCLUDE_JAUTHCHK_H)
#    define  INCLUDE_JAUTHCHK_H


/*
 * dbg - debug, warning and error reporting facility
 */
#include "dbg.h"


/*
 * util - utility functions and definitions
 */
#include "util.h"


/*
 * json - json file structs
 */
#include "json.h"

/*
 * sanity - sanity checks on the IOCCC toolkit
 */
#include "sanity.h"

/*
 * IOCCC size and rule related limitations
 */
#include "limit_ioccc.h"


/*
 * definitions
 */
#define REQUIRED_ARGS (1)	/* number of required arguments on the command line */



/*
 * usage message
 *
 * Use the usage() function to print the these usage_msgX strings.
 */
static const char * const usage_msg =
"usage: %s [-h] [-v level] [-V] [-q] [-T]  [-s] [-F fnamchk] [-t] file\n"
"\n"
"\t-h\t\tprint help message and exit 0\n"
"\t-v level\tset verbosity level: (def level: %d)\n"
"\t-q\t\tquiet mode, unless verbosity level > 0 (def: not quiet)\n"
"\t-V\t\tprint version string and exit\n"
"\t-T\t\t\tshow IOCCC toolset chain release repository tag\n"
"\t-s\t\t\tstrict mode: be more strict on what is allowed (def: not strict)\n"
"\t-F /path/to/fnamchk\tpath to fnamchk tool (def: %s)\n"
"\t-t\t\t\ttest mode: only issue warnings in some cases\n"
"\n"
"\tfile\t\tpath to a .author.json file\n"
"\n"
"exit codes:\n"
"\t0\t\tno errors or warnings detected\n"
"\t>0\t\tsome error(s) and/or warning(s) were detected\n"
"\n"
"jauthchk version: %s\n";


/*
 * globals
 */
int verbosity_level = DBG_DEFAULT;	/* debug level set by -v */
bool msg_output_allowed = true;		/* false ==> disable output from msg() */
bool dbg_output_allowed = true;		/* false ==> disable output from dbg() */
bool warn_output_allowed = true;	/* false ==> disable output from warn() and warnp() */
bool err_output_allowed = true;		/* false ==> disable output from err() and errp() */
bool usage_output_allowed = true;	/* false ==> disable output from vfprintf_usage() */
static char const *program = NULL;			/* our name */
static char *program_basename = NULL;			/* our basename */
static bool quiet = false;				/* true ==> quiet mode */
struct author author;					/* the .author.json struct */
static bool strict = false;				/* true ==> disallow anything before/after the '{' and '}' */
static bool test = false;				/* true ==> some tests are not performed */
static struct json_field *found_author_json_fields;	/* list of fields specific to .author.json found */
extern struct json_field author_json_fields[];
extern size_t SIZEOF_AUTHOR_JSON_FIELDS_TABLE;		/* number of elements in the author_json_fields table */

/*
 * forward declarations
 */
static void usage(int exitcode, char const *name, char const *str) __attribute__((noreturn));
static void jauthchk_sanity_chks(char const *file, char const *fnamchk);
static int check_author_json(char const *file, char const *fnamchk);
static struct json_field *add_found_author_json_field(char const *name, char const *val);
static int get_author_json_field(char const *file, char *name, char *val);
static int check_found_author_json_fields(char const *file, bool test);
static void free_found_author_json_fields(void);


#endif /* INCLUDE_JAUTHCHK_H */
