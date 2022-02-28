/* vim: set tabstop=8 softtabstop=4 shiftwidth=4 noexpandtab : */
/*
 * json - JSON functions supporting mkiocccentry code
 *
 * JSON related functions to support formation of .info.json files
 * and .author.json files their related check tools, test code,
 * and string encoding/decoding tools.
 *
 * "Because JSON embodies a commitment to original design flaws." :-)
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
 * Share and enjoy! :-)
 */


#if !defined(INCLUDE_JSON_H)
#    define  INCLUDE_JSON_H


#include <time.h>
#include <stdint.h>


/*
 * byte as octet constants
 */
#define BITS_IN_BYTE (8)	    /* assume 8 bit bytes */
#define MAX_BYTE (0xff)		    /* maximum byte value */
#define BYTE_VALUES (MAX_BYTE+1)    /* number of different combinations of bytes */

/*
 * the next five are for the json_filename() function
 */
#define INFO_JSON	(0)	    /* file is assumed to be a .info.json file */
#define AUTHOR_JSON	(1)	    /* file is assumed to be a .author.json file */
#define INFO_JSON_FILENAME ".info.json"
#define AUTHOR_JSON_FILENAME ".author.json"
#define INVALID_JSON_FILENAME "null"

#define FORMED_UTC_FMT "%a %b %d %H:%M:%S %Y UTC"   /* format for strptime() of formed_UTC */

/*
 * JSON value: a linked list of all values of the same json_field (below)
 */
struct json_value
{
    char *value;

    struct json_value *next;
};

#define JSON_NUMBER	    (0)	    /* json field is supposed to be a number */
#define JSON_BOOL	    (1)	    /* json field is supposed to be a boolean */
#define JSON_STRING	    (2)	    /* json field is supposed to be a string */
#define JSON_ARRAY	    (3)	    /* json field is supposed to be an array */
#define JSON_ARRAY_NUMBER   (5)	    /* json field is supposed to be a number in an array */
#define JSON_ARRAY_BOOL	    (6)	    /* json field is supposed to be a number in an array (NB: not used) */
#define JSON_ARRAY_STRING   (7)	    /* json field is supposed to be a string in an array */
#define JSON_NULL	    (-1)    /* json field is NULL (not null): used internally to mark end of the tables */

/*
 * JSON field: a JSON field consists of the name and all the values (if more
 * than one field of the same name is found in the file).
 */
struct json_field
{
    char *name;			/* field name */
    struct json_value *values;	/* linked list of values */

    /*
     * the below are for the tables: common_json_fields, info_json_fields and
     * author_json_fields:
     *
     * Number of times this field has been seen in the list, how many are
     * actually allowed and whether the field has been found: This is for the
     * tables that say tell many times a field has been seen, how many times it
     * is allowed and whether or not it's been seen (it is true that this could
     * simply be count > 0 but this is to be more clear, however slight).
     *
     * In other words this is done as part of the checks after the field:value
     * pairs have been extracted.
     *
     * NOTE: A max_count == 0 means that there's no limit.
     */
    size_t count;		/* how many of this field in the list (or how many values) */
    size_t max_count;		/* how many of this field is allowed */
    bool found;			/* if this field was found */

    /*
     * Data type: one of JSON_NUMBER, JSON_BOOL, JSON_STRING or
     * JSON_ARRAY_ equivalents.
     */
    int field_type;

    struct json_field *next;	/* the next in the whatever list (XXX don't add to more than one list!) */
};

extern struct json_field common_json_fields[];

/*
 * linked list of the common json fields found in the .info.json and
 * .author.json files.
 *
 * A common json field is a field that is supposed to be in both .info.json and
 * .author.json.
 */
struct json_field *found_common_json_fields;

/*
 * common json fields - for use in mkiocccentry.
 *
 * NOTE: We don't use the json_field or json_value fields because this struct is
 * for mkiocccentry which is in control of what's written whereas for jinfochk
 * and jauthchk we don't have control of what's in the file.
 */
struct json_common
{
    /*
     * version
     */
    char *mkiocccentry_ver;	/* mkiocccentry version (MKIOCCCENTRY_VERSION) */
    char const *iocccsize_ver;	/* iocccsize version (compiled in, same as iocccsize -V) */
    /*
     * entry
     */
    char *ioccc_id;		/* IOCCC contest ID */
    int entry_num;		/* IOCCC entry number */
    char *tarball;		/* tarball filename */
    bool test_mode;		/* true ==> test mode entered */

