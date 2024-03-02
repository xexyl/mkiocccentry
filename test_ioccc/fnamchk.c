/*
 * fnamchk - IOCCC compressed tarball filename sanity check tool
 *
 * "Because even fprintf has a return value worth paying attention to." :-)
 *
 * Copyright (c) 2022 by Landon Curt Noll.  All Rights Reserved.
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
 * "Because everyone hates when someone gets their name wrong." :-)
 *
 * Share and enjoy! :-)
 */


/* special comments for the seqcexit tool */
/* exit code out of numerical order - ignore in sequencing - ooo */
/* exit code change of order - use new value in sequencing - coo */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/*
 * fnamchk - IOCCC compressed tarball filename sanity check tool
 */
#include "fnamchk.h"


/*
 * definitions
 */
#define REQUIRED_ARGS (1)	/* number of required arguments on the command line */

/*
 * usage message
 *
 * Use the usage() function to print the usage_msg([0-9]?)+ strings.
 */
static const char * const usage_msg =
    "usage: %s [-h] [-v level] [-q] [-V] [-E ext] [-t|-u] filepath\n"
    "\n"
    "\t-h\t\t\tprint help message and exit\n"
    "\t-v level\t\tset verbosity level: (def level: %d)\n"
    "\t-q\t\t\tquiet mode: silence msg(), warn(), warnp() if -v 0 (def: not quiet)\n"
    "\t-V\t\t\tprint version string and exit\n"
    "\t-E ext\t\t\tchange extension to test (def: txz)\n"
    "\t-t\t\t\tfilename must match test entry filename\n"
    "\t-u\t\t\tfilename must match real entry filename\n"
    "\t\t\t\t\tNOTE: -t and -u cannot be used together.\n\n"
    "\tfilepath\t\tpath to an IOCCC compressed tarball\n"
    "\n"
    "Exit codes:\n"
    "     0\t\tall OK\n"
    "     2\t\t-h and help string printed or -V and version string printed\n"
    "     3\t\tcommand line error\n"
    "     4\t\t\"entry.test-\" separated token length != %ju\n"
    "     5\t\t\"entry.UUID-\" token length != %ju\n"
    "     >=10\tinternal error\n"
    "fnamchk version: %s";


/*
 * forward declarations
 */
static void usage(int exitcode, char const *prog, char const *str) __attribute__((noreturn));


