/*
 * mkiocccentry - form IOCCC entry compressed tarball
 *
 * Make an IOCCC compressed tarball for an IOCCC entry.
 *
 * We will form the IOCCC entry compressed tarball "by hand" in C.
 * Not in some high level language, but standard vanilla (with a healthy
 * overdose of chocolate :-) ) C.  Why?  Because this is an obfuscated C
 * contest.  But then why isn't this code obfuscated?  Because the IOCCC judges
 * prefer to write in robust unobfuscated code.  Besides, the IOCCC was started
 * as an ironic commentary on the Bourne shell source and finger daemon source.
 * Moreover, irony is well baked-in to the IOCCC.  :-)
 *
 * OK, we do make use of shell scripts to help build and test
 * this repo: but who doesn't use a bit of shell scripting now and then?  :-)
 * Nevertheless, the core of building your IOCCC entry compressed tarball,
 * the code that you, the IOCCC contestant will use, is all written in
 * C because this is the IOCCC.
 *
 * If you do find a problem with this code, please let the judges know.
 * To contact the judges please see:
 *
 *      https://www.ioccc.org/judges.html
 *
 * "Because even printf has a return value worth paying attention to." :-)
 *
 * Many thanks are due to a number of people who provided important
 * and valuable testing, suggestions, issue reports and GitHub pull
 * requests to this code.  Without their time and effort, this tool
 * would not work very well!
 *
 * Among the GitHub users we wish to thank include these fine developers
 * in alphabetical GitHub username order:
 *
 *	@ilyakurdyukov		Ilya Kurdyukov
 *	@SirWumpus		Anthony Howe
 *	@vog			Volker Diels-Grabsch
 *	@xexyl			Cody Boone Ferguson
 *
 * Copyright (c) 2021-2024 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *       supporting documentation
 *       source copies
 *       source works derived from this source
 *       binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * Share and enjoy! :-)
 */


/* special comments for the seqcexit tool */
/* exit code out of numerical order - ignore in sequencing - ooo */
/* exit code change of order - use new value in sequencing - coo */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h> /* for WEXITSTATUS() */
#include <fcntl.h> /* for open() */
#include <string.h> /* for strdup() */

/*
 * mkiocccentry - form IOCCC entry compressed tarball
 */
#include "mkiocccentry.h"


/*
 * definitions
 */
#define REQUIRED_ARGS (4)	/* number of required arguments on the command line */

/*
 * Answers file constants.
 *
 * Version of answers file.
 * Use format:
 *
 *         MKIOCCCENTRY_ANSWERS-IOCCCMOCK-major.minor
 *         MKIOCCCENTRY_ANSWERS-IOCCC[0-9[0-9]-major.minor
 *
 * The following is NOT the version of this mkiocccentry tool!
 *
 * NOTE: These MUST be here and **NOT** in version.h nor in mkiocccentry.h!
 */
#define MKIOCCCENTRY_ANSWERS_VERSION "MKIOCCCENTRY_ANSWERS-IOCCCMOCK-1.0"
/* Answers file EOF marker */
#define MKIOCCCENTRY_ANSWERS_EOF "ANSWERS_EOF"


/*
 * usage message
 *
 * Use the usage() function to print the usage_msg([0-9]?)+ strings.
 */
static const char * const usage_msg0 =
    "usage: %s [options] work_dir prog.c Makefile remarks.md [file ...]\n"
    "\noptions:\n"
    "\t-h\t\tprint help message and exit\n"
    "\t-v level\tset verbosity level: (def level: %d)\n"
    "\t-J level\tset JSON verbosity level (def level: %d)\n"
    "\t-q\t\tquiet mode (def: not quiet)\n"
    "\t\t\t    NOTE: -q will also silence msg(), warn(), warnp() if -v 0\n"
    "\t-V\t\tprint version string and exit\n"
    "\t-W\t\tignore all warnings (this does NOT mean the judges will! :) )\n"
    "\t-y\t\tanswer yes to most questions (use with EXTREME caution!)\n";
static const char * const usage_msg1 =
    "\t-t tar\t\tpath to tar executable that supports the -J (xz) option (def: %s)\n"
    "\t-c cp\t\tpath to cp executable (def: %s)\n"
    "\t-l ls\t\tpath to ls executable (def: %s)\n"
    "\t-T txzchk\tpath to txzchk executable (def: %s)\n"
    "\t-F fnamchk\tpath to fnamchk executable used by txzchk (def: %s)";
static const char * const usage_msg2 =
    "\t-C chkentry	path to chkentry executable (def: %s)\n";
static const char * const usage_msg3 =
    "\t-a answers\twrite answers to a file for easier updating of a submission\n"
    "\t-A answers\twrite answers file even if it already exists\n"
    "\t-i answers\tread answers from file previously written by -a|-A answers\n\n"
    "\t    NOTE: One cannot use both -a/-A answers and -i answers at the same time.\n"
    "\n"
    "\twork_dir\tdirectory where the submission directory and tarball are formed\n"
    "\tprog.c\t\tpath to the C source for your submission\n";
static const char * const usage_msg4 =
    "\n"
    "\tMakefile\tMakefile to build (make all) and cleanup (make clean & make clobber)\n"
    "\n"
    "\tremarks.md\tRemarks about your submission in markdown format\n"
    "\t\t\tNOTE: The following is a guide to markdown:\n"
    "\n"
    "\t\t\t    https://www.markdownguide.org/basic-syntax\n"
    "\n"
    "\t[file ...]\textra data files to include with your submission\n"
    "\n"
    "Exit codes:\n"
    "     0   all OK\n"
    "     2   -h and help string printed or -V and version string printed\n"
    "     3   invalid command line, invalid option or option missing an argument\n"
    " >= 10   internal error\n"
    "\n"
    "mkiocccentry version: %s";

/*
 * globals
 */
static bool quiet = false;			/* true ==> quiet mode */
static bool need_confirm = true;	/* true ==> ask for confirmations */
static bool need_hints = true;		/* true ==> show hints */
static bool ignore_warnings = false;	/* true ==> ignore all warnings (this does NOT mean the judges will! :) */
static FILE *input_stream = NULL;	/* input file: stdin or answers file  */
static unsigned answers_errors;		/* > 0 ==> output errors on answers file */
static bool answer_yes = false;		/* true ==> -y used: always answer yes (use with EXTREME caution!) */

/*
 * forward declarations
 */
static void usage(int exitcode, char const *program, char const *str) __attribute__((noreturn));


int
main(int argc, char *argv[])
{
    char const *program = NULL;			/* our name */
    extern char *optarg;			/* option argument */
    extern int optind;				/* argv index of the next arg */
    struct timeval tp;				/* gettimeofday time value */
    char const *work_dir = NULL;		/* where the submission directory and tarball are formed */
    char const *prog_c = NULL;			/* path to prog.c */
    char const *Makefile = NULL;		/* path to Makefile */
    char const *remarks_md = NULL;		/* path to remarks.md */
    char *tar = TAR_PATH_0;			/* path to tar executable that supports the -J (xz) option */
    char *cp = CP_PATH_0;			/* path to cp executable */
    char *ls = LS_PATH_0;			/* path to ls executable */
    char *txzchk = TXZCHK_PATH_0;		/* path to txzchk executable */
    char *fnamchk = FNAMCHK_PATH_0;		/* path to fnamchk executable */
    char *chkentry = CHKENTRY_PATH_0;		/* path to chkentry executable */
    char const *answers = NULL;			/* path to the answers file (recording input given on stdin) */
    FILE *answerp = NULL;			/* file pointer to the answers file */
    char *submission_dir = NULL;		/* submission directory from which to form a compressed tarball */
    char *tarball_path = NULL;			/* path of the compressed tarball to form */
    int extra_count = 0;			/* number of extra files */
    char **extra_list = NULL;			/* list of extra files (if any) */
    struct info info;				/* data to form .info.json */
    struct auth auth;				/* data to form .auth.json */
    int author_count = 0;			/* number of authors */
    struct author *author_set = NULL;		/* list of authors */
    bool tar_flag_used = false;			/* true ==> -t /path/to/tar was given */
    bool cp_flag_used = false;			/* true ==> -c /path/to/cp was given */
    bool ls_flag_used = false;			/* true ==> -l /path/to/ls was given */
    bool answers_flag_used = false;		/* true ==> -a write answers to answers file */
    bool read_answers_flag_used = false;	/* true ==> -i read answers from answers file */
    bool overwrite_answers_flag_used = false;	/* true ==> don't prompt to overwrite answers if it already exists */
    bool txzchk_flag_used = false;		/* true ==> -T /path/to/txzchk was given */
    bool fnamchk_flag_used = false;		/* true ==> -F /path/to/fnamchk was given */
    bool chkentry_flag_used = false;		/* true ==> -C /path/to/chkentry was given */
    bool overwrite_answers = true;		/* true ==> overwrite answers file even if it already exists */
    RuleCount size;				/* rule_count() processing results */
    int ret;					/* libc return code */
    int i;

    /*
     * parse args
     */
    input_stream = stdin;	/* default to reading from standard in */
    program = argv[0];
    while ((i = getopt(argc, argv, ":hv:J:qVt:c:l:a:i:A:WT:F:C:y")) != -1) {
	switch (i) {
	case 'h':		/* -h - print help to stderr and exit 2 */
	    usage(2, program, ""); /*ooo*/
	    not_reached();
	    break;
	case 'v':		/* -v verbosity */
	    /*
	     * parse verbosity
	     */
	    verbosity_level = parse_verbosity(optarg);
	    if (verbosity_level < 0) {
		usage(3, program, "invalid -v verbosity"); /*ooo*/
		not_reached();
	    }
	    break;
	case 'J': /* -J json_verbosity */
	     /*
             * parse json verbosity level
             */
            json_verbosity_level = parse_verbosity(optarg);
	    if (verbosity_level < 0) {
		usage(3, program, "invalid -J json_verbosity"); /*ooo*/
		not_reached();
	    }
	    break;
	case 'q':
	    quiet = true;
	    msg_warn_silent = true;
	    break;
	case 'V':		/* -V - print version and exit 2 */
	    print("%s\n", MKIOCCCENTRY_VERSION);
	    exit(2); /*ooo*/
	    not_reached();
	    break;
	case 't':		/* -t /path/to/tar */
	    tar = optarg;
	    tar_flag_used = true;
	    break;
	case 'c':		/* -c /path/to/cp */
	    cp = optarg;
	    cp_flag_used = true;
	    break;
	case 'l':		/* -l /path/to/ls */
	    ls = optarg;
	    ls_flag_used = true;
	    break;
	case 'A':		/* -A answers overwrite answers file */
	    answers = optarg;
	    overwrite_answers_flag_used = true;
	    /* FALL THROUGH */
	case 'a':		/* -a record_answers */
	    answers = optarg;
	    answers_flag_used = true;
	    break;
	case 'i':		/* -i input_recorded_answers */
	    answers = optarg;
	    read_answers_flag_used = true;
	    need_confirm = false;
	    need_hints = false;
	    break;
	case 'W':		/* -W ignores all warnings (this does NOT the judges will! :) ) */
	    ignore_warnings = true;
	    break;
	case 'T':
	    txzchk_flag_used = true;
	    txzchk = optarg;
	    break;
	case 'F':
	    fnamchk_flag_used = true;
	    fnamchk = optarg;
	    break;
	case 'C':
	    chkentry_flag_used = true;
	    chkentry = optarg;
	    break;
	case 'y':
	    answer_yes = true;
	    need_confirm = false;
	    ignore_warnings = true;
	    break;
	case ':':   /* option requires an argument */
	case '?':   /* illegal option */
	default:    /* anything else but should not actually happen */
	    check_invalid_option(program, i, optopt);
	    usage(3, program, ""); /*ooo*/
	    not_reached();
	    break;
	}
    }
    /* must have at least the required number of args */
    if (argc - optind < REQUIRED_ARGS) {
	usage(3, program, "wrong number of arguments"); /*ooo*/
	not_reached();
    }

    /*
     * guess where tar, cp and ls utilities are located
     *
     * If the user did not give a -t, -c and/or -l /path/to/x path, then look at
     * the historic location for the utility.  If the historic location of the utility
     * isn't executable, look for an executable in the alternate location.
     *
     * On some systems where /usr/bin != /bin, the distribution made the mistake of
     * moving historic critical applications, look to see if the alternate path works instead.
     */
    find_utils(tar_flag_used, &tar, cp_flag_used, &cp, ls_flag_used, &ls,
	       txzchk_flag_used, &txzchk, fnamchk_flag_used, &fnamchk,
	       chkentry_flag_used, &chkentry);

    /* check that conflicting answers file options are not used together */
    if (answers_flag_used && read_answers_flag_used) {
	err(3, __func__, "-a answers and -i answers cannot be used together"); /*ooo*/
	not_reached();
    }
    /* collect required the required args */
    extra_count = (argc - optind > REQUIRED_ARGS) ? argc - optind - REQUIRED_ARGS : 0;
    extra_list = argv + optind + REQUIRED_ARGS;
    dbg(DBG_LOW, "tar: %s", tar);
    dbg(DBG_LOW, "cp: %s", cp);
    dbg(DBG_LOW, "ls: %s", ls);
    work_dir = argv[optind];
    dbg(DBG_LOW, "work_dir: %s", work_dir);
    prog_c = argv[optind + 1];
    dbg(DBG_LOW, "prog.c: %s", prog_c);
    Makefile = argv[optind + 2];
    dbg(DBG_LOW, "Makefile: %s", Makefile);
    remarks_md = argv[optind + 3];
    dbg(DBG_LOW, "remarks: %s", remarks_md);
    dbg(DBG_LOW, "number of extra data file args: %d", extra_count);
    dbg(DBG_LOW, "answers file: %s", answers);

    /*
     * zeroize info
     */
    memset(&info, 0, sizeof(info));

    /*
     * record the time
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = gettimeofday(&tp, NULL);
    if (ret < 0) {
	errp(10, __func__, "gettimeofday failed");
	not_reached();
    }
    info.tstamp = tp.tv_sec;
    if ((time_t)(-1) > 0) {
	/* case: unsigned time_t */
	dbg(DBG_HIGH, "info.tstamp: %ju", (uintmax_t)info.tstamp);
    } else {
	/* case: signed time_t */
	dbg(DBG_HIGH, "info.tstamp: %jd", (intmax_t)info.tstamp);
    }
    info.usec = tp.tv_usec;
    dbg(DBG_HIGH, "info.usec: %jd", (intmax_t)info.usec);

    /*
     * Welcome
     */
    if (!quiet) {
	print("Welcome to mkiocccentry version: %s\n", MKIOCCCENTRY_VERSION);
    }

    /*
     * save our version
     */
    info.mkiocccentry_ver = MKIOCCCENTRY_VERSION;
    dbg(DBG_HIGH, "info.mkiocccentry_ver: %s", info.mkiocccentry_ver);


    /* warn about -y option */
    if (answer_yes) {
	para("",
	     "WARNING: you've chosen to answer yes to almost all prompts. If this was",
	     "unintentional, run the program again without specifying -y. We cannot",
	     "stress the importance of this enough! Well OK, we can over-stress most things",
	     "but you get the point. Do not use the -y option without EXTREME caution!",
	     "",
	     NULL);
    }

    /* if the user requested to ignore warnings, ignore this once and warn them :) */
    if (ignore_warnings) {
	para("",
	     "WARNING: You've chosen to ignore all warnings. While we will not show",
	     "you any additional warnings, you should note that The Judges will NOT",
	     "ignore warnings! If this was unintentional, run the program again",
	     "without specifying -W. We cannot stress the importance of this enough!",
	     "Well OK, we can over-stress most things, but you get the point.",
	     "Do not use the -W option!",
	     "",
	     NULL);
    }
    /*
     * environment sanity checks
     */
    if (!quiet) {
	para("", "Performing sanity checks on your environment ...", NULL);
    }
    mkiocccentry_sanity_chks(&info, work_dir, tar, cp, ls, txzchk, fnamchk, chkentry);
    if (!quiet) {
	para("... environment looks OK", "", NULL);
    }
    /*
     * if -a answers was specified and answers file exists, prompt user if they
     * want to overwrite it; if they don't tell them how to use it and abort.
     * Else it will be overwritten.
     */
    if (answers_flag_used && !overwrite_answers_flag_used && answers != NULL && strlen(answers) > 0 && exists(answers)) {
	overwrite_answers = yes_or_no("WARNING: The answers file already exists! Do you wish to overwrite it? [yn]");
	if (!overwrite_answers) {
	    errno = 0;		/* pre-clear errno for errp() */
	    ret = printf("\nTo use the answers file, try:\n\n\t./mkiocccentry -i %s [...]\n\n", answers);
	    if (ret <= 0) {
		errp(11, __func__, "printf error telling the user how to use the answers file");
		not_reached();
	    }
	    err(12, __func__, "won't overwrite answers file");
	    not_reached();
	}
    }

    /*
     * check if we should read input from answers file
     */
    if (read_answers_flag_used && answers != NULL && strlen(answers) > 0) {
	if (!is_read(answers)) {
	    errp(13, __func__, "answers file not readable");
	    not_reached();
	}
	errno = 0;		/* pre-clear errno for errp() */
	answerp = fopen(answers, "r");
	if (answerp == NULL) {
	    errp(14, __func__, "cannot open answers file");
	    not_reached();
	}
	input_stream = answerp;
    }
    /*
     * obtain the IOCCC contest ID
     */
    info.ioccc_id = get_contest_id(&info.test_mode, &read_answers_flag_used);
    dbg(DBG_MED, "IOCCC contest ID: %s", info.ioccc_id);

    /*
     * found the answer file header in stdin
     */
    if (read_answers_flag_used && answers == NULL) {
	answerp = stdin;
    }

    /*
     * obtain submission number
     */
    info.submission_num = get_submission_num(&info);
    dbg(DBG_MED, "submission number: %d", info.submission_num);

    /*
     * create submission directory
     */
    submission_dir = mk_submission_dir(work_dir, info.ioccc_id, info.submission_num, &tarball_path, info.tstamp, info.test_mode);
    errno = 0;
    info.tarball = strdup(tarball_path);
    if (info.tarball == NULL) {
	errp(15, __func__, "strdup() tarball path %s failed", tarball_path);
	not_reached();
    }

    dbg(DBG_LOW, "formed submission directory: %s", submission_dir);

    /*
     * if -a, open the answers file. We only do it after verifying that we can
     * make the submission directory because if we get input before and find out the
     * directory already exists then the answers file will have invalid data.
     */
    if (answers_flag_used && answers != NULL && strlen(answers) > 0) {

	errno = 0;			/* pre-clear errno for errp() */
	answerp = fopen(answers, "w");
	if (answerp == NULL) {
	    errp(16, __func__, "cannot create answers file: %s", answers);
	    not_reached();
	}
        ret = fprintf(answerp, "%s\n", MKIOCCCENTRY_ANSWERS_VERSION);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error printing header to the answers file");
	    ++answers_errors;
	}
    }

    /*
     * write the IOCCC id and submission number to the answers file
     */
    if (answerp != NULL && answers_flag_used) {
	errno = 0;			/* pre-clear errno for warnp() */
        ret = fprintf(answerp, "%s\n", info.ioccc_id);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error printing IOCCC contest id to the answers file");
	    ++answers_errors;
	}
	errno = 0;			/* pre-clear errno for warnp() */
	ret = fprintf(answerp, "%d\n", info.submission_num);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error printing submission number to the answers file");
	    ++answers_errors;
	}
    }

    /*
     * check prog.c
     */
    if (!quiet) {
	para("", "Checking prog.c ...", NULL);
    }
    size = check_prog_c(&info, submission_dir, cp, prog_c);
    if (!quiet) {
	para("... completed prog.c check.", "", NULL);
    }

    /*
     * check Makefile
     */
    if (!quiet) {
	para("Checking Makefile ...", NULL);
    }

    check_Makefile(&info, submission_dir, cp, Makefile);

    if (!quiet) {
	para("... completed Makefile check.", "", NULL);
    }

    /*
     * check remarks.md
     */
    if (!quiet) {
	para("Checking remarks.md ...", NULL);
    }
    check_remarks_md(&info, submission_dir, cp, remarks_md);
    if (!quiet) {
	para("... completed remarks.md check.", "", NULL);
    }

    /*
     * check, if needed, extra data files
     */
    if (!quiet) {
	para("Checking extra data files ...", NULL);
    }
    check_extra_data_files(&info, submission_dir, cp, extra_count, extra_list);
    if (!quiet) {
	para("... completed extra data files check.", "", NULL);
    }

    /*
     * obtain the title
     */
    info.title = get_title(&info);
    dbg(DBG_LOW, "submission title: %s", info.title);
    if (answerp != NULL && answers_flag_used) {
	errno = 0;			/* pre-clear errno for warnp() */
	ret = fprintf(answerp, "%s\n", info.title);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error printing title to the answers file");
	    ++answers_errors;
	}
    }

    /*
     * obtain the abstract
     */
    info.abstract = get_abstract(&info);
    dbg(DBG_LOW, "submission abstract: %s", info.abstract);
    if (answerp != NULL && answers_flag_used) {
	errno = 0;			/* pre-clear errno for warnp() */
	ret = fprintf(answerp, "%s\n", info.abstract);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error printing abstract to the answers file");
	    ++answers_errors;
	}
    }

    /*
     * obtain author information
     */
    author_count = get_author_info(&author_set);
    dbg(DBG_LOW, "collected information on %d authors", author_count);

    /*
    * if we have an answers file, record the verified author information
    */
    if (answerp != NULL && !read_answers_flag_used) {
	errno = 0;			/* pre-clear errno for warnp() */
        ret = fprintf(answerp, "%d\n", author_count);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error printing IOCCC author count to the answers file");
	    ++answers_errors;
	}

	/*
	 * write answers for each author to the answers file
	 */
	for (i = 0; i < author_count; i++) {
	    errno = 0;			/* pre-clear errno for warnp() */
	    ret = fprintf(answerp,
		"%s\n"	/* name */
		"%s\n"	/* location code */
		"%s\n"	/* email */
		"%s\n"	/* url */
		"%s\n"	/* alt_url */
		"%s\n"	/* mastodon handle */
		"%s\n"	/* GitHub */
		"%s\n"	/* affiliation */
		"%s\n"  /* past winning author */
		"%s\n"  /* author_handle */
		,
		author_set[i].name,
		author_set[i].location_code,
		author_set[i].email,
		author_set[i].url,
		author_set[i].alt_url,
		author_set[i].mastodon,
		author_set[i].github,
		author_set[i].affiliation,
		author_set[i].past_winning_author?"y":"n",
		author_set[i].author_handle);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error printing author info to the answers file");
		++answers_errors;
	    }
	}
    }

    /*
     * write the .info.json file
     */
    if (!quiet) {
	para("", "Forming the .info.json file ...", NULL);
    }
    write_info(&info, submission_dir, chkentry, fnamchk);
    if (!quiet) {
	para("... completed the .info.json file.", "", NULL);
    }

    /*
     * form auth
     */
    form_auth(&auth, &info, author_count, author_set);

    /*
     * write the .auth.json file
     */
    if (!quiet) {
	para("", "Forming the .auth.json file ...", NULL);
    }
    write_auth(&auth, submission_dir, chkentry, fnamchk);
    if (!quiet) {
	para("... completed .auth.json file.", "", NULL);
    }

    /*
     * finalize the answers file: either write the final answers (if writing
     * answers) or read the answers EOF marker and then finally closing the
     * stream.
     */
    if (answerp != NULL) {
	if (read_answers_flag_used) {
	    char *linep = NULL;
	    char *line;
	    bool error = true;

	    line = readline_dup(&linep, true, NULL, answerp);
	    if (linep != NULL) {
		error = strcmp(line, MKIOCCCENTRY_ANSWERS_EOF) != 0;
		free(linep);
		linep = NULL;
	    }
	    if (error) {
	        errp(17, __func__, "expected ANSWERS_EOF marker at the end of the answers file");
	        not_reached();
	    }
	    input_stream = stdin;
	    if (line != NULL) {
		free(line);
		line = NULL;
	    }
	} else {
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(answerp, "%s\n", MKIOCCCENTRY_ANSWERS_EOF);
	    if (ret <= 0) {
	        warnp(__func__, "fprintf error writing ANSWERS_EOF marker to the answers file");
		++answers_errors;
	    }
	}
	if (answers != NULL) {
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fclose(answerp);
	    if (ret != 0) {
	        warnp(__func__, "error in fclose to the answers file");
		++answers_errors;
	    }
	}
	answerp = NULL;
    }

    /*
     * remind the user about their answers file
     */
    if (answers != NULL) {
	if (need_hints) {
	    errno = 0;			/* pre-clear errno for warnp() */
	    ret = printf("\nTo more easily update this submission you can run:\n\n    ./mkiocccentry -i %s ...\n", answers);
	    if (ret <= 0) {
		warnp(__func__, "unable to tell user how to more easily update submission");
	    }
	}
    }

    /*
     * warn the user if there were I/O errors while writing the answers file
     */
    if (answers_flag_used) {
	if (answers_errors > 0) {
	    errno = 0;	/* pre-clear errno for warnp() */
	    ret = printf("Warning: There were %u I/O error%s on the answers file. Make SURE to verify that using the file\n"
			 "results in the proper input before re-uploading!\n",
			 answers_errors, answers_errors == 1 ? "" : "s" );
	    if (ret <= 0) {
		warnp(__func__, "unable to warn user that there were I/O errors on the answers file");
	    }
	}
    }

    /*
     * If the answers file (-i answers) was used, and there were warnings/problems
     * discovered with the submission that were overridden, warn the user.
     */
    if (read_answers_flag_used) {
	if (info.empty_override ||
	    info.rule_2a_override ||
	    info.rule_2a_mismatch ||
	    info.rule_2b_override ||
	    info.highbit_warning ||
	    info.nul_warning ||
	    info.trigraph_warning ||
	    info.wordbuf_warning ||
	    info.ungetc_warning ||
	    info.Makefile_override) {

	    do {
		if (!ignore_warnings) {
		    need_confirm = true;

		    if (info.empty_override) {
			warn_empty_prog(prog_c);
		    }
		    if (info.rule_2a_override) {
			warn_rule_2a_size(&info, prog_c, RULE_2A_BIG_FILE_WARNING, size);
		    }
		    if (info.rule_2a_mismatch) {
			warn_rule_2a_size(&info, prog_c, RULE_2A_IOCCCSIZE_MISMATCH, size);
		    }
		    if (info.rule_2b_override) {
			warn_rule_2b_size(&info, prog_c);
		    }
		    if (info.highbit_warning) {
			warn_high_bit(prog_c);
		    }
		    if (info.nul_warning) {
			warn_nul_chars(prog_c);
		    }
		    if (info.trigraph_warning) {
			warn_trigraph(prog_c);
		    }
		    if (info.wordbuf_warning) {
			warn_wordbuf(prog_c);
		    }
		    if (info.ungetc_warning) {
			warn_ungetc(prog_c);
		    }
		    if (info.Makefile_override) {
			warn_Makefile(Makefile, &info);
		    }
		}
	    } while (0);
	}
    }

    /*
     * form the .txz file
     */
    form_tarball(work_dir, submission_dir, tarball_path, tar, ls, txzchk, fnamchk);

    /*
     * remind user various things e.g., to upload (unless in test mode)
     */
    remind_user(work_dir, submission_dir, tar, tarball_path, info.test_mode);

    /*
     * free storage
     */
    if (submission_dir != NULL) {
	free(submission_dir);
	submission_dir = NULL;
    }
    if (tarball_path != NULL) {
	free(tarball_path);
	tarball_path = NULL;
    }
    free_info(&info);
    memset(&info, 0, sizeof(info));
    free_auth(&auth);
    memset(&auth, 0, sizeof(auth));


    /*
     * All Done!!! - Jessica Noll, age 2
     */
    exit(0); /*ooo*/
}


