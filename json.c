/* vim: set tabstop=8 softtabstop=4 shiftwidth=4 noexpandtab : */
/*
 * json - JSON parse tree supporting functions
 *
 * JSON related functions to support formation of .info.json files
 * and .author.json files, their related check tools, test code,
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


/* special comments for the seqcexit tool */
/*ooo*/ /* exit code out of numerical order - ignore in sequencing */
/*coo*/ /* exit code change of order - use new value in sequencing */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include <inttypes.h>
#include <math.h>

/*
 * dbg - debug, warning and error reporting facility
 */
#include "dbg.h"

/*
 * util - utility functions
 */
#include "util.h"

/*
 * IOCCC limits
 */
#include "limit_ioccc.h"

/*
 * JSON - JSON structures and functions
 */
#include "json.h"


/*
 * JSON encoding of an octet in a JSON string
 *
 * XXX - this table assumes we process on a byte basis - XXX
 * XXX - consider an approach that allowed for smaller UTF-8 non-ASCII encoding - XXX
 */
struct encode jenc[BYTE_VALUES] = {
    /* \x00 - \x0f */
    {0x00, 6, "\\u0000"}, {0x01, 6, "\\u0001"}, {0x02, 6, "\\u0002"}, {0x03, 6, "\\u0003"},
    {0x04, 6, "\\u0004"}, {0x05, 6, "\\u0005"}, {0x06, 6, "\\u0006"}, {0x07, 6, "\\u0007"},
    {0x08, 2, "\\b"}, {0x09, 2, "\\t"}, {0x0a, 2, "\\n"}, {0x0b, 6, "\\u000b"},
    {0x0c, 2, "\\f"}, {0x0d, 2, "\\r"}, {0x0e, 6, "\\u000e"}, {0x0f, 6, "\\u000f"},

    /* \x10 - \x1f */
    {0x10, 6, "\\u0010"}, {0x11, 6, "\\u0011"}, {0x12, 6, "\\u0012"}, {0x13, 6, "\\u0013"},
    {0x14, 6, "\\u0014"}, {0x15, 6, "\\u0015"}, {0x16, 6, "\\u0016"}, {0x17, 6, "\\u0017"},
    {0x18, 6, "\\u0018"}, {0x19, 6, "\\u0019"}, {0x1a, 6, "\\u001a"}, {0x1b, 6, "\\u001b"},
    {0x1c, 6, "\\u001c"}, {0x1d, 6, "\\u001d"}, {0x1e, 6, "\\u001e"}, {0x1f, 6, "\\u001f"},

    /* \x20 - \x2f */
    {' ', 1, " "}, {'!', 1, "!"}, {'"', 2, "\\\""}, {'#', 1, "#"},
    {'$', 1, "$"}, {'%', 1, "%"}, {'&', 6, "\\u0026"}, {'\'', 1, "'"},
    {'(', 1, "("}, {')', 1, ")"}, {'*', 1, "*"}, {'+', 1, "+"},
    {',', 1, ","}, {'-', 1, "-"}, {'.', 1, "."}, {'/', 2, "\\/"},

    /* \x30 - \x3f */
    {'0', 1, "0"}, {'1', 1, "1"}, {'2', 1, "2"}, {'3', 1, "3"},
    {'4', 1, "4"}, {'5', 1, "5"}, {'6', 1, "6"}, {'7', 1, "7"},
    {'8', 1, "8"}, {'9', 1, "9"}, {':', 1, ":"}, {';', 1, ";"},
    {'<', 6, "\\u003c"}, {'=', 1, "="}, {'>', 6, "\\u003e"}, {'?', 1, "?"},

    /* \x40 - \x4f */
    {'@', 1, "@"}, {'A', 1, "A"}, {'B', 1, "B"}, {'C', 1, "C"},
    {'D', 1, "D"}, {'E', 1, "E"}, {'F', 1, "F"}, {'G', 1, "G"},
    {'H', 1, "H"}, {'I', 1, "I"}, {'J', 1, "J"}, {'K', 1, "K"},
    {'L', 1, "L"}, {'M', 1, "M"}, {'N', 1, "N"}, {'O', 1, "O"},

    /* \x50 - \x5f */
    {'P', 1, "P"}, {'Q', 1, "Q"}, {'R', 1, "R"}, {'S', 1, "S"},
    {'T', 1, "T"}, {'U', 1, "U"}, {'V', 1, "V"}, {'W', 1, "W"},
    {'X', 1, "X"}, {'Y', 1, "Y"}, {'Z', 1, "Z"}, {'[', 1, "["},
    {'\\', 2, "\\\\"}, {']', 1, "]"}, {'^', 1, "^"}, {'_', 1, "_"},

    /* \x60 - \x6f */
    {'`', 1, "`"}, {'a', 1, "a"}, {'b', 1, "b"}, {'c', 1, "c"},
    {'d', 1, "d"}, {'e', 1, "e"}, {'f', 1, "f"}, {'g', 1, "g"},
    {'h', 1, "h"}, {'i', 1, "i"}, {'j', 1, "j"}, {'k', 1, "k"},
    {'l', 1, "l"}, {'m', 1, "m"}, {'n', 1, "n"}, {'o', 1, "o"},

    /* \x70 - \x7f */
    {'p', 1, "p"}, {'q', 1, "q"}, {'r', 1, "r"}, {'s', 1, "s"},
    {'t', 1, "t"}, {'u', 1, "u"}, {'v', 1, "v"}, {'w', 1, "w"},
    {'x', 1, "x"}, {'y', 1, "y"}, {'z', 1, "z"}, {'{', 1, "{"},
    {'|', 1, "|"}, {'}', 1, "}"}, {'~', 1, "~"}, {0x7f, 6, "\\u007f"},

    /* \x80 - \x8f */
    {0x80, 6, "\\u0080"}, {0x81, 6, "\\u0081"}, {0x82, 6, "\\u0082"}, {0x83, 6, "\\u0083"},
    {0x84, 6, "\\u0084"}, {0x85, 6, "\\u0085"}, {0x86, 6, "\\u0086"}, {0x87, 6, "\\u0087"},
    {0x88, 6, "\\u0088"}, {0x89, 6, "\\u0089"}, {0x8a, 6, "\\u008a"}, {0x8b, 6, "\\u008b"},
    {0x8c, 6, "\\u008c"}, {0x8d, 6, "\\u008d"}, {0x8e, 6, "\\u008e"}, {0x8f, 6, "\\u008f"},

    /* \x90 - \x9f */
    {0x90, 6, "\\u0090"}, {0x91, 6, "\\u0091"}, {0x92, 6, "\\u0092"}, {0x93, 6, "\\u0093"},
    {0x94, 6, "\\u0094"}, {0x95, 6, "\\u0095"}, {0x96, 6, "\\u0096"}, {0x97, 6, "\\u0097"},
    {0x98, 6, "\\u0098"}, {0x99, 6, "\\u0099"}, {0x9a, 6, "\\u009a"}, {0x9b, 6, "\\u009b"},
    {0x9c, 6, "\\u009c"}, {0x9d, 6, "\\u009d"}, {0x9e, 6, "\\u009e"}, {0x9f, 6, "\\u009f"},

    /* \xa0 - \xaf */
    {0xa0, 6, "\\u00a0"}, {0xa1, 6, "\\u00a1"}, {0xa2, 6, "\\u00a2"}, {0xa3, 6, "\\u00a3"},
    {0xa4, 6, "\\u00a4"}, {0xa5, 6, "\\u00a5"}, {0xa6, 6, "\\u00a6"}, {0xa7, 6, "\\u00a7"},
    {0xa8, 6, "\\u00a8"}, {0xa9, 6, "\\u00a9"}, {0xaa, 6, "\\u00aa"}, {0xab, 6, "\\u00ab"},
    {0xac, 6, "\\u00ac"}, {0xad, 6, "\\u00ad"}, {0xae, 6, "\\u00ae"}, {0xaf, 6, "\\u00af"},

    /* \xb0 - \xbf */
    {0xb0, 6, "\\u00b0"}, {0xb1, 6, "\\u00b1"}, {0xb2, 6, "\\u00b2"}, {0xb3, 6, "\\u00b3"},
    {0xb4, 6, "\\u00b4"}, {0xb5, 6, "\\u00b5"}, {0xb6, 6, "\\u00b6"}, {0xb7, 6, "\\u00b7"},
    {0xb8, 6, "\\u00b8"}, {0xb9, 6, "\\u00b9"}, {0xba, 6, "\\u00ba"}, {0xbb, 6, "\\u00bb"},
    {0xbc, 6, "\\u00bc"}, {0xbd, 6, "\\u00bd"}, {0xbe, 6, "\\u00be"}, {0xbf, 6, "\\u00bf"},

    /* \xc0 - \xcf */
    {0xc0, 6, "\\u00c0"}, {0xc1, 6, "\\u00c1"}, {0xc2, 6, "\\u00c2"}, {0xc3, 6, "\\u00c3"},
    {0xc4, 6, "\\u00c4"}, {0xc5, 6, "\\u00c5"}, {0xc6, 6, "\\u00c6"}, {0xc7, 6, "\\u00c7"},
    {0xc8, 6, "\\u00c8"}, {0xc9, 6, "\\u00c9"}, {0xca, 6, "\\u00ca"}, {0xcb, 6, "\\u00cb"},
    {0xcc, 6, "\\u00cc"}, {0xcd, 6, "\\u00cd"}, {0xce, 6, "\\u00ce"}, {0xcf, 6, "\\u00cf"},

    /* \xd0 - \xdf */
    {0xd0, 6, "\\u00d0"}, {0xd1, 6, "\\u00d1"}, {0xd2, 6, "\\u00d2"}, {0xd3, 6, "\\u00d3"},
    {0xd4, 6, "\\u00d4"}, {0xd5, 6, "\\u00d5"}, {0xd6, 6, "\\u00d6"}, {0xd7, 6, "\\u00d7"},
    {0xd8, 6, "\\u00d8"}, {0xd9, 6, "\\u00d9"}, {0xda, 6, "\\u00da"}, {0xdb, 6, "\\u00db"},
    {0xdc, 6, "\\u00dc"}, {0xdd, 6, "\\u00dd"}, {0xde, 6, "\\u00de"}, {0xdf, 6, "\\u00df"},

