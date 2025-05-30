#!/usr/bin/env make
#
# test_jparse - jparse test tools
#
# "Because specs w/o version numbers are forced to commit to their original design flaws." :-)
#
# Copyright (c) 2022-2025 by Landon Curt Noll and Cody Boone Ferguson.
# All Rights Reserved.
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
# THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
# AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
# DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE OR JSON.
#
#  @xexyl
#	https://xexyl.net		Cody Boone Ferguson
#	https://ioccc.xexyl.net
# and:
#	chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
#
# "Because sometimes even the IOCCC Judges need some help." :-)
#
# Share and enjoy! :-)
#     --  Sirius Cybernetics Corporation Complaints Division, JSON spec department. :-)
#
####


#############
# utilities #
#############

# suggestion: List utility filenames, not paths.
#	      Do not list shell builtin (echo, cd, ...) tools.
#	      Keep the list in alphabetical order.
#
CC= cc
CMP= cmp
CP= cp
CTAGS= ctags
GREP= grep
INDEPEND= independ
INSTALL= install
IS_AVAILABLE= ./is_available.sh
PICKY= picky
RM= rm
SED= sed
SEQCEXIT= seqcexit
SHELL= bash
SHELLCHECK= shellcheck
SORT= sort


####################
# Makefile control #
####################

# The name of this directory
#
# This value is used to print the generic name of this directory
# so that various echo statements below can use this name
# to help distinguish themselves from echo statements used
# by Makefiles in other directories.
#
OUR_NAME= test_jparse

# echo-only action commands

# V= @:					do not echo debug statements (quiet mode)
# V= @					echo debug statements (debug / verbose mode)
#
V= @:
#V= @

# S= @:					do not echo start or end of a make rule (quiet mode)
# S= @					echo start or end of a make rule (debug / verbose mode)
#
#S= @:
S= @

# action commands that are NOT echoed

# Q= @					do not echo internal Makefile actions (quiet mode)
# Q=					echo internal Makefile actions (debug / verbose mode)
#
#Q=
Q= @

# E= @					do not echo calling make in another directory (quiet mode)
# E=					echo calling make in another directory (debug / verbose mode)
#
E=
#E= @

# I= @					do not echo install commands (quiet mode)
# I=					echo install commands (debug / verbose mode
#
I=
#I= @

# other Makefile control related actions

# Q= implies -v 0
# else -v 1
#
ifeq ($(strip ${Q}),@)
VERBOSITY="0"
else
VERBOSITY="1"
endif

# installing variables

# INSTALL_V=				install w/o -v flag (quiet mode)
# INSTALL_V= -v				install with -v (debug / verbose mode
#
#INSTALL_V=
INSTALL_V=

# where to install
#
# Default PREFIX is /usr/local so binaries would be installed in /usr/local/bin,
# libraries in /usr/local/lib etc. If one wishes to override this, say
# installing to /usr, they can do so like:
#
#	make PREFIX=/usr install
#
PREFIX= /usr/local

# uninstalling variables
#

# RM_V=					rm w/o -v flag (quiet mode)
# RM_V= -v				rm with -v (debug / verbose mode)
#
#RM_V= -v
RM_V=


# MAKE_CD_Q= --no-print-directory	silence make cd messages (quiet mode)
# MAKE_CD_Q=				silence make cd messages (quiet mode)
#
MAKE_CD_Q= --no-print-directory
#MAKE_CD_Q=

# Disable parallel Makefile execution
#
# We do NOT support parallel make.  We have found most
# parallel make systems do not get the rule dependency order
# correct, resulting in a failed attempt to compile.
#
.NOTPARALLEL:


##################
# How to compile #
##################

# C source standards being used
#
# This repo supports c17 and later.
#
C_STD= -std=gnu17

# optimization and debug level
#
C_OPT= -O3
#C_OPT= -O0 -g

# Compiler warnings
#
WARN_FLAGS= -Wall -Wextra -Wformat -Wno-char-subscripts
#WARN_FLAGS= -Wall -Wextra -Werror

# special compiler flags
#
C_SPECIAL=


# special linker flags
#
LD_SPECIAL=

# linker options
#
LDFLAGS= ${LD_SPECIAL}

# where to find libdbg.a and libdyn_array.a
#
# LD_DIR2 - locations of libdbg.a and libdyn_array.a passed down from 2 directories above
#
LD_DIR2=