/*
 * usage - print usage to stderr
 *
 * Example:
 *      usage(3, program, "wrong number of arguments");
 *
 * given:
 *	exitcode        value to exit with
 *	program		our program name
 *	str		top level usage message
 *
 * NOTE: We warn with extra newlines to help internal fault messages stand out.
 *       Normally one should NOT include newlines in warn messages.
 *
 * This function does not return.
 */
static void
usage(int exitcode, char const *prog, char const *str)
{
    /*
     * firewall
     */
    if (str == NULL) {
	str = "((NULL str))";
	warn(__func__, "\nin usage(): str was NULL, forcing it to be: %s\n", str);
    }
    if (prog == NULL) {
	prog = "((NULL prog))";
	warn(__func__, "\nin usage(): prog was NULL, forcing it to be: %s\n", prog);
    }


    /*
     * print the formatted usage stream
     */
    if (*str != '\0') {
	fprintf_usage(DO_NOT_EXIT, stderr, "%s\n", str);
    }

    fprintf_usage(DO_NOT_EXIT, stderr, usage_msg0, prog, DBG_DEFAULT, JSON_DBG_DEFAULT);
    fprintf_usage(DO_NOT_EXIT, stderr, usage_msg1, TAR_PATH_0, CP_PATH_0, LS_PATH_0, TXZCHK_PATH_0, FNAMCHK_PATH_0);
    fprintf_usage(DO_NOT_EXIT, stderr, usage_msg2, CHKENTRY_PATH_0);
    fprintf_usage(DO_NOT_EXIT, stderr, "%s", usage_msg3);
    fprintf_usage(exitcode, stderr, usage_msg4, MKIOCCCENTRY_VERSION);
    exit(exitcode); /*ooo*/
    not_reached();
}


/*
 * mkiocccentry_sanity_chks - perform basic sanity checks
 *
 * We perform basic sanity checks on paths and the IOCCC contest ID as well as
 * the IOCCC toolkit tables.
 *
 * given:
 *
 *      infop           - pointer to info structure
 *      work_dir        - where the submission directory and tarball are formed
 *      tar             - path to tar that supports the -J (xz) option
 *	cp		- path to the cp utility
 *	ls		- path to the ls utility
 *	txzchk		- path to txzchk tool
 *	fnamchk		- path to fnamchk tool
 *	chkentry	- path to chkentry tool
 *
 * NOTE: This function does not return on error or if things are not sane.
 */
