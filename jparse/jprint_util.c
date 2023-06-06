/*
 * jprint_util - utility functions for JSON printer jprint
 *
 * "Because specs w/o version numbers are forced to commit to their original design flaws." :-)
 *
 * This tool is being developed by:
 *
 *	@xexyl
 *	https://xexyl.net		Cody Boone Ferguson
 *	https://ioccc.xexyl.net
 *
 * The JSON parser was co-developed in 2022 by Cody and Landon.
 *
 * "Because sometimes even the IOCCC Judges need some help." :-)
 *
 * "Share and Enjoy!"
 *     --  Sirius Cybernetics Corporation Complaints Division, JSON spec department. :-)
 */


#include "jprint_util.h"

/*
 * match_none	- if no types should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types == 0.
 */
bool
match_none(uintmax_t types)
{
    return types == JPRINT_TYPE_NONE;
}
/*
 * match_int	- if ints should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_INT set.
 */
bool
match_int(uintmax_t types)
{
    return types & JPRINT_TYPE_INT;
}
/*
 * match_float	- if floats should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_FLOAT set.
 */
bool
match_float(uintmax_t types)
{
    return types & JPRINT_TYPE_FLOAT;
}
/*
 * match_exp	- if exponents should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_EXP set.
 */
bool
match_exp(uintmax_t types)
{
    return types & JPRINT_TYPE_EXP;
}
/*
 * match_bool	- if booleans should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_BOOL set.
 */
bool
match_bool(uintmax_t types)
{
    return types & JPRINT_TYPE_BOOL;
}
/*
 * match_string	    - if strings should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_STR set.
 */
bool
match_string(uintmax_t types)
{
    return types & JPRINT_TYPE_STR;
}
/*
 * match_null	- if null should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_NULL set.
 */
bool
match_null(uintmax_t types)
{
    return types & JPRINT_TYPE_NULL;
}
/*
 * match_object	    - if objects should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_OBJECT set.
 */
bool
match_object(uintmax_t types)
{
    return types & JPRINT_TYPE_OBJECT;
}
/*
 * match_array	    - if arrays should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has JPRINT_TYPE_ARRAY set.
 */
bool
match_array(uintmax_t types)
{
    return types & JPRINT_TYPE_ARRAY;
}
/*
 * match_any	- if any type should match
 *
 * given:
 *
 *	types	- types set
 *
 * Returns true if types has any type set.
 */
bool
match_any(uintmax_t types)
{
    return types & JPRINT_TYPE_ANY;
}
/*
 * match_simple	- if simple types should match
 *
 * given:
 *
 *	types	- types set
 *
 * Simple is defined as a number, a bool, a string or a null.
 *
 * Returns true if types has JPRINT_TYPE_SIMPLE set.
 */
bool
match_simple(uintmax_t types)
{
    return types & JPRINT_TYPE_SIMPLE;
}
/*
 * match_compound   - if compounds should match
 *
 * given:
 *
 *	types	- types set
 *
 * A compound is defined as an object or array.
 *
 * Returns true if types has JPRINT_TYPE_COMPOUND set.
 */
bool
match_compound(uintmax_t types)
{
    return types & JPRINT_TYPE_COMPOUND;
}

/*
 * jprint_parse_types_option	- parse -t types list
 *
 * given:
 *
 *	optarg	    - option argument to -t option
 *
 * Returns: bitvector of types requested.
 *
 * NOTE: if optarg is NULL (which should never happen) or empty it returns the
 * default, JPRINT_TYPE_SIMPLE (as if '-t simple').
 */
uintmax_t
jprint_parse_types_option(char *optarg)
{
    char *p = NULL;	    /* for strtok_r() */
    char *saveptr = NULL;   /* for strtok_r() */

    uintmax_t type = JPRINT_TYPE_SIMPLE; /* default is simple: num, bool, str and null */

    if (optarg == NULL || !*optarg) {
	/* NULL or empty optarg, assume simple */
	return type;
    }

    /*
     * Go through comma-separated list of types, setting each as a bitvector
     *
     * NOTE: the way this is done might change if it proves there is a better
     * way (and there might be - I've thought of a number of ways already).
     */
    for (p = strtok_r(optarg, ",", &saveptr); p; p = strtok_r(NULL, ",", &saveptr)) {
	if (!strcmp(p, "int")) {
	    type |= JPRINT_TYPE_INT;
	} else if (!strcmp(p, "float")) {
	    type |= JPRINT_TYPE_FLOAT;
	} else if (!strcmp(p, "exp")) {
	    type |= JPRINT_TYPE_EXP;
	} else if (!strcmp(p, "num")) {
	    type |= JPRINT_TYPE_NUM;
	} else if (!strcmp(p, "bool")) {
	    type |= JPRINT_TYPE_BOOL;
	} else if (!strcmp(p, "str")) {
	    type |= JPRINT_TYPE_STR;
	} else if (!strcmp(p, "null")) {
	    type |= JPRINT_TYPE_NULL;
	} else if (!strcmp(p, "object")) {
	    type |= JPRINT_TYPE_OBJECT;
	} else if (!strcmp(p, "array")) {
	    type |= JPRINT_TYPE_ARRAY;
	} else if (!strcmp(p, "simple")) {
	    type |= JPRINT_TYPE_SIMPLE;
	} else if (!strcmp(p, "compound")) {
	    type |= JPRINT_TYPE_COMPOUND;
	} else if (!strcmp(p, "any")) {
	    type |= JPRINT_TYPE_ANY;
	} else {
	    /* unknown type */
	    err(11, __func__, "unknown type '%s'", p);
	    not_reached();
	}
    }

    return type;
}