    /*
     * time
     */
    time_t tstamp;		/* seconds since epoch when .info json was formed (see gettimeofday(2)) */
    int usec;			/* microseconds since the tstamp second */
    char *epoch;		/* epoch of tstamp, currently: Thu Jan 1 00:00:00 1970 UTC */
    char *utctime;		/* UTC converted string for tstamp (see strftime(3)) */
};

/*
 * author info
 */
struct author {
    char *name;			/* name of the author */
    char *location_code;	/* author location/country code */
    char const *location_name;	/* name of author location/country */
    char *email;		/* Email address of author or or empty string ==> not provided */
    char *url;			/* home URL of author or or empty string ==> not provided */
    char *twitter;		/* author twitter handle or or empty string ==> not provided */
    char *github;		/* author GitHub username or or empty string ==> not provided */
    char *affiliation;		/* author affiliation or or empty string ==> not provided */
    char *winner_handle;	/* previous IOCCC winner handle, or empty string ==> not provided */
    int author_num;		/* author number */

    struct json_common common;	/* fields that are common to this struct author and struct info (below) */
};

/*
 * info for JSON
 *
 * Information we will collect in order to form the .info json file.
 */
struct info {
    /*
     * entry
     */
    char *title;		/* entry title */
    char *abstract;		/* entry abstract */
    off_t rule_2a_size;		/* Rule 2a size of prog.c */
    size_t rule_2b_size;	/* Rule 2b size of prog.c */
    bool empty_override;	/* true ==> empty prog.c override requested */
    bool rule_2a_override;	/* true ==> Rule 2a override requested */
    bool rule_2a_mismatch;	/* true ==> file size != rule_count function size */
    bool rule_2b_override;	/* true ==> Rule 2b override requested */
    bool highbit_warning;	/* true ==> high bit character(s) detected */
    bool nul_warning;		/* true ==> NUL character(s) detected */
    bool trigraph_warning;	/* true ==> unknown or invalid tri-graph(s) detected */
    bool wordbuf_warning;	/* true ==> word buffer overflow detected */
    bool ungetc_warning;	/* true ==> ungetc warning detected */
    bool Makefile_override;	/* true ==> Makefile rule override requested */
    bool first_rule_is_all;	/* true ==> Makefile first rule is all */
    bool found_all_rule;	/* true ==> Makefile has an all rule */
    bool found_clean_rule;	/* true ==> Makefile has clean rule */
    bool found_clobber_rule;	/* true ==> Makefile has a clobber rule */
    bool found_try_rule;	/* true ==> Makefile has a try rule */
    bool test_mode;		/* true ==> contest ID is test */
    /*
     * filenames
     */
    char *prog_c;		/* prog.c filename */
    char *Makefile;		/* Makefile filename */
    char *remarks_md;		/* remarks.md filename */
    int extra_count;		/* number of extra files */
    char **extra_file;		/* list of extra filenames followed by NULL */
    /*
     * time
     */

    struct json_common common;	/* fields that are common to this struct info and struct author (above) */
};


/*
 * external function declarations
 */
extern char *malloc_json_encode(char const *ptr, size_t len, size_t *retlen);
extern char *malloc_json_encode_str(char const *str, size_t *retlen);
extern void jencchk(void);
extern bool json_putc(uint8_t const c, FILE *stream);
extern char *malloc_json_decode(char const *ptr, size_t len, size_t *retlen, bool strict);
extern char *malloc_json_decode_str(char const *str, size_t *retlen, bool strict);
/* jinfochk and jauthchk related */
extern struct json_field *find_json_field_in_table(struct json_field *table, char const *name, size_t *loc);
extern char const *json_filename(int type);
extern int check_first_json_char(char const *file, char *data, bool strict, char **first);
extern int check_last_json_char(char const *file, char *data, bool strict, char **last);
extern struct json_field *add_found_common_json_field(char const *name, char const *val);
extern void check_common_json_fields_table(void);
extern int get_common_json_field(char const *program, char const *file, char *name, char *val);
extern int check_found_common_json_fields(char const *program, char const *file, char const *fnamchk, bool test);
extern struct json_field *new_json_field(char const *name, char const *val);
extern struct json_value *add_json_value(struct json_field *field, char const *val);
/* free() functions */
extern void free_json_field_values(struct json_field *field);
extern void free_found_common_json_fields(void);
extern void free_json_field(struct json_field *field);
/* these free() functions are also used in mkiocccentry.c */
extern void free_info(struct info *infop);
extern void free_author_array(struct author *authorp, int author_count);


#endif /* INCLUDE_JSON_H */