    /* \xe0 - \xef */
    {0xe0, 6, "\\u00e0"}, {0xe1, 6, "\\u00e1"}, {0xe2, 6, "\\u00e2"}, {0xe3, 6, "\\u00e3"},
    {0xe4, 6, "\\u00e4"}, {0xe5, 6, "\\u00e5"}, {0xe6, 6, "\\u00e6"}, {0xe7, 6, "\\u00e7"},
    {0xe8, 6, "\\u00e8"}, {0xe9, 6, "\\u00e9"}, {0xea, 6, "\\u00ea"}, {0xeb, 6, "\\u00eb"},
    {0xec, 6, "\\u00ec"}, {0xed, 6, "\\u00ed"}, {0xee, 6, "\\u00ee"}, {0xef, 6, "\\u00ef"},

    /* \xf0 - \xff */
    {0xf0, 6, "\\u00f0"}, {0xf1, 6, "\\u00f1"}, {0xf2, 6, "\\u00f2"}, {0xf3, 6, "\\u00f3"},
    {0xf4, 6, "\\u00f4"}, {0xf5, 6, "\\u00f5"}, {0xf6, 6, "\\u00f6"}, {0xf7, 6, "\\u00f7"},
    {0xf8, 6, "\\u00f8"}, {0xf9, 6, "\\u00f9"}, {0xfa, 6, "\\u00fa"}, {0xfb, 6, "\\u00fb"},
    {0xfc, 6, "\\u00fc"}, {0xfd, 6, "\\u00fd"}, {0xfe, 6, "\\u00fe"}, {0xff, 6, "\\u00ff"}
};


/*
 * json_encode - return a JSON encoding of a block of memory
 *
 * JSON string encoding:
 *
 * These escape characters are required by JSON:
 *
 *      old			new
 *      ----------------------------
 *	\x00-\x07		\u0000 - \u0007
 *	<backspace>		\b	(\x08)
 *	<horizontal_tab>	\t	(\x09)
 *	<newline>		\n	(\x0a)
 *	\x0b			\u000b <vertical_tab>
 *	<form_feed>		\f	(\x0c)
 *	<enter>			\r	(\x0d)
 *	\x0e-\x1f		\u000e - \x001f
 *	"			\"	(\x22)
 *	/			\/	(\x2f)
 *	\			\\	(\x5c)
 *
 * These escape characters are implied by JSON due to
 * HTML and XML encoding, although not strictly required:
 *
 *      old		new
 *      --------------------
 *	&		\u0026		(\x26)
 *	<		\u003c		(\x3c)
 *	>		\u003e		(\x3e)
 *
 * The JSON spec:
 *
 *	https://www.json.org/json-en.html
 *
 * is UTF-8 character based, but for now, we encoded non-ASCII
 * bytes as followed:
 *
 *      old			new
 *      ----------------------------
 *	\x7f-\xff		\u007f - \u00ff
 *
 * This is OK in that it does not violate the JSON spec, it just
 * makes encoded strings with UTF-8 longer than they need to be.
 *
 * XXX - Optimize this function to not encode UTF-8 characters
 *	 that are not strictly needed by the JSON spec.
 *
 * NOTE: While there exist C escapes for characters such as '\v',
 *	 due to flaws in the JSON spec, we must encode such characters
 *	 using the \uffff notation.
 *
 * given:
 *	ptr	start of memory block to encode
 *	len	length of block to encode in bytes
 *	retlen	address of where to store allocated length, if retlen != NULL
 *
 * returns:
 *	allocated JSON encoding of a block, or NULL ==> error
 *	NOTE: retlen, if non-NULL, is set to 0 on error
 */
char *
json_encode(char const *ptr, size_t len, size_t *retlen)
{
    char *ret = NULL;	    /* allocated encoding string or NULL */
    char *beyond = NULL;    /* beyond the end of the allocated encoding string */
    size_t mlen = 0;	    /* length of allocated encoded string */
    char *p;		    /* next place to encode */
    size_t i;

    /*
     * firewall
     */
    if (ptr == NULL) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "called with NULL ptr");
	return NULL;
    }
    if (len <= 0) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "len: %ju must be > 0", (uintmax_t) len);
	return NULL;
    }

    /*
     * count the bytes that will be in the encoded allocated string
     */
    for (i=0; i < len; ++i) {
	mlen += jenc[(uint8_t)(ptr[i])].len;
    }

    /*
     * malloc the encoded string
     */
    ret = malloc(mlen + 1 + 1);
    if (ret == NULL) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "malloc of %ju bytes failed", (uintmax_t)(mlen + 1 + 1));
	return NULL;
    }
    ret[mlen] = '\0';   /* terminate string */
    ret[mlen + 1] = '\0';   /* paranoia */
    beyond = &(ret[mlen]);

    /*
     * JSON encode each byte
     */
    for (i=0, p=ret; i < len; ++i) {
	if (p+jenc[(uint8_t)(ptr[i])].len > beyond) {
	    /* error - clear allocated length */
	    if (retlen != NULL) {
		*retlen = 0;
	    }
	    warn(__func__, "encoding ran beyond end of allocated encoded string");
	    return NULL;
	}
	strcpy(p, jenc[(uint8_t)(ptr[i])].enc);
	p += jenc[(uint8_t)(ptr[i])].len;
    }

    /*
     * return result
     */
    dbg(DBG_VVVHIGH, "returning from json_encode(ptr, %ju, *%ju)",
		     (uintmax_t)len, (uintmax_t)mlen);
    if (retlen != NULL) {
	*retlen = mlen;
    }
    return ret;
}


/*
 * json_encode_str - return a JSON encoding of a string
 *
 * This is an simplified interface for json_encode().
 *
 * given:
 *	str	a string to encode
 *	retlen	address of where to store allocated length, if retlen != NULL
 *
 * returns:
 *	allocated JSON encoding of a block, or NULL ==> error
 *	NOTE: retlen, if non-NULL, is set to 0 on error
 */
char *
json_encode_str(char const *str, size_t *retlen)
{
    void *ret = NULL;	    /* allocated encoding string or NULL */
    size_t len = 0;	    /* length of string to encode */

    /*
     * firewall
     */
    if (str == NULL) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "called with NULL ptr");
	return NULL;
    }
    len = strlen(str);
    if (len <= 0) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "len: %ju must be > 0", (uintmax_t) len);
	return NULL;
    }

    /*
     * convert to json_encode() call
     */
    ret = json_encode(str, len, retlen);
    if (ret == NULL) {
	dbg(DBG_VVHIGH, "returning NULL for encoding of: <%s>", str);
    } else {
	dbg(DBG_VVHIGH, "string: JSON encoded as: <%s>", (char *)ret);
    }

    /*
     * return encoded result or NULL
     */
    return ret;
}


/*
 * jencchk - validate the contents of the jenc[] table
 *
 * This function performs various sanity checks on the jenc[] table.
 *
 * This function does not return on error.
 */