# how to compile
#
# We test by forcing warnings to be errors so you don't have to (allegedly :-) )
#
CFLAGS= ${C_STD} ${C_OPT} -pedantic ${WARN_FLAGS} ${C_SPECIAL} ${LDFLAGS}
#CFLAGS= ${C_STD} -O0 -g -pedantic ${WARN_FLAGS} ${C_SPECIAL} ${LDFLAGS} -fsanitize=address -fno-omit-frame-pointer


###############
# source code #
###############

# source files that are permanent (not made, nor removed)
#
C_SRC= jnum_chk.c jnum_gen.c jnum_header.c pr_jparse_test.c
H_SRC= jnum_chk.h jnum_gen.h

# source files that do not conform to strict picky standards
#
LESS_PICKY_CSRC= jnum_test.c
LESS_PICKY_HSRC=

# all shell scripts
#
SH_FILES= prep.sh jparse_test.sh jstr_test.sh


######################
# intermediate files #
######################

# tags for just the files in this directory
#
LOCAL_DIR_TAGS= .local.dir.tags

# NOTE: ${LIB_OBJS} are objects to put into a library and removed by make clean
#
LIB_OBJS=

# NOTE: ${OTHER_OBJS} are objects NOT put into a library and ARE removed by make clean
#
OTHER_OBJS= jnum_gen.o jnum_chk.o jnum_test.o pr_jparse_test.o

# NOTE: intermediate files to make and removed by make clean
#
BUILT_C_SRC=
BUILT_H_SRC=
ALL_BUILT_SRC= ${BUILT_C_SRC} ${BUILT_H_SRC}

# all intermediate files which are also removed by make clean
#
ALL_OBJS= ${LIB_OBJS} ${OTHER_OBJS}

# all source files
#
ALL_CSRC= ${C_SRC} ${LESS_PICKY_CSRC} ${BUILT_C_SRC}
ALL_HSRC= ${H_SRC} ${LESS_PICKY_HSRC} ${BUILT_H_SRC}
ALL_SRC= ${ALL_CSRC} ${ALL_HSRC} ${SH_FILES}


#######################
# install information #
#######################

# where to install
#
DEST_INCLUDE= /usr/local/include
DEST_LIB= ${PREFIX}/lib
DEST_DIR= ${PREFIX}/bin


#################################
# external Makefile information #
#################################

# may be used outside of this directory
#
EXTERN_H=
EXTERN_O=
EXTERN_LIBA=
EXTERN_PROG=

# NOTE: ${EXTERN_CLOBBER} used outside of this directory and removed by make clobber
#
EXTERN_CLOBBER= ${EXTERN_O} ${EXTERN_LIBA} ${EXTERN_PROG}


######################
# target information #
######################

# shell targets to make by all and removed by clobber
#
SH_TARGETS=

# program targets to make by all, installed by install, and removed by clobber
#
PROG_TARGETS= jnum_chk jnum_gen pr_jparse_test util_test

# what to make by all but NOT to removed by clobber
#
ALL_OTHER_TARGETS= ${SH_TARGETS} extern_everything

# what to make by all, what to install, and removed by clobber (and thus not ${ALL_OTHER_TARGETS})
#
TARGETS= ${PROG_TARGETS}


############################################################
# User specific configurations - override Makefile values  #
############################################################

# The directive below retrieves any user specific configurations from Makefile.local.
#
# The - before include means it's not an error if the file does not exist.
#
# We put this directive just before the first all rule so that you may override
# or modify any of the above Makefile variables.  To override a value, use := symbols.
# For example:
#
#       CC:= gcc
#
-include Makefile.local


######################################
# all - default rule - must be first #
######################################

all: ${TARGETS}
	@:


#################################################
# .PHONY list of rules that do not create files #
#################################################

.PHONY: all \
	rebuild_jnum_test test legacy_clean legacy_clobber \
	configure clean clobber install depend tags local_dir_tags all_tags \
	test_JSON/info.json/good/info.reference.json \
	test_JSON/auth.json/good/auth.reference.json


####################################
# things to make in this directory #
####################################

jnum_test.o: jnum_test.c
	${CC} ${CFLAGS} -I../.. jnum_test.c -c

jnum_chk.o: jnum_chk.c jnum_chk.h ../json_utf8.h ../version.h
	${CC} ${CFLAGS} -I../.. jnum_chk.c -c