static void
mkiocccentry_sanity_chks(struct info *infop, char const *work_dir, char const *tar, char const *cp,
	   char const *ls, char const *txzchk, char const *fnamchk, char const *chkentry)
{
    /*
     * firewall
     */
    if (infop == NULL || work_dir == NULL || tar == NULL || cp == NULL || ls == NULL ||
	txzchk == NULL || fnamchk == NULL || chkentry == NULL) {
	err(18, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * tar must be executable
     */
    if (!exists(tar)) {
	fpara(stderr,
	      "",
	      "We cannot find the tar program.",
	      "",
	      "A tar program that supports the -J (xz) option is required to build a compressed tarball.",
	      "Perhaps you need to use:",
	      "",
	      "    mkiocccentry -t /path/to/tar ...",
	      "",
	      "and/or install a tar program?  You can find the source for tar:",
	      "",
	      "    https://www.gnu.org/software/tar/",
	      "",
	      NULL);
	err(19, __func__, "tar does not exist: %s", tar);
	not_reached();
    }
    if (!is_file(tar)) {
	fpara(stderr,
	      "",
	      "The tar path, while it exists, is not a regular file.",
	      "",
	      "Perhaps you need to use another path:",
	      "",
	      "    mkiocccentry -t /path/to/tar ...",
	      "",
	      "and/or install a tar program?  You can find the source for tar:",
	      "",
	      "    https://www.gnu.org/software/tar/",
	      "",
	      NULL);
	err(20, __func__, "tar is not a regular file: %s", tar);
	not_reached();
    }
    if (!is_exec(tar)) {
	fpara(stderr,
	      "",
	      "The tar path, while it is a file, is not an executable.",
	      "",
	      "We suggest you check the permissions on the tar program, or use another path:",
	      "",
	      "    mkiocccentry -t /path/to/tar ...",
	      "",
	      "and/or install a tar program?  You can find the source for tar:",
	      "",
	      "    https://www.gnu.org/software/tar/",
	      "",
	      NULL);
	err(21, __func__, "tar is not an executable program: %s", tar);
	not_reached();
    }

    /*
     * cp must be executable
     */
    if (!exists(cp)) {
	fpara(stderr,
	      "",
	      "We cannot find the cp program.",
	      "",
	      "The cp program is required to copy files into a directory under work_dir.",
	      "Perhaps you need to use:",
	      "",
	      "    mkiocccentry -c /path/to/cp ...",
	      "",
	      "and/or install a cp program?  You can find the source for cp in core utilities:",
	      "",
	      "    https://www.gnu.org/software/coreutils/",
	      "",
	      NULL);
	err(22, __func__, "cp does not exist: %s", cp);
	not_reached();
    }
    if (!is_file(cp)) {
	fpara(stderr,
	      "",
	      "The cp path, while it exists, is not a regular file.",
	      "",
	      "Perhaps you need to use another path:",
	      "",
	      "    mkiocccentry -c /path/to/cp ...",
	      "",
	      "and/or install a cp program?  You can find the source for cp in core utilities:",
	      "",
	      "    https://www.gnu.org/software/coreutils/",
	      "",
	      NULL);
	err(23, __func__, "cp is not a regular file: %s", cp);
	not_reached();
    }
    if (!is_exec(cp)) {
	fpara(stderr,
	      "",
	      "The cp path, while it is a file, is not executable.",
	      "",
	      "We suggest you check the permissions on the cp program, or use another path:",
	      "",
	      "    mkiocccentry -c /path/to/cp ...",
	      "",
	      "and/or install a cp program?  You can find the source for cp in core utilities:",
	      "",
	      "    https://www.gnu.org/software/coreutils/",
	      "",
	      NULL);
	err(24, __func__, "cp is not an executable program: %s", cp);
	not_reached();
    }

    /*
     * ls must be executable
     */
    if (!exists(ls)) {
	fpara(stderr,
	      "",
	      "We cannot find the ls program.",
	      "",
	      "The ls program is required to help you verify the correct files are included.",
	      "Perhaps you need to use:",
	      "",
	      "    mkiocccentry -l /path/to/ls ...",
	      "",
	      "and/or install a ls program?  You can find the source for ls in core utilities:",
	      "",
	      "    https://www.gnu.org/software/coreutils/",
	      "",
	      NULL);
	err(25, __func__, "ls does not exist: %s", ls);
	not_reached();
    }
    if (!is_file(ls)) {
	fpara(stderr,
	      "",
	      "The ls path, while it exists, is not a regular file.",
	      "",
	      "Perhaps you need to use another path:",
	      "",
	      "    mkiocccentry -l /path/to/ls ...",
	      "",
	      "and/or install a ls program?  You can find the source for ls in core utilities:",
	      "",
	      "    https://www.gnu.org/software/coreutils/",
	      "",
	      NULL);
	err(26, __func__, "ls is not a regular file: %s", ls);
	not_reached();
    }
    if (!is_exec(ls)) {
	fpara(stderr,
	      "",
	      "The ls path, while it is a file, is not executable.",
	      "",
	      "We suggest you check the permissions on the ls program, or use another path:",
	      "",
	      "    mkiocccentry -l /path/to/ls ...",
	      "",
	      "and/or install a ls program?  You can find the source for ls in core utilities:",
	      "",
	      "    https://www.gnu.org/software/coreutils/",
	      "",
	      NULL);
	err(27, __func__, "ls is not an executable program: %s", ls);
	not_reached();
    }

    /*
     * txzchk must be executable
     */
    if (!exists(txzchk)) {
	fpara(stderr,
	      "",
	      "We cannot find txzchk.",
	      "",
	      "The txzchk program performs sanity checks on the compressed tarball.",
	      "Perhaps you need to use:",
	      "",
	      "    mkiocccentry -T /path/to/txzchk ...",
	      "",
	      "and/or install the txzchk tool?  You can find the source for txzchk in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(28, __func__, "txzchk does not exist: %s", txzchk);
	not_reached();
    }
    if (!is_file(txzchk)) {
	fpara(stderr,
	      "",
	      "The txzchk path, while it exists, is not a regular file.",
	      "",
	      "Perhaps you need to use another path:",
	      "",
	      "    mkiocccentry -T /path/to/txzchk ...",
	      "",
	      "and/or install the txzchk tool?  You can find the source for txzchk in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(29, __func__, "txzchk is not a regular file: %s", txzchk);
	not_reached();
    }
    if (!is_exec(txzchk)) {
	fpara(stderr,
	      "",
	      "The txzchk path, while it is a file, is not executable.",
	      "",
	      "We suggest you check the permissions on the txzchk program, or use another path:",
	      "",
	      "    mkiocccentry -T /path/to/txzchk ...",
	      "",
	      "and/or install the txzchk tool?  You can find the source for txzchk in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(30, __func__, "txzchk is not an executable program: %s", txzchk);
	not_reached();
    }

    /*
     * fnamchk must be executable
     */
    if (!exists(fnamchk)) {
	fpara(stderr,
	      "",
	      "We cannot find the fnamchk tool.",
	      "",
	      "The fnamchk program performs sanity checks on the compressed tarball filename.",
	      "Perhaps you need to use:",
	      "",
	      "    mkiocccentry -F /path/to/fnamchk ...",
	      "",
	      "and/or install the fnamchk tool?  You can find the source for fnamchk in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(31, __func__, "fnamchk does not exist: %s", fnamchk);
	not_reached();
    }
    if (!is_file(fnamchk)) {
	fpara(stderr,
	      "",
	      "The fnamchk path, while it exists, is not a regular file.",
	      "",
	      "Perhaps you need to use another path:",
	      "",
	      "    mkiocccentry -F /path/to/fnamchk ...",
	      "",
	      "and/or install the fnamchk tool?  You can find the source for fnamchk in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(32, __func__, "fnamchk is not a regular file: %s", fnamchk);
	not_reached();
    }
    if (!is_exec(fnamchk)) {
	fpara(stderr,
	      "",
	      "The fnamchk path, while it is a file, is not executable.",
	      "",
	      "We suggest you check the permissions on the fnamchk program, or use another path:",
	      "",
	      "    mkiocccentry -F /path/to/fnamchk ...",
	      "",
	      "and/or install the fnamchk tool?  You can find the source for fnamchk in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(33, __func__, "fnamchk is not an executable program: %s", fnamchk);
	not_reached();
    }

    /*
     * chkentry must be executable
     */
    if (!exists(chkentry)) {
	fpara(stderr,
	      "",
	      "We cannot find the chkentry tool.",
	      "",
	      "The chkentry program performs sanity checks on the JSON files.",
	      "Perhaps you need to use:",
	      "",
	      "    mkiocccentry -C /path/to/chkentry ...",
	      "",
	      "and/or install the chkentry tool?  You can find the source for chkentry in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(34, __func__, "chkentry does not exist: %s", chkentry);
	not_reached();
    }
    if (!is_file(chkentry)) {
	fpara(stderr,
	      "",
	      "The chkentry path, while it exists, is not a regular file.",
	      "",
	      "Perhaps you need to use another path:",
	      "",
	      "    mkiocccentry -C /path/to/chkentry ...",
	      "",
	      "and/or install the chkentry tool?  You can find the source for chkentry in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(35, __func__, "chkentry is not a regular file: %s", chkentry);
	not_reached();
    }
    if (!is_exec(chkentry)) {
	fpara(stderr,
	      "",
	      "The chkentry path, while it is a file, is not executable.",
	      "",
	      "We suggest you check the permissions on the chkentry program, or use another path:",
	      "",
	      "    mkiocccentry -C /path/to/chkentry ...",
	      "",
	      "and/or install the chkentry tool?  You can find the source for chkentry in the mkiocccentry GitHub repo:",
	      "",
	      "    https://github.com/ioccc-src/mkiocccentry",
	      "",
	      NULL);
	err(36, __func__, "chkentry is not an executable program: %s", chkentry);
	not_reached();
    }

    /*
     * work_dir must be a writable directory
     */
    if (!exists(work_dir)) {
	fpara(stderr,
	      "",
	      "The work_dir does not exist.",
	      "",
	      "You should either create work_dir, or use a different work_dir directory path on the command line.",
	      "",
	      NULL);
	err(37, __func__, "work_dir does not exist: %s", work_dir);
	not_reached();
    }
    if (!is_dir(work_dir)) {
	fpara(stderr,
	      "",
	      "While work_dir exists, it is not a directory.",
	      "",
	      "You should move or remove work_dir and them make a new work_dir directory, or use a different",
	      "work_dir directory path on the command line.",
	      "",
	      NULL);
	err(38, __func__, "work_dir is not a directory: %s", work_dir);
	not_reached();
    }
    if (!is_write(work_dir)) {
	fpara(stderr,
	      "",
	      "While the directory work_dir exists, it is not a writable directory.",
	      "",
	      "You should change the permission to make work_dir writable, or move or remove work_dir and then",
	      "create a new writable directory, or use a different work_dir directory path on the command line.",
	      "",
	      NULL);
	err(39, __func__, "work_dir is not a writable directory: %s", work_dir);
	not_reached();
    }

    /*
     * obtain version string from IOCCCSIZE_VERSION
     */
    infop->iocccsize_ver = IOCCCSIZE_VERSION;

    /* we also check that all the tables across the IOCCC toolkit are sane */
    ioccc_sanity_chks();

    return;
}


/*
 * prompt - prompt for a string
 *
 * Prompt a string, followed by :<space> on stdout and then read a line from stdin.
 * The line is stripped of the trailing newline and then of all trailing whitespace.
 *
 * given:
 *      str     - string to string followed by :<space>
 *      lenp    - pointer to where to put the length of the response,
 *                NULL ==> do not save length
 *
 *
 * returns:
 *      allocated input string with newline and trailing whitespace removed
 *	if lenp != NULL, *lenp will be set to the length of the response
 *
 * NOTE: This function will NOT return NULL.
 *
 * This function does not return on error.
 */
static char *
prompt(char const *str, size_t *lenp)
{
    char *linep = NULL;		/* readline_dup line buffer */
    int ret;			/* libc function return value */
    size_t len;			/* length of input */
    char *buf;			/* allocated input string */

    /*
     * firewall
     *
     * NOTE: As noted above, lenp can be NULL.
     */
    if (str == NULL) {
	err(40, __func__, "called with NULL str");
	not_reached();
    }

    /*
     * prompt + :<space>
     */
    clearerr(stdout);		/* pre-clear ferror() status */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fputs(str, stdout);
    if (ret == EOF) {
	if (ferror(stdout)) {
	    errp(41, __func__, "error printing prompt string");
	    not_reached();
	} else if (feof(stdout)) {
	    err(42, __func__, "EOF while printing prompt string");
	    not_reached();
	} else {
	    errp(43, __func__, "unexpected fputs error printing prompt string");
	    not_reached();
	}
    }
    clearerr(stdout);		/* pre-clear ferror() status */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fputs(": ", stdout);
    if (ret == EOF) {
	if (ferror(stdout)) {
	    errp(44, __func__, "error printing :<space>");
	    not_reached();
	} else if (feof(stdout)) {
	    err(45, __func__, "EOF while writing :<space>");
	    not_reached();
	} else {
	    errp(46, __func__, "unexpected fputs error printing :<space>");
	    not_reached();
	}
    }
    clearerr(stdout);		/* pre-clear ferror() status */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fflush(stdout);
    if (ret == EOF) {
	if (ferror(stdout)) {
	    errp(47, __func__, "error flushing prompt to stdout");
	    not_reached();
	} else if (feof(stdout)) {
	    err(48, __func__, "EOF while flushing prompt to stdout");
	    not_reached();
	} else {
	    errp(49, __func__, "unexpected fflush error while flushing prompt to stdout");
	    not_reached();
	}
    }

    /*
     * read user input - return input length
     */
    buf = readline_dup(&linep, true, &len, input_stream);
    if (buf == NULL) {
	err(50, __func__, "EOF while reading prompt input");
	not_reached();
    }
    dbg(DBG_VHIGH, "received a %ju byte response", (uintmax_t)len);

    /*
     * save length if requested
     */
    if (lenp != NULL) {
	*lenp = len;
    }

    /*
     * free storage
     */
    if (linep != NULL) {
	free(linep);
	linep = NULL;
    }

    /*
     * return allocated input buffer
     */
    return buf;
}


/*
 * get IOCCC ID or test
 *
 * This function will prompt the user for a contest ID, validate it and return it
 * as a allocated string.  If the contest ID is the special value "test", then
 * *testp will be set to true, otherwise it will be set to false.
 *
 * given:
 *      infop   - pointer to info structure
 *      testp   - pointer to boolean for test mode
 *
 * returns:
 *      allocated contest ID string
 *      *testp ==> contest ID is "test", else contest ID is a UUID.
 *
 * This function does not return on error or if the contest ID is malformed.
 */
static char *
get_contest_id(bool *testp, bool *read_answers_flag_used)
{
    char *malloc_ret;		/* allocated return string */
    size_t len;			/* input string length */
    bool valid = false;		/* true ==> IOCCC_contest_id is valid */
    bool seen_answers_header = false;

    /*
     * firewall
     */
    if (testp == NULL) {
	err(51, __func__, "called with NULL testp");
	not_reached();
    } else if (read_answers_flag_used == NULL) {
	err(52, __func__, "called with NULL read_answers_flag_used");
	not_reached();
    }

    /*
     * explain contest ID
     */
    if (need_hints) {
	show_registration_url();
	para("",
	     "If you do not have an IOCCC contest ID and you wish to test this program,",
	     "you may use the special contest ID:",
	     "",
	     "    test",
	     "",
	     "Note you will not be able to submit the resulting compressed tarball when using test.",
	     "",
	     NULL);
    }

    /*
     * keep asking for a submission number until we get a valid reply
     */
    do {

	/*
	 * prompt for the contest ID
	 */
	malloc_ret = prompt("Enter IOCCC contest ID or test", &len);
	if (!seen_answers_header && !strcmp(malloc_ret, MKIOCCCENTRY_ANSWERS_VERSION)) {
	    dbg(DBG_HIGH, "found answers header");
	    seen_answers_header = true;
	    *read_answers_flag_used = true;
	    need_confirm = false;
	    need_hints = false;

	    free(malloc_ret);
	    malloc_ret = prompt("", &len);
	}
	if (*read_answers_flag_used && !seen_answers_header) {
	    err(53, __func__, "didn't find the correct answers file header");
	    not_reached();
	}

	dbg(DBG_HIGH, "the IOCCC contest ID as entered is: %s", malloc_ret);

	/*
	 * case: IOCCC contest ID is test, quick return
	 */
	if (strcmp(malloc_ret, "test") == 0) {

	    /*
	     * report test mode
	     */
	    para("",
		 "IOCCC contest ID is test, entering test mode.",
		 NULL);
	    *testp = true;
	    return malloc_ret;
	}

	/*
	 * validate format of non-test contest ID
	 *
	 * The contest ID, if not "test" must be a UUID.  The UUID has the 36 character format:
	 *
	 *             xxxxxxxx-xxxx-4xxx-axxx-xxxxxxxxxxxx
	 *
	 * where 'x' is a hex character.  The 4 is the UUID version and the variant 1.
	 */
	valid = test_IOCCC_contest_id(malloc_ret);
	if (valid == false) {

	    /*
	     * reject invalid non-test contest ID
	     */
	    fpara(stderr,
		  "IOCCC contest IDs are in the form:",
		  "",
		  "    xxxxxxxx-xxxx-4xxx-axxx-xxxxxxxxxxxx",
		  "",
		  "where 'x' is a hex character, 4 is the UUID version and the variant 1.",
		  "",
		  NULL);

	    /*
	     * free storage
	     */
	    if (malloc_ret != NULL) {
		free(malloc_ret);
		malloc_ret = NULL;
	    }
	    continue;
	}
    } while (valid != true);

    /*
     * report on the result of the contest ID validation
     */
    dbg(DBG_MED, "IOCCC contest ID is a UUID: %s", malloc_ret);

    /*
     * report contest ID format is valid
     */
    para("",
	 "The format of the non-test IOCCC contest ID appears to be valid.",
	 NULL);
    *testp = false;		/* IOCCC contest ID is not test */

    /*
     * return IOCCC contest ID
     */
    return malloc_ret;
}


/*
 * get_submission_num - obtain the submission number
 *
 * given:
 *      infop   - pointer to info structure
 *
 * returns:
 *      submission number >= 0 <= MAX_SUBMISSION_NUM
 */
static int
get_submission_num(struct info *infop)
{
    int submission_num;		/* submission number */
    char *submission_str;	/* submission number string */
    int ret;			/* libc function return */
    char guard;			/* scanf guard to catch excess amount of input */

    /*
     * firewall
     */
    if (infop == NULL) {
	err(54, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * keep asking for a submission number until we get a valid reply
     */
    do {

	/*
	 * explain submission numbers
	 */
	if (need_hints) {
	    errno = 0;		/* pre-clear errno for errp() */
	    ret = printf("\nYou are allowed to submit up to %d submissions to a given IOCCC.\n", MAX_SUBMISSION_NUM + 1);
	    if (ret <= 0) {
		errp(55, __func__, "printf error printing number of submissions allowed");
		not_reached();
	    }
	    para("",
		 "As in C, submission numbers start with 0.  If you are updating a previous submission, PLEASE",
		 "use the same submission number that you previously uploaded so we know which submission we",
		 "should replace. If this is your first submission to this given IOCCC, enter 0.",
		 "",
		 NULL);
	}

	/*
	 * ask for the submission number
	 */
	submission_str = prompt("Enter the submission number", NULL);

	/*
	 * check the submission number
	 */
	errno = 0;		/* pre-clear errno for warnp() */
	ret = sscanf(submission_str, "%d%c", &submission_num, &guard);
	if (ret != 1 || submission_num < 0 || submission_num > MAX_SUBMISSION_NUM) {
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "\nThe submission number must be a number from 0 through %d; please re-enter.\n",
		    MAX_SUBMISSION_NUM);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error while informing about the valid submission number range");
	    }
	    submission_num = -1;	/* invalidate input */
	}

	/*
	 * free storage
	 */
	if (submission_str != NULL) {
	    free(submission_str);
	    submission_str = NULL;
	}

    } while (submission_num < 0 || submission_num > MAX_SUBMISSION_NUM);

    /*
     * report on the result of the submission number validation
     */
    dbg(DBG_MED, "IOCCC submission number is valid: %d", submission_num);

    /*
     * return the submission number
     */
    return submission_num;
}


/*
 * mk_submission_dir - make the submission directory
 *
 * Make a directory, under work_dir, from which the compressed tarball
 * will be formed.
 *
 * given:
 *      work_dir        - working directory under which the submission directory is formed
 *      ioccc_id        - IOCCC submission ID (or test)
 *      submission_num       - submission number
 *      tarball_path    - pointer to the allocated path to where the compressed tarball will be formed
 *      tstamp          - now as a timestamp
 *      test_mode       - true ==> test mode, do not upload
 *
 * returns:
 *      the path of the working directory
 *
 * This function does not return on error or if the submission directory cannot be formed.
 */
static char *
mk_submission_dir(char const *work_dir, char const *ioccc_id, int submission_num,
	     char **tarball_path, time_t tstamp, bool test_mode)
{
    size_t submission_dir_len;	/* length of submission directory */
    char *submission_dir = NULL;	/* allocated submission directory path */
    bool test = false;		/* test result */
    int ret;			/* libc function return */

    /*
     * firewall
     */
    if (work_dir == NULL || ioccc_id == NULL || tarball_path == NULL) {
	err(56, __func__, "called with NULL arg(s)");
	not_reached();
    }
    test = test_submission_num(submission_num);
    if (test == false) {
	err(57, __func__, "submission number: %d must >= 0 and <= %d", submission_num, MAX_SUBMISSION_NUM);
	not_reached();
    }

    /*
     * determine length of submission directory path
     */
    /*
     * work_dir/ioccc_id-entry
     */
    submission_dir_len = strlen(work_dir) + 1 + strlen(ioccc_id) + 1 + MAX_SUBMISSION_CHARS + 1 + 1;
    errno = 0;			/* pre-clear errno for errp() */
    submission_dir = (char *)malloc(submission_dir_len + 1);
    if (submission_dir == NULL) {
	errp(58, __func__, "malloc #0 of %ju bytes failed", (uintmax_t)(submission_dir_len + 1));
	not_reached();
    }
    errno = 0;			/* pre-clear errno for errp() */
    ret = snprintf(submission_dir, submission_dir_len + 1, "%s/%s-%d", work_dir, ioccc_id, submission_num);
    if (ret <= 0) {
	errp(59, __func__, "snprintf to form submission directory failed");
	not_reached();
    }
    dbg(DBG_HIGH, "submission directory path: %s", submission_dir);

    /*
     * verify that the submission directory does not exist
     */
    if (exists(submission_dir)) {
	errno = 0;		/* pre-clear errno for errp() */
	ret = fprintf(stderr, "\nsubmission directory already exists: %s\n", submission_dir);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error while informing that the submission directory already exists");
	}
	fpara(stderr,
	      "",
	      "You need to move that directory, or remove it, or use a different work_dir.",
	      "",
	      NULL);
	err(60, __func__, "submission directory exists: %s", submission_dir);
	not_reached();
    }
    dbg(DBG_HIGH, "submission directory path: %s", submission_dir);

    /*
     * make the submission directory
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = mkdir(submission_dir, 0755);
    if (ret < 0) {
	errp(61, __func__, "cannot mkdir %s with mode 0755", submission_dir);
	not_reached();
    }

    /*
     * form the compressed tarball path
     *
     * We assume timestamps will be values of 12 decimal digits or less in the future. :-)
     */
    *tarball_path = form_tar_filename(ioccc_id, submission_num, test_mode, tstamp);
    if (*tarball_path == NULL) {
	errp(62, __func__, "failed to form compressed tarball path");
	not_reached();
    }
    dbg(DBG_HIGH, "compressed tarball path: %s", *tarball_path);

    /*
     * return submission directory
     */
    return submission_dir;
}


/*
 * warn_empty_prog - warn user that prog.c is empty
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_empty_prog(char const *prog_c)
{
    bool yorn = false;

    /*
     * firewall
     */
    if (prog_c == NULL) {
	err(63, __func__, "called with NULL prog.c path");
	not_reached();
    }

    dbg(DBG_MED, "prog.c: %s is empty", prog_c);
    if (need_confirm && !ignore_warnings && !answer_yes) {
	fpara(stderr,
	  "WARNING: prog.c is empty.  An empty prog.c has been submitted before:",
	  "",
	  "    https://www.ioccc.org/years.html#1994_smr",
	  "",
	  "The guidelines indicate that we tend to dislike programs that:",
	  "",
	  "    * are rather similar to previous winning entries  :-(",
	  "",
	  "Perhaps you have a different twist on an empty prog.c than yet another",
	  "smallest self-replicating program.  If so, the you may proceed, although",
	  "we STRONGLY suggest that you put into your remarks.md file, why your",
	  "submission prog.c is not another smallest self-replicating program.",
	  "",
	  NULL);
	    yorn = yes_or_no("Are you sure you want to submit an empty prog.c file? [yn]");
	    if (!yorn) {
		err(64, __func__, "please fix your prog.c file: %s", prog_c);
		not_reached();
	    }
	    dbg(DBG_LOW, "user says that their empty prog.c: %s is OK", prog_c);
	}
}


/*
 * warn_rule_2a_size - warn if prog.c is too large under Rule 2a
 *
 * given:
 *      infop           - pointer to info structure
 *      prog_c          - prog_c arg: given path to prog.c
 *      mode		- determines which warning to give
 *			  Either RULE_2A_BIG_FILE_WARNING or
 *				 RULE_2A_IOCCCSIZE_MISMATCH
 *
 * This function does not return on error.
 */
static void
warn_rule_2a_size(struct info *infop, char const *prog_c, int mode, RuleCount size)
{
    bool yorn = false;
    int ret;

    /*
     * firewall
     */
    if (infop == NULL || prog_c == NULL) {
	err(65, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * File appears to be too big under Rule 2a warning
     */
    if (mode == RULE_2A_BIG_FILE_WARNING) {
	dbg(DBG_MED, "prog.c: %s size: %jd > Rule 2a size: %jd", prog_c,
		     (intmax_t)infop->rule_2a_size, (intmax_t)RULE_2A_SIZE);
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "\nWARNING: The prog.c %s size: %jd > Rule 2a maximum: %jd\n", prog_c,
		      (intmax_t)infop->rule_2a_size, (intmax_t)RULE_2A_SIZE);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error when printing prog.c Rule 2a warning");
	}
	if (need_confirm && !ignore_warnings && !answer_yes) {
	    fpara(stderr,
	      "If you are attempting some clever rule abuse, then we STRONGLY suggest that you",
	      "tell us about your rule abuse in your remarks.md file.  Be sure you have read the",
	      "\"ABUSING THE RULES\" section of the guidelines.  And more importantly, read rule 12!",
	      "",
	      NULL);
	    yorn = yes_or_no("Are you sure you want to submit such a large prog.c file? [yn]");
	    if (!yorn) {
		err(66, __func__, "please fix your prog.c file: %s", prog_c);
		not_reached();
	    }
	    dbg(DBG_LOW, "user says that their prog.c %s size: %jd > Rule 2a max size: %jd is OK", prog_c,
		(intmax_t)infop->rule_2a_size, (intmax_t)RULE_2A_SIZE);
	}

    /*
     * File size and iocccsize file size differ warning
     */
    } else if (mode == RULE_2A_BIG_FILE_WARNING) {
	if (need_confirm && !ignore_warnings && !answer_yes) {
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "\nInteresting: prog.c: %s file size: %jd != rule_count function size: %jd\n"
				  "In order to avoid a possible Rule 2a violation, BE SURE TO CLEARLY MENTION THIS IN\n"
				  "YOUR remarks.md FILE!\n\n",
				  prog_c, (intmax_t)infop->rule_2a_size, (intmax_t)size.rule_2a_size);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error when printing prog.c file size and Rule 2a mismatch");
	    }
	    yorn = yes_or_no("Are you sure you want to proceed? [yn]");
	    if (!yorn) {
		err(67, __func__, "please fix your prog.c file: %s", prog_c);
		not_reached();
	    }
	    dbg(DBG_LOW, "user says that prog.c %s size: %jd != rule_count function size: %jd is OK", prog_c,
		(intmax_t)infop->rule_2a_size, (intmax_t)size.rule_2a_size);
	}

    /*
     * invalid mode
     */
    } else {
	err(68, __func__, "invalid mode passed to function: %d", mode);
	not_reached();
    }
    return;
}


/*
 * warn_high_bit - warn user that prog.c has high bit chars in prog.c
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_high_bit(char const *prog_c)
{
    int ret, yorn;

    /*
     * firewall
     */
    if (prog_c == NULL) {
	err(69, __func__, "called with NULL prog.c path");
	not_reached();
    }

    /*
     * warn about high bit chars, if we are allowed
     */
    if (need_confirm && !ignore_warnings && !answer_yes) {
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "\nprog.c: %s has non-ASCII and/or character(s) with high bit set!\n"
			      "Be careful you don't violate rule 13!\n\n", prog_c);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error when printing prog.c char_warning");
	}
	yorn = yes_or_no("Are you sure you want to proceed? [yn]");
	if (!yorn) {
	    err(70, __func__, "please fix your prog.c file: %s", prog_c);
	    not_reached();
	}
	dbg(DBG_LOW, "user says that prog.c %s having non-ASCII and/or character(s) with high bit is OK", prog_c);
    }
}


/*
 * warn_nul_chars - warn user that prog.c has high one or more NUL char(s)
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_nul_chars(char const *prog_c)
{
    int ret, yorn;

    /*
     * firewall
     */
    if (prog_c == NULL) {
	err(71, __func__, "called with NULL prog.c path");
	not_reached();
    }

    /*
     * warn about NUL chars(s) if we are allowed
     */
    if (need_confirm && !ignore_warnings && !answer_yes) {
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "\nprog.c: %s has NUL character(s)!\n"
			      "Be careful you don't violate rule 13!\n\n", prog_c);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error when printing prog.c nul_warning");
	}
	yorn = yes_or_no("Are you sure you want to proceed? [yn]");
	if (!yorn) {
	    err(72, __func__, "please fix your prog.c file: %s", prog_c);
	    not_reached();
	}
	dbg(DBG_LOW, "user says that prog.c %s having NUL character(s) is OK", prog_c);
    }
}


/*
 * warn_trigraph - warn user that prog.c has unknown or invalid trigraph(s)
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_trigraph(char const *prog_c)
{
    bool yorn = false;
    int ret;

    /*
     * firewall
     */
    if (prog_c == NULL) {
	err(73, __func__, "called with NULL prog.c path");
	not_reached();
    }

    /*
     * warn the user about unknown or invalid trigraph(s), if we are allowed
     */
    if (need_confirm && !ignore_warnings && !answer_yes) {
	errno = 0;		/* pre-clear errno for errp() */
	ret = fprintf(stderr, "\nprog.c: %s has unknown or invalid trigraph(s) found!\n"
			      "Is that a bug in, or a feature of your code?\n\n", prog_c);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error when printing prog.c trigraph_warning");
	}
	yorn = yes_or_no("Are you sure you want to proceed? [yn]");
	if (!yorn) {
	    err(74, __func__, "please fix your prog.c file: %s", prog_c);
	    not_reached();
	}
	dbg(DBG_LOW, "user says that prog.c %s having unknown or invalid trigraph(s) is OK", prog_c);
    }
}


