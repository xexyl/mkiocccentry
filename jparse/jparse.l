/*
 * JSON scanner - JSON scanner in flex grammar form
 *
 * "Because specs w/o version numbers are forced to commit to their original design flaws." :-)
 *
 * This JSON scanner was co-developed in 2022 by:
 *
 *	@xexyl
 *	https://xexyl.net		Cody Boone Ferguson
 *	https://ioccc.xexyl.net
 * and:
 *	chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * "Because sometimes even the IOCCC Judges need some help." :-)
 *
 * "Share and Enjoy!"
 *     --  Sirius Cybernetics Corporation Complaints Division, JSON spec department. :-)
 */

/*
 * Section 0 - Declarations and option settings
 */

/*
 * The
 *
 *      %option noyywrap
 *
 * prevents needing to link in the flex(1) library (flex(3))
 * which means those without flex can compile the code. Even if everyone had
 * flex(1), though, under macOS you have to pass -ll to the compiler to link in
 * the flex library whereas in other systems you have to use -lfl which would
 * cause a lot of problems. Another way is to provide 'int yywrap() { return 1;
 * }' but this is unnecessary.
 */
%option noyywrap

/*
 * We also need these options set:
 *
 *      %option nodefault
 *      %option nounput
 *      %option noinput
 *      %option yylineno
 *      %option 8bit
 *      %option bison-bridge
 *      %option bison-locations
 *      %option reentrant
 *      %option prefix="jparse_"
 *      %option header-file="jparse.lex.h"
 *      %option extra-type="struct json_extra *"
 */
%option nodefault
%option nounput
%option noinput
%option yylineno
%option 8bit
%option bison-bridge
%option bison-locations
%option reentrant
%option prefix="jparse_"
%option header-file="jparse.lex.h"
%option extra-type="struct json_extra *"

%{
/* Declarations etc. go here.
 *
 * Code is copied verbatim near the top of the generated code.
 */

/*
 * jparse - JSON parser
 */
#include "jparse.h"

extern int jparse_lex(JPARSE_STYPE *yylval_param, JPARSE_LTYPE *yylloc_param, yyscan_t scanner);
static YY_BUFFER_STATE bs;

/*
 * locations in the file / json block
 *
 * Why is it that columns start out at 0 and the line starts out at 1? This is
 * because of the way the scanning works. Whenever a token is matched this macro
 * is run. Each time a token is matched if the column is 0 we have to make sure
 * it is 1 and otherwise we have to increase it: unless it's a tab or a newline.
 * If it's a newline we set the column back to 0 and increase the line. If
 * however it's a tab we increase the column count by 8 as that is the default
 * value for '\t'.
 *
 * Now as far as the final 'else' in the for loop it seems more complicated than
 * it might need to be. This was done during the fixing of error location
 * reporting, specifically with the first column being reported as column 0 and
 * then column 2, but the extra defence in the way it is done does not hurt
 * anything so we have left it as is. Thus if the token is not a newline and not
 * a tab we check if first_column is 0 or if last_column is 0 and if either one
 * is 0, we explicitly set the respective variable to 1. Otherwise we increment
 * the first_column and last_column.
 */
#define YY_USER_ACTION \
			yylloc->filename = yyextra != NULL ? yyextra->filename:""; \
			yylloc->first_line = yylloc->last_line + 1; \
			yylloc->first_column = yylloc->last_column; \
			for (char *p = yytext; *p; ++p) { \
			    if (*p == '\n') { \
				yylloc->last_line++; \
				yylloc->last_column = 0; \
			    } \
			    else if (*p == '\t') { \
				yylloc->last_column += 8; \
			    } \
			    else { \
				if (yylloc->first_column == 0 || yylloc->last_column == 0) { \
				    if (yylloc->first_column == 0) { \
					yylloc->first_column = 1; \
				    } \
				    if (yylloc->last_column == 0) { \
					yylloc->last_column = 1; \
				    } \
				} else { \
				    yylloc->last_column++; \
				    yylloc->first_column++; \
				} \
			    } \
			}
%}

/*
 * Section 1 - Patterns (regular expressions) and actions.
 */

/*
 * JSON_WS              [ \t\r]+
 */
JSON_WS			[ \t\r]+
/*
 * JSON_NL              \n+
 */