jnum_chk: jnum_chk.o jnum_test.o ../libjparse.a
	${CC} ${CFLAGS} $^ -lm -o $@ ${LD_DIR2} -ldyn_array -ldbg

jnum_gen.o: jnum_gen.c jnum_gen.h ../json_utf8.h ../version.h
	${CC} ${CFLAGS} jnum_gen.c -c

jnum_gen: jnum_gen.o ../libjparse.a
	${CC} ${CFLAGS} $^ -lm -o $@ ${LD_DIR2} -ldyn_array -ldbg

pr_jparse_test.o: pr_jparse_test.c pr_jparse_test.h ../json_utf8.h ../version.h
	${CC} ${CFLAGS} pr_jparse_test.c -c

pr_jparse_test: pr_jparse_test.o ../libjparse.a
	${CC} ${CFLAGS} $^ -o $@ ${LD_DIR2} -ldyn_array -ldbg


#########################################################
# rules that invoke Makefile rules in other directories #
#########################################################

../libjparse.a: ../Makefile
	${Q} ${MAKE} ${MAKE_CD_Q} -C .. extern_liba C_SPECIAL=${C_SPECIAL}


####################################
# rules for use by other Makefiles #
####################################

test_JSON/info.json/good/info.reference.json:
	@:

test_JSON/auth.json/good/auth.reference.json:
	@:


###########################################################
# repo tools - rules for those who maintain the this repo #
###########################################################

# use jnum_gen to regenerate test jnum_chk test suite
#
# IMPORTANT: DO NOT run this tool unless you KNOW that
#	     the tables produced by jnum_gen are CORRECT!
#
# WARNING: If you use this rule and generate invalid tables, then you will cause the
#	   jnum_chk(8) tool to check against bogus test results!
#
rebuild_jnum_test: jnum_gen jnum.testset jnum_header.c
	${Q} ${RM} ${RM_V} -f jnum_test.c
	${CP} -f -v jnum_header.c jnum_test.c
	./jnum_gen -v ${VERBOSITY} jnum.testset >> jnum_test.c

# rebuild jparse error files for testing
#
# IMPORTANT: DO NOT run this rule unless you KNOW that the output produced by
#	     jparse on each file is CORRECT!
#
rebuild_jparse_err_files:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} cd .. && \
	    ${MAKE} $@
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

test: util_test
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${Q} if [[ ! -x ./run_jparse_tests.sh ]]; then \
	    echo "${OUR_NAME}: ERROR: executable not found: ./run_jparse_tests.sh" 1>&2; \
	    echo "${OUR_NAME}: ERROR: unable to perform complete test" 1>&2; \
	    exit 1; \
	elif [[ ! -x ./util_test ]]; then \
	    echo "${OUR_NAME}: ERROR: executable not found: ./util_test" 1>&2; \
	    echo "${OUR_NAME}: ERROR: unable to perform complete test" 1>&2; \
	elif [[ ! -x ./jstr_test.sh ]]; then \
	    echo "${OUR_NAME}: ERROR: executable not found: ./jstr_test.sh" 1>&2; \
	    echo "${OUR_NAME}: ERROR: unable to perform complete test" 1>&2; \
	    exit 1; \
	elif [[ ! -x ./jnum_chk ]]; then \
	    echo "${OUR_NAME}: ERROR: executable not found: ./jnum_chk" 1>&2; \
	    echo "${OUR_NAME}: ERROR: unable to perform complete test" 1>&2; \
	    exit 1; \
	else \
	    echo "${OUR_NAME}: RUNNING: ./run_jparse_test.sh -D ${VERBOSITY} -v ${VERBOSITY} -j ./jparse -p test_jparse/pr_jparse_test -c test_jparse/jnum_chk"; \
	    ./run_jparse_tests.sh -D ${VERBOSITY} -v ${VERBOSITY} -j ./jparse -p test_jparse/pr_jparse_test -c test_jparse/jnum_chk; \
	    EXIT_CODE="$$?"; \
	    if [[ $$EXIT_CODE -ne 0 ]]; then \
		echo "${OUR_NAME}: ERROR: run_jparse_tests.sh failed, error code: $$EXIT_CODE"; \
		exit "$$EXIT_CODE"; \
	    else \
		echo "${OUR_NAME}: PASSED: run_jparse_tests.sh"; \
	    fi; \
	fi
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