/*
 * warn_wordbuf - warn user that prog.c triggered a word buffer overflow
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_wordbuf(char const *prog_c)
{
    int ret, yorn;

    /*
     * firewall
     */
    if (prog_c == NULL) {
	err(75, __func__, "called with NULL prog.c path");
	not_reached();
    }

    /*
     * warn the user about triggered a word buffer overflow in iocccsize, if we are allowed
     */
    if (need_confirm && !ignore_warnings && !answer_yes) {
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "\nprog.c: %s triggered a word buffer overflow!\n"
			      "In order to avoid a possible Rule 2b violation, BE SURE TO CLEARLY MENTION THIS IN\n"
			      "YOUR remarks.md FILE!\n\n", prog_c);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error when printing prog.c wordbuf_warning");
	}
	yorn = yes_or_no("Are you sure you want to proceed? [yn]");
	if (!yorn) {
	    err(76, __func__, "please fix your prog.c file: %s", prog_c);
	    not_reached();
	}
	dbg(DBG_LOW, "user says that prog.c %s triggered a word buffer overflow is OK", prog_c);
    }
}


/*
 * warn_ungetc - warn user that prog.c triggered an ungetc error
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_ungetc(char const *prog_c)
{
    bool yorn = false;
    int ret;

    /*
     * firewall
     */
    if (prog_c == NULL) {
	err(77, __func__, "called with NULL prog.c path");
	not_reached();
    }

    /*
     * warn the user abort iocccsize ungetc error, if we are allowed
     */
    if (need_confirm && !ignore_warnings && !answer_yes) {
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "\nprog.c: %s triggered an ungetc error: @SirWumpus goofed\n"
			      "In order to avoid a possible Rule 2b violation, BE SURE TO CLEARLY MENTION THIS IN\n"
			      "YOUR remarks.md FILE!\n\n", prog_c);
	if (ret <= 0) {
	    warnp(__func__, "fprintf error when printing prog.c ungetc_warning");
	}
	yorn = yes_or_no("Are you sure you want to proceed? [yn]");
	if (!yorn) {
	    err(78, __func__, "please fix your prog.c file: %s", prog_c);
	    not_reached();
	}
	dbg(DBG_LOW, "user says that prog.c %s triggering an ungetc warning OK", prog_c);
    }
}