JSON_NL			\n+
/*
 * NOTE: on the subject of JSON_STRING one might ask the question about the
 * tighter restrictions on JSON strings and why we don't even consider them.
 * This is a good question but the answer is simple: the JSON string conversion
 * routines actually do these checks. This simplifies the lexer regex and so we
 * don't have to worry about complicating the lexer or parser unnecessarily.
 *
 * We do exclude specific lower bytes but we also pre-scan for these same bytes
 * and if they're there and unescaped we warn about it. The reason to still
 * exclude them is because this way they are not passed to the parser. This
 * might not be necessary because if we find invalid bytes it is an error and
 * the parser will never find them but this is more defensive. For the
 * interested, the old regexp was:
 *
 *	    \"([^\n"]|\\\")*\"
 *
 * ...and this is the current one:
 *
 * JSON_STRING	        \"([^"\x01-\x1f]|\\\")*\"
 */
JSON_STRING		\"([^"\x01-\x1f]|\\\")*\"
/*
 * JSON_INTEGER         -?([1-9][0-9]*|0)
 */
JSON_INTEGER		-?([1-9][0-9]*|0)
/*
 * JSON_FRACTION        "."[0-9]+
 */
JSON_FRACTION		"."[0-9]+
/*
 * JSON_EXPONENT	[Ee][-+]?[0-9]+
 */
JSON_EXPONENT		[Ee][-+]?[0-9]+
/*
 * JSON_NUMBER		({JSON_INTEGER}|{JSON_INTEGER}{JSON_FRACTION}|{JSON_INTEGER}{JSON_FRACTION}{JSON_EXPONENT}|{JSON_INTEGER}{JSON_EXPONENT})
 */
JSON_NUMBER		({JSON_INTEGER}|{JSON_INTEGER}{JSON_FRACTION}|{JSON_INTEGER}{JSON_FRACTION}{JSON_EXPONENT}|{JSON_INTEGER}{JSON_EXPONENT})
/*
 * JSON_TRUE		"true"
 */
JSON_TRUE		"true"
/*
 * JSON_FALSE		"false"
 */
JSON_FALSE		"false"
/*
 * JSON_NULL		"null"
 */
JSON_NULL		"null"
/*
 * JSON_OPEN_BRACE	"{"
 */
JSON_OPEN_BRACE		"{"
/*
 * JSON_CLOSE_BRACE	"}"
 */
JSON_CLOSE_BRACE	"}"
/*
 * JSON_OPEN_BRACKET	"["
 */
JSON_OPEN_BRACKET	"["
/*
 * JSON_CLOSE_BRACKET	"]"
 */
JSON_CLOSE_BRACKET	"]"
/*
 * JSON_COLON		":"
 */
JSON_COLON		":"
/*
 * JSON_COMMA		","
 */
JSON_COMMA		","

/* Actions. */
%%
{JSON_WS}		{
			    /*
			     * Whitespace excluding newlines
			     *
			     * NOTE: we MUST include this action (JSON_WS) as
			     * otherwise the action '.' will return an invalid
			     * token to the parser!
			     *
			     * We don't really need the below debug message but
                             * when we were developing the scanner/parser we
                             * used it to see how many whitespaces were ignored
                             * (depending on if the JSON debug level was high
                             * enough).
			     */
			    (void) json_dbg(JSON_DBG_VVHIGH, __func__, "\nignoring %ju whitespace%s\n",
							     (uintmax_t)yyleng, yyleng==1?"":"s");
			}

{JSON_NL}		{
                            /*
                             * on newline (JSON_NL) we need to reset the column
                             * for error location tracking.
                             */
			    yycolumn = 1;
			}

{JSON_STRING}		{
			    /*
                             * string (JSON_STRING)
                             */
			    return JSON_STRING;
			}

{JSON_NUMBER}		{
			    /*
                             * number (JSON_NUMBER)
                             */
			    return JSON_NUMBER;
			}

{JSON_NULL}		{
			    /*
                             * null object (JSON_NULL)
                             */
			    return JSON_NULL;
			}

{JSON_TRUE}		{
			    /*
                             * true (JSON_TRUE)
                             */
			    return JSON_TRUE;
			}
{JSON_FALSE}		{
			    /*
                             * false (JSON_FALSE)
                             *
                             * NOTE: the fact this is false does not make it
                             * really false, as we do need false! :-)
                             */
			    return JSON_FALSE;
			}

{JSON_OPEN_BRACE}	{
			    /*
                             * start of object - open brace i.e. "{" (JSON_OPEN_BRACE)
                             */
			    return JSON_OPEN_BRACE;
			}
{JSON_CLOSE_BRACE}	{
			    /*
                             * end of object - close brace i.e. "}" (JSON_CLOSE_BRACE)
                             */
			    return JSON_CLOSE_BRACE;
			}

