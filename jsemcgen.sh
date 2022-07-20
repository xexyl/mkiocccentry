#!/usr/bin/env bash
#
# jsemgen.sh - generate JSON semantics table
#
# Copyright (c) 2022 by Landon Curt Noll.  All Rights Reserved.
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright, this permission notice and text
# this comment, and the disclaimer below appear in all of the following:
#
#       supporting documentation
#       source copies
#       source works derived from this source
#       binaries derived from this source or from derived source
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
#
# Share and enjoy! :-)

# setup
#
PATCH_TOOL="$(type -P patch)"
export USAGE="usage: $0 [-h] [-v level] [-J level] [-q] [-V] [-s] [-I] [-N name] [-D def_func] [-P prefix]
	[-1 func] [-S func] [-B func] [-0 func] [-M func] [-O func] [-A func] [-U func]
	[-j jsemtblgen] [-p patch_tool] file.json head patch tail

	-h		print help message and exit 2
	-v level	set verbosity level (def level: 0)
	-J level	set JSON verbosity level (def level: 0)
	-q		quiet mode: silence msg(), warn(), warnp() if -v 0 (def: not quiet)
	-V		print version string and exit 2
	-s		file.json is a string (def: arg is a filename)

	-I		output as .h include file (def: output as .c src)

	-N name		name of the semantics table (def: sem_tbl)

	-D def_func	validate with def_func() unless overridden by another flag (def: NULL)

	-P prefix	validate JTYPE_MEMBER JSON nodes with prefix_name() (def: do not)

			NOTE: The name is based on the JTYPE_MEMBER JSON decoded name string.
			NOTE: Underscore (_) replaces any name chars that are not valid in a C function name.
			NOTE: -P overrides any use of -M.

	-1 func		validate JTYPE_NUMBER JSON nodes with func() (def: do not)
	-S func		validate JTYPE_STRING JSON nodes with func() (def: do not)
	-B func		validate JTYPE_BOOL JSON nodes with func() (def: do not)
	-0 func		validate JTYPE_NULL JSON nodes with func() (def: do not)
	-M func		validate JTYPE_MEMBER JSON nodes with func() (def: do not)
	-O func		validate JTYPE_OBJECT JSON nodes with func() (def: do not)
	-A func		validate JTYPE_ARRAY JSON nodes with func() (def: do not)
	-U func		validate nodes with unknown types with func() (def: do not)

	NOTE: All of the above are passed down to the jsemtblgen tool.
	      All of the below are only used by this script.

	-j jsemtblgen	path to jsemtblgen (def: ./jsemtblgen)
	-p patch_tool	path to the patch tool (def: $PATCH_TOOL)

	file.json	A valid JSON file used to generate the initial JSON semantics table
	head		file add to the beginning jsemtblgen output (before patched jsemtblgen output), . ==> do not add
	patch		patch to apply to the output of jsemtblgen, . --> do not patch
	tail		file add to the end of jsemtblgen output (after patched jsemtblgen output), . ==> do not add

exit codes:
    0	JSON is valid
    1	JSON is invalid
    2	-h and help string printed or -V and version string printed
    3	command line error
    4	jsemtblgen or patch_tool is not an executable
    5	file.json, head, patch, and/or tail is not a readable file
    6	jsemtblgen failed
    >=7	internal error"
export V_FLAG="0"
export J_FLAG="0"
export Q_FLAG=
export CAP_V_FLAG=
export S_FLAG=
export I_FLAG=
export NAME=
export DEFAULT_FUNC=
export PREFIX=
export NUMBER_FUNC=
export STRING_FUNC=
export BOOL_FUNC=
export NULL_FUNC=
export MEMBER_FUNC=
export OBJECT_FUNC=
export ARRAY_FUNC=
export UNKNOWN_FUNC=
export JSEMTBLGEN="./jsemtblgen"
export JSEMTBLGEN_ARGS=
export PATCH_TOOL