/*
 * warn_rule_2b_size - warn user that prog.c triggered an ungetc error
 *
 * given:
 *
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static void
warn_rule_2b_size(struct info *infop, char const *prog_c)
{
    int ret, yorn;

     /*
     * firewall
     */
    if (infop == NULL || prog_c == NULL) {
	err(79, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * warn the user about a possible Rule 2b violation, if we are allowed
     */
    if (need_confirm && !ignore_warnings && !answer_yes) {
	errno = 0;
	ret = fprintf(stderr, "\nWARNING: The prog.c %s size: %ju > Rule 2b maximum: %ju\n", prog_c,
		      (uintmax_t)infop->rule_2b_size, (uintmax_t)RULE_2B_SIZE);
	if (ret <= 0) {
	    errp(80, __func__, "printf error printing prog.c size > Rule 2b maximum");
	    not_reached();
	}

	fpara(stderr,
	      "If you are attempting some clever rule abuse, then we STRONGLY suggest that you",
	      "tell us about your rule abuse in your remarks.md file.  Be sure you have read the",
	      "\"ABUSING THE RULES\" section of the guidelines.  And more importantly, read rule 12!",
	      "",
	      NULL);
	yorn = yes_or_no("Are you sure you want to submit such a large prog.c file? [yn]");
	if (!yorn) {
	    err(81, __func__, "please fix your prog.c file: %s", prog_c);
	    not_reached();
	}
	dbg(DBG_LOW, "user says that their prog.c %s size: %ju > Rule 2B max size: %ju is OK", prog_c,
	    (uintmax_t)infop->rule_2b_size, (uintmax_t)RULE_2B_SIZE);
    }
}


/*
 * check_prog_c - check prog_c arg and if OK, copy into submission_dir/prog.c
 *
 * Check if the prog_c argument is a readable file, and
 * if it is within the guidelines of iocccsize (or if the author overrides),
 * and if all is OK or overridden,
 * use cp to copy into submission_dir/prog.c.
 *
 * given:
 *      infop           - pointer to info structure
 *      submission_dir  - newly created submission directory (by mk_submission_dir()) under work_dir
 *      cp              - cp utility path
 *      prog_c          - prog_c arg: given path to prog.c
 *
 * This function does not return on error.
 */
static RuleCount
check_prog_c(struct info *infop, char const *submission_dir, char const *cp, char const *prog_c)
{
    FILE *prog_stream;		/* prog.c open file stream */
    size_t prog_c_len;		/* length of the prog_c path */
    size_t submission_dir_len;	/* length of the submission_dir path */
    int exit_code;		/* exit code from shell_cmd() */
    int ret;			/* libc function return */
    RuleCount size;		/* rule_count() processing results */

    /*
     * firewall
     */
    if (infop == NULL || submission_dir == NULL || cp == NULL || prog_c == NULL) {
	err(82, __func__, "called with NULL arg(s)");
	not_reached();
    }
    submission_dir_len = strlen(submission_dir);
    if (submission_dir_len <= 0) {
	err(83, __func__, "submission_dir arg is an empty string");
	not_reached();
    }
    prog_c_len = strlen(prog_c);
    if (prog_c_len <= 0) {
	err(84, __func__, "prog_c arg is an empty string");
	not_reached();
    }

    /*
     * prog.c must be a readable file
     */
    if (!exists(prog_c)) {
	fpara(stderr,
	      "",
	      "We cannot find the prog.c file.",
	      "",
	      NULL);
	err(85, __func__, "prog.c does not exist: %s", prog_c);
	not_reached();
    }
    if (!is_file(prog_c)) {
	fpara(stderr,
	      "",
	      "The prog.c path, while it exists, is not a regular file.",
	      "",
	      NULL);
	err(86, __func__, "prog.c is not a regular file: %s", prog_c);
	not_reached();
    }
    if (!is_read(prog_c)) {
	fpara(stderr,
	      "",
	      "The prog.c path, while it is a file, is not readable.",
	      "",
	      NULL);
	err(87, __func__, "prog.c is not a readable file: %s", prog_c);
	not_reached();
    }

    /*
     * determine Rule 2b for prog.c
     *
     * If the size returned is outside of the allowed range, the user will
     * be asked if they want to proceed.  We will allow it if the user insists
     * on proceeding, or of iocccsize says the size is allowed under Rule 2a.
     */
    errno = 0;			/* pre-clear errno for errp() */
    prog_stream = fopen(prog_c, "r");
    if (prog_stream == NULL) {
	errp(88, __func__, "failed to fopen: %s", prog_c);
	not_reached();
    }
    size = rule_count(prog_stream);
    infop->rule_2b_size = size.rule_2b_size;
    dbg(DBG_MED, "prog.c: %s Rule 2b size: %ju", prog_c, (uintmax_t)infop->rule_2b_size);
    errno = 0;			/* pre-clear errno for errp() */
    ret = fclose(prog_stream);
    if (ret != 0) {
	errp(89, __func__, "failed to fclose: %s", prog_c);
	not_reached();
    }

    /*
     * warn if prog.c is empty
     */
    infop->rule_2a_size = file_size(prog_c);
    dbg(DBG_MED, "Rule 2a size: %jd", (intmax_t)infop->rule_2a_size);
    if (infop->rule_2a_size < 0) {
	err(90, __func__, "file_size error: %jd on prog.c: %s", (intmax_t)infop->rule_2a_size, prog_c);
	not_reached();
    } else if (infop->rule_2a_size == 0 || infop->rule_2b_size == 0) {
	warn_empty_prog(prog_c);
	infop->empty_override = true;
	infop->rule_2a_override = false;

    /*
     * warn if prog.c is too large under Rule 2a
     */
    } else if (infop->rule_2a_size > RULE_2A_SIZE) {
	warn_rule_2a_size(infop, prog_c, RULE_2A_BIG_FILE_WARNING, size);
	infop->empty_override = false;
	infop->rule_2a_override = true;
    } else {
	infop->empty_override = false;
	infop->rule_2a_override = false;
    }

    /*
     * sanity check on file size vs rule_count function size for Rule 2a
     */
    if (infop->rule_2a_size != size.rule_2a_size) {
	warn_rule_2a_size(infop, prog_c, RULE_2A_IOCCCSIZE_MISMATCH, size);
	infop->rule_2a_mismatch = true;
    } else {
	infop->rule_2a_mismatch = false;
    }

    /*
     * sanity check on high bit character(s)
     */
    if (size.char_warning) {
	warn_high_bit(prog_c);
	infop->highbit_warning = true;
    } else {
	infop->highbit_warning = false;
    }

    /*
     * sanity check on NUL character(s)
     */
    if (size.nul_warning) {
	warn_nul_chars(prog_c);
	infop->nul_warning = true;
    } else {
	infop->nul_warning = false;
    }

    /*
     * sanity check on unknown trigraph(s)
     */
    if (size.trigraph_warning) {
	warn_trigraph(prog_c);
	infop->trigraph_warning = true;
    } else {
	infop->trigraph_warning = false;
    }

    /*
     * sanity check on word buffer overflow
     */
    if (size.wordbuf_warning) {
	warn_wordbuf(prog_c);
	infop->wordbuf_warning = true;
    } else {
	infop->wordbuf_warning = false;
    }

    /*
     * sanity check on ungetc warning
     */
    if (size.ungetc_warning) {
	warn_ungetc(prog_c);
	infop->ungetc_warning = true;
    } else {
	infop->ungetc_warning = false;
    }

    /*
     * inspect the Rule 2b size
     */
    if (infop->rule_2b_size > RULE_2B_SIZE) {
	warn_rule_2b_size(infop, prog_c);
	infop->rule_2b_override = true;
    } else {
	infop->rule_2b_override = false;
    }

    /*
     * copy prog.c under submission_dir
     */
    dbg(DBG_HIGH, "about to perform: %s -- %s %s/prog.c", cp, prog_c, submission_dir);
    exit_code = shell_cmd(__func__, false, true, "% -- % %/prog.c", cp, prog_c, submission_dir);
    if (exit_code != 0) {
	err(91, __func__, "%s -- %s %s/prog.c failed with exit code: %d",
			  cp, prog_c, submission_dir, WEXITSTATUS(exit_code));
	not_reached();
    }

    /*
     * save prog.c filename
     */
    errno = 0;			/* pre-clear errno for errp() */
    infop->prog_c = strdup("prog.c");
    if (infop->prog_c == NULL) {
	errp(92, __func__, "malloc #2 of %ju bytes failed", (uintmax_t)(LITLEN("prog.c") + 1));
	not_reached();
    }
    return size;
}


/*
 * inspect_Makefile - inspect the rule contents of Makefile
 *
 * Determine if the first rule contains all.  Determine if there is a clean rule.
 * Determine if there is a clobber rule.  Determine if there is a try rule.
 *
 * NOTE: This is a simplistic Makefile line parser.  It is possible to
 *       fool the line scanner and to evade rule detection due to use of
 *       Makefile variables, line continuation, conditional Gnu-make controls, etc.
 *
 * given:
 *      Makefile  - path to Makefile
 *      infop	  - pointer to info struct
 *
 * returns:
 *      true ==> the rule set in Makefile is OK,
 *      false ==> the Makefile has an issue
 *
 * This function does not return on error.
 */
static bool
inspect_Makefile(char const *Makefile, struct info *infop)
{
    FILE *stream;		/* open file stream */
    int ret;			/* libc function return */
    char *linep = NULL;		/* allocated line read from iocccsize */
    char *line;			/* Makefile line to parse */
    int rulenum = 0;		/* current rule number */
    char *p;
    char *saveptr = NULL;	/* for strtok_r */

    /*
     * firewall
     */
    if (Makefile == NULL || infop == NULL) {
	err(93, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * open Makefile
     */
    errno = 0;			/* pre-clear errno for errp() */
    stream = fopen(Makefile, "r");
    if (stream == NULL) {
	errp(94, __func__, "cannot open Makefile: %s", Makefile);
	not_reached();
    }

    /*
     * process lines until EOF
     */
    do {

	/*
	 * process the next line
	 */
	line = readline_dup(&linep, true, NULL, stream);
	if (line == NULL) {
	    break;
	}

	/*
	 * free storage
	 */
	if (linep != NULL) {
	    free(linep);
	    linep = NULL;
	}

	/*
	 * trim off comments
	 */
	p = strchr(line, '#');
	if (p != NULL) {
	    /*
	     * trim off comment
	     */
	    *p = '\0';
	}

	/*
	 * skip line if there is no :
	 */
	p = strchr(line, ':');
	if (p == NULL) {

	    /*
	     * free storage
	     */
	    if (line != NULL) {
		free(line);
		line = NULL;
	    }

	    /*
	     * non-: line
	     */
	    continue;

	/*
	 * we appear to have a make rule line
	 */
	} else {
	    /* trim off : and later dependency text */
	    *p = 0;
	}
	++rulenum;

	/*
	 * split the line into whitespace separated tokens
	 */
	for (p = strtok_r(line, " \t", &saveptr); p != NULL; p = strtok_r(NULL, ":", &saveptr)) {

	    /*
	     * detect all rule
	     */
	    dbg(DBG_VHIGH, "rulenum[%d]: token: %s", rulenum, p);
	    if (!infop->found_all_rule && strcmp(p, "all") == 0) {
		/*
		 * first all rule found
		 */
		dbg(DBG_HIGH, "rulenum[%d]: all token found", rulenum);
		infop->found_all_rule = true;
		if (rulenum == 1) {
		    /*
		     * all rule is in first rule line
		     */
		    infop->first_rule_is_all = true;
		    break;
		}

	    /*
	     * detect clean rule
	     */
	    } else if (!infop->found_clean_rule && strcmp(p, "clean") == 0) {
		/*
		 * first clean rule found
		 */
		dbg(DBG_HIGH, "rulenum[%d]: clean token found", rulenum);
		infop->found_clean_rule = true;

	    /*
	     * detect clobber rule
	     */
	    } else if (!infop->found_clobber_rule && strcmp(p, "clobber") == 0) {
		/*
		 * first clobber rule found
		 */
		dbg(DBG_HIGH, "rulenum[%d]: clobber token found", rulenum);
		infop->found_clobber_rule = true;

	    /*
	     * detect try rule
	     */
	    } else if (!infop->found_try_rule && strcmp(p, "try") == 0) {
		/*
		 * first try rule found
		 */
		dbg(DBG_HIGH, "rulenum[%d]: try token found", rulenum);
		infop->found_try_rule = true;
	    }
	}

	/*
	 * free storage
	 */
	if (line != NULL) {
	    free(line);
	    line = NULL;
	}

    } while (!infop->first_rule_is_all || !infop->found_all_rule || !infop->found_clean_rule ||
	     !infop->found_clobber_rule || !infop->found_try_rule);

    /*
     * close Makefile
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fclose(stream);
    if (ret < 0) {
	errp(95, __func__, "fclose error");
	not_reached();
    }

    /*
     * free storage
     */
    if (line != NULL) {
	free(line);
	line = NULL;
    }

    /*
     * if our parse of Makefile was successful
     */
    if (infop->first_rule_is_all && infop->found_all_rule && infop->found_clean_rule &&
	infop->found_clobber_rule && infop->found_try_rule) {
	dbg(DBG_MED, "Makefile appears to pass");
	return true;
    }

    return false;
}


/*
 * warn_Makefile - warn user that Makefile is invalid/incomplete
 *
 * given:
 *
 *      Makefile          - Makefile arg: given path to Makefile
 *      infop		  - pointer to info struct
 *
 * This function does not return on error.
 */
static void
warn_Makefile(char const *Makefile, struct info *infop)
{
    bool yorn = false;
    int ret;

    /*
     * firewall
     */
    if (Makefile == NULL || infop == NULL) {
	err(96, __func__, "called with NULL arg(s)");
	not_reached();
    }
    if (need_confirm && !ignore_warnings) {

	/*
	 * report problem with Makefile
	 */
	fpara(stderr,
	      "",
	      "At least one problem was detected with the Makefile provided:",
	      "",
	      NULL);
	if (!infop->first_rule_is_all) {
	    fpara(stderr, "The all rule appears to not be the first (default) rule.",
		  "",
		  NULL);
	}
	if (!infop->found_all_rule) {
	    fpara(stderr,
		  "  The Makefile appears to not have an all rule.",
		  "    The all rule should make your compiled/built program.",
		  "",
		  NULL);
	}
	if (!infop->found_clean_rule) {
	    fpara(stderr,
		  "  The Makefile appears to not have a clean rule.",
		  "    The clean rule should remove any intermediate build files.",
		  "    For example, remove .o files and other intermediate build files .",
		  "    The clean rule should NOT remove compiled/built program built by the all rule.",
		  "",
		  NULL);
	}
	if (!infop->found_clobber_rule) {
	    fpara(stderr,
		  "  The Makefile appears to not have a clobber rule.",
		  "    The clobber rule should restore the directory to the original submission state.",
		  "    The clobber rule should depend on the clean rule and should remove the submission's program,",
		  "    clean up after program execution (if needed) and restore the entire directory back",
		  "    to the original submission state.",
		  "",
		  NULL);
	}
	if (!infop->found_try_rule) {
	    fpara(stderr,
		  "  The Makefile appears to not have a try rule.",
		  "    The try rule should execute the program with suggested arguments (if any needed).",
		  "    The program may be executed more than once if such examples are informative.",
		  "	   The try rule should depend on the all rule.",
		  "",
		  NULL);
	}

	/*
	 * Explain again what is needed in a Makefile
	 */
	fpara(stderr,
	      "Makefiles must have the following Makefile rules:",
	      "",
	      "    all - compile the submission, must be the first submission",
	      "    clean - remove intermediate compilation files",
	      "    clobber - clean, remove compiled submission, restore to the original submission state",
	      "    try - invoke the submission at least once",
	      "",
	      "While this program's parser may have missed finding those Makefile rules,",
	      "chances are this file is not a proper Makefile under the IOCCC rules:",
	      "",
	      NULL);
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "    %s\n\n", Makefile);
	if (ret <= 0) {
	    warnp(__func__, "fprintf for Makefile error: %d", ret);
	}

	/*
	 * Ask if they want to submit it anyway unless -y
	 */
	if (!answer_yes) {
	    yorn = yes_or_no("Do you still want to submit this Makefile in the hopes that it is OK? [yn]");
	    if (!yorn) {
		err(97, __func__, "Use a different Makefile or modify this file: %s", Makefile);
		not_reached();
	    }
	}
    }
}


/*
 * check_Makefile - check Makefile arg and if OK, copy into submission_dir/Makefile
 *
 * Check if the Makefile argument is a readable file, and
 * if it has the proper rules (starting with all:),
 * use cp to copy into submission_dir/Makefile.
 *
 * given:
 *      infop           - pointer to info structure
 *      submission_dir  - newly created submission directory (by mk_submission_dir()) under work_dir
 *      cp              - cp utility path
 *      Makefile        - Makefile arg: given path to Makefile
 *
 * This function does not return on error.
 */
static void
check_Makefile(struct info *infop, char const *submission_dir, char const *cp, char const *Makefile)
{
    off_t filesize = 0;		/* size of Makefile */
    int exit_code;		/* exit code from shell_cmd() */

    /*
     * firewall
     */
    if (infop == NULL || submission_dir == NULL || cp == NULL || Makefile == NULL) {
	err(98, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * Makefile must be a non-empty readable file
     */
    if (!exists(Makefile)) {
	fpara(stderr,
	      "",
	      "We cannot find the Makefile.",
	      "",
	      NULL);
	err(99, __func__, "Makefile does not exist: %s", Makefile);
	not_reached();
    }
    if (!is_file(Makefile)) {
	fpara(stderr,
	       "",
	       "The Makefile path, while it exists, is not a regular file.",
	       "",
	       NULL);
	err(100, __func__, "Makefile is not a regular file: %s", Makefile);
	not_reached();
    }
    if (!is_read(Makefile)) {
	fpara(stderr,
	      "",
	      "The Makefile path, while it is a file, is not readable.",
	      "",
	      NULL);
	err(101, __func__, "Makefile is not readable file: %s", Makefile);
	not_reached();
    }
    filesize = file_size(Makefile);
    if (filesize < 0) {
	err(102, __func__, "file_size error: %jd on Makefile  %s", (intmax_t)filesize, Makefile);
	not_reached();
    } else if (filesize == 0) {
	err(103, __func__, "Makefile cannot be empty: %s", Makefile);
	not_reached();
    }

    /*
     * scan Makefile for critical rules
     */
    if (!inspect_Makefile(Makefile, infop)) {
	warn_Makefile(Makefile, infop);
	infop->Makefile_override = true;
    } else {
	infop->Makefile_override = false;
    }

    /*
     * copy Makefile under submission_dir
     */
    dbg(DBG_HIGH, "about to perform: %s --  %s %s/Makefile", cp, Makefile, submission_dir);
    exit_code = shell_cmd(__func__, false, true, "% -- % %/Makefile", cp, Makefile, submission_dir);
    if (exit_code != 0) {
	err(104, __func__, "%s --  %s %s/Makefile failed with exit code: %d",
			   cp, Makefile, submission_dir, WEXITSTATUS(exit_code));
	not_reached();
    }

    /*
     * save Makefile filename
     */
    errno = 0;			/* pre-clear errno for errp() */
    infop->Makefile = strdup("Makefile");
    if (infop->Makefile == NULL) {
	errp(105, __func__, "malloc #1 of %ju bytes failed", (uintmax_t)(LITLEN("Makefile") + 1));
	not_reached();
    }
    return;
}


/*
 * check_remarks_md - check remarks_md arg and if OK, copy into submission_dir/Makefile
 *
 * Check if the remarks_md argument is a readable file, and
 * if it is not empty, use cp to copy into submission_dir/remarks.md.
 *
 * given:
 *      infop           - pointer to info structure
 *      submission_dir  - the newly created submission directory (by mk_submission_dir()) under work_dir
 *      cp              - cp utility path
 *      remarks_md      - remarks_md arg: given path to author's remarks markdown file
 *
 * This function does not return on error.
 */
static void
check_remarks_md(struct info *infop, char const *submission_dir, char const *cp, char const *remarks_md)
{
    off_t filesize = 0;		/* size of remarks.md */
    int exit_code;		/* exit code from shell_cmd() */

    /*
     * firewall
     */
    if (infop == NULL || submission_dir == NULL || cp == NULL || remarks_md == NULL) {
	err(106, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * remarks_md must be a non-empty readable file
     */
    if (!exists(remarks_md)) {
	fpara(stderr,
	       "",
	       "We cannot find the remarks.md file.",
	       "",
	       NULL);
	err(107, __func__, "remarks.md does not exist: %s", remarks_md);
	not_reached();
    }
    if (!is_file(remarks_md)) {
	fpara(stderr, "",
	      "The remarks.md path, while it exists, is not a regular file.",
	      "",
	      NULL);
	err(108, __func__, "remarks.md is not a regular file: %s", remarks_md);
	not_reached();
    }
    if (!is_read(remarks_md)) {
	fpara(stderr,
	      "",
	      "The remarks.md path, while it is a file, is not readable.",
	      "",
	      NULL);
	err(109, __func__, "remarks.md is not readable file: %s", remarks_md);
	not_reached();
    }
    filesize = file_size(remarks_md);
    if (filesize < 0) {
	err(110, __func__, "file_size error: %jd on remarks.md %s", (intmax_t)filesize, remarks_md);
	not_reached();
    } else if (filesize == 0) {
	err(111, __func__, "remarks.md cannot be empty: %s", remarks_md);
	not_reached();
    }

    /*
     * copy remarks.md under submission_dir
     */
    dbg(DBG_HIGH, "about to perform: %s -- %s %s/remarks.md", cp, remarks_md, submission_dir);
    exit_code = shell_cmd(__func__, false, true, "% -- % %/remarks.md", cp, remarks_md, submission_dir);
    if (exit_code != 0) {
	err(112, __func__, "%s -- %s %s/remarks.md failed with exit code: %d",
			   cp, remarks_md, submission_dir, WEXITSTATUS(exit_code));
	not_reached();
    }

    /*
     * save remarks_md filename
     */
    errno = 0;			/* pre-clear errno for errp() */
    infop->remarks_md = strdup("remarks.md");
    if (infop->remarks_md == NULL) {
	errp(113, __func__, "malloc #1 of %ju bytes failed", (uintmax_t)(LITLEN("remarks.md") + 1));
	not_reached();
    }
    return;
}


/*
 * check_extra_data_files - check extra data files args and if OK, copy into submission_dir/Makefile
 *
 * Check if the check extra data files are readable, and
 * use cp to copy into submission_dir/remarks.md.
 *
 * given:
 *      infop           - pointer to info structure
 *      submission_dir  - newly created submission directory (by mk_submission_dir()) under work_dir
 *      cp              - cp utility path
 *      count           - number of extra data files arguments
 *      args            - pointer to an array of strings starting with first extra data file
 *
 * This function does not return on error.
 */
static void
check_extra_data_files(struct info *infop, char const *submission_dir, char const *cp, int count, char **args)
{
    char *base;			/* basename of extra data file */
    char *dest;			/* destination path of an extra data file */
    size_t base_len;		/* length of the basename of the data file */
    size_t dest_len;		/* length of the extra data file path */
    size_t submission_dir_len;	/* length of the submission_dir path */
    int exit_code;		/* exit code from shell_cmd() */
    int ret;			/* libc function return */
    int i;

    /*
     * firewall
     */
    if (infop == NULL || submission_dir == NULL || cp == NULL || args == NULL) {
	err(114, __func__, "called with NULL arg(s)");
	not_reached();
    }
    if (count < 0) {
	err(115, __func__, "count: %d < 0", count);
	not_reached();
    }

    /*
     * save extra data file count
     */
    infop->extra_count = count;

    /*
     * allocate extra file arrays
     */
    errno = 0;			/* pre-clear errno for errp() */
    /* + 1 for trailing NULL */
    infop->extra_file = (char **)calloc((size_t)(count + 1), sizeof(char *));
    if (infop->extra_file == NULL) {
	errp(116, __func__, "calloc #0 of %d char * pointers failed", count + 1);
	not_reached();
    }

    /*
     * process all of the extra args
     */
    submission_dir_len = strlen(submission_dir);
    for (i = 0; i < count && args[i] != NULL; ++i) {

	/*
	 * extra data file must be a readable file
	 */
	dbg(DBG_HIGH, "processing extra data file %d: %s", i, args[i]);
	if (!exists(args[i])) {
	    fpara(stderr,
		  "",
		  "We cannot find an extra data file.",
		  "",
		  NULL);
	    err(117, __func__, "extra[%i] does not exist: %s", i, args[i]);
	    not_reached();
	}
	if (!is_file(args[i])) {
	    fpara(stderr,
		   "",
		   "The path, while it exists, is not a regular file.",
		   "",
		   NULL);
	    err(118, __func__, "extra[%i] is not a regular file: %s", i, args[i]);
	    not_reached();
	}
	if (!is_read(args[i])) {
	    fpara(stderr,
		  "",
		  "The file, while it is a regular file, is not readable.",
		  "",
		  NULL);
	    err(119, __func__, "extra[%i] is not readable file: %s", i, args[i]);
	    not_reached();
	}

	/*
	 * basename cannot be too long
	 */
	base = base_name(args[i]);
	dbg(DBG_VHIGH, "basename(%s): %s", args[i], base);
	base_len = strlen(base);
	if (base_len == 0) {
	    err(120, __func__, "basename of extra data file: %s has a length of 0", args[i]);
	    not_reached();
	} else if (base_len > MAX_BASENAME_LEN) {
	    fpara(stderr,
		  "",
		  "The basename of an extra file is too long.",
		  "",
		  NULL);
	    err(121, __func__, "basename of extra data file: %s is %ju characters an is > the limit: %ju",
			       args[i], (uintmax_t)base_len, (uintmax_t)MAX_BASENAME_LEN);
	    not_reached();
	}

	/*
	 * basename must use only POSIX portable filename and + chars
	 */
	if (posix_plus_safe(base, false, false, true) == false) {
	    fpara(stderr,
		  "",
		  "The basename of an extra file must match the following regexp:",
		  "",
		  "    ^[0-9A-Za-z][0-9A-Za-z._+-]*$",
		  "",
		  NULL);
	    err(122, __func__, "basename of %s does not match regexp: ^[0-9A-Za-z][0-9A-Za-z._+-]*$",
			       args[i]);
	    not_reached();
	}

	if (!strcasecmp(args[i], README_MD_FILENAME) || !strcasecmp(args[i], INDEX_HTML_FILENAME) ||
	    !strcasecmp(args[i], INVENTORY_HTML_FILENAME) || !strcasecmp(args[i], PROG_FILENAME) ||
	    !strcasecmp(args[i], PROG_ALT_FILENAME) || !strcasecmp(args[i], PROG_ORIG_FILENAME) ||
	    !strcasecmp(args[i], PROG_ORIG_C_FILENAME)) {
	    fpara(stderr,
		"",
		"An extra file cannot be named any of:",
		"",
		"   index.html, inventory.html, README.md, prog.orig.c, prog.orig or prog",
		"",
		NULL);
		err(123, __func__, "filename %s not allowed", args[i]);
	    not_reached();
	}

	/*
	 * form destination path
	 */
	infop->extra_file[i] = base;
	dest_len = submission_dir_len + 1 + base_len + 1;
	errno = 0;		/* pre-clear errno for errp() */
	dest = (char *)malloc(dest_len + 1);
	if (dest == NULL) {
	    errp(124, __func__, "malloc #0 of %ju bytes failed", (uintmax_t)(dest_len + 1));
	    not_reached();
	}
	ret = snprintf(dest, dest_len, "%s/%s", submission_dir, base);
	if (ret <= 0) {
	    errp(125, __func__, "snprintf #0 error: %d", ret);
	    not_reached();
	}
	dbg(DBG_VHIGH, "destination path: %s", dest);

	/*
	 * destination cannot exist
	 */
	if (exists(dest)) {
	    fpara(stderr,
		  "",
		  "extra data files cannot overwrite other files.",
		  "",
		  NULL);
	    err(126, __func__, "for extra file: %s destination already exists: %s", args[i], dest);
	    not_reached();
	}

	/*
	 * copy remarks_md under submission_dir
	 */
	dbg(DBG_HIGH, "about to perform: %s -- %s %s", cp, args[i], dest);
	exit_code = shell_cmd(__func__, false, true, "% -- % %", cp, args[i], dest);
	if (exit_code != 0) {
	    err(128, __func__, "%s -- %s %s failed with exit code: %d",
			       cp, args[i], dest, WEXITSTATUS(exit_code));
	    not_reached();
	}

	/*
	 * free storage
	 */
	if (dest != NULL) {
	    free(dest);
	    dest = NULL;
	}
    }
    infop->extra_file[i] = NULL;
    return;
}


/*
 * yes_or_no - determine if input is yes or no
 *
 * given:
 *      question        - string to prompt for a question
 *
 * returns:
 *      true ==> input is yes in some form,
 *      false ==> input is not yes
 */
static bool
yes_or_no(char const *question)
{
    char *response;		/* response to the question */
    char *p;

    /*
     * firewall
     */
    if (question == NULL) {
	err(129, __func__, "called with NULL question");
	not_reached();
    }

    /*
     * ask the question and obtain the response
     */
    do {
	response = prompt(question, NULL);

	/*
	 * convert response to lower case
	 */
	for (p = response; *p != '\0'; ++p) {
	    if (isascii(*p) && isalpha(*p)) {
		*p = (char)tolower(*p);
	    }
	}

	/*
	 * check for a valid reply
	 *
	 * If -y was used the answer is yes.
	 */
	if (answer_yes || strcmp(response, "y") == 0 || strcmp(response, "yes") == 0) {

	    /*
	     * free storage
	     */
	    if (response != NULL) {
		free(response);
		response = NULL;
	    }

	    /*
	     * return yes
	     */
	    return true;

	} else if (strcmp(response, "n") == 0 || strcmp(response, "no") == 0) {

	    /*
	     * free storage
	     */
	    if (response != NULL) {
		free(response);
		response = NULL;
	    }

	    /*
	     * return no
	     */
	    return false;
	}

	/*
	 * reject response and ask again
	 */
	fpara(stderr,
	      "Please enter either y (yes) or n (no)",
	      NULL);

	/*
	 * free storage
	 */
	if (response != NULL) {
	    free(response);
	    response = NULL;
	}

    } while (response == NULL);

    /*
     * free storage
     */
    if (response != NULL) {
	free(response);
	response = NULL;
    }

    /*
     * should not get here - but assume no if we do
     */
    return false;
}


/*
 * get_title - get the title of the submission
 *
 * Ask the user for an submission title, validate the response
 * and return the allocated title.
 *
 * given:
 *      infop   - pointer to info structure
 *
 * returns:
 *      allocated and validated title
 *
 * This function does not return on error.
 */
static char *
get_title(struct info *infop)
{
    char *title = NULL;		/* submission title to return or NULL */
    size_t len;			/* length of title */
    int ret;			/* libc function return */

    /*
     * firewall
     */
    if (infop == NULL) {
	err(130, __func__, "called with NULL infop");
	not_reached();
    }

    /*
     * inform the user of the title
     */
    if (need_hints) {
	para("A submission title is a short name using the [a-z0-9][a-z0-9_+-]* regex pattern.",
	      "",
	      "If your submission wins, the title might become the directory name of the winning entry,",
	      "although the IOCCC judges might change the title for various reason.",
	      "",
	      "If you have more than one submission to submit, please make your titles unique",
	      "amongst the submissions that you submit to the current IOCCC.",
	      "",
	      NULL);
	errno = 0;		/* pre-clear errno for warnp() */
	ret = fprintf(stderr, "You title must be between 1 and %d ASCII characters long.\n\n", MAX_TITLE_LEN);
	if (ret <= 0) {
	    warnp(__func__, "fprintf #0 error: %d", ret);
	}
    }

    /*
     * ask the question and obtain the response
     */
    do {

	/*
	 * obtain the reply
	 */
	title = prompt("Enter a title for your submission", NULL);

	/*
	 * title cannot be empty
	 */
	len = strlen(title);
	if (len == 0) {

	    /*
	     * reject empty title
	     */
	    fpara(stderr,
		  "",
		  "The title cannot be an empty string.",
		  "",
		  NULL);

	    /*
	     * free storage
	     */
	    if (title != NULL) {
		free(title);
		title = NULL;
	    }
	    continue;

	/*
	 * title cannot be too long
	 */
	} else if (len > MAX_TITLE_LEN) {

	    /*
	     * reject long title
	     */
	    fpara(stderr,
		  "",
		  "That title is too long.",
		  "",
		  NULL);
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "Your title must be between 1 and %d ASCII characters long.\n\n", MAX_TITLE_LEN);
	    if (ret <= 0) {
		warnp(__func__, "fprintf #1 error: %d", ret);
	    }

	    /*
	     * free storage
	     */
	    if (title != NULL) {
		free(title);
		title = NULL;
	    }
	    continue;
	}

	/*
	 * verify that the title has only POSIX portable filename and + chars
	 */
	if (posix_plus_safe(title, true, false, true) == false) {

	    /*
	     * reject invalid chars in title
	     */
	    fpara(stderr,
		  "",
		  "Your title contains invalid characters. A title must match the following regex:",
		  "",
		  "    ^[0-9a-z][0-9a-z._+-]*$",
		  "",
		  "That is, it must start with a lower case letter ASCII [a-z] or digit [0-9]",
		  "followed by zero or more lower case letters ASCII [a-z], digits [0-9],",
		  "- (ASCII dash), + (ASCII plus), or _ (ASCII underscore).",
		  "",
		  NULL);

	    /*
	     * free storage
	     */
	    if (title != NULL) {
		free(title);
		title = NULL;
	    }
	    continue;
	}
    } while (title == NULL);


    /*
     * returned allocated title
     */
    return title;
}


/*
 * get_abstract - get the abstract of the submission
 *
 * Ask the user for a submission abstract, validate the response
 * and return the allocated abstract.
 *
 * given:
 *      infop           - pointer to info structure
 *
 * returns:
 *      allocated and validated abstract
 *
 * This function does not return on error.
 */
static char *
get_abstract(struct info *infop)
{
    char *abstract = NULL;	/* submission abstract to return or NULL */
    size_t len;			/* length of abstract */
    int ret;			/* libc function return */

    /*
     * firewall
     */
    if (infop == NULL) {
	err(131, __func__, "called with NULL infp");
	not_reached();
    }

    /*
     * inform the user of the abstract
     */
    para("",
	 "A submission abstract is 1-line summary of your submission.",
	 "",
	 NULL);

    /*
     * ask the question and obtain the response
     */
    do {

	/*
	 * obtain the reply
	 */
	abstract = prompt("Enter a 1-line abstract of your submission", NULL);

	/*
	 * abstract cannot be empty
	 */
	len = strlen(abstract);
	if (len == 0) {

	    /*
	     * reject empty abstract
	     */
	    fpara(stderr,
		  "",
		  "The abstract cannot be an empty string.",
		  "",
		  NULL);

	    /*
	     * free storage
	     */
	    if (abstract != NULL) {
		free(abstract);
		abstract = NULL;
	    }
	    continue;

	/*
	 * abstract cannot be too long
	 */
	} else if (len > MAX_ABSTRACT_LEN) {

	    /*
	     * reject long abstract
	     */
	    fpara(stderr,
		  "",
		  "That abstract is too long.",
		  "",
		  NULL);
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "Your abstract must be between 1 and %d characters long.\n\n", MAX_ABSTRACT_LEN);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error: %d", ret);
	    }

	    /*
	     * free storage
	     */
	    if (abstract != NULL) {
		free(abstract);
		abstract = NULL;
	    }
	    continue;
	}
    } while (abstract == NULL);

    /*
     * returned allocated abstract
     */
    return abstract;
}


/*
 * get_author_info - obtain information on submission authors
 *
 * given:
 *      author_set      - pointer to array of authors
 *
 * returns:
 *      number of authors
 *
 * This function does not return on error.
 */
static int
get_author_info(struct author **author_set_p)
{
    struct author *author_set = NULL;	/* allocated author set */
    int author_count = -1;		/* number of authors or -1 */
    char *author_count_str = NULL;	/* author count string */
    bool yorn = false;		/* response to a question */
    size_t len;			/* length of reply */
    int ret;			/* libc function return */
    char guard;			/* scanf guard to catch excess amount of input */
    char *def_handle = NULL;	/* default author handle computed from name */
    bool pass = false;		/* true ==> passed test */
    char *p, *last_at;
    int i = 0;
    int j = 0;

    /*
     * firewall
     */
    if (author_set_p == NULL) {
	err(132, __func__, "called with NULL author_set_p");
	not_reached();
    }

    /*
     * keep asking for an author number until we get a valid reply
     */
    do {

	/*
	 * ask for author count
	 */
	author_count_str = prompt("\nEnter the number of authors of this submission", NULL);

	/*
	 * convert author_count_str to number
	 */
	ret = sscanf(author_count_str, "%d%c", &author_count, &guard);
	if (ret != 1 || test_author_count(author_count) == false) {
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "\nThe number of authors must be a number from 1 through %d; please re-enter.\n", MAX_AUTHORS);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error #0 while printing author number range");
	    }
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "If you happen to have more than %d authors, we ask that you pick\n", MAX_AUTHORS);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error #1 while printing author number range");
	    }
	    errno = 0;		/* pre-clear errno for warnp() */
	    ret = fprintf(stderr, "just %d authors and mention the remainder of the authors in the remarks file.\n", MAX_AUTHORS);
	    if (ret <= 0) {
		warnp(__func__, "fprintf error #2 while printing author number range");
	    }
	    author_count = -1;	/* invalidate input */
	}

	/*
	 * free storage
	 */
	if (author_count_str != NULL) {
	    free(author_count_str);
	    author_count_str = NULL;
	}

    } while ((author_count < 1 || author_count > MAX_AUTHORS));

    dbg(DBG_HIGH, "will request information on %d authors", author_count);

    /*
     * allocate the author array
     */
    errno = 0;			/* pre-clear errno for errp() */
    author_set = (struct author *) malloc(sizeof(struct author) * (size_t)author_count);
    if (author_set == NULL) {
	errp(133, __func__, "malloc a struct author array of length: %d failed", author_count);
	not_reached();
    }

    /*
     * pre-zeroize the author array
     */
    memset(author_set, 0, sizeof(struct author) * (size_t)author_count);

    /*
     * inform the user about the author information we need to collect
     */
    if (need_hints) {
	para("",
	     "We will now ask for information about the author(s) of this submission.",
	     "",
	     "Information that you supply, if your submission is selected as a winning entry,",
	     "will be published with your submission.",
	     "",
	     "Except for your name and location/country code, you can opt out of providing it,",
	     "(or if you don't have the thing we are asking for), by just pressing return.",
	     "",
	     "A name is required. If an author wishes to be anonymous, use a pseudonym.",
	     "Keep in mind that if an author wins multiple years, or has won before, you might",
	     "want to be consistent and provide the same name or pseudonym each time.",
	     "",
	     "We will ask for the location/country as a 2 character ISO 3166-1 Alpha-2 code.",
	     "",
	     "If you wish your location/country to be anonymous, use the code: XX",
	     "",
	     "See the following URLs for information on ISO 3166-1 Alpha-2 codes:",
	     "",
	     NULL);
	errno = 0;		/* pre-clear errno for warnp() */
	ret = puts(ISO_3166_1_CODE_URL0);
	if (ret < 0) {
	    warnp(__func__, "puts error printing ISO 3166-1 URL0");
	}
	errno = 0;		/* pre-clear errno for warnp() */
	ret = puts(ISO_3166_1_CODE_URL1);
	if (ret < 0) {
	    warnp(__func__, "puts error printing ISO 3166-1 URL1");
	}
	errno = 0;		/* pre-clear errno for warnp() */
	ret = puts(ISO_3166_1_CODE_URL2);
	if (ret < 0) {
	    warnp(__func__, "puts error printing ISO 3166-1 URL2");
	}
	errno = 0;		/* pre-clear errno for warnp() */
	ret = puts(ISO_3166_1_CODE_URL3);
	if (ret < 0) {
	    warnp(__func__, "puts error printing ISO 3166-1 URL3");
	}
	errno = 0;		/* pre-clear errno for warnp() */
	ret = puts(ISO_3166_1_CODE_URL4);
	if (ret < 0) {
	    warnp(__func__, "puts error printing ISO 3166-1 URL4");
	}
	para("",
	     "We will ask for the author(s) Email address. Press return if you don't want to provide it.",
	     "We will ask for a home URL (starting with http:// or https://). Each author may provide",
	     "up to two URLs. Press return to skip.",
	     "We will ask a Mastodon handle (must start with @), or press return to skip.",
	     "We will ask a GitHub account (must start with @), or press return to skip.",
	     "We will ask for an affiliation (company, school, group) of the author.",
	     "We will ask if you have won the IOCCC before. Your answer will not affect your chances of winning.",
	     "We will ask you for an author handle. You should select the default unless you have won the IOCCC before.",
	     NULL);
    }

    /*
     * collect information on authors
     */
    for (i = 0; i < author_count; ++i) {

	/*
	 * announce author number
	 */
	errno = 0;		/* pre-clear errno for warnp() */
	ret = printf("\nEnter information for author #%d\n\n", i);
	if (ret <= 0) {
	    warnp(__func__, "printf error printing author number");
	}
	author_set[i].author_num = i;

	/*
	 * obtain author name
	 */
	do {

	    /*
	     * prompt for the author name
	     */
	    author_set[i].name = NULL;
	    author_set[i].name = prompt("Enter author name", &len);

	    /*
	     * reject empty author name
	     */
	    if (len <= 0) {

		/*
		 * issue rejection message
		 */
		fpara(stderr,
		      "",
		      "The author name cannot be empty, try again.  If they want to be anonymous, give a pseudonym.",
		      "",
		      NULL);

		/*
		 * free storage
		 */
		if (author_set[i].name != NULL) {
		    free(author_set[i].name);
		    author_set[i].name = NULL;
		}

	    /*
	     * reject if name is too long
	     */
	    } else if (len > MAX_NAME_LEN) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "\nSorry ( tm Canada :-) ), we limit names to %d characters\n\n", MAX_NAME_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while reject name that is too long");
		}

		/*
		 * free storage
		 */
		if (author_set[i].name != NULL) {
		    free(author_set[i].name);
		    author_set[i].name = NULL;
		}

	    /*
	     * reject if author name is a duplicate of a previous author name
	     */
	    } else if (i > 0) {
		for (j=0; j < i; ++j) {
		    if (strcmp(author_set[i].name, author_set[j].name) == 0) {

			/*
			 * issue rejection message
			 */
			errno = 0;		/* pre-clear errno for warnp() */
			ret = fprintf(stderr, "\nauthor #%d name duplicates previous author #%d name", i, j);
			if (ret <= 0) {
			    warnp(__func__, "fprintf error while reject duplicate name");
			}

			/*
			 * free storage
			 */
			if (author_set[i].name != NULL) {
			    free(author_set[i].name);
			    author_set[i].name = NULL;
			}
			break;
		    }
		}
	    }
	} while (author_set[i].name == NULL);
	dbg(DBG_MED, "Author #%d Name %s", i, author_set[i].name);

	/*
	 * obtain author location/country code
	 */
	do {

	    /*
	     * request location/country code
	     */
	    author_set[i].location_code = NULL;
	    author_set[i].location_code = prompt("Enter author 2 character location/country code (XX for anonymous)", &len);
	    dbg(DBG_VHIGH, "location/country code as entered: %s", author_set[i].location_code);

	    /*
	     * inspect code input
	     */
	    if (len != 2 ||
		!isascii(author_set[i].location_code[0]) || !isalpha(author_set[i].location_code[0]) ||
		!isascii(author_set[i].location_code[1]) || !isalpha(author_set[i].location_code[1])) {

		/*
		 * provide more help on location/country codes
		 */
		fpara(stderr,
		      "",
		      "Location/country codes are two letters.",
		      "",
		      "For ISO 3166-1 2 character codes, see: the Alpha-2 code column of:",
		      "",
		      NULL);
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "%s\n\n", ISO_3166_1_CODE_URL0);
		if (ret <= 0) {
		    warnp(__func__, "fprintf while printing ISO 3166-1 CODE URL #0");
		}
		fpara(stderr,
		      "or from these Wikipedia / ISO web pages:",
		      "",
		      NULL);
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "%s\n", ISO_3166_1_CODE_URL1);
		if (ret <= 0) {
		    warnp(__func__, "fprintf while printing ISO 3166-1 CODE URL #1");
		}
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "    %s\n", ISO_3166_1_CODE_URL2);
		if (ret <= 0) {
		    warnp(__func__, "fprintf while printing ISO 3166-1 CODE URL #2");
		}
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "    %s\n\n", ISO_3166_1_CODE_URL3);
		if (ret <= 0) {
		    warnp(__func__, "fprintf while printing ISO 3166-1 CODE URL #3");
		}

		/*
		 * free storage
		 */
		if (author_set[i].location_code != NULL) {
		    free(author_set[i].location_code);
		    author_set[i].location_code = NULL;
		}

		/*
		 * discard this invalid location/country code input
		 */
		author_set[i].location_name = NULL;
		yorn = false;
		continue;

	    }

	    /*
	     * force ASCII alpha code into canonical UPPER CASE code
	     */
	    author_set[i].location_code[0] = (char)toupper(author_set[i].location_code[0]);
	    author_set[i].location_code[1] = (char)toupper(author_set[i].location_code[1]);

	    /*
	     * determine if code is known
	     */
	    if (test_location_code(author_set[i].location_code) == false) {

		/*
		 * provide more help on location/country codes
		 */
		fpara(stderr,
		      "",
		      "That is not a known location/country code.",
		      "",
		      "For ISO 3166-1 2 character codes, see: the Alpha-2 code column of:",
		      "",
		      NULL);
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "    %s\n\n", ISO_3166_1_CODE_URL0);
		if (ret <= 0) {
		    warnp(__func__, "fprintf when printing ISO 3166-1 CODE URL #0");
		}
		fpara(stderr,
		      "or from these Wikipedia / ISO web pages:",
		      "",
		      NULL);
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "    %s\n", ISO_3166_1_CODE_URL1);
		if (ret <= 0) {
		    warnp(__func__, "fprintf when printing ISO 3166-1 CODE URL #1");
		}
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "    %s\n", ISO_3166_1_CODE_URL2);
		if (ret <= 0) {
		    warnp(__func__, "fprintf when printing ISO 3166-1 CODE URL #2");
		}
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "    %s\n\n", ISO_3166_1_CODE_URL3);
		if (ret <= 0) {
		    warnp(__func__, "fprintf when printing ISO 3166-1 CODE URL #3");
		}

		/*
		 * free storage
		 */
		if (author_set[i].location_code != NULL) {
		    free(author_set[i].location_code);
		    author_set[i].location_code = NULL;
		}

		/*
		 * discard this invalid location/country code input
		 */
		author_set[i].location_name = NULL;
		yorn = false;
		continue;
	    }
	    author_set[i].location_name = lookup_location_name(author_set[i].location_code);

	    /*
	     * verify the known location/country code
	     */
	    if (need_confirm && !answer_yes) {
		errno = 0;		/* pre-clear errno for warnp() */
		ret = printf("The location/country code you entered is assigned to: %s\n", author_set[i].location_name);
		if (ret <= 0) {
		    warnp(__func__, "fprintf location/country code assignment");
		}
		yorn = yes_or_no("Is that location/country code correct? [yn]");

		/*
		 * free storage if no (re-enter location/country code)
		 */
		if (!yorn) {
		    if (author_set[i].location_code != NULL) {
			free(author_set[i].location_code);
			author_set[i].location_code = NULL;
		    }
		}
	    } else {
		yorn = true;
	    }
	} while (author_set[i].location_code == NULL || author_set[i].location_name == NULL || !yorn);

	dbg(DBG_MED, "Author #%d location/country: %s (%s)", i, author_set[i].location_code, author_set[i].location_name);

	/*
	 * ask for Email address
	 */
	do {

	    /*
	     * request Email address
	     */
	    author_set[i].email = NULL;
	    author_set[i].email = prompt(need_hints ?
		"Enter author email address, or press return to skip" :
		"Enter author email address", &len);
	    if (len == 0) {
		dbg(DBG_VHIGH, "Email address withheld");
	    } else {
		dbg(DBG_VHIGH, "Email address: %s", author_set[i].email);
	    }

	    /*
	     * sanity check the Email address
	     */
	    pass = test_email(author_set[i].email);
	    if (pass == false) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "\nSorry ( tm Canada :-) ), we limit Email address to %d characters\n", MAX_EMAIL_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing Email address length limit");
		}
		fpara(stderr,
		      "and we require that Email addresses must only a single @ somewhere inside the string.",
		      "",
		      NULL);

		/*
		 * free storage
		 */
		if (author_set[i].email != NULL) {
		    free(author_set[i].email);
		    author_set[i].email = NULL;
		}
		continue;
	    }
	} while (author_set[i].email == NULL);

	dbg(DBG_MED, "Author #%d Email: %s", i, author_set[i].email);

	/*
	 * ask for main URL
	 */
	do {

	    /*
	     * request URL
	     */
	    author_set[i].url = NULL;
	    author_set[i].url =
		prompt(need_hints ?
		    "Enter author home page URL (starting with http:// or https://), or press return to skip" :
		    "Enter author home page URL", &len);
	    if (len == 0) {
		dbg(DBG_VHIGH, "URL withheld");
	    } else {
		dbg(DBG_VHIGH, "URL: %s", author_set[i].url);
	    }

	    /*
	     * reject if too long
	     */
	    if (len > MAX_URL_LEN) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "\nSorry ( tm Canada :-) ), we limit URLs to %d characters\n\n", MAX_URL_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing URL length limit");
		}

		/*
		 * free storage
		 */
		if (author_set[i].url != NULL) {
		    free(author_set[i].url);
		    author_set[i].url = NULL;
		}
		continue;
	    }

	    /*
	     * if it starts with http:// or https:// and has more characters, it is OK
	     */
	    if (len > 0) {
		if (((strncmp(author_set[i].url, "http://", LITLEN("http://")) == 0) &&
		     (author_set[i].url[LITLEN("http://")] != '\0')) ||
		    ((strncmp(author_set[i].url, "https://", LITLEN("https://")) == 0) &&
		     (author_set[i].url[LITLEN("https://")] != '\0'))) {

		    /*
		     * URL appears to in valid form
		     */
		    break;

		/*
		 * reject if it does not start with http:// or https://
		 */
		} else if (len > 0) {

		    /*
		     * issue rejection message
		     */
		    fpara(stderr,
			  "",
			  "URLs must begin with http:// or https:// followed by the rest of the home page URL",
			  "",
			  NULL);

		    /*
		     * free storage
		     */
		    if (author_set[i].url != NULL) {
			free(author_set[i].url);
			author_set[i].url = NULL;
		    }
		    continue;
		}
	    }
	} while (author_set[i].url == NULL);

	dbg(DBG_MED, "Author #%d URL #0: %s", i, author_set[i].url);

	/*
	 * ask for alt URL
	 */
	do {

	    /*
	     * request URL
	     */
	    author_set[i].alt_url = NULL;
	    author_set[i].alt_url =
		prompt(need_hints ?
		    "Enter second URL (starting with http:// or https://), or press return to skip" :
		    "Enter second URL", &len);
	    if (len == 0) {
		dbg(DBG_VHIGH, "URL withheld");
	    } else {
		dbg(DBG_VHIGH, "URL #1: %s", author_set[i].alt_url);
	    }

	    /*
	     * reject if too long
	     */
	    if (len > MAX_URL_LEN) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "\nSorry ( tm Canada :-) ), we limit URLs to %d characters\n\n", MAX_URL_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing URL length limit");
		}

		/*
		 * free storage
		 */
		if (author_set[i].alt_url != NULL) {
		    free(author_set[i].alt_url);
		    author_set[i].alt_url = NULL;
		}
		continue;
	    }

	    /*
	     * if it starts with http:// or https:// and has more characters, it is OK
	     */
	    if (len > 0) {
		if (((strncmp(author_set[i].alt_url, "http://", LITLEN("http://")) == 0) &&
		     (author_set[i].alt_url[LITLEN("http://")] != '\0')) ||
		    ((strncmp(author_set[i].alt_url, "https://", LITLEN("https://")) == 0) &&
		     (author_set[i].alt_url[LITLEN("https://")] != '\0'))) {

		    /*
		     * URL appears to in valid form
		     */
		    break;

		/*
		 * reject if it does not start with http:// or https://
		 */
		} else if (len > 0) {

		    /*
		     * issue rejection message
		     */
		    fpara(stderr,
			  "",
			  "URLs must begin with http:// or https:// followed by the rest of the home page URL",
			  "",
			  NULL);

		    /*
		     * free storage
		     */
		    if (author_set[i].alt_url != NULL) {
			free(author_set[i].alt_url);
			author_set[i].alt_url = NULL;
		    }
		    continue;
		}
	    }
	} while (author_set[i].alt_url == NULL);

	dbg(DBG_MED, "Author #%d alt URL: %s", i, author_set[i].alt_url);


	/*
	 * ask for mastodon handle
	 */
	do {

	    /*
	     * request mastodon handle
	     */
	    author_set[i].mastodon = NULL;
	    author_set[i].mastodon = prompt(need_hints ?
		"Enter author Mastodon handle, starting with @, or press return to skip" :
		"Enter author Mastodon handle", &len);
	    if (len == 0) {
		dbg(DBG_VHIGH, "Mastodon handle not given");
	    } else {
		dbg(DBG_VHIGH, "Mastodon handle: %s", author_set[i].mastodon);
	    }

	    /*
	     * reject if too long
	     */
	    if (len > MAX_MASTODON_LEN) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "\nSorry ( tm Canada :-) ), we limit Mastodon handles to %d "
			"characters, starting with the @\n\n", MAX_MASTODON_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing mastodon handle length limit");
		}

		/*
		 * free storage
		 */
		if (author_set[i].mastodon != NULL) {
		    free(author_set[i].mastodon);
		    author_set[i].mastodon = NULL;
		}
		continue;
	    }

	    /*
	     * reject if no leading @ or there's not a second @
	     */
	    if (len > 0) {
		p = strchr(author_set[i].mastodon, '@');
		last_at = strrchr(author_set[i].mastodon, '@');
		if (author_set[i].mastodon[0] != '@' || p == NULL || last_at == NULL ||
		    last_at == p || count_char(author_set[i].mastodon, '@') != 2 || author_set[i].mastodon[1] == '\0' ||
		    strstr(author_set[i].mastodon, "@@") != NULL || author_set[i].mastodon[len-1] == '@') {

		    /*
		     * issue rejection message
		     */
		    fpara(stderr,
			  "",
			  "Mastodon handles must start with a @, must have one other @ that's not adjacent to",
			  "the first and is not the last character in the handle.",
			  "",
			  NULL);

		    /*
		     * free storage
		     */
		    if (author_set[i].mastodon != NULL) {
			free(author_set[i].mastodon);
			author_set[i].mastodon = NULL;
		    }
		    continue;
		}
	    }
	} while (author_set[i].mastodon == NULL);

	dbg(DBG_MED, "Author #%d mastodon: %s", i, author_set[i].mastodon);

	/*
	 * ask for GitHub account
	 */
	do {

	    /*
	     * request GitHub account
	     */
	    author_set[i].github = NULL;
	    author_set[i].github = prompt(need_hints ?
		"Enter author GitHub account, starting with @, or press return to skip" :
		"Enter author GitHub account", &len);
	    if (len == 0) {
		dbg(DBG_VHIGH, "GitHub account not given");
	    } else {
		dbg(DBG_VHIGH, "GitHub account: %s", author_set[i].github);
	    }

	    /*
	     * reject if too long
	     */
	    if (len > MAX_GITHUB_LEN) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret =
		    fprintf(stderr,
			    "\nSorry ( tm Canada :-) ), we limit GitHub account names to %d characters, starting with the @\n\n",
			    MAX_GITHUB_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing GitHub user length limit");
		}

		/*
		 * free storage
		 */
		if (author_set[i].github != NULL) {
		    free(author_set[i].github);
		    author_set[i].github = NULL;
		}
		continue;
	    }

	    /*
	     * reject if no leading @, or if more than one @
	     */
	    if (len > 0) {
		p = strchr(author_set[i].github, '@');
		if (p == NULL || author_set[i].github[0] != '@' || p != strrchr(author_set[i].github, '@')
		    || author_set[i].github[1] == '\0') {

		    /*
		     * issue rejection message
		     */
		    fpara(stderr,
			  "",
			  "GitHub accounts must start with a @ and have no other @-signs.",
			  "",
			  NULL);

		    /*
		     * free storage
		     */
		    if (author_set[i].github != NULL) {
			free(author_set[i].github);
			author_set[i].github = NULL;
		    }
		    continue;
		}
	    }
	} while (author_set[i].github == NULL);

	dbg(DBG_MED, "Author #%d GitHub: %s", i, author_set[i].github);

	/*
	 * ask for affiliation
	 */
	do {

	    /*
	     * request affiliation
	     */
	    author_set[i].affiliation = NULL;
	    author_set[i].affiliation = prompt(need_hints ?
		"Enter author affiliation, or press return to skip" :
		"Enter author affiliation", &len);
	    if (len == 0) {
		dbg(DBG_VHIGH, "Affiliation not given");
	    } else {
		dbg(DBG_VHIGH, "Affiliation: %s", author_set[i].affiliation);
	    }

	    /*
	     * reject if too long
	     */
	    if (len > MAX_AFFILIATION_LEN) {

		/*
		 * issue rejection message
		 */
		errno = 0;		/* pre-clear errno for warnp() */
		ret =
		    fprintf(stderr, "\nSorry ( tm Canada :-) ), we limit affiliation names to %d characters\n\n",
			    MAX_AFFILIATION_LEN);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing affiliation length limit");
		}

		/*
		 * free storage
		 */
		if (author_set[i].affiliation != NULL) {
		    free(author_set[i].affiliation);
		    author_set[i].affiliation = NULL;
		}
		continue;
	    }
	} while (author_set[i].affiliation == NULL);
	dbg(DBG_MED, "Author #%d affiliation: %s", i, author_set[i].affiliation);

	/*
	 * ask if they are a past IOCCC winning author
	 */
	if (need_hints) {
	    para("",
	        "Please note: your next answer will not affect your chances of winning the IOCCC;",
		"we just need to know if you are a past IOCCC winning author in case you do win.",
		"This will simply help us identify all of your winning entries on the IOCCC website.",
		"",
		NULL);
	}
	author_set[i].past_winning_author = yes_or_no("Are you a past IOCCC winning author? [yn]");

	/*
	 * ask for IOCCC author handle
	 */
	do {

	    /*
	     * past IOCCC winning author extra prompt
	     */
	    if (author_set[i].past_winning_author == true) {

#if defined(IOCCC_WINNER_HANDLE_READY)
		/*
		 * explain to the past IOCCC winning author
		 */
		if (need_hints) {
		    /* TODO: update to show how past IOCCC winning authors can
		     * find their IOCCC winner handles
		     */
		    para("",
		         "As a self-declared past IOCCC winning author, we recommend that you enter your",
			 "IOCCC winner handle instead of just pressing return, unless of course, the",
			 "below mentioned default IOCCC author handle happens to be your IOCCC winner handle. :-)",
			 "",
			 "If you do not know your IOCCC winner handle, then you can find your IOCCC winner handle",
			 "by doing the following:",
			 "",
			 "     TODO: explain how to find an IOCCC winner handle",
			 "",
			 "By entering your IOCCC winner handle, you will help us match up this submission",
			 "on the website should you happen to win (again) with this submission.",
			 NULL);
		}
#endif /* IOCCC_WINNER_HANDLE_READY */
	    }

	    /*
	     * determine the default author handle from the name so we can use it in a prompt
	     */
	    def_handle = default_handle(author_set[i].name);
	    if (def_handle == NULL) {
		err(134, __func__, "default_handle() returned NULL!");
		not_reached();
	    }
	    dbg(DBG_VHIGH, "default IOCCC author handle: <%s>", def_handle);
	    if (need_hints) {
		errno = 0;		/* pre-clear errno for warnp() */
		ret = printf("\nThe default IOCCC author handle for author #%d is:\n\n    %s\n\n", i, def_handle);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing default IOCCC author handle");
		}
	    }

	    /*
	     * request IOCCC author handle
	     */
	    author_set[i].author_handle =
	        prompt("Enter author's IOCCC author handle, or press return to use the default", &len);

	    /*
	     * case: accept default IOCCC author handle
	     */
	    if (len == 0) {
		/* note accepting the default IOCCC author handle */
		dbg(DBG_VHIGH, "will use default IOCCC author handle");
		author_set[i].default_handle = true;

		/* free storage of prompt */
		if (author_set[i].author_handle != NULL) {
		    free(author_set[i].author_handle);
		    author_set[i].author_handle = NULL;
		}

		/* use default IOCCC author handle */
		author_set[i].author_handle = def_handle;

	    /*
	     * case: accept user input
	     */
	    } else {

		/* note using user supplied IOCCC author handle */
		dbg(DBG_VHIGH, "will use IOCCC author handle entered by user");
		author_set[i].default_handle = false;

		/* free storage of unused default IOCCC author handle */
		if (def_handle != NULL) {
		    free(def_handle);
		    def_handle = NULL;
		}
	    }
	    dbg(DBG_VHIGH, "IOCCC author handle: %s", author_set[i].author_handle);

	    /*
	     * reject if handle is invalid
	     */
	    if (test_author_handle(author_set[i].author_handle) == false) {

		/*
		 * issue rejection message
		 */
		fpara(stderr,
		      "",
		      "The IOCCC author handle must match the following regexp:",
		      "",
		      "    ^[0-9a-z][0-9a-z_]*$",
		      "",
		      NULL);
		errno = 0;		/* pre-clear errno for warnp() */
		ret = fprintf(stderr, "\nThe IOCCC author handle is limited to %d characters\n\n", MAX_HANDLE);
		if (ret <= 0) {
		    warnp(__func__, "fprintf error while printing IOCCC author handle length limit");
		}

		/*
		 * free storage
		 */
		if (author_set[i].author_handle != NULL) {
		    free(author_set[i].author_handle);
		    author_set[i].author_handle = NULL;
		}
		continue;
	    }

	    /*
	     * reject if author handle is a duplicate of a previous author handle
	     */
	    if (i > 0) {
		for (j=0; j < i; ++j) {
		    if (strcmp(author_set[i].author_handle, author_set[j].author_handle) == 0) {

			/*
			 * issue rejection message
			 */
			errno = 0;		/* pre-clear errno for warnp() */
			ret = fprintf(stderr, "\nauthor #%d author_handle duplicates previous author #%d author_handle", i, j);
			if (ret <= 0) {
			    warnp(__func__, "fprintf error while reject duplicate author_handle");
			}

			/*
			 * free storage
			 */
			if (author_set[i].author_handle != NULL) {
			    free(author_set[i].author_handle);
			    author_set[i].author_handle = NULL;
			}
			break;
		    }
		}
	    }
	} while (author_set[i].author_handle == NULL);
	dbg(DBG_MED, "Author #%d IOCCC author handle: %s", i, author_set[i].author_handle);

	/*
	 * verify the information for this author
	 */
	errno = 0;	/* pre-clear errno for errp() */
	if (printf("\nPlease verify the information about author #%d\n\n", i) <= 0 ||
	    printf("Name: %s\n", author_set[i].name) <= 0 ||
	    printf("Location/country code: %s (%s)\n", author_set[i].location_code, author_set[i].location_name) <= 0 ||
	    ((author_set[i].email[0] == '\0') ? printf("Email not given\n") :
						printf("Email: %s\n", author_set[i].email)) <= 0 ||
	    ((author_set[i].url[0] == '\0') ? printf("URL not given\n") :
					      printf("URL: %s\n", author_set[i].url)) <= 0 ||
	    ((author_set[i].alt_url[0] == '\0') ? printf("Alt URL not given\n") :
					      printf("Alt URL: %s\n", author_set[i].alt_url)) <= 0 ||
	    ((author_set[i].mastodon[0] == '\0') ? printf("Mastodon handle not given\n") :
						  printf("Mastodon handle: %s\n", author_set[i].mastodon)) <= 0 ||
	    ((author_set[i].github[0] == '\0') ? printf("GitHub username not given\n") :
						 printf("GitHub username: %s\n", author_set[i].github)) <= 0 ||
	    ((author_set[i].affiliation[0] == '\0') ? printf("Affiliation not given\n") :
						      printf("Affiliation: %s\n", author_set[i].affiliation)) <= 0 ||
	    ((author_set[i].past_winning_author == true) ? printf("Author claims to be a past IOCCC winning author\n") :
						   printf("Author claims to not be a past IOCCC winning author\n")) <= 0 ||
	    ((author_set[i].default_handle == true) ? printf("Default IOCCC author handle was accepted\n") :
						      printf("IOCCC author handle was manually entered\n"))  <= 0 ||
	    ((author_set[i].author_handle[0] == '\0') ? printf("IOCCC author handle\n\n") :
						        printf("IOCCC author handle: %s\n\n", author_set[i].author_handle)) <= 0) {
	    errp(135, __func__, "error while printing author #%d information\n", i);
	    not_reached();
	}
	if (need_confirm) {
	    yorn = yes_or_no("Is that author information correct? [yn]");
	    if (!yorn) {
		/*
		 * re-enter author information
		 */
		--i;
		continue;
	    }
	}
    }

    /*
     * store author set
     */
    *author_set_p = author_set;

    /*
     * return the author count
     */
    return author_count;
}