util_test.c: ../util.c
	${Q} ${RM} ${RM_V} -f $@
	${CP} -v -f $^ $@

util_test.o: util_test.c
	${CC} ${CFLAGS} -DUTIL_TEST util_test.c -c

util_test: util_test.o ../libjparse.a
	${CC} ${CFLAGS} $^ -o $@ ${LD_DIR2} -ldbg -ldyn_array -lm

# sequence exit codes
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
seqcexit: ${ALL_CSRC}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} if ! ${IS_AVAILABLE} ${SEQCEXIT} >/dev/null 2>&1; then \
	    echo 'The ${SEQCEXIT} tool could not be found or is unreliable in your system.' 1>&2; \
	    echo 'The ${SEQCEXIT} tool is required for the $@ rule.'; 1>&2; \
	    echo ''; 1>&2; \
	    echo 'See the following GitHub repo for ${SEQCEXIT}:'; 1>&2; \
	    echo ''; 1>&2; \
	    echo '    https://github.com/lcn2/seqcexit'; 1>&2; \
	    echo ''; 1>&2; \
	    exit 1; \
	else \
	    echo "${SEQCEXIT} -D werr_sem_val -D werrp_sem_val -- ${ALL_CSRC}"; \
	    ${SEQCEXIT} -D werr_sem_val -D werrp_sem_val -- ${ALL_CSRC}; \
	fi
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"


# NOTE: the -v in the picky command is NOT verbosity so do NOT
# use -v ${VERBOSITY} here!
#
picky: ${ALL_SRC}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} if ! ${IS_AVAILABLE} ${PICKY} >/dev/null 2>&1; then \
	    echo 'The ${PICKY} tool could not be found or is unreliable in your system.' 1>&2; \
	    echo 'The ${PICKY} tool is required for the $@ rule.' 1>&2; \
	    echo 1>&2; \
	    echo 'See the following GitHub repo for ${PICKY}:'; 1>&2; \
	    echo 1>&2; \
	    echo '    https://github.com/lcn2/picky' 1>&2; \
	    echo 1>&2; \
	    exit 1; \
	else \
	    echo "${PICKY} -w132 -u -s -t8 -v -e -- ${C_SRC} ${H_SRC}"; \
	    ${PICKY} -w132 -u -s -t8 -v -e -- ${C_SRC} ${H_SRC}; \
	    EXIT_CODE="$$?"; \
	    if [[ $$EXIT_CODE -ne 0 ]]; then \
		echo "make $@: ERROR: CODE[1]: $$EXIT_CODE" 1>&2; \
		exit 1; \
	    fi; \
	    echo "${PICKY} -w -u -s -t8 -v -e -8 -- ${SH_FILES}"; \
	    ${PICKY} -w -u -s -t8 -v -e -8 -- ${SH_FILES}; \
	    EXIT_CODE="$$?"; \
	    if [[ $$EXIT_CODE -ne 0 ]]; then \
		echo "make $@: ERROR: CODE[2]: $$EXIT_CODE" 1>&2; \
		exit 2; \
	    fi; \
	fi
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# inspect and verify shell scripts
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
shellcheck: ${SH_FILES} .shellcheckrc
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} if ! ${IS_AVAILABLE} ${SHELLCHECK} >/dev/null 2>&1; then \
	    echo 'The ${SHELLCHECK} command could not be found or is unreliable in your system.' 1>&2; \
	    echo 'The ${SHELLCHECK} command is required to run the $@ rule.'; 1>&2; \
	    echo ''; 1>&2; \
	    echo 'See the following GitHub repo for ${SHELLCHECK}:'; 1>&2; \
	    echo ''; 1>&2; \
	    echo '    https://github.com/koalaman/shellcheck.net'; 1>&2; \
	    echo ''; 1>&2; \
	    echo 'Or use the package manager in your OS to install it.' 1>&2; \
	    exit 1; \
	else \
	    echo "${SHELLCHECK} -f gcc -- ${SH_FILES}"; \
	    ${SHELLCHECK} -f gcc -- ${SH_FILES}; \
	    EXIT_CODE="$$?"; \
	    if [[ $$EXIT_CODE -ne 0 ]]; then \
		echo "make $@: ERROR: CODE[1]: $$EXIT_CODE" 1>&2; \
		exit 1; \
	    fi; \
	fi
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# inspect and verify man pages
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
check_man:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${V} echo "${OUR_NAME}: nothing to do"
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# vi/vim tags
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
tags:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} for dir in .. .; do \
	    if [[ -f $$dir/Makefile && ! -f $$dir/${LOCAL_DIR_TAGS} ]]; then \
		echo ${MAKE} ${MAKE_CD_Q} -C $$dir local_dir_tags C_SPECIAL=${C_SPECIAL}; \
		${MAKE} ${MAKE_CD_Q} -C $$dir local_dir_tags C_SPECIAL=${C_SPECIAL}; \
	    fi; \
	done
	${Q} echo
	${E} ${MAKE} local_dir_tags C_SPECIAL=${C_SPECIAL}
	${Q} echo
	${E} ${MAKE} all_tags C_SPECIAL=${C_SPECIAL}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# use the ${CTAGS} tool to form ${LOCAL_DIR_TAGS} of the source in this directory
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
local_dir_tags: ${ALL_CSRC} ${ALL_HSRC}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} ${RM} ${RM_V} -f ${LOCAL_DIR_TAGS}
	-${E} ${CTAGS} -w -f ${LOCAL_DIR_TAGS} ${ALL_CSRC} ${ALL_HSRC}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# for a tags file from all ${LOCAL_DIR_TAGS} in all of the other directories
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
all_tags:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} ${RM} ${RM_V} -f tags
	${Q} for dir in . ..; do \
	    if [[ -s $$dir/${LOCAL_DIR_TAGS} ]]; then \
		echo "${SED} -e 's;\t;\t'$${dir}'/;' $${dir}/${LOCAL_DIR_TAGS} >> tags"; \
		${SED} -e 's;\t;\t'$${dir}'/;' "$${dir}/${LOCAL_DIR_TAGS}" >> tags; \
	    fi; \
	done
	${E} ${SORT} tags -o tags
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# NOTE: do NOT use -v ${VERBOSITY} here!
#
legacy_clean:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${V} echo "${OUR_NAME}: nothing to do"
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# NOTE: do NOT use -v ${VERBOSITY} here!
#
legacy_clobber: legacy_clean
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${V} echo "${OUR_NAME}: nothing to do"
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"


