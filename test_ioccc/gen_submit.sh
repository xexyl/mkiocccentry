#!/usr/bin/env bash
#
# gen_submit.sh - generate good and bad slot trees for a given UUID
#
# Copyright (c) 2025-2026 by Landon Curt Noll.  All Rights Reserved.
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
# CODY BOONE FERGUSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT
# SHALL CODY BOONE FERGUSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.
#
# Share and enjoy! :-)
#     --  Sirius Cybernetics Corporation Complaints Division, JSON spec department. :-)
#

# setup


# IOCCC requires use of C locale
#
export LANG="C"
export LC_CTYPE="C"
export LC_NUMERIC="C"
export LC_TIME="C"
export LC_COLLATE="C"
export LC_MONETARY="C"
export LC_MESSAGES="C"
export LC_PAPER="C"
export LC_NAME="C"
export LC_ADDRESS="C"
export LC_TELEPHONE="C"
export LC_MEASUREMENT="C"
export LC_IDENTIFICATION="C"
export LC_ALL="C"


# setup
#
NAME=$(basename "$0")
export NAME
#
export V_FLAG=0
#
export DO_NOT_PROCESS=
#
export VERSION="2.0.1 2026-10-07"
export GOOD_TEMPLATE="./test_ioccc/slot/template/good"
export BAD_TEMPLATE="./test_ioccc/slot/template/bad"
#
export MKIOCCCENTRY_SLOTS="./test_ioccc/mkiocccentry_slots.sh"
export MKIOCCCENTRY="./mkiocccentry"
export REPO_TOPDIR=
#
RSYNC_BIN=$(type -P rsync)
export RSYNC_BIN
#
export EXIT_CODE=0

# usage message
#
export USAGE="usage: $0 [-h] [-V] [-v level] [-N]
    [-G good_template] [-B bad_template]
    [-m mkiocccentry_slots.sh] [-M mkiocccentry] [-R rsync] [-Z repo_topdir]
    UUID slot_tree

    -h                  print help and exit
    -v level            set verbosity level for this script: (def level: 0)
    -V                  print version and exit

    -N                  do not process anything, just parse arguments (def: process something)

    -G good_template    path of the templates for the good tree (def: $GOOD_TEMPLATE)

    -B bad_template     path of the templates for the bad tree (def: $BAD_TEMPLATE)

    -m mkiocccentry_slots.sh    path to the mkiocccentry_slots.sh tool (def: $MKIOCCCENTRY_SLOTS)
    -M mkiocccentry     path to mkiocccentry executable (def: $MKIOCCCENTRY)
    -R rsync            path to rsync tool (def: $RSYNC_BIN)
    -Z repo_topdir      where to cd for the top of the mkiocccentry toolkit (def: try . or ..)
                            NOTE: repo_topdir/mkiocccentry.c must be a file
                            NOTE: implies -m repo_topdir/test_ioccc/mkiocccentry_slots.sh
                            NOTE: implies -M repo_topdir/mkiocccentry
                            NOTE: implies . for default good_template, bad_template

    UUID                username UUID to form good and bad slot trees for
    slot_tree           create slot_tree/good and slot_tree/bad trees

Exit codes:
     0   all OK
     1   at least one test failed
     2   -h and help string printed or -V and version string printed
     3   invalid command line
     4   missing slot information
     5   UUID is invalid, does not match UUID regex
     6   repo_topdir not valid
 >= 10   internal error or missing file or directory

$NAME version: $VERSION"


# parse args
#
while getopts :hv:VNG:B:m:M:R:Z: flag; do
    case "$flag" in
    h)	echo "$USAGE" 1>&2
	exit 2
	;;
    v)	V_FLAG="$OPTARG";
	;;
    V)	echo "$VERSION"
	exit 2
	;;
    N)  DO_NOT_PROCESS="-N"
	;;
    G)	GOOD_TEMPLATE="$OPTARG";
	;;
    B)	BAD_TEMPLATE="$OPTARG";
	;;
    m)	MKIOCCCENTRY_SLOTS="$OPTARG";
	;;
    M)	MKIOCCCENTRY="$OPTARG";
	;;
    R)  RSYNC_BIN="$OPTARG";
        ;;
    Z)  REPO_TOPDIR="$OPTARG";
	MKIOCCCENTRY_SLOTS="$REPO_TOPDIR/test_ioccc/mkiocccentry_slots.sh"
	MKIOCCCENTRY="$REPO_TOPDIR/mkiocccentry"
        ;;
    \?) echo "$0: ERROR: invalid option: -$OPTARG" 1>&2
	echo 1>&2
	echo "$USAGE" 1>&2
	exit 3
	;;
    :)	echo "$0: ERROR: option -$OPTARG requires an argument" 1>&2
	echo 1>&2
	echo "$USAGE" 1>&2
	exit 3
	;;
   *)
	;;
    esac