/*
 * verify_submission_dir - ask user to verify the contents of the submission directory
 *
 * given:
 *      submission_dir       - path to submission directory
 *      ls              - path to ls utility
 *
 * This function does not return on error.
 */
static void
verify_submission_dir(char const *submission_dir, char const *ls)
{
    int exit_code;		/* exit code from shell_cmd() */
    bool yorn = false;		/* response to a question */
    FILE *ls_stream;		/* pipe from iocccsize -V */
    char *linep = NULL;		/* allocated line read from iocccsize */
    ssize_t readline_len;	/* readline return length */
    int kdirsize;		/* number of kibibyte (2^10) blocks in submission directory */
    char guard;			/* scanf guard to catch excess amount of input */
    int ret;			/* libc function return */

    /*
     * firewall
     */
    if (submission_dir == NULL || ls == NULL) {
	err(136, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * list the contents of the submission_dir
     */
    para("The following is a listing of the submission directory:",
	 "",
	 NULL);
    errno = 0;		/* pre-clear errno for warnp() */
    ret = printf("    %s\n", submission_dir);
    if (ret <= 0) {
	warnp(__func__, "printf error code: %d", ret);
    }
    para("",
	 "from which the xz tarball will be formed:",
	 "",
	 NULL);
    dbg(DBG_HIGH, "about to perform: cd -- %s && %s -lak .", submission_dir, ls);
    exit_code = shell_cmd(__func__, false, true, "cd -- % && % -lak .", submission_dir, ls);
    if (exit_code != 0) {
	err(137, __func__, "cd -- %s && %s -lak . failed with exit code: %d",
			   submission_dir, ls, WEXITSTATUS(exit_code));
	not_reached();
    }

    /*
     * open pipe to the ls command
     */
    dbg(DBG_HIGH, "about to popen: cd -- %s && %s -lak .", submission_dir, ls);
    ls_stream = pipe_open(__func__, false, true, "cd -- % && % -lak .", submission_dir, ls);
    if (ls_stream == NULL) {
	err(138, __func__, "popen filed for: cd -- %s && %s -lak .", submission_dir, ls);
	not_reached();
    }

    /*
     * read the first line - contains the total kibibyte (2^10) block line
     */
    dbg(DBG_HIGH, "reading first line from popen of ls of submission_dir: %s", submission_dir);
    readline_len = readline(&linep, ls_stream);
    if (readline_len < 0) {
	err(139, __func__, "EOF while reading first line from ls: %s", ls);
	not_reached();
    } else {
	dbg(DBG_HIGH, "ls first line read length: %jd buffer: %s", (intmax_t)readline_len, linep);
    }

    /*
     * parse k-block line from ls
     */
    ret = sscanf(linep, "total %d%c", &kdirsize, &guard);
    if (ret != 1) {
	err(140, __func__, "failed to parse block line from ls: %s", linep);
	not_reached();
    }
    if (kdirsize <= 0) {
	err(141, __func__, "ls k-block value: %d <= 0", kdirsize);
	not_reached();
    }
    dbg(DBG_LOW, "submission directory %s size in kibibyte (1024 byte blocks): %d", submission_dir, kdirsize);

    /*
     * close down pipe
     */
    errno = 0;		/* pre-clear errno for errp() */
    ret = pclose(ls_stream);
    if (ret < 0) {
	warnp(__func__, "pclose error on ls stream");
    }
    ls_stream = NULL;

    /*
     * ask the user to verify the list if not -y
     */
    if (!answer_yes) {
	yorn = yes_or_no("\nIs the above list a correct list of files in your submission? [yn]");
	if (!yorn) {
	    fpara(stderr,
		  "",
		  "We suggest you remove the existing submission directory, and then",
		  "rerun this tool with the correct set of file arguments.",
		  NULL);
	    err(142, __func__, "user rejected listing of submission_dir: %s", submission_dir);
	    not_reached();
	}
    }
    /*
     * free storage
     */
    if (linep != NULL) {
	free(linep);
	linep = NULL;
    }
    return;
}


/*
 * write_info - create the .info.json file
 *
 * Form a simple JSON .info file describing the submission.
 *
 * given:
 *      infop           - pointer to info structure
 *      submission_dir  - path to submission directory
 *      chkentry	- path to chkentry tool
 *      fnamchk		- path to fnamchk tool
 *
 * returns:
 *	true
 *
 * This function does not return on error.
 */
static void
write_info(struct info *infop, char const *submission_dir, char const *chkentry, char const *fnamchk)
{
    struct tm *timeptr;		/* localtime return */
    char *info_path;		/* path to .info.json file */
    size_t info_path_len;	/* length of path to .info.json */
    FILE *info_stream;		/* open write stream to the .info.json file */
    size_t strftime_ret;	/* length of strftime() string without the trailing newline */
    size_t utctime_len;		/* length of utctime string (utctime() + " UTC") */
    int ret;			/* libc function return */
    char **q;			/* extra filename array pointer */
    int exit_code;		/* exit code from shell_cmd() */
    int i;

    /*
     * firewall
     */
    if (infop == NULL || submission_dir == NULL || chkentry == NULL || fnamchk == NULL) {
	err(143, __func__, "called with NULL arg(s)");
	not_reached();
    }
    if (infop->extra_count < 0) {
	warn(__func__, "extra_count %d < 0", infop->extra_count);
    }


    /*
     * fill out time information in the info structure
     */

    /*
     * timestamp epoch
     */
    infop->epoch = TIMESTAMP_EPOCH;
    dbg(DBG_VVHIGH, "infop->epoch: %s", infop->epoch);

    /*
     * reset to UTC time zone
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = setenv("TZ", "UTC", 1);
    if (ret < 0) {
	errp(144, __func__, "cannot set TZ=UTC");
	not_reached();
    }
    errno = 0;			/* pre-clear errno for errp() */
    timeptr = gmtime(&(infop->tstamp));
    if (timeptr == NULL) {
	errp(145, __func__, "gmtime returned NULL");
	not_reached();
    }

    /*
     * allocate ASCII UTC string
     */
    utctime_len = MAX_TIMESTAMP_LEN + 1;    /* + 1 for trailing NUL byte */
    errno = 0;			/* pre-clear errno for errp() */
    infop->utctime = (char *)calloc(utctime_len + 1, sizeof(char)); /* + 1 for paranoia padding */
    if (infop->utctime == NULL) {
	errp(146, __func__, "calloc of %ju bytes failed", (uintmax_t)utctime_len + 1);
	not_reached();
    }

    /*
     * fill out ASCII UTC string for time now
     *
     * The ASCII UTC string is of the form:
     *
     * example:	    Thu Jan 01 00:00:00 1970 UTC
     * format:	    %a  %b  %d %H %M %S %Y
     */
    errno = 0;			/* pre-clear errno for errp() */
    strftime_ret = strftime(infop->utctime, utctime_len, "%a %b %d %H:%M:%S %Y UTC", timeptr);
    if (strftime_ret == 0) {
	errp(147, __func__, "strftime returned 0");
	not_reached();
    }
    dbg(DBG_VHIGH, "infop->utctime: %s", infop->utctime);

    /*
     * open .info.json for writing
     */
    info_path_len = strlen(submission_dir) + 1 + LITLEN(INFO_JSON_FILENAME) + 1;
    errno = 0;			/* pre-clear errno for errp() */
    info_path = (char *)malloc(info_path_len + 1);
    if (info_path == NULL) {
	errp(148, __func__, "malloc of %ju bytes failed", (uintmax_t)info_path_len + 1);
	not_reached();
    }
    errno = 0;			/* pre-clear errno for errp() */
    ret = snprintf(info_path, info_path_len, "%s/%s", submission_dir, INFO_JSON_FILENAME);
    if (ret <= 0) {
	errp(149, __func__, "snprintf #0 error: %d", ret);
	not_reached();
    }
    dbg(DBG_HIGH, ".info.json path: %s", info_path);
    errno = 0;			/* pre-clear errno for errp() */
    info_stream = fopen(info_path, "w");
    if (info_stream == NULL) {
	errp(150, __func__, "failed to open for writing: %s", info_path);
	not_reached();
    }

    /*
     * write leading part of info to the open .info.json file
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fprintf(info_stream, "{\n") > 0 &&
	json_fprintf_value_string(info_stream, "\t", JSON_PARSING_DIRECTIVE_NAME, " : ", JSON_PARSING_DIRECTIVE_VALUE, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "IOCCC_info_version", " : ", INFO_VERSION, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "IOCCC_contest", " : ", IOCCC_CONTEST, ",\n") &&
	json_fprintf_value_long(info_stream, "\t", "IOCCC_year", " : ", (long)IOCCC_YEAR, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "mkiocccentry_version", " : ", MKIOCCCENTRY_VERSION, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "iocccsize_version", " : ", infop->iocccsize_ver, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "chkentry_version", " : ", CHKENTRY_VERSION, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "fnamchk_version", " : ", FNAMCHK_VERSION, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "txzchk_version", " : ", TXZCHK_VERSION, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "IOCCC_contest_id", " : ", infop->ioccc_id, ",\n") &&
	json_fprintf_value_long(info_stream, "\t", "submission_num", " : ", (long)infop->submission_num, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "title", " : ", infop->title, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "abstract", " : ", infop->abstract, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "tarball", " : ", infop->tarball, ",\n") &&
	json_fprintf_value_long(info_stream, "\t", "rule_2a_size", " : ", (long)infop->rule_2a_size, ",\n") &&
	json_fprintf_value_long(info_stream, "\t", "rule_2b_size", " : ", (long)infop->rule_2b_size, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "empty_override", " : ", infop->empty_override, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "rule_2a_override", " : ", infop->rule_2a_override, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "rule_2a_mismatch", " : ", infop->rule_2a_mismatch, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "rule_2b_override", " : ", infop->rule_2b_override, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "highbit_warning", " : ", infop->highbit_warning, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "nul_warning", " : ", infop->nul_warning, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "trigraph_warning", " : ", infop->trigraph_warning, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "wordbuf_warning", " : ", infop->wordbuf_warning, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "ungetc_warning", " : ", infop->ungetc_warning, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "Makefile_override", " : ", infop->Makefile_override, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "first_rule_is_all", " : ", infop->first_rule_is_all, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "found_all_rule", " : ", infop->found_all_rule, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "found_clean_rule", " : ", infop->found_clean_rule, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "found_clobber_rule", " : ", infop->found_clobber_rule, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "found_try_rule", " : ", infop->found_try_rule, ",\n") &&
	json_fprintf_value_bool(info_stream, "\t", "test_mode", " : ", infop->test_mode, ",\n") &&
	fprintf(info_stream, "\t\"manifest\" : [\n") > 0;
    if (!ret) {
	errp(151, __func__, "fprintf error writing leading part of info to %s", info_path);
	not_reached();
    }

    /*
     * write mandatory files to the open .info.json file
     */
    ret = json_fprintf_value_string(info_stream, "\t\t{", "info_JSON", " : ", INFO_JSON_FILENAME, "},\n") &&
	  json_fprintf_value_string(info_stream, "\t\t{", "auth_JSON", " : ", AUTH_JSON_FILENAME, "},\n") &&
	  json_fprintf_value_string(info_stream, "\t\t{", "c_src", " : ", infop->prog_c, "},\n") &&
	  json_fprintf_value_string(info_stream, "\t\t{", "Makefile", " : ", infop->Makefile, "},\n") &&
	  json_fprintf_value_string(info_stream, "\t\t{", "remarks", " : ", infop->remarks_md,
				    ((infop->extra_count > 0) ?  "},\n" : "}\n"));
    if (!ret) {
	errp(152, __func__, "fprintf error writing mandatory filename to %s", info_path);
	not_reached();
    }

    /*
     * write extra files to the open .info.json file
     */
    for (i=0, q=infop->extra_file; i < infop->extra_count && *q != NULL; ++i, ++q) {
        if (json_fprintf_value_string(info_stream, "\t\t{", "extra_file", " : ", *q,
				     (((i+1) < infop->extra_count) ? "},\n" : "}\n")) != true) {
	    errp(153, __func__, "fprintf error writing extra filename[%d] to %s", i, info_path);
	    not_reached();
	}
    }

    /*
     * write trailing part of info to the open .info.json file
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fprintf(info_stream, "\t],\n") > 0 &&
	json_fprintf_value_time_t(info_stream, "\t", "formed_timestamp", " : ", infop->tstamp, ",\n") &&
	json_fprintf_value_long(info_stream, "\t", "formed_timestamp_usec", " : ", (long)infop->usec, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "timestamp_epoch", " : ", TIMESTAMP_EPOCH, ",\n") &&
	json_fprintf_value_long(info_stream, "\t", "min_timestamp", " : ", MIN_TIMESTAMP, ",\n") &&
	json_fprintf_value_string(info_stream, "\t", "formed_UTC", " : ", infop->utctime, "\n") &&
	fprintf(info_stream, "}\n") > 0;
    if (!ret) {
	errp(154, __func__, "fprintf error writing trailing part of info to %s", info_path);
	not_reached();
    }

    /*
     * close the file prior to running chkentry
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fclose(info_stream);
    if (ret < 0) {
	errp(155, __func__, "fclose error");
	not_reached();
    }

    /*
     * verify .info.json
     */
    if (!quiet) {
	para("",
	    "Checking the format of .info.json ...", NULL);
    }
    dbg(DBG_HIGH, "about to perform: %s -q -- %s .", chkentry, info_path);
    exit_code = shell_cmd(__func__, false, true, "% -q -- % .", chkentry, info_path);
    if (exit_code != 0) {
	err(156, __func__, "%s -q -- %s . failed with exit code: %d",
			   chkentry, info_path, WEXITSTATUS(exit_code));
	not_reached();
    }
    if (!quiet) {
	para("... all appears well with the .info.json file.", NULL);
    }

    /*
     * free storage
     */
    if (info_path != NULL) {
	free(info_path);
	info_path = NULL;
    }
    return;
}


/*
 * form_auth - fill out struct auth using common data from struct info plus author structure array
 *
 * given:
 *      authp           - pointer to auth structure to fill out
 *	infop		- pointer to info structure
 *      author_count    - length of the author structure array in elements
 *      authorp         - pointer to author structure array
 */
static void
form_auth(struct auth *authp, struct info *infop, int author_count, struct author *authorp)
{
    /*
     * firewall
     */
    if (authp == NULL || infop == NULL || authorp == NULL) {
	err(157, __func__, "called with NULL arg(s)");
	not_reached();
    }
    if (infop->ioccc_id == NULL) {
	err(158, __func__, "infop->ioccc_id is NULL");
	not_reached();
    }
    if (infop->tarball == NULL) {
	err(159, __func__, "infop->tarball is NULL");
	not_reached();
    }
    if (infop->utctime == NULL) {
	err(160, __func__, "infop->utctime is NULL");
	not_reached();
    }
    memset(authp, 0, sizeof(*authp));

    /*
     * copy over information before the authors array
     */
    /* copy over file format strings as compiled in compiled in constants */
    authp->no_comment = infop->no_comment;
    authp->auth_version = AUTH_VERSION;
    authp->IOCCC_contest = infop->IOCCC_contest;
    /* contest year */
    authp->year = infop->year;
    /* copy over IOCCC tool versions as compiled in compiled in constants */
    authp->mkiocccentry_ver = infop->mkiocccentry_ver;
    authp->chkentry_ver = infop->chkentry_ver;
    authp->fnamchk_ver = infop->fnamchk_ver;
    authp->iocccsize_ver = infop->iocccsize_ver;
    authp->txzchk_ver = infop->txzchk_ver;
    /* copy over submission information */
    errno = 0;			/* pre-clear errno for errp() */
    authp->ioccc_id = strdup(infop->ioccc_id);
    if (authp->ioccc_id == NULL) {
	errp(161, __func__, "strdup() ioccc_id path %s failed", infop->ioccc_id);
	not_reached();
    }
    authp->submission_num = infop->submission_num;
    errno = 0;			/* pre-clear errno for errp() */
    authp->tarball = strdup(infop->tarball);
    if (authp->tarball == NULL) {
	errp(162, __func__, "strdup() tarball path %s failed", infop->tarball);
	not_reached();
    }
    /* copy over test or non-test mode */
    authp->test_mode = infop->test_mode;

    /*
     * setup author set
     */
    authp->author_count = author_count;
    authp->author = authorp;

    /*
     * copy over information after the authors array
     */
    authp->tstamp = infop->tstamp;
    authp->usec = infop->usec;
    authp->epoch = infop->epoch;
    authp->min_stamp = infop->min_stamp;
    errno = 0;			/* pre-clear errno for errp() */
    authp->utctime = strdup(infop->utctime);
    if (authp->utctime == NULL) {
	errp(163, __func__, "strdup() utctime path %s failed", infop->utctime);
	not_reached();
    }
    return;
}


/*
 * write_auth - create the .auth.json file
 *
 * Form a simple JSON .author file describing the submission.
 *
 * given:
 *      authp           - pointer to auth structure
 *      submission_dir  - path to submission directory
 *      chkentry	- path to chkentry tool
 *      fnamchk		- path to fnamchk tool
 *
 * This function does not return on error.
 */
static void
write_auth(struct auth *authp, char const *submission_dir, char const *chkentry, char const *fnamchk)
{
    char *auth_path;		/* path to .auth.json file */
    size_t auth_path_len;	/* length of path to .auth.json */
    FILE *auth_stream;	/* open write stream to the .auth.json file */
    int ret;			/* libc function return */
    int exit_code;		/* exit code from shell_cmd() */
    int i;

    /*
     * firewall
     */
    if (authp == NULL || submission_dir == NULL || chkentry == NULL || fnamchk == NULL) {
	err(164, __func__, "called with NULL arg(s)");
	not_reached();
    }
    if (authp->author_count <= 0) {
	err(165, __func__, "author_count %d <= 0", authp->author_count);
	not_reached();
    } else if (authp->author_count > MAX_AUTHORS) {
	err(166, __func__, "author count %d > max authors %d", authp->author_count, MAX_AUTHORS);
	not_reached();
    }


    /*
     * open .auth.json for writing
     */
    auth_path_len = strlen(submission_dir) + 1 + LITLEN(AUTH_JSON_FILENAME) + 1;
    errno = 0;			/* pre-clear errno for errp() */
    auth_path = (char *)malloc(auth_path_len + 1);
    if (auth_path == NULL) {
	errp(167, __func__, "malloc of %ju bytes failed", (uintmax_t)auth_path_len + 1);
	not_reached();
    }
    errno = 0;			/* pre-clear errno for errp() */
    ret = snprintf(auth_path, auth_path_len, "%s/%s", submission_dir, AUTH_JSON_FILENAME);
    if (ret <= 0) {
	errp(168, __func__, "snprintf #0 error: %d", ret);
	not_reached();
    }
    dbg(DBG_HIGH, ".auth.json path: %s", auth_path);
    errno = 0;			/* pre-clear errno for errp() */
    auth_stream = fopen(auth_path, "w");
    if (auth_stream == NULL) {
	errp(169, __func__, "failed to open for writing: %s", auth_path);
	not_reached();
    }

    /*
     * write leading part of authorship to the open .auth.json file
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fprintf(auth_stream, "{\n") > 0 &&
	json_fprintf_value_string(auth_stream, "\t", JSON_PARSING_DIRECTIVE_NAME, " : ", JSON_PARSING_DIRECTIVE_VALUE, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "IOCCC_auth_version", " : ", AUTH_VERSION, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "IOCCC_contest", " : ", IOCCC_CONTEST, ",\n") &&
	json_fprintf_value_long(auth_stream, "\t", "IOCCC_year", " : ", (long)IOCCC_YEAR, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "mkiocccentry_version", " : ", MKIOCCCENTRY_VERSION, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "chkentry_version", " : ", CHKENTRY_VERSION, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "fnamchk_version", " : ", FNAMCHK_VERSION, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "IOCCC_contest_id", " : ", authp->ioccc_id, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "tarball", " : ", authp->tarball, ",\n") &&
	json_fprintf_value_long(auth_stream, "\t", "submission_num", " : ", (long)authp->submission_num, ",\n") &&
	json_fprintf_value_long(auth_stream, "\t", "author_count", " : ", (long)authp->author_count, ",\n") &&
	json_fprintf_value_bool(auth_stream, "\t", "test_mode", " : ", authp->test_mode, ",\n") &&
	fprintf(auth_stream, "\t\"authors\" : [\n") > 0;
    if (!ret) {
	errp(170, __func__, "fprintf error writing leading part of authorship to %s", auth_path);
	not_reached();
    }

    /*
     * write author info to the open .auth.json file
     */
    for (i = 0; i < authp->author_count; ++i) {
	struct author *ap = &(authp->author[i]);
	errno = 0;		/* pre-clear errno for errp() */
	ret = fprintf(auth_stream, "\t\t{\n") > 0 &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "name", " : ", ap->name, ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "location_code", " : ", ap->location_code, ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "location_name", " : ", ap->location_name, ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "email", " : ", strnull(ap->email), ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "url", " : ", strnull(ap->url), ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "alt_url", " : ", strnull(ap->alt_url), ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "mastodon", " : ", strnull(ap->mastodon), ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "github", " : ", strnull(ap->github), ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "affiliation", " : ", strnull(ap->affiliation), ",\n") &&
	    json_fprintf_value_bool(auth_stream, "\t\t\t", "past_winning_author", " : ", ap->past_winning_author, ",\n") &&
	    json_fprintf_value_bool(auth_stream, "\t\t\t", "default_handle", " : ", ap->default_handle, ",\n") &&
	    json_fprintf_value_string(auth_stream, "\t\t\t", "author_handle", " : ", strnull(ap->author_handle), ",\n") &&
	    json_fprintf_value_long(auth_stream, "\t\t\t", "author_number", " : ", ap->author_num, "\n") &&
	    fprintf(auth_stream, "\t\t}%s\n", (((i + 1) < authp->author_count) ? "," : "")) > 0;
	if (ret == false) {
	    errp(171, __func__, "fprintf error writing author %d info to %s", i, auth_path);
	    not_reached();
	}
    }

    /*
     * write trailing part of authorship to the open .auth.json file
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fprintf(auth_stream, "\t],\n") > 0 &&
	json_fprintf_value_time_t(auth_stream, "\t", "formed_timestamp", " : ", authp->tstamp, ",\n") &&
	json_fprintf_value_long(auth_stream, "\t", "formed_timestamp_usec", " : ", (long)authp->usec, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "timestamp_epoch", " : ", authp->epoch, ",\n") &&
	json_fprintf_value_long(auth_stream, "\t", "min_timestamp", " : ", MIN_TIMESTAMP, ",\n") &&
	json_fprintf_value_string(auth_stream, "\t", "formed_UTC", " : ", authp->utctime, "\n") &&
	fprintf(auth_stream, "}\n") > 0;
    if (!ret) {
	errp(172, __func__, "fprintf error writing trailing part of authorship to %s", auth_path);
	not_reached();
    }

    /*
     * close the file before checking it with chkentry
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fclose(auth_stream);
    if (ret < 0) {
	errp(173, __func__, "fclose error");
	not_reached();
    }

    /*
     * verify .info.json
     */
    if (!quiet) {
	para("",
	    "Checking the format of .auth.json ...", NULL);
    }
    dbg(DBG_HIGH, "about to perform: %s -q -- . %s", chkentry, auth_path);
    exit_code = shell_cmd(__func__, false, true, "% -q -- . %", chkentry, auth_path);
    if (exit_code != 0) {
	err(174, __func__, "%s -q -- . %s failed with exit code: %d",
			   chkentry, auth_path, WEXITSTATUS(exit_code));
	not_reached();
    }
    if (!quiet) {
	para("... all appears well with the .auth.json file.", NULL);
    }

    /*
     * free storage
     */
    if (auth_path != NULL) {
	free(auth_path);
	auth_path = NULL;
    }
    return;
}


/*
 * form_tarball - form the compressed tarball
 *
 * Given the completed submission directory, form a compressed tarball for the user to submit.
 * Remind the user where to submit their compressed tarball file. The function
 * shows the listing of the tarball contents via the txzchk tool and the fnamchk tool.
 *
 * given:
 *      work_dir        - working directory under which the submission directory is formed
 *      submission_dir  - path to submission directory
 *      tarball_path    - path of the compressed tarball to form
 *      tar             - path to the tar utility
 *      ls              - path to ls utility
 *      txzchk		- path to txzchk tool
 *      fnamchk		- path to fnamchk tool
 *
 * This function does not return on error.
 */
static void
form_tarball(char const *work_dir, char const *submission_dir, char const *tarball_path, char const *tar,
	     char const *ls, char const *txzchk, char const *fnamchk)
{
    char *basename_submission_dir;	/* basename of the submission directory */
    char *basename_tarball_path;/* basename of tarball_path */
    int exit_code;		/* exit code from shell_cmd() */
    struct stat buf;		/* stat of the tarball */
    int cwd = -1;		/* current working directory */
    int ret;			/* libc function return */

    /*
     * firewall
     */
    if (work_dir == NULL || submission_dir == NULL || tarball_path == NULL || tar == NULL || ls == NULL ||
        txzchk == NULL || fnamchk == NULL) {
	err(175, __func__, "called with NULL arg(s)");
	not_reached();
    }

    /*
     * verify submission directory contents
     */
    verify_submission_dir(submission_dir, ls);
    dbg(DBG_LOW, "verified submission directory: %s", submission_dir);

    /*
     * note the current directory so we can restore it later, after the chdir(work_dir) below
     */
    errno = 0;			/* pre-clear errno for errp() */
    cwd = open(".", O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    if (cwd < 0) {
	errp(176, __func__, "cannot open .");
	not_reached();
    }

    /*
     * cd into the work_dir, just above the submission_dir and where the compressed tarball will be formed
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = chdir(work_dir);
    if (ret < 0) {
	errp(177, __func__, "cannot cd %s", work_dir);
	not_reached();
    }

    /*
     * perform the tar create command
     *
     * IMPORTANT NOTE: The reason why we form a v7 tarball is that we do NOT want to
     *		       preserve user and group names (to help keep authors anonymous),
     *		       (modern flags to force a username/groupname are not very portable),
     *		       and we don't want special files, symlinks, etc.
     */
    if (!quiet) {
	para("",
	     "About to run the tar command to form the compressed tarball ...",
	     "",
	     NULL);
    } else {
	para("", NULL);
    }

    basename_submission_dir = base_name(submission_dir);
    basename_tarball_path = base_name(tarball_path);
    dbg(DBG_HIGH, "about to perform: %s --format=v7 -cJf %s -- %s",
		   tar, basename_tarball_path, basename_submission_dir);
    exit_code = shell_cmd(__func__, false, true, "% --format=v7 -cJf % -- %",
				    tar, basename_tarball_path, basename_submission_dir);
    if (exit_code != 0) {
	err(178, __func__, "%s --format=v7 -cJf %s -- %s failed with exit code: %d",
			   tar, basename_tarball_path, basename_submission_dir, WEXITSTATUS(exit_code));
	not_reached();
    }

    /*
     * enforce the maximum size of the compressed tarball
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = stat(basename_tarball_path, &buf);
    if (ret != 0) {
	errp(179, __func__, "stat of the compressed tarball failed: %s", basename_tarball_path);
	not_reached();
    }
    if (buf.st_size > MAX_TARBALL_LEN) {
	fpara(stderr,
	      "",
	      "The compressed tarball exceeds the maximum allowed size, sorry.",
	      "",
	      NULL);
	err(180, __func__, "The compressed tarball: %s size: %ju > %jd",
		 basename_tarball_path, (uintmax_t)buf.st_size, (intmax_t)MAX_TARBALL_LEN);
	not_reached();
    }

    /*
     * switch back to the previous current directory
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = fchdir(cwd);
    if (ret < 0) {
	errp(181, __func__, "cannot fchdir to the previous current directory");
	not_reached();
    }
    errno = 0;			/* pre-clear errno for errp() */
    ret = close(cwd);
    if (ret < 0) {
	errp(182, __func__, "close of previous current directory failed");
	not_reached();
    }

    /*
     * perform the txzchk which will indirectly show the user the tarball contents
     */
    dbg(DBG_HIGH, "about to perform: %s -q -F %s -- %s/../%s",
		  txzchk, fnamchk, submission_dir, basename_tarball_path);
    exit_code = shell_cmd(__func__, false, true, "% -q -F % -- %/../%",
					  txzchk, fnamchk, submission_dir, basename_tarball_path);
    if (exit_code != 0) {
	err(183, __func__, "%s -q -F %s -- %s/../%s failed with exit code: %d",
			   txzchk, fnamchk, submission_dir, basename_tarball_path, WEXITSTATUS(exit_code));
	not_reached();
    }
    para("",
	 "... the output above is the listing of the compressed tarball.",
	 "",
	 NULL);

    /*
     * free memory
     */
    if (basename_submission_dir != NULL) {
	free(basename_submission_dir);
	basename_submission_dir = NULL;
    }
    if (basename_tarball_path != NULL) {
	free(basename_tarball_path);
	basename_tarball_path = NULL;
    }
    return;
}


/*
 * remind_user - remind the user to upload (if not in test mode)
 *
 * given:
 *      work_dir        - working directory under which the submission directory is formed
 *      submission_dir  - path to submission directory
 *      tar             - path to the tar utility
 *      tarball_path    - path of the compressed tarball to form
 *      test_mode       - true ==> test mode, do not upload
 *      answers		- path to the answers file (if specified)
 *      infop		- pointer to info structure
 */
static void
remind_user(char const *work_dir, char const *submission_dir, char const *tar, char const *tarball_path, bool test_mode)
{
    int ret;			/* libc function return */
    char *submission_dir_esc;
    char *work_dir_esc;

    /*
     * firewall
     */
    if (work_dir == NULL || submission_dir == NULL || tar == NULL || tarball_path == NULL) {
	err(184, __func__, "called with NULL arg(s)");
	not_reached();
    }

    submission_dir_esc = cmdprintf("%", submission_dir);
    if (submission_dir_esc == NULL) {
	err(185, __func__, "failed to cmdprintf: submission_dir");
	not_reached();
    }

    /*
     * tell user they can now remove submission_dir
     */
    para("Now that we have formed the compressed tarball file, you",
	 "can remove the submission directory we have formed by executing:",
	 "",
	 NULL);
    ret = printf("    rm -rf %s%s\n", submission_dir[0] == '-' ? "-- " : "", submission_dir_esc);
    if (ret <= 0) {
	errp(186, __func__, "printf #0 error");
	not_reached();
    }
    free(submission_dir_esc);

    work_dir_esc = cmdprintf("%", work_dir);
    if (work_dir_esc == NULL) {
	err(187, __func__, "failed to cmdprintf: work_dir");
	not_reached();
    }

    para("",
	 "If you are curious, you may examine the newly created compressed tarball",
	 "by running the following command:",
	 "",
	 NULL);
    ret = printf("    %s -Jtvf %s%s/%s\n", tar, work_dir[0] == '-' ? "./" : "", work_dir_esc, tarball_path);
    if (ret <= 0) {
	errp(188, __func__, "printf #2 error");
	not_reached();
    }
    free(work_dir_esc);

    /*
     * case: test mode
     */
    if (test_mode) {

	/*
	 * remind them that this is a test submission, not an official
	 * submission
	 */
	para("",
	     "As you entered an IOCCC contest ID of 'test', the compressed tarball",
	     "that was just formed CANNOT be used as an IOCCC submission. Please",
	     "do NOT email the Judges your submission!"
	     "", "",
	     NULL);

    /*
     * case: submission mode
     */
    } else {

	/*
	 * inform them of the compressed tarball file
	 */
	show_registration_url();
	show_submit_url(work_dir, tarball_path);
    }
    return;
}


/*
 * show_registration_url - print if the submit server is ready
 *
 * If the submit server is ready we will tell the user how to register as a
 * contestant. If it is NOT ready we will tell them they cannot register as a
 * contestant and implicitly that they may NOT submit to the contest.
 *
 */
static void
show_registration_url(void)
{
    int ret;			/* libc function return */

#if defined (IOCCC_REGISTRATION_READY)

    /*
     * print information about how to register for the IOCCC
     */
    para("",
	 "To submit to the IOCCC, you must be a registered contestant and have received an",
	 "IOCCC contest ID (via email) shortly after you've successfully registered.  To do so,",
	 "please visit:",
	 "",
	 NULL);
    errno = 0;		/* pre-clear errno for warnp() */
    ret = fprintf(stderr, "    %s\n", IOCCC_REGISTER_URL);
    if (ret <= 0) {
	err(189, __func__, "fprintf error printing IOCCC_REGISTER_URL");
	not_reached();
    }

#else /* IOCCC_REGISTRATION_READY */

    /*
     * print that the registration system is not ready
     */
    para("",
         "The IOCCC registration process and submit servers are not ready.  You cannot",
	 "register for the IOCCC nor use the IOCCC submit server at this time.",
	 "Please try again later, and monitor the IOCCC news for updates:",
	 "",
	 NULL);
    errno = 0;		/* pre-clear errno for warnp() */
    ret = fprintf(stderr, "    %s\n\n", IOCCC_NEWS_URL);
    if (ret <= 0) {
	err(190, __func__, "fprintf error printing IOCCC_NEWS_URL");
	not_reached();
    }
    para("",
	 "Sorry, tm Canada :-).",
	 "",
	 NULL);

#endif /* IOCCC_REGISTRATION_READY */
}


/*
 * show_submit_url
 *
 * If the submit server is ready we will tell the user where they may submit
 * their submission if the contest is still open. If the submit server is NOT open
 * we will tell them that the server is not ready and that they CANNOT submit
 * their submission.
 *
 * given:
 *	    work_dir	    - work directory
 *	    tarball_path    - path to the submission tarball
 *
 * NOTE: if either work_dir or tarball_path is NULL we will do nothing in the
 * assumption that the server is NOT ready (though they shouldn't be NULL
 * anyway). Nevertheless we still make sure to check that the server IS ready
 * before printing anything to the user.
 */
static void
show_submit_url(char const *work_dir, char const *tarball_path)
{
#if defined (IOCCC_SUBMIT_SERVER_READY) || defined (IOCCC_REGISTRATION_READY)
    int ret;			/* libc function return */
#endif /* IOCCC_SUBMIT_SERVER_READY || IOCCC_REGISTRATION_READY */

    /*
     * firewall
     */
    if (work_dir == NULL || tarball_path == NULL) {
	return;
    }

#if defined (IOCCC_SUBMIT_SERVER_READY)

    /*
     * print information the tarball that was formed
     */
    para("",
	 "Once you've registered, you may submit the following tarball:",
	 "",
	 NULL);
    ret = printf("    %s/%s\n", work_dir, tarball_path);
    if (ret <= 0) {
	errp(191, __func__, "printf #4 error");
	not_reached();
    }

    /*
     * print the submit server URL
     */
    para("",
	 "If the contest is still open, you may upload the above",
	 "tarball to the following submission URL:",
	 "",
	 NULL);
    ret = printf("    %s\n", IOCCC_SUBMIT_URL);
    if (ret < 0) {
	errp(192, __func__, "printf #5 error");
	not_reached();
    }

#elif defined (IOCCC_REGISTRATION_READY)

    /*
     * print that the submit server not ready for submissions
     */
    para("",
	 "The IOCCC submit server is NOT ready to receive submissions.",
	 "Please try again later, and monitor the IOCCC news for updates:",
	 "",
	 NULL);
    errno = 0;		/* pre-clear errno for warnp() */
    ret = fprintf(stderr, "    %s\n", IOCCC_NEWS_URL);
    if (ret <= 0) {
	err(193, __func__, "fprintf error printing IOCCC_NEWS_URL");
	not_reached();
    }
    para("",
	 "Sorry, tm Canada :-).",
	 "",
	 NULL);

#endif /* IOCCC_REGISTRATION_READY */
}