{JSON_OPEN_BRACKET}	{
			    /*
                             * start of array - open bracket i.e. "[" (JSON_OPEN_BRACKET)
                             */
			    return JSON_OPEN_BRACKET;
			}
{JSON_CLOSE_BRACKET}	{
			    /*
                             * end of array - close bracket i.e. "]" (JSON_CLOSE_BRACKET)
                             */
			    return JSON_CLOSE_BRACKET;
			}

{JSON_COLON}		{
			    /*
                             * colon or 'equals' (JSON_COLON)
                             */
			    return JSON_COLON;
			}

{JSON_COMMA}		{
			    /*
                             * comma: name/value pair separator (JSON_COMMA)
                             */
			    return JSON_COMMA;
			}

.			{
			    /*
                             * invalid token: any other character (regexp ".")
                             */
			    dbg(DBG_LOW, "at line %d column %d: invalid token: 0x%02x = <%c>", yylloc->first_line,
                                    yylloc->first_column, *yytext, *yytext);

			    /*
			     * Returning 'token' (return token;) is a hack for
                             * better error messages with invalid tokens. Bison
                             * syntax error messages are in the form of:
			     *
			     *	    syntax error, unexpected <token name>
			     *	    syntax error, unexpected <token name>, expecting } or JSON_STRING
			     *
			     * etc. where <token name> is whatever we return
                             * here in the lexer actions (e.g.  JSON_STRING or
                             * in this case literally token) to the parser. But
                             * the problem is what do we call an invalid token
                             * without knowing what what the token actually is?
                             * Thus we call it token so that it will read
                             * literally as 'unexpected token' which removes any
                             * ambiguity (it could otherwise be read as 'it's
                             * unexpected in this place but it is valid in other
                             * contexts' but it's never actually valid: it's a
                             * catch all for anything that's not valid).
			     *
			     * We also make use of yytext in yyerror() which
                             * makes for a somewhat reasonable error message
                             * even when yyerror() is called from memory
                             * exhaustion. See that function and the comments
                             * for '%token token' in jparse.y.
			     */
			    return token;
			}

%%

/*
 * Section 2 - Code that's copied to the generated scanner
 */

/*
 * low_byte_scan - detect certain low byte values
 *
 * Scan data for bytes in the class: [\x00-\x08\x0e-\x1f]
 *
 * A flex programmer's apology:
 *
 * Whenever a scanner matches a token, the text of the token is stored in
 * the NUL byte terminated string yytext.  The length in yyleng is the same
 * as the value that would be returned by the strlen(3) function.
 * Even though regular expressions can detect the presence of a NUL (\x00) bytes
 * in a block of data, error reporting and internal pointer advancement
 * appears to assume NUL (\x00) terminated strings AND thus the state machine
 * can become "confused" when scanning a block of data with NUL bytes inside
 * instead of just terminating it.
 *
 * To avoid such problems, we pre-scan the block of data for NUL (\x00) bytes
 * and report their presence if we find any NUL (\x00) bytes in data.
 *
 * The so-called JSON spec does not allow byte values [\x00-\x1f] within
 * JSON strings.  Attempts to modify JSON_STRING to exclude such byte values
 * within a JSON string has not worked well, especially when it comes to
 * printing error messages with strings that contain such bytes.
 *
 * To make it easier for the scanner and error messages, we pre-scan data
 * for bytes in the class: [\x00-\x08\x0e-\x1f].  Compared to [\x00-\x1f],
 * this class allows the following subset of whitespace characters:
 *
 *	\x09	Horizontal Tab (\t)
 *	\x0a	Line Feed (\n)
 *	\x0b	Vertical Tab (\v)
 *	\x0c	Form Feed (\f)
 *	\x0d	Carriage Return (\r)
 *
 *	NOTE: Along with \x20 (space) this represents the ASCII whitespace set.
 *	      We avoid scanning for \x20 (space) by limiting our upper exclusion
 *	      range to \x1f.
 *
 * While this subset of whitespace characters are strictly not allowed in
 * JSON strings, we won't scan for them because it might be reasonable for
 * those characters to used outside elements of JSON syntax:
 *
 * We scan data for bytes in the class: [\x00-\x08\x0e-\x1f].  When such
 * bytes are detected, we increment the external variable and we will indicate to
 * the caller that the JSON is invalid by returning true.
 *
 * We also keep track of newlines.  Not to exclude them, but to make it easy
 * for this routine to reference line number and byte position in the line.
 *
 * When a byte in the class: [\x00-\x08\x0e-\x1f] is detected we consider
 * issuing a warning message that indicates the line number, byte position in
 * the line, and byte value.  To avoid printing a huge flood of warning messages
 * (say should a large block of NUL (\x00) and SOM (\x01) bytes are
 * encountered), we limit the number of NUL (\x00) byte warnings to the first
 * MAX_NUL_BYTES_REPORTED, and limit the number of low ([\x01-\x08\x0e-\x1f]) byte
 * warnings to the first MAX_LOW_BYTES_REPORTED.  Should we find more warnings than
 * the limit, a one time "too many" warning is issues.
 *
 * given:
 *
 *	data	    - pointer to block of data to scan for NUL bytes
 *	len	    - length of data in bytes
 *	low_bytes   - pointer to set the number of low bytes found (that aren't NUL) in
 *	nul_bytes   - pointer to set the number of NUL bytes found in
 *
 * return:
 *	true ==> data == NULL OR len <= 0 OR one or more [\x00-\x08\x0e-\x1f]
 *		 bytes are found OR low_bytes == NULL OR nul_bytes == NULL
 *	false ==> data != NULL AND len > 0 AND no NUL [\x00-\x08\x0e-\x1f] bytes were found
 */