# parse args
#
while getopts :hv:J:qVsIN:D:P:1:S:B:0:M:O:A:U:j:p: flag; do
    case "$flag" in
    h) echo "$USAGE" 1>&2
       exit 2
       ;;
    v) V_FLAG="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -v '$V_FLAG'";
       ;;
    J) J_FLAG="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -J '$J_FLAG'";
       ;;
    q) Q_FLAG="-q";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -q";
       ;;
    V) CAP_V_FLAG="-V";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -V";
       ;;
    s) S_FLAG="-s";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -s";
       ;;
    I) I_FLAG="-I";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -I";
       ;;
    N) NAME="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -N '$NAME'";
       ;;
    D) DEFAULT_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -D '$DEFAULT_FUNC'";
       ;;
    P) PREFIX="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -P '$PREFIX'";
       ;;
    1) NUMBER_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -1 '$NUMBER_FUNC'";
       ;;
    S) STRING_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -S '$STRING_FUNC'";
       ;;
    B) BOOL_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -B '$BOOL_FUNC'";
       ;;
    0) NULL_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -0 '$NULL_FUNC'";
       ;;
    M) MEMBER_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -M '$MEMBER_FUNC'";
       ;;
    O) OBJECT_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -O '$OBJECT_FUNC'";
       ;;
    A) ARRAY_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -A '$ARRAY_FUNC'";
       ;;
    U) UNKNOWN_FUNC="$OPTARG";
       JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -U '$UNKNOWN_FUNC'";
       ;;
    j) JSEMTBLGEN="$OPTARG";
       ;;
    p) PATCH_TOOL="$OPTARG";
       ;;
    \?) echo "$0: ERROR: invalid option: -$OPTARG" 1>&2
       exit 3
       ;;
    :) echo "$0: ERROR: option -$OPTARG requires an argument" 1>&2
       exit 3
       ;;
   *)
       ;;
    esac
done