int
main(int argc, char *argv[])
{
    char const *program = NULL;	/* our name */
    extern char *optarg;	/* option argument */
    extern int optind;		/* argv index of the next arg */
    char *filepath;		/* filepath argument to check */
    char *filename;		/* basename of filepath to check */
    int ret;			/* libc return code */
    char *entry;		/* first '.' separated token - entry */
    char *uuid;			/* first '-' separated token - test or UUID */
    size_t len;			/* UUID length */
    unsigned int a, b, c, d, e, f;	/* parts of the UUID string */
    unsigned int version = 0;	/* UUID version hex character */
    unsigned int variant = 0;	/* UUID variant hex character */
    char guard;			/* scanf guard to catch excess amount of input */
    int submission_num;		/* third .-separated token as a number */
    char *timestamp_str;	/* fourth .-separated token - timestamp */
    intmax_t timestamp;		/* fifth .-separated token as a timestamp */
    char *extension;		/* sixth .-separated token as a filename extension */
    char *ext = "txz";		/* user supplied extension (def: txz): used for testing purposes only */
    int i;
    bool test_mode = false;	/* true ==> force check to test if it's a test entry filename */
    bool uuid_mode = false;	/* true ==> force check to test if it's a UUID entry filename */
    char *saveptr = NULL;	/* for strtok_r() */


    /*
     * parse args
     */
    program = argv[0];
    while ((i = getopt(argc, argv, ":hv:qVtquE:")) != -1) {
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
	case 'q':
	    msg_warn_silent = true;
	    break;
	case 'V':		/* -V - print version and exit 2 */
	    print("%s\n", FNAMCHK_VERSION);
	    exit(2); /*ooo*/
	    not_reached();
	    break;
	case 't': /* force check to verify that the filename is a test entry filename */
	    test_mode = true;
	    break;
	case 'u': /* force check to verify that the filename is a UUID (real) entry filename */
	    uuid_mode = true;
	    break;
	case 'E': /* force extension check to be optarg instead of "txz": used for txzchk test suite */
	    ext = optarg;
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
    /* must have the exact required number of args */
    if (argc - optind != REQUIRED_ARGS) {
	usage(3, program, "wrong number of arguments"); /*ooo*/
	not_reached();
    }

    /* -t and -u cannot be used together as the tests they enable conflict */
    if (test_mode && uuid_mode) {
	err(3, __func__, "-t and -u cannot be used together"); /*ooo*/
	not_reached();
    }

    /* collect arg */
    filepath = argv[optind];
    dbg(DBG_LOW, "filepath: %s", filepath);

    /*
     * obtain the basename of the path to examine
     */
    filename = base_name(filepath);
    dbg(DBG_LOW, "filename: %s", filename);

    /*
     * first '.' separated token must be entry
     */
    entry = strtok_r(filename, ".", &saveptr);
    if (entry == NULL) {
	err(10, __func__, "first strtok_r() returned NULL");
	not_reached();
    }
    if (strcmp(entry, "entry") != 0) {
	err(11, __func__, "filename does not start with \"entry.\": %s", filepath);
	not_reached();
    }
    dbg(DBG_LOW, "filename starts with \"entry.\": %s", filename);

    /*
     * second '.' separated token must be test or a UUID
     */
    uuid = strtok_r(NULL, ".", &saveptr);
    if (uuid == NULL) {
	err(12, __func__, "nothing found after \"entry.\"");
	not_reached();
    }
    len = strlen(uuid);

    /*
     * parse a test-submission_num IOCCC contest ID
     */
    if (strncmp(uuid, "test-", LITLEN("test-")) == 0) {
	/* if it starts as "test-" and -u was specified it's an error */
	if (uuid_mode) {
	    err(13, __func__, "-u specified and entry starts as a test mode filename");
	    not_reached();
	}

	/*
	 * NOTE: we prevent seqcexit from modifying the exit code because the
	 * txzchk_test.sh script has a test file where it expects this error.
	 */
	if (len != LITLEN("test-")+MAX_SUBMISSION_CHARS) {
	    err(4, __func__, "\"entry.test-\" separated token length: %ju != %ju: %s", /*ooo*/
			     (uintmax_t)len, (uintmax_t)(LITLEN("test-")+MAX_SUBMISSION_CHARS), filepath);
	    not_reached();
	}
	ret = sscanf(uuid, "test-%d%c", &submission_num, &guard);
	if (ret != 1) {
	    err(14, __func__, "submission_number not found after \"test-\": %s", filepath);
	    not_reached();
	}
	dbg(DBG_LOW, "entry ID is test: %s", uuid);
	if (submission_num < 0) {
	    err(15, __func__, "submission_number %d is < 0: %s", submission_num, filepath);
	    not_reached();
	}
	if (submission_num > MAX_SUBMISSION_NUM) {
	    err(16, __func__, "submission_number %d is > %d: %s", submission_num, MAX_SUBMISSION_NUM, filepath);
	    not_reached();
	}
	dbg(DBG_LOW, "submission_number %d is valid: %s", submission_num, filepath);

    /*
     * parse a UUID-submission_num IOCCC contest ID
     */
    } else {
	/*
	 * if -t is specified and we get here (filename does not start with
	 * "entry.test-") then it's an error.
	 */
	if (test_mode) {
	    err(17, __func__, "-t specified and entry does not start with \"entry.test-\"");
	    not_reached();
	}

	/*
	 * NOTE: we prevent seqcexit from modifying the exit code because the
	 * txzchk_test.sh script has a test file where it expects this error.
	 */
	if (len != UUID_LEN+1+MAX_SUBMISSION_CHARS) {
	    err(5, __func__, "\"entry.UUID-\" separated token length: %ju != %ju: %s", /*ooo*/
			     (uintmax_t)len, (uintmax_t)(UUID_LEN+1+MAX_SUBMISSION_CHARS), filepath); /*ooo*/
	    not_reached();
	}
	ret = sscanf(uuid, "%8x-%4x-%1x%3x-%1x%3x-%8x%4x-%d%c", &a, &b, &version, &c, &variant,
		&d, &e, &f, &submission_num, &guard);
	if (ret != 9) {
	    err(18, __func__, "UUID-submission_number not found after \"entry-\": %s", filepath);
	    not_reached();
	}
	if (version != UUID_VERSION) {
	    err(19, __func__, "UUID token version %x != %x: %s", version, UUID_VERSION, filepath);
	    not_reached();
	}
	if (variant != UUID_VARIANT) {
	    err(20, __func__, "UUID token variant %x != %x: %s", variant, UUID_VARIANT, filepath);
	    not_reached();
	}
	dbg(DBG_LOW, "entry ID is a valid UUID: %s", uuid);
	if (submission_num < 0) {
	    err(21, __func__, "submission_number %d is < 0: %s", submission_num, filepath);
	    not_reached();
	}
	if (submission_num > MAX_SUBMISSION_NUM) {
	    err(22, __func__, "submission_number %d is > %d: %s", submission_num, MAX_SUBMISSION_NUM, filepath);
	    not_reached();
	}
	dbg(DBG_LOW, "entry number is valid: %d", submission_num);
    }

    /*
     * third '.' separated token must be a valid timestamp
     */
    timestamp_str = strtok_r(NULL, ".", &saveptr);
    if (timestamp_str == NULL) {
	err(23, __func__, "nothing found after second '.' separated token of entry number");
	not_reached();
    }
    ret = sscanf(timestamp_str, "%jd%c", &timestamp, &guard);
    if (ret != 1) {
	err(24, __func__, "timestamp not found after \"submission_number.\": %s is not a timestamp: %s", timestamp_str, filepath);
	not_reached();
    }
    if (timestamp < MIN_TIMESTAMP) {
	err(25, __func__, "timestamp: %jd is < %jd: %s", timestamp, (intmax_t)MIN_TIMESTAMP, filepath);
	not_reached();
    }
    dbg(DBG_LOW, "timestamp is valid: %jd", timestamp);

    /*
     * fourth .-separated token must be the filename extension
     */
    extension = strtok_r(NULL, ".", &saveptr);
    if (extension == NULL) {
	err(26, __func__, "nothing found after third '.' separated token of timestamp");
	not_reached();
    }
    if (strcmp(extension, ext) != 0) {
	err(27, __func__, "extension %s != %s: %s", extension, ext, filepath);
	not_reached();
    }
    dbg(DBG_LOW, "filename extension is valid: %s", extension);

    /*
     * filepath must use only POSIX portable filename and + chars /
     */
    if (posix_plus_safe(filepath, false, true, false) == false) {
	err(28, __func__, "filepath: posix_plus_safe(%s, false, true, false) is false", filepath);
	not_reached();
    }

    /*
     * filename must use only lower case POSIX portable filename and + chars
     */
    if (posix_plus_safe(filename, true, false, true) == false) {
	err(29, __func__, "basename: posix_plus_safe(%s, true, false, true) is false", filename);
	not_reached();
    }

    /*
     * All is OK with the filepath - print entry directory basename
     */
    dbg(DBG_LOW, "filepath passes all checks: %s", filepath);
    errno = 0;		/* pre-clear errno for errp() */
    ret = printf("%s\n", uuid);
    if (ret <= 0) {
	errp(30, __func__, "printf of entry directory basename failed");
	not_reached();
    }

    /*
     * free filename allocated from base_name()
     */
    if (filename != NULL) {
	free(filename);
	filename = NULL;
    }

    /*
     * All Done!!! - Jessica Noll, age 2
     *
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
 *	prog		our program name
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

    fprintf_usage(exitcode, stderr, usage_msg, prog, DBG_DEFAULT, (uintmax_t)(UUID_LEN+1+MAX_SUBMISSION_CHARS),
	    (uintmax_t)(LITLEN("test-")+MAX_SUBMISSION_CHARS), FNAMCHK_VERSION);
    exit(exitcode); /*ooo*/
    not_reached();
}