###################################
# standard Makefile utility rules #
###################################

configure:
	@echo nothing to $@

# NOTE: do NOT use -v ${VERBOSITY} here!
#
clean:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} ${RM} ${RM_V} -f ${ALL_OBJS} ${ALL_BUILT_SRC}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# NOTE: do NOT use -v ${VERBOSITY} here!
#
clobber: legacy_clobber clean
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} ${RM} ${RM_V} -f ${TARGETS}
	${Q} ${RM} ${RM_V} -f jparse_test.log chkentry_test.log txzchk_test.log
	${Q} ${RM} ${RM_V} -f tags ${LOCAL_DIR_TAGS}
	${Q} ${RM} ${RM_V} -rf a b
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# NOTE: do NOT use -v ${VERBOSITY} here!
#
install: all
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${I} ${INSTALL} ${INSTALL_V} -d -m 0775 ${DEST_DIR}
	${I} ${INSTALL} ${INSTALL_V} -m 0555 ${SH_TARGETS} ${PROG_TARGETS} ${DEST_DIR}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

# uninstall: we provide this in case someone wants to deobfuscate their system. :-)
#
# NOTE: do NOT use -v ${VERBOSITY} here!
#
uninstall:
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${S} echo
	${Q} ${RM} ${RM_V} -f ${DEST_DIR}/jnum_chk
	${Q} ${RM} ${RM_V} -f ${DEST_DIR}/jnum_gen
	${Q} ${RM} ${RM_V} -f ${DEST_DIR}/pr_jparse_test
	${Q} ${RM} ${RM_V} -f ${DEST_DIR}/util_test
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

###############
# make depend #
###############

