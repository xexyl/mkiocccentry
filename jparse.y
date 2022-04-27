/* vim: set tabstop=8 softtabstop=4 shiftwidth=4 noexpandtab : */

/* json_parser - bison grammar
 *
 * XXX This is VERY incomplete but the .info.json and .author.json files
 * generated by mkiocccentry do not cause any errors. No parse tree is generated
 * yet and so no verification is done yet either.
 *
 * In addition to the parser not doing anything there is missing grammar as
 * well; escaped characters are not handled for example.
 *
 * There are no actions yet. I'm not sure when I will be adding the actions and
 * it's very likely that I won't add all at once.
 *
 * The memory returned by strdup() (json_parser.l action for JSON_STRING) will
 * not yet be freed but since the parser doesn't do anything but just finishes
 * until a parse error or EOF (or end of string) this is not a problem. Actually
 * struct json_string will be used in this case so the way strings are handled will
 * be changed anyway.
 *
 * I'm not sure when I will be adding more and I won't add all at once but
 * rather a bit each time. This is a work in progress!
 */

/* Section 1: Declarations */

/*
 * We enable verbose error messages during development but once the parser is
 * complete we will disable this as it's very verbose.
 *
 * NOTE: Previously we used the -D option to bison because the %define is not
 * POSIX Yacc portable but we no longer do that because we make use of another
 * feature that's not POSIX Yacc portable that we deem worth it as it produces
 * easier to read error messages.
 */
%define parse.error verbose
/*
 * We enable lookahead correction parser for improved errors
 */
%define parse.lac full
/*
 * As we utterly object to the hideous code that bison and flex generate we
 * point it out in an ironic way by changing the prefix yy to ugly_.
 */
%define api.prefix {ugly_}
/*
 * We use our struct json (see json.h for its definition) instead of bison
 * %union.
 *
 * This means that to access the struct json's union type in the lexer we can do
 * (because the prefix is ugly_ as described above):
 *
 *	ugly_lval.type = ...
 *
 * A negative consequence here is that because of the api.prefix being set to
 * ugly_ there's a typedef that might suggest our struct json is ugly (beyond
 * the original ugliness of JSON that is):
 *
 *	typedef struct json UGLY_STYPE;
 *
 * Whether this is a problem will be decided later.
 */
%define api.value.type {struct json}

%{
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h> /* getopt */
#include "jparse.h"

bool output_newline = true;		/* true ==> -n not specified, output new line after each arg processed */
unsigned num_errors = 0;		/* > 0 number of errors encountered */


/* debug information during development */
int yydebug = 1;

int token_type = 0;
%}


/*
 * Terminal symbols (token kind)
 *
 * For most of the terminal symbols we use string literals to identify them as
 * this makes it easier to read error messages. This feature is not POSIX Yacc
 * compatible but we've decided that the benefit outweighs this fact.
 *
 * XXX We will be changing the number types into a single type later on but
 * everything else should be okay though that might prove to be false later on.
 */
%token JSON_OPEN_BRACE "{"
%token JSON_CLOSE_BRACE "}"
%token JSON_OPEN_BRACKET "["
%token JSON_CLOSE_BRACKET "]"
%token JSON_COMMA ","
%token JSON_COLON ":"
%token JSON_NULL "null"
%token JSON_TRUE "true"
%token JSON_FALSE "false"
%token JSON_STRING
%token JSON_NUMBER


/* Section 2: Rules
 *
 * XXX I believe all the rules are here but there are no actions. However the
 * json_number should be simplified to e.g. JSON_INTEGER | JSON_FLOAT or
 * something like that. This will require a regex change in the flex grammar
 * which will happen later.
 *
 * There are no actions here so this is incomplete in that way too.
 */
%%
json:		/* empty */
		| json_element
		| JSON_OPEN_BRACE JSON_CLOSE_BRACE
		| JSON_OPEN_BRACKET JSON_CLOSE_BRACKET
		;

json_value:	  json_object
		| json_array
		| JSON_STRING
		| json_number
		| JSON_TRUE
		| JSON_FALSE
		| JSON_NULL
		;

json_number:	JSON_NUMBER ;

json_object:	JSON_OPEN_BRACE json_members JSON_CLOSE_BRACE
		;

json_members:	json_member
		| json_member JSON_COMMA json_members
		;

json_member:	JSON_STRING JSON_COLON json_element
		;

json_array:	JSON_OPEN_BRACKET json_elements JSON_CLOSE_BRACKET
		;

json_elements:	json_element
		| json_element JSON_COMMA json_elements
		;

json_element:	json_value