# check args
#
shift $(( OPTIND - 1 ));
if [[ $# -ne 4 ]]; then
    echo "$0: ERROR: expected 4 arguments, found $#" 1>&2
    echo "$USAGE" 1>&2
    exit 3
fi
JSON_FILE="$1"
JSEMTBLGEN_ARGS="$JSEMTBLGEN_ARGS -- '$JSON_FILE'"
HEAD_FILE="$2"
PATCH_FILE="$3"
TAIL_FILE="$4"
export JSON_FILE HEAD_FILE PATCH_FILE TAIL_FILE OUT_FILE
if [[ $HEAD_FILE = "." ]]; then
    HEAD_FILE=
fi
if [[ $PATCH_FILE = "." ]]; then
    PATCH_FILE=
fi
if [[ $TAIL_FILE = "." ]]; then
    TAIL_FILE=
fi
if [[ $V_FLAG -ge 5 ]]; then
    echo "$0: debug[5]: V_FLAG: $V_FLAG" 1>&2
    echo "$0: debug[5]: J_FLAG: $J_FLAG" 1>&2
    echo "$0: debug[5]: Q_FLAG: $Q_FLAG" 1>&2
    echo "$0: debug[5]: CAP_V_FLAG: $CAP_V_FLAG" 1>&2
    echo "$0: debug[5]: S_FLAG: $S_FLAG" 1>&2
    echo "$0: debug[5]: I_FLAG: $I_FLAG" 1>&2
    echo "$0: debug[5]: NAME: $NAME" 1>&2
    echo "$0: debug[5]: DEFAULT_FUNC: $DEFAULT_FUNC" 1>&2
    echo "$0: debug[5]: PREFIX: $PREFIX" 1>&2
    echo "$0: debug[5]: NUMBER_FUNC: $NUMBER_FUNC" 1>&2
    echo "$0: debug[5]: STRING_FUNC: $STRING_FUNC" 1>&2
    echo "$0: debug[5]: BOOL_FUNC: $BOOL_FUNC" 1>&2
    echo "$0: debug[5]: NULL_FUNC: $NULL_FUNC" 1>&2
    echo "$0: debug[5]: MEMBER_FUNC: $MEMBER_FUNC" 1>&2
    echo "$0: debug[5]: OBJECT_FUNC: $OBJECT_FUNC" 1>&2
    echo "$0: debug[5]: ARRAY_FUNC: $ARRAY_FUNC" 1>&2
    echo "$0: debug[5]: UNKNOWN_FUNC: $UNKNOWN_FUNC" 1>&2
    echo "$0: debug[5]: jsemtblgen: $JSEMTBLGEN" 1>&2
    echo "$0: debug[5]: jsemtblgen args: $JSEMTBLGEN_ARGS" 1>&2
    echo "$0: debug[5]: file.json file: $JSON_FILE" 1>&2
    if [[ -z $HEAD_FILE ]]; then
	echo "$0: debug[5]: no head file" 1>&2
    else
	echo "$0: debug[5]: head file: $HEAD_FILE" 1>&2
    fi
    if [[ -z $PATCH_FILE ]]; then
	echo "$0: debug[5]: no patch file" 1>&2
    else
	echo "$0: debug[5]: patch file: $PATCH_FILE" 1>&2
    fi
    if [[ -z $TAIL_FILE ]]; then
	echo "$0: debug[5]: no tail file" 1>&2
    else
	echo "$0: debug[5]: tail file: $TAIL_FILE" 1>&2
    fi
    echo "$0: debug[5]: out file: $OUT_FILE" 1>&2
fi

# firewall
#
if [[ ! -x $JSEMTBLGEN ]]; then
    echo "$0: ERROR: jsemtblgen is not an executable: $JSEMTBLGEN" 1>&2
    exit 4
fi
if [[ ! -x $PATCH_TOOL ]]; then
    echo "$0: ERROR: patch tool is not an executable: $PATCH_TOOL" 1>&2
    exit 4
fi
if [[ ! -e $JSON_FILE ]]; then
    echo "$0: ERROR: file.json does not exist: $JSON_FILE" 1>&2
    exit 5
fi
if [[ ! -f $JSON_FILE ]]; then
    echo "$0: ERROR: file.json is not a file: $JSON_FILE" 1>&2
    exit 5
fi
if [[ ! -r $JSON_FILE ]]; then
    echo "$0: ERROR: file.json is not a readable file: $JSON_FILE" 1>&2
    exit 5
fi
if [[ -n $HEAD_FILE ]]; then
    if [[ ! -e $HEAD_FILE ]]; then
	echo "$0: ERROR: head does not exist: $HEAD_FILE" 1>&2
	exit 5
    fi
    if [[ ! -f $HEAD_FILE ]]; then
	echo "$0: ERROR: head is not a file: $HEAD_FILE" 1>&2
	exit 5
    fi
    if [[ ! -r $HEAD_FILE ]]; then
	echo "$0: ERROR: head is not a readable file: $HEAD_FILE" 1>&2
	exit 5
    fi
fi
if [[ -n $PATCH_FILE ]]; then
    if [[ ! -e $PATCH_FILE ]]; then
	echo "$0: ERROR: patch does not exist: $HEAD_FILE" 1>&2
	exit 5
    fi
    if [[ ! -f $PATCH_FILE ]]; then
	echo "$0: ERROR: patch is not a file: $PATCH_FILE" 1>&2
	exit 5
    fi
    if [[ ! -r $PATCH_FILE ]]; then
	echo "$0: ERROR: patch is not a readable file: $PATCH_FILE" 1>&2
	exit 5
    fi
fi
if [[ -n $TAIL_FILE ]]; then
    if [[ ! -e $TAIL_FILE ]]; then
	echo "$0: ERROR: tail does not exist: $TAIL_FILE" 1>&2
	exit 5
    fi
    if [[ ! -f $TAIL_FILE ]]; then
	echo "$0: ERROR: tail is not a file: $TAIL_FILE" 1>&2
	exit 5
    fi
    if [[ ! -r $TAIL_FILE ]]; then
	echo "$0: ERROR: tail is not a readable file: $TAIL_FILE" 1>&2
	exit 5
    fi
fi

# output the header
#
if [[ -n "$HEAD_FILE" ]]; then
    cat "$HEAD_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: cat $HEAD_FILE exit status: $status" 1>&2
	exit 7
    fi
fi

# case: jsemtblgen is patched
#
if [[ -n "$PATCH_FILE" ]]; then

    # obtain a temporary filenames
    #
    export MKTEMP_TEMPLATE TMP_FILE
    MKTEMP_TEMPLATE=".jsemcgen.out.XXXXXXXXXX"
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: about to run: mktemp $MKTEMP_TEMPLATE" 1>&2
    fi
    TMP_FILE=$(mktemp "$MKTEMP_TEMPLATE")
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: mktemp $MKTEMP_TEMPLATE exit code: $status" 1>&2
	exit 8
    fi
    if [[ ! -e $TMP_FILE ]]; then
	echo "$0: ERROR: tmp file not found: $TMP_FILE" 1>&2
	exit 9
    fi
    if [[ ! -f $TMP_FILE ]]; then
	echo "$0: ERROR: tmp not a regular file: $TMP_FILE" 1>&2
	exit 10
    fi
    if [[ ! -r $TMP_FILE ]]; then
	echo "$0: ERROR: tmp not a readable file: $TMP_FILE" 1>&2
	exit 11
    fi
    if [[ ! -w $TMP_FILE ]]; then
	echo "$0: ERROR: tmp not a writable file: $TMP_FILE" 1>&2
	exit 12
    fi
    trap "rm -f \$TMP_FILE; exit" 1 2 3 15
    export ORIG_FILE="$TMP_FILE.orig"
    export REJ_FILE="$TMP_FILE.rej"

    # write the jsemtblgen generated code to a temporary file
    #
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[1]: about to run: $JSEMTBLGEN $JSEMTBLGEN_ARGS > $TMP_FILE" 1>&2
    fi
    eval "$JSEMTBLGEN" "$JSEMTBLGEN_ARGS" > "$TMP_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: jsemtblgen failed, exit status: $status" 1>&2
	exit 6
    fi
    if [[ ! -e $TMP_FILE ]]; then
	echo "$0: ERROR: jsemtblgen tmp file does not exist: $TMP_FILE" 1>&2
	exit 13
    fi
    if [[ ! -f $TMP_FILE ]]; then
	echo "$0: ERROR: jsemtblgen tmp file is not a file: $TMP_FILE" 1>&2
	exit 14
    fi
    if [[ ! -r $TMP_FILE ]]; then
	echo "$0: ERROR: jsemtblgen tmp file is not a readable file: $TMP_FILE" 1>&2
	exit 15
    fi
    if [[ ! -w $TMP_FILE ]]; then
	echo "$0: ERROR: jsemtblgen tmp file is not a writable file: $TMP_FILE" 1>&2
	exit 16
    fi

    # patch the temporary file
    #
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: about to run: rm -f $ORIG_FILE $REJ_FILE" 1>&2
    fi
    rm -f "$ORIG_FILE" "$REJ_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: rm -f $ORIG_FILE $REJ_FILE filed, exit status: $status" 1>&2
	exit 17
    fi
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: about to run: $PATCH_TOOL $TMP_FILE $PATCH_FILE" 1>&2
    fi
    "$PATCH_TOOL" "$TMP_FILE" "$PATCH_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: patch failed, exit status: $status" 1>&2
	exit 18
    fi
    if [[ -s $REJ_FILE ]]; then
	echo "$0: ERROR: patch did not succeed, reject file found: $REJ_FILE" 1>&2
	exit 19
    fi

    # output patched jsemtblgen generated code
    #
    cat "$TMP_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: cat $TMP_FILE exit status: $status" 1>&2
	exit 20
    fi

    # cleanup
    #
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: about to run: rm -f $TMP_FILE $ORIG_FILE $REJ_FILE" 1>&2
    fi
    rm -f "$TMP_FILE" "$ORIG_FILE" "$REJ_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: rm -f $TMP_FILE $ORIG_FILE $REJ_FILE filed, exit status: $status" 1>&2
	exit 21
    fi
    trap - 1 2 3 15

# case: jsemtblgen is not patched
#
else

    # just output unpatched jsemtblgen generated code
    #
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[1]: about to run: $JSEMTBLGEN $JSEMTBLGEN_ARGS" 1>&2
    fi
    eval "$JSEMTBLGEN" "$JSEMTBLGEN_ARGS"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: jsemtblgen failed, exit status: $status" 1>&2
	exit 6
    fi

fi

# output the trailer
#
if [[ -n "$TAIL_FILE" ]]; then
    cat "$TAIL_FILE"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: cat $TAIL_FILE exit status: $status" 1>&2
	exit 22
    fi
fi

# All Done!!! -- Jessica Noll, Age 2
#
exit 0