static bool
low_byte_scan(char const *data, size_t len, size_t *low_bytes, size_t *nul_bytes)
{
    size_t linenum = 0;	    /* number of newline terminated lines we have processed */
    size_t byte_pos = 0;    /* byte position within the current line number */
    bool nul_limit = false;	/* true ==> found too many NUL (\x00) bytes to report all */
    bool low_limit = false;	/* true ==> found too many low ([\x01-\x08\x0e-\x1f]) bytes to report all */
    size_t i = 0;

    /*
     * firewall
     */
    if (data == NULL) {
	werr(30, __func__, "data is NULL");
	return true;
    }
    if (len <= 0) {
	werr(31, __func__, "len: %zu <= 0 ", len);
	return true;
    }
    if (low_bytes == NULL) {
	werr(32, __func__, "low_bytes is NULL");
	return true;
    } else {
	/* set *low_bytes to 0 */
	*low_bytes = 0;
    }
    if (nul_bytes == NULL) {
	werr(33, __func__, "nul_bytes is NULL"); /* nul_bytes is ironically NULL! :-) */
	return true;
    } else {
	*nul_bytes = 0;
    }

    /*
     * start with the first line number
     */
    ++linenum;

    /*
     * scan the data
     */
    for (i=0; i < len; ++i) {

	/* count this byte within the line */
	++byte_pos;

	/*
	 * process the current data byte
	 */
        switch (data[i]) {
	case '\0':	/* NUL byte */

	    /* count NUL byte */
	    ++(*nul_bytes);

	    /*
	     * case below the report limit
	     */
	    if (*nul_bytes <= MAX_NUL_BYTES_REPORTED) {
		werr(34, __func__, "invalid NUL (0) byte detected in line: %zu byte position: %zu",
			 linenum, byte_pos);

	    /*
	     * case at or above the report limit
	     */
	    } else if (nul_limit == false) {
		werr(35, __func__, "too many NUL ('\\0') bytes detected: "
			 "additional NUL byte reports disabled");
		nul_limit = true;	/* report reaching report limit only once */
	    }
	    break;

	/*
	 * For grins, we document some of the historical byte meanings :-)
	 */
	case '\x01':	/* SOH - Start of Heading */
	case '\x02':	/* EOA - Start of Text */
	case '\x03':	/* EOM - End of Text */
	case '\x04':	/* EOT - End of Transmission */
	case '\x05':	/* ENQ - Enquiry */
	case '\x06':	/* ACK - Acknowledgement */
	case '\x07':	/* BEL - Bell */
	case '\x08':	/* BS - Backspace */
	case '\x0e':	/* SO - Shift Out */
	case '\x0f':	/* SI - Shift In */
	case '\x10':	/* DCO / DLE - Data Link Escape */
	case '\x11':	/* DC1 / XON - Device Control 1 */
	case '\x12':	/* DC2 - Device Control 2 */
	case '\x13':	/* DC3 / XOFF - Device Control 3 */
	case '\x14':	/* DC4 - Device Control 4 */
	case '\x15':	/* ERR / NAK - Negative Acknowledgement */
	case '\x16':	/* SYNC / SYN - Synchronous Idle */
	case '\x17':	/* LEM / ETB - End of Transmission Block */
	case '\x18':	/* S0 / CAN - Cancel */
	case '\x19':	/* S1 / EM - End of Medium */
	case '\x1a':	/* S2 / SS / SUB - Substitute */
	case '\x1b':	/* S3 / ESC - Escape */
	case '\x1c':	/* S4 / FS - File Separator */
	case '\x1d':	/* S5 / GS - Group Separator */
	case '\x1e':	/* S6 / RS - Record Separator */
	case '\x1f':	/* S7 / US - Unit Separator */

	    /* count low ([\x01-\x08\x0e-\x1f]) byte */
	    ++(*low_bytes);

	    /*
	     * case: below the report limit
	     */
	    if (*low_bytes <= MAX_LOW_BYTES_REPORTED) {
		werr(36, __func__, "invalid LOW byte 0x%02x detected in line: %zu byte position: %zu",
			 data[i], linenum, byte_pos);

	    /*
	     * case: at or above the report limit
	     */
	    } else if (low_limit == false) {
		werr(37, __func__, "too many LOW bytes ([\\x01-\\x08\\x0e-\\x1f] detected: "
		         "additional low byte reports disabled");
		low_limit = true;	/* report reaching report limit only once */
	    }
	    break;

	case '\n':	/* \x0a - LF - Line Feed / Newline */

	    /* count newline */
	    ++linenum;
	    byte_pos = 0;	/* reset byte position */
	    break;

	default:	/* normal data byte */
	    break;
	}
    }

    /*
     * return the NUL detection status
     */
    if (*nul_bytes > 0 || *low_bytes > 0) {
	return true;
    }
    return false;
}