done

# check args
#
shift $(( OPTIND - 1 ));
if [[ $# -ne 2 ]]; then
    echo "$0: ERROR: expected 2 arguments, found $#" 1>&2
    echo 1>&2
    echo "$USAGE" 1>&2
    exit 3
fi
export UUID="$1"
export SLOT_TREE="$2"
export GOOD_TREE="$SLOT_TREE/good"
export BAD_TREE="$SLOT_TREE/bad"

# UUID regex
#
# A UUID has the 36 character format:
#
#     xxxxxxxx-xxxx-4xxx-Nxxx-xxxxxxxxxxxx
#
# where 'x' is a hex character, 4 is the UUID version, and N is one of 8, 9, a, or b.
#
export RE_HEX='[0-9A-Fa-f]'			    # 1 hex
export RE_HEX_3="$RE_HEX$RE_HEX$RE_HEX"		    # 3 hex
export RE_HEX_4="$RE_HEX_3$RE_HEX"		    # 4 hex
export RE_N='[89ABab]'				    # N is one of 8, 9, a, or b
export RE_UUID_1="$RE_HEX$RE_HEX$RE_HEX$RE_HEX"	    # 4 hex
export RE_UUID_0="$RE_UUID_1$RE_UUID_1"		    # 8 hex
export RE_UUID_2="4$RE_HEX$RE_HEX$RE_HEX"	    # UUID version 4
export RE_UUID_3="$RE_N$RE_HEX_3"		    # N + 3 hex
export RE_UUID_4="$RE_HEX_4$RE_UUID_0"		    # 12 hex
# UUID regex for IOCCC
export RE_UUID="$RE_UUID_0-$RE_UUID_1-$RE_UUID_2-$RE_UUID_3-$RE_UUID_4"

# slot number regex
#
export RE_SLOT_NUM='[0-9]'			    # IOCCC slot number is 1 digit

# UUID-SLOT_NUM regex
#
export RE_UUID_SLOT_NUM="$RE_UUID-$RE_SLOT_NUM"	    # IOCCC UUID-SLOT_NUM

# change to the top level directory as needed
#
if [[ -n $REPO_TOPDIR ]]; then
    if [[ ! -d $REPO_TOPDIR ]]; then
	echo "$0: ERROR: -Z $REPO_TOPDIR given: not a directory: $REPO_TOPDIR" 1>&2
	exit 6
    fi
    if [[ $V_FLAG -ge 1 ]]; then
	echo "$0: debug[1]: -Z $REPO_TOPDIR given, about to cd $REPO_TOPDIR" 1>&2
    fi
    # SC2164 (warning): Use 'cd ... || exit' or 'cd ... || return' in case cd fails.
    # https://www.shellcheck.net/wiki/SC2164
    # shellcheck disable=SC2164
    cd "$REPO_TOPDIR"
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: -Z $REPO_TOPDIR given: cd $REPO_TOPDIR exit code: $status" 1>&2
	exit 6
    fi
elif [[ -f mkiocccentry.c ]]; then
    REPO_TOPDIR="$PWD"
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: assume REPO_TOPDIR is .: $REPO_TOPDIR" 1>&2
    fi
elif [[ -f ../mkiocccentry.c ]]; then
    cd ..
    status="$?"
    if [[ $status -ne 0 ]]; then
	echo "$0: ERROR: cd .. exit code: $status" 1>&2
	exit 6
    fi
    REPO_TOPDIR="$PWD"
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: assume REPO_TOPDIR is ..: $REPO_TOPDIR" 1>&2
    fi
else
    echo "$0: ERROR: cannot determine REPO_TOPDIR, use -Z topdir" 1>&2
    exit 6
fi
if [[ $V_FLAG -ge 3 ]]; then
    echo "$0: debug[3]: REPO_TOPDIR is the current directory: $REPO_TOPDIR" 1>&2
fi

# debugging
#
if [[ $V_FLAG -ge 3 ]]; then
    echo "$0: debug[3]: NAME=$NAME" 1>&2
    echo "$0: debug[3]: V_FLAG=$V_FLAG" 1>&2
    echo "$0: debug[3]: DO_NOT_PROCESS=$DO_NOT_PROCESS" 1>&2
    echo "$0: debug[3]: VERSION=$VERSION" 1>&2
    echo "$0: debug[3]: GOOD_TEMPLATE=$GOOD_TEMPLATE" 1>&2
    echo "$0: debug[3]: BAD_TEMPLATE=$BAD_TEMPLATE" 1>&2
    echo "$0: debug[3]: MKIOCCCENTRY_SLOTS=$MKIOCCCENTRY_SLOTS" 1>&2
    echo "$0: debug[3]: MKIOCCCENTRY=$MKIOCCCENTRY" 1>&2
    echo "$0: debug[3]: REPO_TOPDIR=$REPO_TOPDIR" 1>&2
    echo "$0: debug[3]: UUID=$UUID" 1>&2
    echo "$0: debug[3]: SLOT_TREE=$SLOT_TREE" 1>&2
    echo "$0: debug[3]: GOOD_TREE=$GOOD_TREE" 1>&2
    echo "$0: debug[3]: BAD_TREE=$BAD_TREE" 1>&2
    echo "$0: debug[3]: RSYNC_BIN=$RSYNC_BIN" 1>&2
    echo "$0: debug[3]: RE_UUID=$RE_UUID" 1>&2
    echo "$0: debug[3]: RE_SLOT_NUM=$RE_SLOT_NUM" 1>&2
    echo "$0: debug[3]: RE_UUID_SLOT_NUM=$RE_UUID_SLOT_NUM" 1>&2
fi

# verify that the UUID matches the RE_UUID regex
#
if [[ ! $UUID =~ $RE_UUID ]]; then
    echo "$0: ERROR: invalid UUID: $UUID" 1>&2
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: ERROR: does not match regex: $RE_UUID" 1>&2
    fi
    exit 5
fi

# -N stops early before any processing is performed
#
if [[ -n $DO_NOT_PROCESS ]]; then
    if [[ $V_FLAG -ge 3 ]]; then
	echo "$0: debug[3]: arguments parsed, -N given, exiting 0" 1>&2
    fi
    exit 0
fi

# run mkiocccentry_slots.sh
#
if [[ $V_FLAG -ge 1 ]]; then
    echo "$0: debug[1]: about to: $MKIOCCCENTRY_SLOTS -v $V_FLAG -g $GOOD_TREE -G $GOOD_TEMPLATE -b $BAD_TREE -B $BAD_TEMPLATE" \
						     "-M $MKIOCCCENTRY -R $RSYNC_BIN -U $UUID -Z $REPO_TOPDIR" 1>&2
fi
"$MKIOCCCENTRY_SLOTS" -v "$V_FLAG" -g "$GOOD_TREE" -G "$GOOD_TEMPLATE" -b "$BAD_TREE" -B "$BAD_TEMPLATE" \
		      -M "$MKIOCCCENTRY" -R "$RSYNC_BIN" -U "$UUID" -Z "$REPO_TOPDIR"
status="$?"
if [[ $status -ne 0 ]]; then
    echo "$0: ERROR: $MKIOCCCENTRY_SLOTS -v $V_FLAG -g $GOOD_TREE -G $GOOD_TEMPLATE -b $BAD_TREE -B $BAD_TEMPLATE" \
	 "-M $MKIOCCCENTRY -R $RSYNC_BIN -U $UUID -Z $REPO_TOPDIR" \
	 "failed, exit code: $status" 1>&2
    exit "$status"
fi

# All Done!!! All Done!!! -- Jessica Noll, Age 2
#
exit 0