%%
/* Section 3: C code */
int
main(int argc, char **argv)
{
    char const *program = NULL;	    /* our name */
    extern char *optarg;	    /* option argument */
    extern int optind;		    /* argv index of the next arg */
    bool strict = false;	    /* true ==> strict mode (currently unused: this is for when a JSON parser is added) */
    bool string_flag_used = false;  /* true ==> -S string was used */
    int ret;			    /* libc return code */
    int i;


    /*
     * parse args
     */
    program = argv[0];
    while ((i = getopt(argc, argv, "hv:qVnSs:")) != -1) {
	switch (i) {
	case 'h':		/* -h - print help to stderr and exit 0 */
	    usage(2, "-h help mode", program); /*ooo*/
	    not_reached();
	    break;
	case 'v':		/* -v verbosity */
	    /*
	     * parse verbosity
	     */
	    verbosity_level = parse_verbosity(program, optarg);
	    break;
	case 'q':
	    msg_warn_silent = true;
	    break;
	case 'V':		/* -V - print version and exit */
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = printf("%s\n", JPARSE_VERSION);
	    if (ret <= 0) {
		warnp(__func__, "printf error printing version string: %s", JPARSE_VERSION);
	    }
	    exit(0); /*ooo*/
	    not_reached();
	    break;
	case 'n':
	    output_newline = false;
	    break;
	case 'S':
	    /*
	     * XXX currently this is unused as json parsing is not done yet.
	     */
	    strict = true;
	    /* the if is only to prevent the warning that it's not yet used */
	    if (strict)
		dbg(DBG_MED, "enabling strict mode");
	    break;
	case 's':
	    /*
	     * So we don't trigger missing arg. Maybe there's another way but
	     * nothing is coming to my mind right now.
	     */
	    string_flag_used = true;

	    dbg(DBG_NONE, "Calling parse_json_string(\"%s\"):", optarg);
	    /* parse arg as a string */
	    parse_json_string(optarg);
	    /*
	     * XXX Rather than having an option to disable strict mode so that
	     * in the same invocation we can test some strings in strict mode
	     * and some not strict after each string is parsed the strict mode
	     * is disabled so that another -s has to be specified prior to the
	     * string. This does mean that if you want strict parsing of files
	     * and you specify the -s option then you must have -S after the
	     * string args.
	     *
	     * But the question is: should it be this way or should it be
	     * another design choice? For example should there be an option that
	     * specifically disables strict mode so that one can not worry about
	     * having to specify -s repeatedly? I think it might be better this
	     * way but I'm not sure what letter should do it. Perhaps -x? If we
	     * didn't use -S for strict it could be S but we do so that won't
	     * work.
	     */

	    /* the if is only to prevent the warning that it's not yet used */
	    if (!strict)
		dbg(DBG_MED, "disabling strict mode");
	    strict = false;
	    break;
	default:
	    usage(2, "invalid -flag", program); /*ooo*/
	    not_reached();
	}
    }

    /* perform IOCCC sanity checks */
    ioccc_sanity_chks();

    /*
     * case: process arguments on command line
     */
    if (argc - optind > 0) {

	/*
	 * process each argument in order
	 */
	for (i=optind; i < argc; ++i) {
	    dbg(DBG_NONE, "Calling parse_json_file(\"%s\"):", argv[i]);
	    parse_json_file(argv[i]);
	}

    } else if (!string_flag_used) {
	usage(2, "no file specified", program); /*ooo*/
	not_reached();
    }


    /*
     * All Done!!! - Jessica Noll, age 2
     */
    exit(num_errors != 0); /*ooo*/
}

void
yyerror(char const *format, ...)
{
    va_list ap;

    va_start(ap, format);

    /*
     * We use fprintf and vfprintf instead of err() but in the future this might
     * use an error function of some kind, perhaps a variant of jerr() (a
     * variant because the parser cannot provide all the information that the
     * jerr() function expects). In the validation code we will likely use
     * jerr(). It's possible that the function jerr() will change as well but
     * this will be decided after the parser is complete.
     */
    fprintf(stderr, "JSON parser error (num errors: %d) on line %d: ", yynerrs, ugly_lineno);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

/*
 * usage - print usage to stderr
 *
 * Example:
 *      usage(3, "missing required argument(s), program: %s", program);
 *
 * given:
 *	exitcode        value to exit with
 *	str		top level usage message
 *	program		our program name
 *
 * NOTE: We warn with extra newlines to help internal fault messages stand out.
 *       Normally one should NOT include newlines in warn messages.
 *
 * This function does not return.
 */
static void
usage(int exitcode, char const *str, char const *prog)
{
    /*
     * firewall
     */
    if (str == NULL) {
	str = "((NULL str))";
	warn(__func__, "\nin usage(): program was NULL, forcing it to be: %s\n", str);
    }
    if (prog == NULL) {
	prog = "((NULL prog))";
	warn(__func__, "\nin usage(): program was NULL, forcing it to be: %s\n", prog);
    }

    /*
     * print the formatted usage stream
     */
    fprintf_usage(DO_NOT_EXIT, stderr, "%s\n", str);
    fprintf_usage(exitcode, stderr, usage_msg, prog, DBG_DEFAULT, JPARSE_VERSION);
    exit(exitcode); /*ooo*/
    not_reached();
}