void
jencchk(void)
{
    unsigned int hexval;/* hex value from xxxx part of \uxxxx */
    char guard;		/* scanf guard to catch excess amount of input */
    int indx;		/* test index */
    char const *encstr;	/* encoding string */
    int ret;		/* libc function return value */
    char str[2];	/* character string to encode */
    char *mstr = NULL;	/* allocated encoding string */
    size_t mlen = 0;	/* length of allocated encoding string */
    char *mstr2 = NULL;	/* allocated decoding string */
    size_t mlen2 = 0;	/* length of allocated decoding string */
    unsigned int i;

    /*
     * assert: bits in byte must be 8
     */
    if (BITS_IN_BYTE != 8) {
	err(100, __func__, "BITS_IN_BYTE: %d != 8", BITS_IN_BYTE);
	not_reached();
    }

    /*
     * assert: table must be 256 elements long
     */
    if (sizeof(jenc)/sizeof(jenc[0]) != BYTE_VALUES) {
	err(101, __func__, "jenc table as %ju elements instead of %d",
			   (uintmax_t)sizeof(jenc)/sizeof(jenc[0]), BYTE_VALUES);
	not_reached();
    }

    /*
     * assert: byte must be an index from 0 to 256
     */
    for (i=0; i < BYTE_VALUES; ++i) {
	if (jenc[i].byte != i) {
	    err(102, __func__, "jenc[0x%02x].byte: %d != %d", i, jenc[i].byte, i);
	    not_reached();
	}
    }

    /*
     * assert: enc string must be non-NULL
     */
    for (i=0; i < BYTE_VALUES; ++i) {
	if (jenc[i].enc == NULL) {
	    err(103, __func__, "jenc[0x%02x].enc == NULL", i);
	    not_reached();
	}
    }

    /*
     * assert: length of enc string must match len
     */
    for (i=0; i < BYTE_VALUES; ++i) {
	if (strlen(jenc[i].enc) != jenc[i].len) {
	    err(104, __func__, "jenc[0x%02x].enc length: %ju != jenc[0x%02x].len: %ju",
			       i, (uintmax_t)strlen(jenc[i].enc),
			       i, (uintmax_t)jenc[i].len);
	    not_reached();
	}
    }

    /*
     * assert: \x00-\x07 encodes to \uxxxx
     */
    for (i=0x00; i <= 0x07; ++i) {
	if (jenc[i].len != LITLEN("\\uxxxx")) {
	    err(105, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			       i, (uintmax_t)strlen(jenc[i].enc),
			       (uintmax_t)LITLEN("\\uxxxx"));
	    not_reached();
	}
	ret = sscanf(jenc[i].enc, "\\u%04x%c", &hexval, &guard);
	if (ret != 1) {
	    err(106, __func__, "jenc[0x%02x].enc: <%s> is not in <\\uxxxx> form", i, jenc[i].enc);
	    not_reached();
	}
	if (i != hexval) {
	    err(107, __func__, "jenc[0x%02x].enc: <%s> != <\\u%04x> form", i, jenc[i].enc, i);
	    not_reached();
	}
    }

    /*
     * assert: \x08 encodes to \b
     */
    indx = 0x08;
    encstr = "\\b";
    if (jenc[indx].len != LITLEN("\\b")) {
	err(108, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(109, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x09 encodes to \t
     */
    indx = 0x09;
    encstr = "\\t";
    if (jenc[indx].len != LITLEN("\\b")) {
	err(110, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(111, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x0a encodes to \n
     */
    indx = 0x0a;
    encstr = "\\n";
    if (jenc[indx].len != strlen(encstr)) {
	err(112, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(113, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x0b encodes to \u000b
     */
    indx = 0x0b;
    encstr = "\\u000b";
    if (jenc[indx].len != strlen(encstr)) {
	err(114, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(115, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x0c encodes to \f
     */
    indx = 0x0c;
    encstr = "\\f";
    if (jenc[indx].len != strlen(encstr)) {
	err(116, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(117, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x0d encodes to \r
     */
    indx = 0x0d;
    encstr = "\\r";
    if (jenc[indx].len != strlen(encstr)) {
	err(118, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(119, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x0e-\x1f encodes to \uxxxx
     */
    for (i=0x0e; i <= 0x1f; ++i) {
	if (jenc[i].len != LITLEN("\\uxxxx")) {
	    err(120, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			       i, (uintmax_t)strlen(jenc[i].enc),
			       (uintmax_t)LITLEN("\\uxxxx"));
	    not_reached();
	}
	ret = sscanf(jenc[i].enc, "\\u%04x%c", &hexval, &guard);
	if (ret != 1) {
	    err(121, __func__, "jenc[0x%02x].enc: <%s> is not in <\\uxxxx> form", i, jenc[i].enc);
	    not_reached();
	}
	if (i != hexval) {
	    err(122, __func__, "jenc[0x%02x].enc: <%s> != <\\u%04x> form", i, jenc[i].enc, i);
	    not_reached();
	}
    }

    /*
     * assert: \x20-\x21 encodes to the character
     */
    for (i=0x20; i <= 0x21; ++i) {
	if (jenc[i].len != 1) {
	    err(123, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(124, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x22 encodes to \"
     */
    indx = 0x22;
    encstr = "\\\"";
    if (jenc[indx].len != strlen(encstr)) {
	err(125, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(126, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x23-\x25 encodes to the character
     */
    for (i=0x23; i <= 0x25; ++i) {
	if (jenc[i].len != 1) {
	    err(128, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(129, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x26 encodes to \u0026
     */
    indx = 0x26;
    encstr = "\\u0026";
    if (jenc[indx].len != strlen(encstr)) {
	err(130, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(131, __func__, "jenc[0x%02x].enc: <%s> is not <\\b> form", indx, encstr);
	not_reached();
    }

    /*
     * assert: \x27-\x2e encodes to the character
     */
    for (i=0x27; i <= 0x2e; ++i) {
	if (jenc[i].len != 1) {
	    err(132, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(133, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x2f encodes to \/
     */
    indx = 0x2f;
    encstr = "\\/";
    if (jenc[indx].len != strlen(encstr)) {
	err(134, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(135, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x30-\x3b encodes to the character
     */
    for (i=0x30; i <= 0x3b; ++i) {
	if (jenc[i].len != 1) {
	    err(136, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(137, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x3c encodes to \u003c
     */
    indx = 0x3c;
    encstr = "\\u003c";
    if (jenc[indx].len != strlen(encstr)) {
	err(138, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(139, __func__, "jenc[0x%02x].enc: <%s> is not <\\b> form", indx, encstr);
	not_reached();
    }

    /*
     * assert: \x3d-\x3d encodes to the character
     */
    for (i=0x3d; i <= 0x3d; ++i) {
	if (jenc[i].len != 1) {
	    err(140, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(141, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x3e encodes to \u003e
     */
    indx = 0x3e;
    encstr = "\\u003e";
    if (jenc[indx].len != strlen(encstr)) {
	err(142, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(143, __func__, "jenc[0x%02x].enc: <%s> is not in <\\b> form", indx, encstr);
	not_reached();
    }

    /*
     * assert: \x3f-\x5b encodes to the character
     */
    for (i=0x3f; i <= 0x5b; ++i) {
	if (jenc[i].len != 1) {
	    err(144, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(145, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x5c encodes to \\
     */
    indx = 0x5c;
    encstr = "\\\\";
    if (jenc[indx].len != strlen(encstr)) {
	err(146, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			   indx, (uintmax_t)strlen(jenc[indx].enc),
			   (uintmax_t)strlen(encstr));
	not_reached();
    }
    if (strcmp(jenc[indx].enc, encstr) != 0) {
	err(147, __func__, "jenc[0x%02x].enc: <%s> != <%s>", indx, jenc[indx].enc, encstr);
	not_reached();
    }

    /*
     * assert: \x5d-\x7e encodes to the character
     */
    for (i=0x5d; i <= 0x7e; ++i) {
	if (jenc[i].len != 1) {
	    err(148, __func__, "jenc[0x%02x].enc length: %ju != %d",
			       i, (uintmax_t)strlen(jenc[i].enc), 1);
	    not_reached();
	}
	if ((unsigned int)(jenc[i].enc[0]) != i) {
	    err(149, __func__, "jenc[0x%02x].enc: <%s> is not <%c>", i, jenc[i].enc, (char)i);
	    not_reached();
	}
    }

    /*
     * assert: \x7f-\xff encodes to \uxxxx
     */
    for (i=0x7f; i <= 0xff; ++i) {
	if (jenc[i].len != LITLEN("\\uxxxx")) {
	    err(150, __func__, "jenc[0x%02x].enc length: %ju != %ju",
			       i, (uintmax_t)strlen(jenc[i].enc),
			       (uintmax_t)LITLEN("\\uxxxx"));
	    not_reached();
	}
	ret = sscanf(jenc[i].enc, "\\u%04x%c", &hexval, &guard);
	if (ret != 1) {
	    err(151, __func__, "jenc[0x%02x].enc: <%s> is not in <\\uxxxx> form", i, jenc[i].enc);
	    not_reached();
	}
	if (i != hexval) {
	    err(152, __func__, "jenc[0x%02x].enc: <%s> != <\\u%04x> form", i, jenc[i].enc, i);
	    not_reached();
	}
    }

    /*
     * special test for encoding a NUL byte
     */
    dbg(DBG_VVVHIGH, "testing json_encode(0x00, 1, *mlen)");
    memset(str, 0, sizeof(str));    /* clear all bytes in str, including the final \0 */
    mstr = json_encode(str, 1,  &mlen);
    if (mstr == NULL) {
	err(153, __func__, "json_encode(0x00, 1, *mlen: %ju) == NULL", (uintmax_t)mlen);
	not_reached();
    }
    if (mlen != jenc[0].len) {
	err(154, __func__, "json_encode(0x00, 1, *mlen: %ju != %ju)",
			   (uintmax_t)mlen, (uintmax_t)(jenc[0].len));
	not_reached();
    }
    if (strcmp(jenc[0].enc, mstr) != 0) {
	err(155, __func__, "json_encode(0x00, 1, *mlen: %ju) != <%s>",
			   (uintmax_t)mlen, jenc[0].enc);
	not_reached();
    }
    /* free the allocated encoded string */
    if (mstr != NULL) {
	free(mstr);
	mstr = NULL;
    }

    /*
     * finally try to encode every possible string with a single non-NUL character
     */
    for (i=1; i < BYTE_VALUES; ++i) {

	/*
	 * test JSON encoding
	 */
	dbg(DBG_VVVHIGH, "testing json_encode_str(0x%02x, *mlen)", i);
	/* load input string */
	str[0] = (char)i;
	/* test json_encode_str() */
	mstr = json_encode_str(str, &mlen);
	/* check encoding result */
	if (mstr == NULL) {
	    err(156, __func__, "json_encode_str(0x%02x, *mlen: %ju) == NULL",
			       i, (uintmax_t)mlen);
	    not_reached();
	}
	if (mlen != jenc[i].len) {
	    err(157, __func__, "json_encode_str(0x%02x, *mlen %ju != %ju)",
			       i, (uintmax_t)mlen, (uintmax_t)jenc[i].len);
	    not_reached();
	}
	if (strcmp(jenc[i].enc, mstr) != 0) {
	    err(158, __func__, "json_encode_str(0x%02x, *mlen: %ju) != <%s>", i,
			       (uintmax_t)mlen, jenc[i].enc);
	    not_reached();
	}
	dbg(DBG_VVHIGH, "testing json_encode_str(0x%02x, *mlen) encoded to <%s>", i, mstr);

	/*
	 * test decoding the JSON encoded string
	 */
	dbg(DBG_VVVHIGH, "testing json_decode_str(<%s>, *mlen, true)", mstr);
	/* test json_decode_str() */
	mstr2 = json_decode_str(mstr, &mlen2);
	if (mstr2 == NULL) {
	    err(159, __func__, "json_decode_str(<%s>, *mlen: %ju, true) == NULL",
			       mstr, (uintmax_t)mlen2);
	    not_reached();
	}
	if (mlen2 != 1) {
	    err(160, __func__, "json_decode_str(<%s>, *mlen2 %ju != 1, true)",
			       mstr, (uintmax_t)mlen2);
	    not_reached();
	}
	if ((uint8_t)(mstr2[0]) != i) {
	    err(161, __func__, "json_decode_str(<%s>, *mlen: %ju, true): 0x%02x != 0x%02x",
			       mstr, (uintmax_t)mlen2, (uint8_t)(mstr2[0]), i);
	    not_reached();
	}

	/* free the allocated encoded string */
	if (mstr != NULL) {
	    free(mstr);
	    mstr = NULL;
	}
	if (mstr2 != NULL) {
	    free(mstr2);
	    mstr2 = NULL;
	}
    }

    /*
     * all seems well win the jenc[] table
     */
    dbg(DBG_VVHIGH, "jenc[] table passes");
    return;
}


/*
 * json_decode - return a decoding of a block of JSON encoded memory
 *
 * given:
 *	ptr	start of memory block to decode
 *	len	length of block to decode in bytes
 *	retlen	address of where to store allocated length, if retlen != NULL
 *
 * returns:
 *	allocated JSON decoding of a block, or NULL ==> error
 *	NOTE: retlen, if non-NULL, is set to 0 on error
 */
char *
json_decode(char const *ptr, size_t len, size_t *retlen)
{
    char *ret = NULL;	    /* allocated encoding string or NULL */
    char *beyond = NULL;    /* beyond the end of the allocated encoding string */
    size_t mlen = 0;	    /* length of allocated encoded string */
    char *p = NULL;	    /* next place to encode */
    char n = 0;		    /* next character beyond a \\ */
    char a = 0;		    /* first hex character after \u */
    int xa = 0;		    /* first hex character numeric value */
    char b = 0;		    /* second hex character after \u */
    int xb = 0;		    /* second hex character numeric value */
    char c = 0;		    /* character to decode or third hex character after \u */
    int xc = 0;		    /* 3nd hex character numeric value */
    char d = 0;		    /* fourth hex character after \u */
    int xd = 0;		    /* fourth hex character numeric value */
    size_t i;

    /*
     * firewall
     */
    if (ptr == NULL) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "called with NULL ptr");
	return NULL;
    }
    if (len <= 0) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "len: %ju must be > 0", (uintmax_t) len);
	return NULL;
    }

    /*
     * count the bytes that will be in the decoded allocated string
     */
    for (i=0; i < len; ++i) {

	/*
	 * examine the current character
	 */
	c = (char)((uint8_t)(ptr[i]));

	/*
	 * valid non-\-escaped characters count as 1
	 */
        if (c != '\\') {

	    /*
	     * disallow characters that should have been escaped
	     */
	    switch (c) {
	    case '\b':  /*fallthrough*/
	    case '\t':  /*fallthrough*/
	    case '\n':  /*fallthrough*/
	    case '\f':  /*fallthrough*/
	    case '\r':
		/* error - clear allocated length */
		if (retlen != NULL) {
		    *retlen = 0;
		}
		warn(__func__, "found \\-escaped char: 0x%02x", (uint8_t)c);
		return NULL;
		break;
	    case '"':   /*fallthrough*/
	    case '/':   /*fallthrough*/
	    case '\\':
		/* error - clear allocated length */
		if (retlen != NULL) {
		    *retlen = 0;
		}
		warn(__func__, "found \\-escaped char: %c", c);
		return NULL;
		break;

	    /*
	     * count a valid character
	     */
	    default:
		++mlen;
		break;
	    }

	/*
	 * valid \-escaped characters count as 2 or 6
	 */
	} else {

	    /*
	     * there must be at least one more character beyond \
	     */
	    if (i+1 >= len) {
		/* error - clear allocated length */
		if (retlen != NULL) {
		    *retlen = 0;
		}
		warn(__func__, "found \\ at end of buffer, missing next character");
		return NULL;
	    }

	    /*
	     * look at the next character beyond \
	     */
	    n = (char)((uint8_t)(ptr[i+1]));

	    /*
	     * process single \c escaped pairs
	     */
	    switch (n) {
	    case 'b':	/*fallthrough*/
	    case 't':	/*fallthrough*/
	    case 'n':	/*fallthrough*/
	    case 'f':	/*fallthrough*/
	    case 'r':	/*fallthrough*/
	    case '"':	/*fallthrough*/
	    case '/':	/*fallthrough*/
	    case '\\':
		/*
		 * count \c escaped pair as 1 character
		 */
		++mlen;
		++i;
		break;

	    /*
	     * process \uxxxx where x is a hex character
	     */
	    case 'u':

		/*
		 * there must be at least five more characters beyond \
		 */
		if (i+5 >= len) {
		    /* error - clear allocated length */
		    if (retlen != NULL) {
			*retlen = 0;
		    }
		    warn(__func__, "found \\u, but not enough for 4 hex chars at end of buffer");
		    return NULL;
		}
		a = (char)((uint8_t)(ptr[i+2]));
		b = (char)((uint8_t)(ptr[i+3]));
		c = (char)((uint8_t)(ptr[i+4]));
		d = (char)((uint8_t)(ptr[i+5]));

		/*
		 * the next 4 characters beyond \u must be hex characters
		 */
		if (isxdigit(a) && isxdigit(b) && isxdigit(c) && isxdigit(d)) {

		    /*
		     * case: \u00xx is 1 character
		     */
		    if (a == '0' && b == '0') {
			++mlen;
			i += 5;

		    /*
		     * case: count \uxxxx as 2 characters
		     */
		    } else {
			mlen += 2;
			i += 5;
		    }
		    break;

		/*
		 * case: \uxxxx is invalid because xxxx is not HEX
		 */
		} else {
			warn(__func__, "\\u, not foolowed by 4 hex chars");
			return NULL;
		}
		break;

	    /*
	     * found invalid JSON \-escape character
	     */
	    default:
		/* error - clear allocated length */
		if (retlen != NULL) {
		    *retlen = 0;
		}
		warn(__func__, "found invalid JSON \\-escape: followed by 0x%02x", (uint8_t)c);
		return NULL;
	    }
	}
    }

    /*
     * allocated decoded string
     */
    ret = malloc(mlen + 1 + 1);
    if (ret == NULL) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "malloc of %ju bytes failed", (uintmax_t)(mlen + 1 + 1));
	return NULL;
    }
    ret[mlen] = '\0';   /* terminate string */
    ret[mlen + 1] = '\0';   /* paranoia */
    beyond = &(ret[mlen]);

    /*
     * JSON string decode
     *
     * In the above counting code, prior to the malloc for the decoded string,
     * we determined that the JSON encoded string is valid.
     */
    for (i=0, p=ret; i < len; ++i) {

	/*
	 * examine the current character
	 */
	c = (char)((uint8_t)(ptr[i]));

	/*
	 * paranoia
	 */
	if (p >= beyond) {
	    /* error - clear allocated length */
	    if (retlen != NULL) {
		*retlen = 0;
	    }
	    warn(__func__, "ran beyond end of decoded string");
	    return NULL;
	}

	/*
	 * case: JSON decode non \-escape character
	 */
	if (c != '\\') {
	    /* no translation encoding */
	    *p++ = c;

	/*
	 * case: JSON decode \-escape character
	 */
	} else {

	    /*
	     * look at the next character beyond \
	     */
	    n = (char)((uint8_t)(ptr[i+1]));

	    /*
	     * decode single \c escaped pairs
	     */
	    switch (n) {
	    case 'b':	/* ASCII backspace */
		++i;
		*p++ = '\b';
		break;
	    case 't':	/* ASCII horizontal tab */
		++i;
		*p++ = '\t';
		break;
	    case 'n':	/* ASCII line feed */
		++i;
		*p++ = '\n';
		break;
	    case 'f':	/* ASCII form feed */
		++i;
		*p++ = '\f';
		break;
	    case 'r':	/* ASCII carriage return */
		++i;
		*p++ = '\r';
		break;
	    case 'a':	/* ASCII bell */
		++i;
		*p++ = '\a';
		break;
	    case 'v':	/* ASCII vertical tab */
		++i;
		*p++ = '\v';
		break;
	    case 'e':	/* ASCII escape */
		++i;
		*p++ = 0x0b;  /* no all C compilers understand /e */
		break;
	    case '"':	/*fallthrough*/
	    case '/':	/*fallthrough*/
	    case '\\':
		++i;
		*p++ = n;	/* escape decodes to itself */
		break;

	    /*
	     * decode \uxxxx
	     */
	    case 'u':

		/*
		 * there must be at least five more characters beyond \
		 */
		if (i+5 >= len) {
		    /* error - clear allocated length */
		    if (retlen != NULL) {
			*retlen = 0;
		    }
		    warn(__func__, "found \\u while decoding, but not enough for 4 hex chars at end of buffer");
		    return NULL;
		}
		xa = hexval[(uint8_t)(ptr[i+2])];
		xb = hexval[(uint8_t)(ptr[i+3])];
		xc = hexval[(uint8_t)(ptr[i+4])];
		xd = hexval[(uint8_t)(ptr[i+5])];

		/*
		 * case: \u00xx
		 */
		if (xa == 0 && xb == 0) {
		    /* single byte \u00xx */
		    i += 5;
		    *p++ = (char)((xc << 4) | xd);

		/*
		 * case: \uxxxx
		 */
		} else {

		    /*
		     * paranoia
		     */
		    if (p+1 >= beyond) {
			/* error - clear allocated length */
			if (retlen != NULL) {
			    *retlen = 0;
			}
			warn(__func__, "ran beyond end of decoded string for non-UTF-8 \\u encoding");
			return NULL;
		    }
		    /* double byte \uxxxx */
		    i += 5;
		    *p++ = (char)((xa << 4) | xb);
		    *p++ = (char)((xc << 4) | xd);
		}
		break;

	    /*
	     * unknown \c escaped pairs
	     */
	    default:
		/* error - clear allocated length */
		if (retlen != NULL) {
		    *retlen = 0;
		}
		warn(__func__, "found invalid JSON \\-escape while decoding: followed by 0x%02x", (uint8_t)c);
		return NULL;
	    }
	}
     }

    /*
     * return result
     */
    dbg(DBG_VVVHIGH, "returning from json_decode(ptr, %ju, *%ju)",
		     (uintmax_t)len, (uintmax_t)mlen);
    if (retlen != NULL) {
	*retlen = mlen;
    }
    return ret;
}


/*
 * json_decode_str - return a JSON decoding of a string
 *
 * This is a simplified interface for json_decode().
 *
 * given:
 *	str	a string to decode
 *	retlen	address of where to store allocated length, if retlen != NULL
 *
 * returns:
 *	allocated JSON decoding of a block, or NULL ==> error
 *
 * NOTE: retlen, if non-NULL, is set to 0 on error
 *
 * NOTE: This function assumes that str is a valid C-style string with a terminating
 *	 NUL byte.  This code assumes that the first NUL byte found is the end of
 *	 the string.  When working with raw binary data, or data that could have
 *	 a NUL byte inside the block of memory (instead at the very end), this
 *	 function should NOT be used.  In such cases, json_decode() should
 *	 be used instead!
 */
char *
json_decode_str(char const *str, size_t *retlen)
{
    void *ret = NULL;	    /* allocated decoding string or NULL */
    size_t len = 0;	    /* length of string to decode */

    /*
     * firewall
     */
    if (str == NULL) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "called with NULL ptr");
	return NULL;
    }
    len = strlen(str);
    if (len <= 0) {
	/* error - clear allocated length */
	if (retlen != NULL) {
	    *retlen = 0;
	}
	warn(__func__, "len: %ju must be > 0", (uintmax_t) len);
	return NULL;
    }

    /*
     * convert to json_decode() call
     */
    ret = json_decode(str, len, retlen);
    if (ret == NULL) {
	dbg(DBG_VVHIGH, "returning NULL for decoding of: <%s>", str);
    } else {
	dbg(DBG_VVHIGH, "string: <%s> JSON decoded", str);
    }

    /*
     * return decoded result or NULL
     */
    return ret;
}


/*
 * json_conv_free - free JSON parser tree node
 *
 * given:
 *	node	pointer to JSON parser tree node to free
 *
 * NOTE: This function will free the internals of a JSON parser tree node.
 *	 It is up to the caller to free the struct json if needed.
 *
 * NOTE: This function does NOT walk the JSON parser tree, so it will
 *	 ignore links form this node to other JSON parser tree nodes.
 *
 * NOTE: If the pointer to allocated storage == NULL,
 *	 this function does nothing.
 *
 * NOTE: This function does nothing if node == NULL.
 *
 * NOTE: This function does nothing if the node type is invalid.
 */
void
json_conv_free(struct json *node)
{
    /*
     * firewall - nothing to do for a NULL node
     */
    if (node == NULL) {
	return;
    }

    /*
     * free internals based in node type
     */
    switch (node->type) {

    case JTYPE_EOT:	/* special end of the table value */
	/* nothing to free */
	break;

    case JTYPE_UNSET:	/* JSON element has not been set - must be the value 0 */
	/* nothing to free */
	break;

    case JTYPE_INT:	/* JSON element is an integer - see struct json_integer */
	if (node->element.integer.as_str != NULL) {
	    free(node->element.integer.as_str);
	    node->element.integer.as_str = NULL;
	}
	break;

    case JTYPE_FLOAT:	/* JSON element is a float - see struct json_floating */
	if (node->element.floating.as_str != NULL) {
	    free(node->element.floating.as_str);
	    node->element.floating.as_str = NULL;
	}
	break;

    case JTYPE_STRING:	/* JSON element is a string - see struct json_string */
	if (node->element.string.as_str != NULL) {
	    free(node->element.string.as_str);
	    node->element.string.as_str = NULL;
	}
	if (node->element.string.str != NULL) {
	    free(node->element.string.str);
	    node->element.string.str = NULL;
	}
	break;

    case JTYPE_BOOL:	/* JSON element is a boolean - see struct json_boolean */
	if (node->element.boolean.as_str != NULL) {
	    free(node->element.boolean.as_str);
	    node->element.boolean.as_str = NULL;
	}
	break;

    case JTYPE_NULL:	/* JSON element is a null - see struct json_null */
	if (node->element.null.as_str != NULL) {
	    free(node->element.null.as_str);
	    node->element.null.as_str = NULL;
	}
	break;

    case JTYPE_OBJECT:	/* JSON element is a { members } */
	/* XXX - update when struct json_object is defined - XXX */
	break;

    case JTYPE_MEMBER:	/* JSON element is a member */
	/* XXX - update when struct json_member is defined - XXX */
	break;

    case JTYPE_ARRAY:	/* JSON element is a [ elements ] */
	/* XXX - update when struct json_array is defined - XXX */
	break;

    default:
	/* nothing we can free */
	warn(__func__, "node type is unknown: %d", node->type);
	break;
    }
    return;
}


/*
 * json_conv_int - convert JSON integer string to C integer
 *
 * given:
 *	str	a JSON integer string
 *	len	length, starting at str, of the JSON string
 *
 * returns:
 *	allocated JSON parser tree node converted JSON integer
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_int(char const *str, size_t len)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    struct json_integer *item = NULL;	    /* JSON integer element inside JSON parser tree node */
    char *endptr;			    /* first invalid character or str */
    size_t digits = 0;			    /* number of digits in JSON integer not including leading sign */
    size_t i;

    /*
     * allocate the JSON parse tree element
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = calloc(1, sizeof(*ret));
    if (ret == NULL) {
	errp(162, __func__, "calloc #0 error allocating %ju bytes", (uintmax_t)sizeof(*ret));
	not_reached();
    }

    /*
     * initialize the JSON parse tree element
     */
    ret->type = JTYPE_INT;
    ret->parent = NULL;
    ret->prev = NULL;
    ret->next = NULL;

    /*
     * initialize the JSON element
     */
    item = &(ret->element.integer);
    item->as_str = NULL;
    item->converted = false;
    item->is_negative = false;
    item->int8_sized = false;
    item->uint8_sized = false;
    item->int16_sized = false;
    item->uint16_sized = false;
    item->int32_sized = false;
    item->uint32_sized = false;
    item->int64_sized = false;
    item->uint64_sized = false;
    item->int_sized = false;
    item->uint_sized = false;
    item->long_sized = false;
    item->ulong_sized = false;
    item->longlong_sized = false;
    item->ulonglong_sized = false;
    item->ssize_sized = false;
    item->size_sized = false;
    item->off_sized = false;
    item->maxint_sized = false;
    item->umaxint_sized = false;

    /*
     * firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
	return ret;
    }
    if (len <= 0) {
	warn(__func__, "called with len: %ju <= 0", (uintmax_t)len);
	return ret;
    }
    item->orig_len = len;	/* save original length */
    if (str[0] == '\0') {
	warn(__func__, "called with empty string");
	return ret;
    }

    /*
     * duplicate the JSON integer string
     */
    errno = 0;			/* pre-clear errno for errp() */
    item->as_str = calloc(len+1+1, sizeof(char));
    if (item->as_str == NULL) {
	errp(163, __func__, "calloc #1 error allocating %ju bytes", (uintmax_t)(len+1+1));
	not_reached();
    }
    strncpy(item->as_str, str, len+1);
    item->as_str_len = len;	/* save length of as_str */

    /*
     * paranoia
     *
     * While the common use of this function is via bison/flex produced C code,
     * we want to keep the general case working where this function might
     * someday be called from some other code.  For such a future case we
     * want to trim off leading and trailing whitespace so that the code below
     * checking for < 0 and the code checking the conversion into uintmax_t
     * or into intmax_t is not confused.
     */
    /* skip over any leading space */
    for (i=0; i < len; ++i) {
	if (!isascii(item->as_str[i]) || !isspace(item->as_str[i])) {
	    break;
	}
    }
    /* trim any leading space */
    len -= i;
    item->as_str += i;
    if (len <= 0) {
	dbg(DBG_HIGH, "%s: called with string containing only whitespace: <%s>", __func__, item->as_str);
	return ret;
    }
    /* trim off any trailing whitespace */
    while (len > 0 && isascii(item->as_str[len-1]) && isspace(item->as_str[len-1])) {
	/* trim trailing whitespace */
	item->as_str[len-1] = '\0';
	--len;
    }
    if (len <= 0) {
	dbg(DBG_HIGH, "%s: called with string containing all whitespace: <%s>", __func__, item->as_str);
	return ret;
    }
    item->as_str_len = len;	/* save length of as_str */

    /*
     * determine if JSON integer negative
     */
    if (item->as_str[0] == '-') {

	/* parse JSON integer that is < 0 */
	item->is_negative = true;

        /*
	 * JSON spec detail: lone - invalid JSON
	 */
	if (len <= 1 || item->as_str[1] == '\0') {
	    dbg(DBG_HIGH, "%s called with just -: <%s>", __func__, item->as_str);
	    return ret;

        /*
	 * JSON spec detail: leading -0 followed by digits - invalid JSON
	 */
	} else if (len > 2 && item->as_str[1] == '0' && isascii(item->as_str[2]) && isdigit(item->as_str[2])) {
	    dbg(DBG_HIGH, "%s called with -0 followed by more digits: <%s>", __func__, item->as_str);
	    return ret;
	}

	/*
	 * paranoia
	 *
	 * The only characters beyond the - remaining should be only digits.
	 * This shouldn't happen via the bison / flex code that has an integer
	 * regexp, but we check anyway as a matter of defense in depth.
	 */
	digits = strspn(item->as_str+1, "0123456789");
	if (digits != len-1) {
	    dbg(DBG_HIGH, "%s: called with string with - followed by non-digits: <%s>", __func__, item->as_str);
	    return ret;
	}

    /* case: JSON integer is >= 0 */
    } else {

	/* parse JSON integer that is >= 0 */
	item->is_negative = false;

        /*
	 * JSON spec detail: leading 0 followed by digits - invalid JSON
	 */
	if (len > 1 && item->as_str[0] == '0' && isascii(item->as_str[1]) && isdigit(item->as_str[1])) {
	    dbg(DBG_HIGH, "%s called with 0 followed by more digits: <%s>", __func__, item->as_str);
	    return ret;
	}

	/*
	 * paranoia
	 *
	 * The only characters beyond the remaining should be only digits.
	 * This shouldn't happen via the bison / flex code, but we check anyway
	 * as a matter of defense in depth.
	 */
	digits = strspn(item->as_str, "0123456789");
	if (digits != len) {
	    dbg(DBG_HIGH, "%s: called with string containing non-digits: <%s>", __func__, item->as_str);
	    return ret;
	}
    }

    /*
     * attempt to convert to the largest possible integer
     */
    if (item->is_negative) {

	/* case: negative, try for largest signed integer */
	errno = 0;			/* pre-clear errno for errp() */
	item->as_maxint = strtoimax(item->as_str, &endptr, 10);
	if (errno != 0 || endptr == item->as_str || endptr == NULL || *endptr != '\0') {
	    dbg(DBG_VVHIGH, "strtoimax failed to convert");
	    item->converted = false;
	    return ret;
	}
	item->converted = true;
	item->maxint_sized = true;
	dbg(DBG_VVHIGH, "strtoimax for <%s> returned: %jd", item->as_str, item->as_maxint);

	/* case int8_t: range check */
	if (item->as_maxint >= (intmax_t)INT8_MIN && item->as_maxint <= (intmax_t)INT8_MAX) {
	    item->int8_sized = true;
	    item->as_int8 = (int8_t)item->as_maxint;
	}

	/* case uint8_t: cannot be because JSON string is < 0 */
	item->uint8_sized = false;

	/* case int16_t: range check */
	if (item->as_maxint >= (intmax_t)INT16_MIN && item->as_maxint <= (intmax_t)INT16_MAX) {
	    item->int16_sized = true;
	    item->as_int16 = (int16_t)item->as_maxint;
	}

	/* case uint16_t: cannot be because JSON string is < 0 */
	item->uint16_sized = false;

	/* case int32_t: range check */
	if (item->as_maxint >= (intmax_t)INT32_MIN && item->as_maxint <= (intmax_t)INT32_MAX) {
	    item->int32_sized = true;
	    item->as_int32 = (int32_t)item->as_maxint;
	}

	/* case uint32_t: cannot be because JSON string is < 0 */
	item->uint32_sized = false;

	/* case int64_t: range check */
	if (item->as_maxint >= (intmax_t)INT64_MIN && item->as_maxint <= (intmax_t)INT64_MAX) {
	    item->int64_sized = true;
	    item->as_int64 = (int64_t)item->as_maxint;
	}

	/* case uint64_t: cannot be because JSON string is < 0 */
	item->uint64_sized = false;

	/* case int: range check */
	if (item->as_maxint >= (intmax_t)INT_MIN && item->as_maxint <= (intmax_t)INT_MAX) {
	    item->int_sized = true;
	    item->as_int = (int)item->as_maxint;
	}

	/* case unsigned int: cannot be because JSON string is < 0 */
	item->uint_sized = false;

	/* case long: range check */
	if (item->as_maxint >= (intmax_t)LONG_MIN && item->as_maxint <= (intmax_t)LONG_MAX) {
	    item->long_sized = true;
	    item->as_long = (long)item->as_maxint;
	}

	/* case unsigned long: cannot be because JSON string is < 0 */
	item->ulong_sized = false;

	/* case long long: range check */
	if (item->as_maxint >= (intmax_t)LLONG_MIN && item->as_maxint <= (intmax_t)LLONG_MAX) {
	    item->longlong_sized = true;
	    item->as_longlong = (long long)item->as_maxint;
	}

	/* case unsigned long long: cannot be because JSON string is < 0 */
	item->ulonglong_sized = false;

	/* case size_t: cannot be because JSON string is < 0 */
	item->size_sized = false;

	/* case ssize_t: range check */
	if (item->as_maxint >= (intmax_t)SSIZE_MIN && item->as_maxint <= (intmax_t)SSIZE_MAX) {
	    item->ssize_sized = true;
	    item->as_ssize = (ssize_t)item->as_maxint;
	}

	/* case off_t: range check */
	if (item->as_maxint >= (intmax_t)OFF_MIN && item->as_maxint <= (intmax_t)OFF_MAX) {
	    item->off_sized = true;
	    item->as_off = (off_t)item->as_maxint;
	}

	/* case intmax_t: was handled by the above call to strtoimax() */

	/* case uintmax_t: cannot be because JSON string is < 0 */
	item->umaxint_sized = false;

    } else {

	/* case: non-negative, try for largest unsigned integer */
	errno = 0;			/* pre-clear errno for errp() */
	item->as_umaxint = strtoumax(item->as_str, &endptr, 10);
	if (errno != 0 || endptr == item->as_str || endptr == NULL || *endptr != '\0') {
	    dbg(DBG_VVHIGH, "strtoumax failed to convert");
	    item->converted = false;
	    return ret;
	}
	item->converted = true;
	item->umaxint_sized = true;
	dbg(DBG_VVHIGH, "strtoumax for <%s> returned: %ju", item->as_str, item->as_umaxint);

	/* case int8_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)INT8_MAX) {
	    item->int8_sized = true;
	    item->as_int8 = (int8_t)item->as_umaxint;
	}

	/* case uint8_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)UINT8_MAX) {
	    item->uint8_sized = true;
	    item->as_uint8 = (uint8_t)item->as_umaxint;
	}

	/* case int16_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)INT16_MAX) {
	    item->int16_sized = true;
	    item->as_int16 = (int16_t)item->as_umaxint;
	}

	/* case uint16_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)UINT16_MAX) {
	    item->uint16_sized = true;
	    item->as_uint16 = (uint16_t)item->as_umaxint;
	}

	/* case int32_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)INT32_MAX) {
	    item->int32_sized = true;
	    item->as_int32 = (int32_t)item->as_umaxint;
	}

	/* case uint32_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)UINT32_MAX) {
	    item->uint32_sized = true;
	    item->as_uint32 = (uint32_t)item->as_umaxint;
	}

	/* case int64_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)INT64_MAX) {
	    item->int64_sized = true;
	    item->as_int64 = (int64_t)item->as_umaxint;
	}

	/* case uint64_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)UINT64_MAX) {
	    item->uint64_sized = true;
	    item->as_uint64 = (uint64_t)item->as_umaxint;
	}

	/* case int: bounds check */
	if (item->as_umaxint <= (uintmax_t)INT_MAX) {
	    item->int_sized = true;
	    item->as_int = (int)item->as_umaxint;
	}

	/* case unsigned int: bounds check */
	if (item->as_umaxint <= (uintmax_t)UINT_MAX) {
	    item->uint_sized = true;
	    item->as_uint = (unsigned int)item->as_umaxint;
	}

	/* case long: bounds check */
	if (item->as_umaxint <= (uintmax_t)LONG_MAX) {
	    item->long_sized = true;
	    item->as_long = (long)item->as_umaxint;
	}

	/* case unsigned long: bounds check */
	if (item->as_umaxint <= (uintmax_t)ULONG_MAX) {
	    item->ulong_sized = true;
	    item->as_ulong = (unsigned long)item->as_umaxint;
	}

	/* case long long: bounds check */
	if (item->as_umaxint <= (uintmax_t)LLONG_MAX) {
	    item->longlong_sized = true;
	    item->as_longlong = (long long)item->as_umaxint;
	}

	/* case unsigned long long: bounds check */
	if (item->as_umaxint <= (uintmax_t)ULLONG_MAX) {
	    item->ulonglong_sized = true;
	    item->as_ulonglong = (unsigned long long)item->as_umaxint;
	}

	/* case ssize_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)SSIZE_MAX) {
	    item->ssize_sized = true;
	    item->as_ssize = (ssize_t)item->as_umaxint;
	}

	/* case size_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)SIZE_MAX) {
	    item->size_sized = true;
	    item->as_size = (size_t)item->as_umaxint;
	}

	/* case off_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)OFF_MAX) {
	    item->off_sized = true;
	    item->as_off = (off_t)item->as_umaxint;
	}

	/* case intmax_t: bounds check */
	if (item->as_umaxint <= (uintmax_t)INTMAX_MAX) {
	    item->maxint_sized = true;
	    item->as_maxint = (intmax_t)item->as_umaxint;
	}

	/* case uintmax_t: was handled by the above call to strtoumax() */
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_int_str - convert JSON integer string to C integer
 *
 * This is an simplified interface for json_conv_int().
 *
 * given:
 *	str	a JSON integer string to convert
 *	retlen	address of where to store length of str, if retlen != NULL
 *
 * returns:
 *	allocated JSON parser tree node converted JSON integer
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: retlen, if non-NULL, is set to 0 on error
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_int_str(char const *str, size_t *retlen)
{
    struct json *ret = NULL;	    /* JSON parser tree node to return */
    size_t len = 0;		    /* length of string to encode */

    /*
     * firewall
     *
     * NOTE: We will let the json_conv_int() handle the arg firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
    } else {
	len = strlen(str);
    }

    /*
     * convert to json_conv_int() call
     */
    ret = json_conv_int(str, len);
    if (ret == NULL) {
	err(164, __func__, "json_conv_int() returned NULL");
	not_reached();
    }

    /*
     * save length if allowed
     */
    if (retlen != NULL) {
	*retlen = len;
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_float - convert JSON floating point string to C floating point
 *
 * given:
 *	str	a JSON integer floating point string
 *	len	length, starting at str, of the JSON string
 *
 * returns:
 *	allocated JSON parser tree node converted JSON floating point string
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_float(char const *str, size_t len)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    struct json_floating *item = NULL;	    /* JSON floating point element inside JSON parser tree node */
    char *endptr;			    /* first invalid character or str */
    char *e_found = NULL;		    /* strchr() search for e */
    char *cap_e_found = NULL;		    /* strchr() search for E */
    char *e = NULL;			    /* strrchr() search for 2nd e or E */
    size_t i;

    /*
     * allocate the JSON parse tree element
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = calloc(1, sizeof(*ret));
    if (ret == NULL) {
	errp(165, __func__, "calloc #0 error allocating %ju bytes", (uintmax_t)sizeof(*ret));
	not_reached();
    }

    /*
     * initialize the JSON parse tree element
     */
    ret->type = JTYPE_FLOAT;
    ret->parent = NULL;
    ret->prev = NULL;
    ret->next = NULL;

    /*
     * initialize the JSON element
     */
    item = &(ret->element.floating);
    item->converted = false;
    item->is_negative = false;
    item->is_e_notation = false;
    item->float_sized = false;
    item->as_float = 0.0;
    item->as_float_int = false;
    item->double_sized = false;
    item->as_double = 0.0;
    item->as_double_int = false;
    item->longdouble_sized = false;
    item->as_longdouble = 0.0;
    item->as_longdouble_int = false;

    /*
     * firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
	return ret;
    }
    if (len <= 0) {
	warn(__func__, "called with len: %ju <= 0", (uintmax_t)len);
	return ret;
    }
    if (str[0] == '\0') {
	warn(__func__, "called with empty string");
	return ret;
    }

    /*
     * duplicate the JSON integer string
     */
    errno = 0;			/* pre-clear errno for errp() */
    item->as_str = calloc(len+1+1, sizeof(char));
    if (item->as_str == NULL) {
	errp(166, __func__, "calloc #1 error allocating %ju bytes", (uintmax_t)(len+1+1));
	not_reached();
    }
    strncpy(item->as_str, str, len+1);
    item->as_str_len = len;	/* save length of as_str */

    /*
     * paranoia
     *
     * While the common use of this function is via bison/flex produced C code,
     * we want to keep the general case working where this function might
     * someday be called from some other code.
     */
    /* skip over any leading space */
    for (i=0; i < len; ++i) {
	if (!isascii(item->as_str[i]) || !isspace(item->as_str[i])) {
	    break;
	}
    }
    /* trim any leading space */
    len -= i;
    item->as_str += i;
    if (len <= 0) {
	dbg(DBG_HIGH, "%s: called with string containing only whitespace: <%s>",
		      __func__, item->as_str);
	return ret;
    }
    /* trim off any trailing whitespace */
    while (len > 0 && isascii(item->as_str[len-1]) && isspace(item->as_str[len-1])) {
	/* trim trailing whitespace */
	item->as_str[len-1] = '\0';
	--len;
    }
    if (len <= 0) {
	dbg(DBG_HIGH, "%s: called with string with all whitespace: <%s>",
		      __func__, item->as_str);
	return ret;
    }

    /*
     * JSON spec detail: floating point numbers cannot start with .
     */
    if (item->as_str[0] == '.') {
	dbg(DBG_HIGH, "%s: floating point numbers cannot start with .: <%s>",
		       __func__, item->as_str);
	return ret;

    /*
     * JSON spec detail: floating point numbers cannot end with .
     */
    } else if (item->as_str[len-1] == '.') {
	dbg(DBG_HIGH, "%s: floating point numbers cannot end with .: <%s>",
		      __func__, item->as_str);
	return ret;

    /*
     * JSON spec detail: floating point numbers cannot end with -
     */
    } else if (item->as_str[len-1] == '-') {
	dbg(DBG_HIGH, "%s: floating point numbers cannot end with -: <%s>",
		      __func__, item->as_str);
	return ret;

    /*
     * JSON spec detail: floating point numbers cannot end with +
     */
    } else if (item->as_str[len-1] == '+') {
	dbg(DBG_HIGH, "%s: floating point numbers cannot end with +: <%s>",
		      __func__, item->as_str);
	return ret;

    /*
     * JSON spec detail: floating point numbers must end in a digit
     */
    } else if (!isascii(item->as_str[len-1]) || !isdigit(item->as_str[len-1])) {
	dbg(DBG_HIGH, "%s: floating point numbers must end in a digit: <%s>",
		       __func__, item->as_str);
	return ret;
    }

    /*
     * detect use of e notation
     */
    e_found = strchr(item->as_str, 'e');
    cap_e_found = strchr(item->as_str, 'E');
    /* case: both e and E found */
    if (e_found != NULL && cap_e_found != NULL) {

	dbg(DBG_HIGH, "%s: floating point numbers cannot use both e and E: <%s>",
			  __func__, item->as_str);
	return ret;

    /* case: just e found, no E */
    } else if (e_found != NULL) {

	/* firewall - search for two e's */
	e = strrchr(item->as_str, 'e');
	if (e_found != e) {
	    dbg(DBG_HIGH, "%s: floating point numbers cannot have more than one e: <%s>",
			  __func__, item->as_str);
	    return ret;
	}

	/* note e notation */
	item->is_e_notation = true;

    /* case: just E found, no e */
    } else if (cap_e_found != NULL) {

	/* firewall - search for two E's */
	e = strrchr(item->as_str, 'E');
	if (cap_e_found != e) {
	    dbg(DBG_HIGH, "%s: floating point numbers cannot have more than one E: <%s>",
			  __func__, item->as_str);
	    return ret;
	}

	/* note e notation */
	item->is_e_notation = true;
    }

    /*
     * perform additional JSON number checks on e notation
     *
     * NOTE: If item->is_e_notation == true, then e points to the e or E
     */
    if (item->is_e_notation == true) {

	/*
	 * JSON spec detail: e notation number cannot start with e or E
	 */
	if (e == item->as_str) {
	    dbg(DBG_HIGH, "%s: e notation numbers cannot start with e or E: <%s>",
			  __func__, item->as_str);
	    return ret;

	/*
	 * JSON spec detail: e notation number cannot end with e or E
	 */
	} else if (e == &(item->as_str[len-1])) {
	    dbg(DBG_HIGH, "%s: e notation numbers cannot end with e or E: <%s>",
			  __func__, item->as_str);
	    return ret;

	/*
	 * JSON spec detail: e notation number cannot have e or E after .
	 */
	} else if (e > item->as_str && e[-1] == '.') {
	    dbg(DBG_HIGH, "%s: e notation numbers cannot have . before e or E: <%s>",
			  __func__, item->as_str);
	    return ret;

	/*
	 * JSON spec detail: e notation number must have digit before e or E
	 */
	} else if (e > item->as_str && (!isascii(e[-1]) || !isdigit(e[-1]))) {
	    dbg(DBG_HIGH, "%s: e notation numbers must have digit before e or E: <%s>",
			  __func__, item->as_str);
	    return ret;

	/*
	 * case: e notation number with a leading + in the exponent
	 *
	 * NOTE: From "floating point numbers cannot end with +" we know + is not at the end.
	 */
	} else if (e[1] == '+') {

	    /*
	     * JSON spec detail: e notation number with e+ or E+ must be followed by a digit
	     */
	    if (e+1 < &(item->as_str[len-1]) && (!isascii(e[2]) || !isdigit(e[2]))) {
		dbg(DBG_HIGH, "%s: :e notation number with e+ or E+ must be followed by a digit <%s>",
			      __func__, item->as_str);
		return ret;
	    }

	/*
	 * case: e notation number with a leading - in the exponent
	 *
	 * NOTE: From "floating point numbers cannot end with -" we know - is not at the end.
	 */
	} else if (e[1] == '-') {

	    /*
	     * JSON spec detail: e notation number with e- or E- must be followed by a digit
	     */
	    if (e+1 < &(item->as_str[len-1]) && (!isascii(e[2]) || !isdigit(e[2]))) {
		dbg(DBG_HIGH, "%s: :e notation number with e- or E- must be followed by a digit <%s>",
			      __func__, item->as_str);
		return ret;
	    }

	/*
	 * JSON spec detail: e notation number must have + or - or digit after e or E
	 */
	} else if (!isascii(e[1]) || !isdigit(e[1])) {
	    dbg(DBG_HIGH, "%s: e notation numbers must follow e or E with + or - or digit: <%s>",
			  __func__, item->as_str);
	    return ret;
	}
    }

    /*
     * convert to largest floating point value
     */
    errno = 0;			/* pre-clear errno for errp() */
    item->as_longdouble = strtold(item->as_str, &endptr);
    if (errno != 0 || endptr == item->as_str || endptr == NULL || *endptr != '\0') {
	dbg(DBG_VVHIGH, "strtold failed to convert");
	item->converted = false;
	return ret;
    }
    item->converted = true;
    item->longdouble_sized = true;
    item->as_longdouble_int = (item->as_longdouble == floorl(item->as_longdouble));
    dbg(DBG_VVHIGH, "strtold for <%s> returned as %%Lg: %.22Lg", item->as_str, item->as_longdouble);
    dbg(DBG_VVHIGH, "strtold for <%s> returned as %%Le: %.22Le", item->as_str, item->as_longdouble);
    dbg(DBG_VVHIGH, "strtold for <%s> returned as %%Lf: %.22Lf", item->as_str, item->as_longdouble);
    dbg(DBG_VVHIGH, "strtold returned an integer value: %s", item->as_longdouble_int ? "true" : "false");

    /*
     * note if value < 0
     */
    if (item->as_longdouble < 0) {
	item->is_negative = true;
    }

    /*
     * convert to double
     */
    errno = 0;			/* pre-clear conversion test */
    item->as_double = strtod(item->as_str, &endptr);
    if (errno != 0 || endptr == item->as_str || endptr == NULL || *endptr != '\0') {
	item->double_sized = false;
	dbg(DBG_VVHIGH, "strtod for <%s> failed", item->as_str);
    } else {
	item->double_sized = true;
	item->as_double_int = (item->as_double == floorl(item->as_double));
	dbg(DBG_VVHIGH, "strtod for <%s> returned as %%lg: %.22lg", item->as_str, item->as_double);
	dbg(DBG_VVHIGH, "strtod for <%s> returned as %%le: %.22le", item->as_str, item->as_double);
	dbg(DBG_VVHIGH, "strtod for <%s> returned as %%lf: %.22lf", item->as_str, item->as_double);
	dbg(DBG_VVHIGH, "strtod returned an integer value: %s", item->as_double_int ? "true" : "false");
    }

    /*
     * convert to float
     */
    errno = 0;			/* pre-clear conversion test */
    item->as_float = strtof(item->as_str, &endptr);
    if (errno != 0 || endptr == item->as_str || endptr == NULL || *endptr != '\0') {
	item->float_sized = false;
	dbg(DBG_VVHIGH, "strtof for <%s> failed", item->as_str);
    } else {
	item->float_sized = true;
	item->as_float_int = (item->as_longdouble == floorl(item->as_longdouble));
	dbg(DBG_VVHIGH, "strtof for <%s> returned as %%g: %.22g", item->as_str, item->as_float);
	dbg(DBG_VVHIGH, "strtof for <%s> returned as %%e: %.22e", item->as_str, item->as_float);
	dbg(DBG_VVHIGH, "strtof for <%s> returned as %%f: %.22f", item->as_str, item->as_float);
	dbg(DBG_VVHIGH, "strtof returned an integer value: %s", item->as_float_int ? "true" : "false");
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_float_str - convert JSON floating point string to C floating point
 *
 * This is an simplified interface for json_conv_float().
 *
 * given:
 *	str	a JSON floating point string to convert
 *	retlen	address of where to store length of str, if retlen != NULL
 *
 * returns:
 *	allocated JSON parser tree node converted JSON JSON floating point string
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: retlen, if non-NULL, is set to 0 on error
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_float_str(char const *str, size_t *retlen)
{
    struct json *ret = NULL;		    /* JSON floating point element inside JSON parser tree node */
    size_t len = 0;			    /* length of string to encode */

    /*
     * firewall
     *
     * NOTE: We will let the json_conv_float() handle the arg firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
    } else {
	len = strlen(str);
    }

    /*
     * convert to json_conv_float() call
     */
    ret = json_conv_float(str, len);
    if (ret == NULL) {
	err(167, __func__, "json_conv_float() returned NULL");
	not_reached();
    }

    /*
     * save length if allowed
     */
    if (retlen != NULL) {
	*retlen = len;
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_string - convert JSON encoded string to C string
 *
 * given:
 *	str	a JSON encoded string
 *	len	length, starting at str, of the JSON string
 *	quote	true ==> ignore JSON double quotes, both str[0] & str[len-1] must be "
 *		false ==> the entire str is to be converted
 *
 * returns:
 *	allocated JSON parser tree node converted JSON encoded string
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_string(char const *str, size_t len, bool quote)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    struct json_string *item = NULL;	    /* JSON string element inside JSON parser tree node */

    /*
     * allocate the JSON parse tree element
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = calloc(1, sizeof(*ret));
    if (ret == NULL) {
	errp(168, __func__, "calloc #0 error allocating %ju bytes", (uintmax_t)sizeof(*ret));
	not_reached();
    }

    /*
     * initialize the JSON parse tree element
     */
    ret->type = JTYPE_STRING;
    ret->parent = NULL;
    ret->prev = NULL;
    ret->next = NULL;

    /*
     * initialize the JSON element
     */
    item = &(ret->element.string);
    item->as_str = NULL;
    item->str = NULL;
    item->converted = false;
    item->quote = false;
    item->same = false;
    item->has_nul = false;
    item->slash = false;
    item->posix_safe = false;
    item->first_alphanum = false;
    item->upper = false;

    /*
     * firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
	return ret;
    }

    /*
     * case: JSON surrounding '"'s are to be ignored
     */
    if (quote == true) {

	/*
	 * firewall
	 */
	if (len < 2) {
	    warn(__func__, "quote === true: called with len: %ju < 2", (uintmax_t)len);
	    return ret;
	}
	if (str[0] != '"') {
	    warn(__func__, "quote === true: string does NOT start with a \"");
	    return ret;
	}
	if (str[len-1] != '"') {
	    warn(__func__, "quote === true: string does NOT end with a \"");
	    return ret;
	}

	/*
	 * ignore JSON surrounding '"'s
	 */
	item->quote = true;
	++str;
	len -= 2;
    }

    /*
     * duplicate the JSON integer string
     */
    errno = 0;			/* pre-clear errno for errp() */
    item->as_str = calloc(len+1+1, sizeof(char));
    if (item->as_str == NULL) {
	errp(169, __func__, "calloc #1 error allocating %ju bytes", (uintmax_t)(len+1+1));
	not_reached();
    }
    strncpy(item->as_str, str, len+1);
    item->as_str_len = len;	/* save length of as_str */

    /*
     * decode the JSON encoded string
     */
    /* decode the entire string */
    item->str = json_decode_str(item->as_str, &(item->str_len));
    if (item->str == NULL) {
	warn(__func__, "quote === %s: JSON string decode failed for: <%s>",
		       (quote ? "true" : "false"), item->as_str);
	return ret;
    }
    item->converted = true;	/* JSON decoding successful */

    /*
     * determine if decoded string is identical to the original JSON encoded string
     */
    if (item->as_str_len == item->str_len && memcmp(item->as_str, item->str, item->as_str_len) == 0) {
	item->same = true;	/* decoded string same an original JSON encoded string (perhaps sans '"'s) */
    }

    /*
     * determine if decoded JSON string is a C string
     */
    item->has_nul = is_string(item->str, item->str_len);

    /*
     * determine POSIX state of the decoded string
     */
    posix_safe_chk(item->str, item->str_len, &item->slash, &item->posix_safe, &item->first_alphanum, &item->upper);

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_string_str - convert JSON string to C string
 *
 * This is an simplified interface for json_conv_string().
 *
 * given:
 *	str	a JSON encoded string
 *	retlen	address of where to store length of str, if retlen != NULL
 *	quote	true ==> ignore JSON double quotes, both str[0] & str[len-1] must be "
 *		false ==> the entire str is to be converted
 *
 * returns:
 *	allocated JSON parser tree node converted JSON string
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: retlen, if non-NULL, is set to 0 on error
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_string_str(char const *str, size_t *retlen, bool quote)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    size_t len = 0;			    /* length of string to encode */

    /*
     * firewall
     *
     * NOTE: We will let the json_conv_string() handle the arg firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
    } else {
	len = strlen(str);
    }

    /*
     * convert to json_conv_string() call
     */
    ret = json_conv_string(str, len, quote);
    if (ret == NULL) {
	err(170, __func__, "json_conv_string() returned NULL");
	not_reached();
    }

    /*
     * save length if allowed
     */
    if (retlen != NULL) {
	*retlen = len;
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_bool - convert JSON encoded boolean to C bool
 *
 * given:
 *	str	a JSON boolean
 *	len	length, starting at str, of the JSON boolean
 *
 * returns:
 *	allocated JSON parser tree node converted JSON encoded boolean
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_bool(char const *str, size_t len)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    struct json_boolean *item = NULL;	    /* allocated decoding string or NULL */

    /*
     * allocate the JSON parse tree element
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = calloc(1, sizeof(*ret));
    if (ret == NULL) {
	errp(171, __func__, "calloc #0 error allocating %ju bytes", (uintmax_t)sizeof(*ret));
	not_reached();
    }

    /*
     * initialize the JSON parse tree element
     */
    ret->type = JTYPE_BOOL;
    ret->parent = NULL;
    ret->prev = NULL;
    ret->next = NULL;

    /*
     * initialize the JSON element
     */
    item = &(ret->element.boolean);
    item->as_str = NULL;
    item->converted = false;
    item->value = false;

    /*
     * firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
	return ret;
    }
    if (len <= 0) {
	warn(__func__, "called with len: %ju <= 0", (uintmax_t)len);
	return ret;
    }
    if (str[0] == '\0') {
	warn(__func__, "called with empty string");
	return ret;
    }

    /*
     * duplicate the JSON encoded string
     */
    errno = 0;			/* pre-clear errno for errp() */
    item->as_str = malloc(len+1+1);
    if (item->as_str == NULL) {
	errp(172, __func__, "malloc #1 error allocating %ju bytes", (uintmax_t)(len+1+1));
	not_reached();
    }
    memcpy(item->as_str, str, len+1);
    item->as_str[len] = '\0';	/* paranoia */
    item->as_str[len+1] = '\0';	/* paranoia */
    item->as_str_len = len;

    /*
     * decode the JSON boolean
     */
    if (strcmp(item->as_str, "true") == 0) {
	item->converted = true;
	item->value = true;
    } else if (strcmp(item->as_str, "false") == 0) {
	item->converted = true;
	item->value = false;
    } else {
	warn(__func__, "JSON boolean string neither true nor false: <%s>", item->as_str);
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_bool_str - convert JSON string to C bool
 *
 * This is an simplified interface for json_conv_bool().
 *
 * given:
 *	str	a JSON encoded boolean
 *	retlen	address of where to store length of str, if retlen != NULL
 *
 * returns:
 *	allocated JSON parser tree node converted JSON boolean
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: retlen, if non-NULL, is set to 0 on error
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_bool_str(char const *str, size_t *retlen)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    size_t len = 0;			    /* length of string to encode */

    /*
     * firewall
     *
     * NOTE: We will let the json_conv_bool() handle the arg firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
    } else {
	len = strlen(str);
    }

    /*
     * convert to json_conv_bool() call
     */
    ret = json_conv_bool(str, len);
    if (ret == NULL) {
	err(173, __func__, "json_conv_bool() returned NULL");
	not_reached();
    }

    /*
     * save length if allowed
     */
    if (retlen != NULL) {
	*retlen = len;
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_null - convert JSON encoded nullean to C NULL
 *
 * given:
 *	str	a JSON null
 *	len	length, starting at str, of the JSON null
 *
 * returns:
 *	allocated JSON parser tree node converted JSON encoded null
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_null(char const *str, size_t len)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    struct json_null *item = NULL;	    /* allocated decoding string or NULL */

    /*
     * allocate the JSON parse tree element
     */
    errno = 0;			/* pre-clear errno for errp() */
    ret = calloc(1, sizeof(*ret));
    if (ret == NULL) {
	errp(174, __func__, "calloc #0 error allocating %ju bytes", (uintmax_t)sizeof(*ret));
	not_reached();
    }

    /*
     * initialize the JSON parse tree element
     */
    ret->type = JTYPE_NULL;
    ret->parent = NULL;
    ret->prev = NULL;
    ret->next = NULL;

    /*
     * initialize the JSON element
     */
    item = &(ret->element.null);
    item->as_str = NULL;
    item->value = NULL;

    /*
     * firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
	return ret;
    }
    if (len <= 0) {
	warn(__func__, "called with len: %ju <= 0", (uintmax_t)len);
	return ret;
    }
    if (str[0] == '\0') {
	warn(__func__, "called with empty string");
	return ret;
    }

    /*
     * duplicate the JSON encoded string
     */
    errno = 0;			/* pre-clear errno for errp() */
    item->as_str = malloc(len+1+1);
    if (item->as_str == NULL) {
	errp(175, __func__, "malloc #1 error allocating %ju bytes", (uintmax_t)(len+1+1));
	not_reached();
    }
    memcpy(item->as_str, str, len+1);
    item->as_str[len] = '\0';	/* paranoia */
    item->as_str[len+1] = '\0';	/* paranoia */
    item->as_str_len = len;

    /*
     * decode the JSON nullean
     */
    if (strcmp(item->as_str, "null") == 0) {
	item->converted = true;
	item->value = NULL;
    } else {
	warn(__func__, "JSON null string is not null: <%s>", item->as_str);
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}


/*
 * json_conv_null_str - convert JSON string to C NULL
 *
 * This is an simplified interface for json_conv_null().
 *
 * given:
 *	str	a JSON encoded null
 *	retlen	address of where to store length of str, if retlen != NULL
 *
 * returns:
 *	allocated JSON parser tree node converted JSON null
 *
 * NOTE: It is the responsibility of the calling function to link this
 *	 allocated JSON parser tree node into the JSON parse tree.
 *
 * NOTE: retlen, if non-NULL, is set to 0 on error
 *
 * NOTE: This function will not return on malloc error.
 * NOTE: This function will not return NULL.
 */
struct json *
json_conv_null_str(char const *str, size_t *retlen)
{
    struct json *ret = NULL;		    /* JSON parser tree node to return */
    size_t len = 0;			    /* length of string to encode */

    /*
     * firewall
     *
     * NOTE: We will let the json_conv_null() handle the arg firewall
     */
    if (str == NULL) {
	warn(__func__, "called with NULL str");
    } else {
	len = strlen(str);
    }

    /*
     * convert to json_conv_null() call
     */
    ret = json_conv_null(str, len);
    if (ret == NULL) {
	err(176, __func__, "json_conv_null() returned NULL");
	not_reached();
    }

    /*
     * save length if allowed
     */
    if (retlen != NULL) {
	*retlen = len;
    }

    /*
     * return the JSON parse tree element
     */
    return ret;
}