depend: ${ALL_CSRC}
	${S} echo
	${S} echo "${OUR_NAME}: make $@ starting"
	${Q} if ! ${IS_AVAILABLE} ${INDEPEND} >/dev/null 2>&1; then \
	    echo '${OUR_NAME}: The ${INDEPEND} command could not be found or is unreliable in your system.' 1>&2; \
	    echo '${OUR_NAME}: The ${INDEPEND} command is required to run the $@ rule'; 1>&2; \
	    echo ''; 1>&2; \
	    echo 'See the following GitHub repo for ${INDEPEND}:'; 1>&2; \
	    echo ''; 1>&2; \
	    echo '    https://github.com/lcn2/independ'; 1>&2; \
	elif ! ${IS_AVAILABLE} ${SED} >/dev/null 2>&1; then \
	    echo '${OUR_NAME}: The ${SED} command could not be found.' 1>&2; \
	    echo '${OUR_NAME}: The ${SED} command is required to run the $@ rule'; 1>&2; \
	    echo ''; 1>&2; \
	elif ! ${IS_AVAILABLE} ${GREP} >/dev/null 2>&1; then \
	    echo '${OUR_NAME}: The ${GREP} command could not be found.' 1>&2; \
	    echo '${OUR_NAME}: The ${GREP} command is required to run the $@ rule'; 1>&2; \
	    echo ''; 1>&2; \
	elif ! ${IS_AVAILABLE} ${CMP} >/dev/null 2>&1; then \
	    echo '${OUR_NAME}: The ${CMP} command could not be found.' 1>&2; \
	    echo '${OUR_NAME}: The ${CMP} command is required to run the $@ rule'; 1>&2; \
	    echo ''; 1>&2; \
	else \
	    if ! ${GREP} -q '^### DO NOT CHANGE MANUALLY BEYOND THIS LINE$$' Makefile; then \
	        echo "${OUR_NAME}: make $@ aborting, Makefile missing: ### DO NOT CHANGE MANUALLY BEYOND THIS LINE" 1>&2; \
		exit 1; \
	    fi; \
	    ${SED} -i\.orig -n -e '1,/^### DO NOT CHANGE MANUALLY BEYOND THIS LINE$$/p' Makefile; \
	    ${CC} ${CFLAGS} -MM -I. ${ALL_CSRC} | \
	      ${SED} -E -e 's;\s/usr/local/include/\S+;;g' -e 's;\s/usr/include/\S+;;g' | \
	      ${SED} -e 's;../../jparse/;../;g' | \
	      ${INDEPEND} -v ${VERBOSITY} >> Makefile; \
	    if ${CMP} -s Makefile.orig Makefile; then \
		${RM} ${RM_V} -f Makefile.orig; \
	    else \
		echo "${OUR_NAME}: Makefile dependencies updated"; \
		echo; \
		echo "${OUR_NAME}: Previous version may be found in: Makefile.orig"; \
	    fi; \
	fi
	${S} echo
	${S} echo "${OUR_NAME}: make $@ ending"

### DO NOT CHANGE MANUALLY BEYOND THIS LINE
jnum_chk.o: ../../dbg/dbg.h ../../dyn_array/../dbg/dbg.h \
    ../../dyn_array/dyn_array.h ../jparse.h ../jparse.tab.h ../json_parse.h \
    ../json_sem.h ../json_utf8.h ../json_util.h ../util.h ../version.h \
    jnum_chk.c jnum_chk.h
jnum_gen.o: ../../dbg/dbg.h ../../dyn_array/../dbg/dbg.h \
    ../../dyn_array/dyn_array.h ../jparse.h ../jparse.tab.h ../json_parse.h \
    ../json_sem.h ../json_utf8.h ../json_util.h ../util.h ../version.h \
    jnum_gen.c jnum_gen.h
jnum_header.o: ../../dbg/dbg.h ../../dyn_array/../dbg/dbg.h \
    ../../dyn_array/dyn_array.h ../jparse.h ../jparse.tab.h ../json_parse.h \
    ../json_sem.h ../json_utf8.h ../json_util.h ../util.h ../version.h \
    jnum_chk.h jnum_header.c
jnum_test.o: ../../dbg/dbg.h ../../dyn_array/../dbg/dbg.h \
    ../../dyn_array/dyn_array.h ../jparse.h ../jparse.tab.h ../json_parse.h \
    ../json_sem.h ../json_utf8.h ../json_util.h ../util.h ../version.h \
    jnum_chk.h jnum_test.c
pr_jparse_test.o: ../../dbg/dbg.h ../../dyn_array/../dbg/dbg.h \
    ../../dyn_array/dyn_array.h ../jparse.h ../jparse.tab.h ../json_parse.h \
    ../json_sem.h ../json_utf8.h ../json_util.h ../util.h ../version.h \
    pr_jparse_test.c pr_jparse_test.h