/*
 * parse_json - parse a JSON file or string of a given length
 *
 * Given a pointer to char and a length, use the parser to determine if the JSON
 * is valid or not.
 *
 * given:
 *
 *	ptr	    - pointer to start of JSON blob
 *	len	    - length of the JSON blob
 *	filename    - filename, empty string or "-" for stdin or NULL to
 *	              indicate it is a string
 *	is_valid    - non-NULL pointer to boolean to set depending on JSON validity
 *
 * return:
 *	pointer to a JSON parse tree
 *
 * NOTE: the reason this is in the scanner and not the parser is because
 * YY_BUFFER_STATE is part of the scanner and not the parser.
 *
 * NOTE: if filename is NULL, it will be set to "-" for stdin but if it is an
 * empty string any error message will report it as a string (i.e. it will not
 * show that it's a file).
 *
 * NOTE: this function only warns on error, except for NULL is_valid, in which
 *       case it is an error; warning on errors is so that an entire report of
 *       all the problems can be given at the end if the verbosity level is high
 *       enough (or otherwise if this information is requested).
 */
struct json *
parse_json(char const *ptr, size_t len, char const *filename, bool *is_valid)
{
    struct json *tree = NULL;		/* the JSON parse tree */
    int ret = 0;			/* jparse_parse() return value */
    yyscan_t scanner;			/* scanner instance: is a void * */
    struct json_extra extra;

    /*
     * firewall
     */
    if (is_valid == NULL) {
	err(38, __func__, "is_valid == NULL");
	not_reached();
    } else {
	/*
	 * assume the JSON block is is valid; set *is_valid to true so that the
         * caller does not need to worry about it (well they should in case this
         * is removed but we do it explicitly for them anyway).
	 */
	*is_valid = true;
    }

    if (filename != NULL && *filename == '-') {
        filename = "-";
    }

    /*
     * firewall
     */
    if (ptr == NULL) {
	werr(39, __func__, "ptr is NULL");

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * initialise scanner
     */
    errno = 0;
    ret = jparse_lex_init_extra(&extra, &scanner);
    if (ret != 0) {
	werrp(40, __func__, "jparse_lex_init_extra failed");

        /*
         * if jparse_lex_init_extra() reports an error (!= 0) then *is_valid
         * must be set to false and we must return NULL
         */
        *is_valid = false;
	return NULL;
    }
    extra.filename = filename;

    /*
     * scan the blob
     */
    bs = yy_scan_bytes(ptr, (int)len, scanner);
    if (bs == NULL) {
	/*
	 * if unable to scan the bytes it indicates an internal error and
	 * perhaps it should call err() instead but for now we make it a
	 * non-fatal error as well.
	 */
	werr(41, __func__, "unable to scan string");

	/*
         * since we cannot scan the bytes, we set *is_valid to false, even
         * though it could very well be valid if it could be scanned.
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	jparse_lex_destroy(scanner);
	return tree;
    }

    /*
     * we cannot set the column (and probably line number) without first having
     * a buffer which is why the yy_scan_bytes() is called first (see above).
     *
     * For why we set the column to 0 but the line to 1 see the comments with
     * the YY_USER_ACTION macro.
     */
    jparse_set_column(0, scanner);
    jparse_set_lineno(1, scanner);

    /*
     * announce beginning of parse, if JSON debug level is high enough
     */
    if (json_dbg_allowed(JSON_DBG_VVHIGH)) {
	fprstr(stderr, "*** BEGIN PARSE\n");
	fprstr(stderr, "<\n");
	(void) fprint_line_buf(stderr, ptr, len, 0, 0);
	fprstr(stderr, "\n>\n");
    }

    /*
     * parse the blob, passing into the parser the node and scanner
     */
    ret = jparse_parse(&tree, scanner);

    /*
     * free memory associated with bytes scanned by yy_scan_bytes()
     */
    yy_delete_buffer(bs, scanner);
    bs = NULL;

    /*
     * announce end of parse, if JSON debug level is high enough
     */
    if (json_dbg_allowed(JSON_DBG_VVHIGH)) {
	fprstr(stderr, "*** END PARSE\n");
    }

    /*
     * report scanner / parser success or failure, if JSON debug level is high
     * enough
     */
    if (json_dbg_allowed(JSON_DBG_VVHIGH)) {
	json_dbg(JSON_DBG_VVHIGH, __func__, "jparse_parse() returned: %d", ret);
    }

    if (ret == 0) {
        /*
         * we will only report valid JSON (whatever that means :-) ) in debug
         * message if JSON debug level is high enough (where high is low :-)
         */
        if (json_dbg_allowed(JSON_DBG_LOW)) {
	    json_dbg(JSON_DBG_LOW, __func__, "valid JSON");
        }
        /*
         * flag that we have valid JSON, even though we assumed it will be
         */
	*is_valid = true;
    } else {
        /*
         * we will only report invalid JSON in debug message if JSON debug level
         * is high enough (where high is low :-)
         */
        if (json_dbg_allowed(JSON_DBG_LOW)) {
	    json_dbg(JSON_DBG_LOW, __func__, "invalid JSON");
        }

        /*
         * whereas if it's valid JSON it is redundant to set *is_valid, here we
         * must set *is_valid to false as we always assume at first that it will
         * be valid JSON (whatever that means :-) )
         */
	*is_valid = false;
    }

    /*
     * free the scanner of the so-called JSON <del>spec</del> nightmare
     */
    jparse_lex_destroy(scanner);

    /*
     * return parse tree
     */
    return tree;
}

/*
 * parse_json_str - parse a string of a given length as JSON
 *
 * Given a pointer to char and a length, use the parser to determine if the JSON
 * is valid or not.
 *
 * This function assumes that the JSON is a string, rather than a file to be
 * read, and it uses parse_json() with a NULL filename. Thus it can be seen as
 * a simplified interface to parse_json() in the case of a string rather than a
 * file. The error function checks if the filename is != NULL and if *filename
 * != '\0' before claiming it is a file. The function parse_json() will, if
 * filename != NULL && *filename == '-', set filename to "-", indicating the
 * input comes from stdin.
 *
 * given:
 *
 *	ptr	    - pointer to start of JSON blob
 *	len	    - length of the JSON blob
 *	is_valid    - non-NULL pointer to boolean to set depending on JSON validity
 *
 * return:
 *	pointer to a JSON parse tree
 *
 * NOTE: the reason this is in the scanner and not the parser is because
 * YY_BUFFER_STATE is part of the scanner and not the parser.
 *
 * NOTE: this function only warns on error, except for NULL is_valid, in which
 *       case it is an error; warning on errors is so that an entire report of
 *       all the problems can be given at the end if the verbosity level is high
 *       enough (or otherwise if this information is requested).
 */
struct json *
parse_json_str(char const *ptr, size_t len, bool *is_valid)
{
    struct json *tree = NULL;   /* for a blank JSON tree if certain errors */

    /*
     * firewall
     */
    if (is_valid == NULL) {
	err(42, __func__, "is_valid == NULL");
	not_reached();
    } else {
	/*
	 * assume the JSON block is is valid; set *is_valid to true so that the
         * caller does not need to worry about it (well they should in case this
         * is removed but we do it explicitly for them anyway).
	 */
	*is_valid = true;
    }

    /*
     * firewall
     */
    if (ptr == NULL) {
	werr(43, __func__, "ptr is NULL");

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * return a call to parse_json() with NULL filename
     */
    return parse_json(ptr, len, NULL, is_valid);
}



/*
 * parse_json_stream - parse an open file stream for JSON data
 *
 * Given an open file stream, we will read in all data (until EOF) and
 * then parse that data as if it were JSON.
 *
 * given:
 *	stream      - open file stream containing JSON data
 *	filename    - name of file or NULL for stdin
 *	is_valid    - non-NULL printer to boolean to set depending on json validity
 *
 * return:
 *	pointer to a JSON parse tree
 *
 * If stream is NULL or stream is not open, or if read_all() fails,
 * then this function warns and sets *is_valid to false.
 *
 * NOTE: the reason this is in the scanner and not the parser is because
 *	 YY_BUFFER_STATE is part of the scanner and not the parser and that's required
 *	 for the parse_json() function.
 *
 * NOTE: if filename is "-" and stream is NULL, then the stream is set to stdin,
 * as if the filename was NULL.
 *
 * NOTE: this function only warns on error, except for NULL is_valid, in which
 *       case it is an error; warning on errors is so that an entire report of
 *       all the problems can be given at the end if the verbosity level is high
 *       enough (or otherwise if this information is requested).
 */
struct json *
parse_json_stream(FILE *stream, char const *filename, bool *is_valid)
{
    struct json *tree = NULL;		/* the JSON parse tree */
    char *data = NULL;			/* used to determine if there are NUL bytes in the file */
    size_t len = 0;			/* length of data read */
    bool low_byte_detected = false;	/* true ==> byte values [\x00-\x08\x0e-\x1f] detected in data */
    size_t nul_bytes = 0;		/* number of NUL bytes found */
    size_t low_bytes = 0;		/* number of low bytes that are not NUL found */

    /*
     * firewall
     */
    if (is_valid == NULL) {
	err(44, __func__, "is_valid == NULL");
	not_reached();
    } else {
	/*
	 * assume the JSON stream is is valid JSON; set *is_valid to true so
         * that the caller does not need to worry about it (well they should in
         * case this is removed but we do it explicitly for them anyway).
	 */
	*is_valid = true;
    }

    if (filename == NULL) {
	json_dbg(JSON_DBG_HIGH, __func__, "filename is NULL, forcing it to be \"-\" for stdin");
	filename = "-";
	stream = stdin;
    } else if (!strcmp(filename, "-") && stream == NULL) {
	stream = stdin;
    }

    if (stream == NULL) {

	/*
         * report NULL stream
         */
	werr(45, __func__, "stream is NULL");

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * if stream is not stdin check if the stream is open
     */
    if (stream != stdin && fd_is_ready(__func__, false, fileno(stream)) == false) {

	/*
         * report closed stream
         */
	werr(46, __func__, "stream is not open");

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * read the entire file into memory
     */
    data = read_all(stream, &len);
    if (data == NULL) {

	/*
         * warn about read error
         */
	werr(47, __func__, "could not read read stream");
        /*
         * we need to clearerr() or fclose(), depending on the stream.
         */
	clearerr_or_fclose(stream);

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/* return a blank JSON tree */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * pre-scan data for byte values in range of [\x00-\x08\x0e-\x1f]
     */
    low_byte_detected = low_byte_scan(data, len, &low_bytes, &nul_bytes);
    if (low_byte_detected || low_bytes > 0 || nul_bytes > 0) {

	/*
         * report invalid bytes
         */
	if (low_bytes > 0 && nul_bytes > 0) {
	    werr(48, __func__, "%ju low byte%s and %ju NUL byte%s detected: data block is NOT valid JSON",
		    (uintmax_t)low_bytes, low_bytes > 1 ? "s":"",
		    (uintmax_t)nul_bytes, nul_bytes > 1 ? "s":"");
	} else if (low_bytes > 0) {
	    werr(49, __func__, "%ju low byte%s detected: data block is NOT valid JSON",
		    (uintmax_t)low_bytes, low_bytes > 1 ? "s":"");
	} else if (nul_bytes > 0) {
	    werr(50, __func__, "%ju NUL byte%s detected: data block is NOT valid JSON",
		    (uintmax_t)nul_bytes, nul_bytes > 1 ? "s":"");
	}

	/*
         * clearerr() or fclose() depending on stream
         */
	clearerr_or_fclose(stream);

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/* free data */
	if (data != NULL) {
	    free(data);
	    data = NULL;
	}

	/* return a blank JSON tree */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * if JSON debug level is high enough we will log the call with the length
     */
    if (json_dbg_allowed(JSON_DBG_HIGH)) {
        json_dbg(JSON_DBG_HIGH, __func__, "calling parse_json on data block with length %ju:", (uintmax_t)len);
    }

    /*
     * JSON parse the data from the file
     */
    tree = parse_json(data, len, filename, is_valid);

    /* free data */
    if (data != NULL) {
	free(data);
	data = NULL;
    }

    /*
     * clear error or close stream, depending on stream
     */
    clearerr_or_fclose(stream);
    stream = NULL;

    /*
     * return the JSON parse tree tree
     */
    return tree;
}


/*
 * parse_json_file	    - parse a JSON file in a given filename
 *
 * given:
 *	filename    - filename of file to parse
 *	is_valid    - non-NULL printer to boolean to set depending on json validity
 *
 * return:
 *	pointer to a JSON parse tree
 *
 * If name is NULL or the name is not a readable file (or is empty) or if
 * read_all() fails, then this function warns and sets *is_valid to false.
 *
 * NOTE: The reason this is in the scanner and not the parser is because
 *	 YY_BUFFER_STATE is part of the scanner and not the parser and that's required
 *	 for the parse_json() function.
 *
 * NOTE: This function only warns on error. It does this via the called function
 *	 parse_json(). This is done so that an entire report of all the problems can
 *	 be given at the end if the verbosity level is high enough (or otherwise if
 *	 this information is requested).
 */
struct json *
parse_json_file(char const *filename, bool *is_valid)
{
    struct json *tree = NULL;		/* the JSON parse tree */
    FILE *stream = NULL;		/* file stream to read from */

    /*
     * firewall
     */
    if (is_valid == NULL) {
	err(51, __func__, "is_valid == NULL");
	not_reached();
    } else {
	/*
	 * assume the JSON file is is valid JSON; set *is_valid to true so
         * that the caller does not need to worry about it (well they should in
         * case this is removed but we do it explicitly for them anyway).
	 */
	*is_valid = true;
    }
    if (filename == NULL) {
	werr(52, __func__, "passed NULL filename");

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }
    if (*filename == '\0') { /* strlen(filename) == 0 */

	/*
         * warn about bogus filename
         */
	werr(53, __func__, "passed empty filename");

	/*
         * flag that we have invalid JSON
         */
	*is_valid = false;

	/*
         * return a blank JSON tree
         */
	tree = json_alloc(JTYPE_UNSET);
	return tree;
    }

    /*
     * if file is "-", then we will parse stdin
     */
    if (strcmp(filename, "-") == 0) {
	stream = stdin;

    /*
     * case: will read from a file
     */
    } else {

	/*
	 * validate filename
	 */
	if (!exists(filename)) {
	    /*
             * report missing file
             */
	    werr(54, __func__, "passed filename that's not actually a file: %s", filename);

	    /*
             * flag that we have invalid JSON
             */
	    *is_valid = false;

	    /*
             * return a blank JSON tree
             */
	    tree = json_alloc(JTYPE_UNSET);
	    return tree;

	}
	if (!is_file(filename)) {
	    /*
             * report that file is not a normal file
             */
	    werr(55, __func__, "passed filename not a normal file: %s", filename);

	    /*
             * report invalid JSON
             */
	    *is_valid = false;

	    /*
             * return a blank JSON tree
             */
	    tree = json_alloc(JTYPE_UNSET);
	    return tree;
	}
	if (!is_read(filename)) {
	    /*
             * report unreadable file
             */
	    werr(56, __func__, "passed filename not a readable file: %s", filename);

	    /*
             * flag that we have invalid JSON
             */
	    *is_valid = false;

	    /*
             * return a blank JSON tree
             */
	    tree = json_alloc(JTYPE_UNSET);
	    return tree;
	}

	/*
	 * open file for scanner to use
	 */
	errno = 0;
	stream = fopen(filename, "r");
	if (stream == NULL) {

	    /*
             * warn about file open error
             */
	    werrp(57, __func__, "couldn't open file %s, ignoring", filename);

	    /*
             * flag that we have invalid JSON
             */
	    *is_valid = false;

	    /*
             * return a blank JSON tree
             */
	    tree = json_alloc(JTYPE_UNSET);
	    return tree;
	}
    }

    /*
     * JSON parse the open stream
     */
    tree = parse_json_stream(stream, filename, is_valid);

    /*
     * return the JSON parse tree tree
     */
    return tree;
}
