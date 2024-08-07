# Command line utilities that read and process JSON in different ways

## WARNING: status of the tools

### XXX - remove this WARNING once the tools are complete - XXX

Please be advised that the tools `jfmt`, `jval` and `jnamval` are **VERY
INCOMPLETE** and **WILL BE HEAVILY MODIFIED**. It is also highly likely that
they will be almost **entirely rewritten** as things changed as they were first
worked on. Almost everything will be redone at this point. When this is done
this notice will be removed.

## Introduction to this document

NOTE: Please see [json_README.md](./json_README.md) for some important **JSON
terms** used in this document.

There is a general lack of JSON aware command line tools that allow someone to
obtain information from within a JSON document.  By "obtaining information" we
refer to the ability to extract (and if desired, format) data, either as a whole
or in part, that is contained within a [JSON
document](./json_README.md#json-document).

There does exist multiple [JSON](./json_README.md) [APIs (Application Program
Interfaces)](https://en.wikipedia.org/wiki/API) for various programming
languages such as: JavaScript, C, Java, Python, Perl, Go, Rust, PHP, etc.  This
[mkiocccentry repo](https://https://github.com/ioccc-src/mkiocccentry) contains
a rich JSON parser API C programs can use, as well as a semantic JSON check
interface for C.  However all those APIs require the user to "program" in a
specific language in order to do something as simple as obtain a selective [JSON
values](./json_README.md#json-value) from a [JSON
document](./json_README.md#json-document). With these tools we attempt to
address some of these problems.



## Scope of this document

In this document, we will concern ourselves only with [Valid
JSON](./json_README.md#valid-json) in the form of a [JSON
document](./json_README.md#json-document), where a document can be a file or
stdin. It must be [valid JSON](./json_README.md#valid-json) only because we can
only extract information with any degree of certainty from [Valid
JSON](./json_README.md#valid-json).

We consider command line utilities that _read_ information from a [JSON
document](./json_README.md#json-document) in the form of either a file or stdin.
In this document _we are not_ considering utilities that either create or modify
JSON.

If a command line utility is given an **invalid JSON document**, the utility
shall exit non-zero with an error message indicating that the [JSON
document](./json_README.md#json-document) is not valid JSON.  Therefore we're
only concerned with reading various information from an existing [JSON
document](./json_README.md#json-document) or stdin that contains [Valid
JSON](./json_README.md#valid-json).


## Common command line options

In this document, we assume that all utilities have at least the following
options:

```
	-h		Print help and exit
	-V		Print version and exit
	-v level	Verbosity level (def: 0)
	-J level	JSON verbosity level (def: 0)
	-q		Quiet mode, do not print to stdout (def: print stuff to stdout)
	-L <num>[{t|s}]	Print JSON level followed by a number of tabs or spaces (def: don't print levels)
	-L tab		Alias for: '-L 1t'

			Trailing 't' implies <num> tabs whereas trailing 's' implies <num> spaces.
			Not specifying 's' or 't' implies spaces.

	-l lvl		Print specific JSON levels (def: print at any level)

			If lvl is a number (e.g.: -l 3), level must == number.
			If lvl is a number followed by : (e.g. '-l 3:'), level must be >= number.
			If lvl is a : followed by a number (e.g. '-l :3'), level must be <= number.
			If lvl is num:num (e.g. '-l 3:5'), level must be inclusively in the range.

	-m		Maximum depth to traverse in the JSON tree
	-K		Run test mode and exit
	-o ofile	Output formatted JSON to ofile (def: standard output, - ==> standard output)
	-F fmt		Change the JSON format style (def: use default)

			tty		When output is to a TTY, use colour, otherwise use simple
			simple		Each line has one JSON level determined by '[]'s and '{}'s
			colour		Simple plus ANSI colour syntax highlighting
			color		Alias for colour (colour excluding you :-) )
			1line		One line output
			nows		One line output, no extra whitespace

```

Additional command line options will be added on a utility by utility basis.



## Common exit codes

In this document, we assume that all utilities have at least the following exit
codes:

```
Exit codes:
    0	all is OK and file is valid JSON
    1	error writing to output file (if -o specified)
    2	-h and help string printed or -V and version string printed
    3	invalid command line, invalid option or option missing an argument
    4	file does not exist, not a file, or unable to read the file
    5	file contents is not valid JSON
    6	test mode failed
...
 >=10	internal error
```

Additional exit codes will be added on a utility by utility basis.


## Command line utilities

We present three **Command line utilities**:

- [jfmt utility](#jfmt-utility)
- [jval utility](#val-utility)
- [jnamval utility](#namval-utility)


### jfmt utility

Given a [Valid JSON](./json_README.md#valid-json) [JSON
document](./json_README.md#json-document), we need a utility to print the JSON
in "canonical"-like well formatted JSON.  The goal of such formatting is to make
is easier for a human to see the JSON structure. This might be likened to a
beautifier (at least in so far as it is possible for JSON to be beautified :-)
).

The `jfmt` utility will write the entire [JSON
document](./json_README.md#json-document) to a file, (defaulting to standard
output) in "canonical"-like well formatted JSON.  The output of the `jfmt`
utility **MUST** be [Valid JSON](./json_README.md#valid-json).

Consider the [JSON document](./json_README.md#json-document) found at
[top_level_array2.json](./test_jparse/test_JSON/good/top_level_array2.json):

```json
["string",42,3.1415,6.02214076e23,{"member_name":"member_value"},["str",31,2.718,2.99792458e8,{"mname":21701},[0,1,2,3,false,true,null,{"hair":"red"}],false,true,null]]
```

One way to format
[top_level_array2.json](./test_jparse/test_JSON/good/top_level_array2.json) is
the way that [jsonlint.com](https://jsonlint.com) formats such a document:


```json
["string", 42, 3.1415, 6.02214076e23, {
		"member_name": "member_value"
	},
	["str", 31, 2.718, 2.99792458e8, {
			"mname": 21701
		},
		[0, 1, 2, 3, false, true, null, {
			"hair": "red"
		}], false, true, null
	]
]
```

Another way is to put each JSON element on its own line like:


```json
[
    "string",
    42,
    3.1415,
    6.02214076e23,
    {
	"member_name" : "member_value"
    },
    [
	"str",
	31,
	2.718,
	2.99792458e8,
	{
	    "mname" : 21701
	},
	[
	    0,
	    1,
	    2,
	    3,
	    false,
	    true,
	    null,
	    {
		"hair" : "red"
	    }
	],
	false,
	true,
	null
    ]
]
```

We write "easier for a human to see the JSON structure"  because some [JSON
documents](./json_README.md#json-document) are large, deep, complex enough and
even minified in order to make any attempt to format them difficult for a human
to understand them.

Should one wish to have more than one style of output, then:

```
	-s style	use JSON output style (def: use common style)
```

_could_ be added as an option to the `jfmt` command line to change from the
default JSON style but we do not do this as it's not easy to define what should
be considered. Should this be requested we can consider this later on. We do
hope, however, that the options documented will be enough.


#### jfmt command line options

In addition to the [Common command line options](#common-command-line-options),
we recommend the following command line options for `jfmt`:

```

	-I <num>{[t|s]}	When printing JSON syntax, indent levels (def: indent with 4 spaces)
	-I tab		Alias for '-I 1t'

			Trailing 't' implies indent with number of tabs whereas trailing 's' implies spaces.
			Not specifying 's' or 't' implies spaces.


	file.json	JSON file to parse (- ==> read from stdin)
```

Additional command line options may be added.

For example, one might wish to add additional command line options to vary the
style of JSON formatting.  This could be done, however, every one of those
formatting style variations, increases the complexity of testing.

We recommend command line simplicity be considered for the `jfmt` utility.

We also recommend that the `jfmt(1)` man page contain a warning that users
should not depend on the exact output of `jfmt` and that future versions of
the utility might change the way [Valid JSON](./json_README.md#valid-json) is
formatted.


#### jfmt exit codes

There are no additional explicit exit codes for `jfmt` besides the [Common exit
codes](#common-exit-codes).

Any other exit codes should probably fall under the "_internal error_" category
and thus fall into the "_>= 10_" range.


#### jfmt examples

In these examples, we write JSON in an inconsistent style.  Feel free to improve
on this style.

Unless otherwise specified, in the examples below you should pretend that we
have 4 space indenting either from `-I 4s` or if decided upon, the default.

##### jfmt example 0

```sh
jfmt jparse/test_jparse/test_JSON/good/42.json
```

should print on standard output:

```json
{
    "foo" : [
        { "foo" : [
	    { "bar" : 42 }
            ]
        }
    ]
}
```

It is not clear how best to indent.  How to indent might be more clear if the
`-L stuff` is used to also print the JSON tree levels.


##### jfmt example 1

```sh
jfmt jparse/test_jparse/test_JSON/good/dyfi_zip.geo.json
```

should print on standard output:

```json
{
    "features" : [
	{
	    "geometry" : {
		"coordinates" : [
		    -149.7791,
		    61.1512],
		"type" : "Point"
	    },
	    "type" : "Feature",
	    "properties" : {
		"population": 0,
		"nresp": 1,
		"name": "99507<br>0.330",
		"cdi": 2,
		"dist": 195
	    }
	}
    ],
    "type" : "FeatureCollection"
}
```

Again, it is unclear how best to indent things such as "[{ .. }]" as these
compound [JSON values](./json_README.md#json-value) are at different JSON tree
levels.

It would be better if the formatted lines of `jfmt` corresponded
line by line with the use of `-L stuff` (printing JSON tree levels).


##### jfmt example 2

The use of `-I` controls how JSON is indented.  Consider this variant of [jfmt
example 1](#jfmt-example-1):

```sh
jfmt -I 2s jparse/test_jparse/test_JSON/good/dyfi_zip.geo.json
```

produces:

```json
{
  "features" : [
    {
      "geometry" : {
        "coordinates" : [
          -149.7791,
          61.1512],
        "type" : "Point"
      },
      "type" : "Feature",
      "properties" : {
        "population": 0,
        "nresp": 1,
        "name": "99507<br>0.330",
        "cdi": 2,
        "dist": 195
      }
    }
  ],
  "type" : "FeatureCollection"
}
```

##### jfmt example 3

Let us now consider no indenting at all:

```sh
jfmt -I 0 jparse/test_jparse/test_JSON/good/dyfi_zip.geo.json
```

produces:

```json
{
"features" : [
{
"geometry" : {
"coordinates" : [
-149.7791,
61.1512],
"type" : "Point"
},
"type" : "Feature",
"properties" : {
"population": 0,
"nresp": 1,
"name": "99507<br>0.330",
"cdi": 2,
"dist": 195
}
}
],
"type" : "FeatureCollection"
}
```


##### jfmt example 4

To print the JSON level you can use the `-L` option. For instance to show the
level followed by 4 spaces:

```sh
jfmt -L 4s jparse/test_jparse/test_JSON/good/foo.json
```

This is a useful way for the user to better understand JSON levels.  Assuming we
don't make a mistake in reading the debugging output from `jparse -J 3` we guess
that this might print:

```
1    {
2        "foo" : "bar",
2        "foo" : [
3	    "bar",
3	    "bar",
3	    "bar"
2        ]
1    }
```

If `jval` uses the "1 level per line" model of printing JSON,
then adding `-L` should simply add the level number for each line.

##### jfmt example 5

To print JSON level numbers without indenting JSON:

```sh
jfmt -L 4s -I 0 jparse/test_jparse/test_JSON/good/foo.json
```

produces:

```
1    {
2    "foo" : "bar",
2    "foo" : [
3    "bar",
3    "bar",
3    "bar"
2    ]
1    }
```


##### jfmt example 6

When restricting the levels of JSON that are printed via the `-l` option, and
such a restriction starts at a deeper level than level 0, one does **NOT** need
to fully indent the top most level.

Consider the initial output of:

```sh
jfmt -L 4s -I 2s jparse/test_jparse/test_JSON/good/party.json
```

Assume the above command prints (here again we are guessing at JSON levels):

```
0    {
1      "event" : "A Long-expected Party",
1      "location" : "Bag End, Bagshot Row, Hobbiton, Westfarthing, the Shire, Middle-earth",
1      "date" : "22 September T.A. 3001",
1      "birthdays" : [
2        {
3          "hobbit" : [
4             {
5               "name" : "Bilbo Baggins",
5               "age" : 111,
5               "inheritance" : false
4             },
4             {
5               "name": "Frodo Baggins",
5               "age": 33,
5               "inheritance" : true
4             }
3           ]
2         }
1      ],
...
```

Assuming those JSON levels, if we restrict to printing at JSON level 4 and
lower, then:


```sh
jfmt -L 4s -I 2s -l 4: jparse/test_jparse/test_JSON/good/party.json
```

need not indent level 4 as far as the previous command did:

```
4    {
5      "name" : "Bilbo Baggins",
5      "age" : 111,
5      "inheritance" : false
4    },
4    {
5      "name": "Frodo Baggins",
5      "age": 33,
5      "inheritance" : true
4    }
```

Since `-l 4:` is in effect, we can ignore the 4 levels of indentation when printing.


### jval utility

Given a [JSON document](./json_README.md#json-document), we need a utility that
prints [JSON values](./json_README.md#json-value) that match or do not match
certain values or value ranges.

The `jval` command, without any "`args`" on the command line, will cause the
printing of any [JSON values](./json_README.md#json-value) that are not
otherwise restricted by a `-t type` or `-l lvl` option.

The presence of "`args`" creates match conditions that can restrict which [JSON
values](./json_README.md#json-value) are printed.  When using the `jval` command
with "`args`", a match criteria is setup to further select which [JSON
values](./json_README.md#json-value) are printed.


#### jval command line options

In addition to the [Common command line options](#common-command-line-options),
we recommend the following command line options for `jval`:

```
	-t type		Match only JSON values of a given comma-separated type (def: simple)

				int	integer values
				float	floating point values
				exp	exponential notation values
				num	alias for 'int,float,exp'
				bool	boolean values
				str	string values
				null	null values
				simple	alias for 'int,float,exp,bool,str,null'


	-Q		Print JSON strings surrounded by double quotes (def: do not)
	-D		Print JSON strings as decoded strings (def: print JSON strings as encoded strings)
	-d		Match the JSON decoded values (def: match as given in the JSON document)
	-i		Invert match: print values that do not match (def: print values that do match)
	-s		Match substrings (def: match entire values)
	-f		Fold case (def: case matters when matching strings)
	-c		Print total match count only (def: print values)

			Using -c with either -C or -L is an error.

	-C		Print match count followed by matched value (def: print values)

			Using -C with either -c or -L is an error.

	-g		Match using grep-like extended regular expressions (def: match values)

			To match from the beginning, start arg with '^'.
			To match to the end, end arg with '$'.
			Using -g with -s is an error.

	-n op=num	Match if numeric op with num is true (def: do not)

			    op may be one of: eq, lt, le, gt, ge

	-S op=str	Match if string op with str is true (def: do not)

			    op may be one of: eq, lt, le, gt, ge

	file.json	JSON file to parse (- ==> read from stdin)
	[arg]		match argument(s)
```

By default, matching is performed based on how the value is represented in the
JSON document.

Because of the complexity of trying to describe a complex JSON value on
the command line, let alone the problem of describing complex value ranges,
the `-t type` does **NOT** include complex JSON types. For that see the
`jnamval` utility, discussed later in this document.

If no "`arg`" is given on the command line then any value can be considered
unless `-i` inverts the match in which case no value is to be considered.

When `jval` looks for a match, it is done in the following general order:

0. Check JSON tree level (`-l` default matches any level).
1. For all JSON nodes that previously matched, check node type (without `-t` it
will default to matching simple types).
2. For all JSON nodes that previously matched, check for "`arg`" value match (no
args match all values).
3. For all JSON nodes that previously matched, check for `-n op=num` and `-S
op=str` matches (def: don't do any comparisons).
4. If `-i` invert all matches (def: don't invert matches).


#### jval exit codes

In addition to the [Common exit codes](#common-exit-codes), we recommend the
following exit codes for `jval`:

```
    7	-n op=num: however num cannot be represetned as a numerical value
    8	no matches found

```

Other exit codes probably should fall under the "_internal error_" category
and thus fall into the "_>= 10_" range.

[JSON values](./json_README.md#json-value) are printed in order that they are
encoded in the JSON parse tree.


#### jval examples

Level values are a guess made while looking at the output of `jparse -J 3`.  The
actual level values depend on the actual JSON parse tree.


##### jval example 0

```sh
jval jparse/test_jparse/test_JSON/good/42.json
```

As there are no args, all values match:

```
foo
foo
bar
42
```


##### jval example 1

```sh
jval -L 4s jparse/test_jparse/test_JSON/good/42.json
```

should print with JSON levels:

```
2    foo
5    foo
8    bar
8    42
```

**IMPORTANT NOTE**: We observe that:

```sh
jparse -J 3 jparse/test_jparse/test_JSON/good/42.json
```

prints a different level for this [JSON member](./json_README.md#json-member):


```json
{ "bar" : 42 }
```

The same level value of _8_ is the same for both the [JSON name](./json_README.md#json-name)
of _bar_ and the value of _42_:

```
JSON tree[3]:	lvl: 8	type: JTYPE_STRING	len{qPa}: 3	value:	"bar"
JSON tree[3]:	lvl: 8	type: JTYPE_NUMBER	val+8: 42
```

This is correct.  Remember that in JSON member structure, there is a convenience
pointer to the [JSON name](./json_README.md#json-name).  This is **NOT** the
level _bar_:

```
JSON tree[3]:	lvl: 7	type: JTYPE_MEMBER	name: "bar"		<<-- NOT THIS LEVEL !!!
```

The `jval(1)` man page will need to help explain JSON tree levels.  We highly
recommend that the command:

```sh
jfmt -L tab foo.json
```

be used to help the user understand JSON tree levels when using `-l lvl`.


##### jval example 2

```sh
jval -Q jparse/test_jparse/test_JSON/good/top_level_array.json
```

Here, `-Q` prints [JSON strings](./json_README.md#json-string) within double
quotes:

```
"hi"
21701
2.718
6.023e23
true
false
null
1
2
3
"hello"
true
false
6.67430e-11
4
5
6
null
"a"
"b"
```

The use of double quotes helps distinguish, for example, a JSON `null` from a
[JSON string](./json_README.md#json-string) of `"null"`.

It is important to note that what `jval` prints is the actual characters from
the [JSON document](./json_README.md#json-document), not some machine
interpreted value.  For example, even though in JSON, `2.0` could be represented
as in integer, what `jval` must print is the literal characters from the [JSON
document](./json_README.md#json-document).  This can be done by printing from
structure elements such as `as_str`, or if a decoded string is needed (`-D`),
from the `str` element.

Consider the following [JSON document](./json_README.md#json-document),
`hi-hello.json`:

```json
"hi\nhello"
```

The command:

```sh
jval -Q hi-hello.json
```

should print the [JSON encoded string](./json_README.md#json-encoded-string):

```
"hi\nhello"
```

whereas the command:

```sh
jval -Q -D hi-hello.json
```

should print the [JSON decoded string](./json_README.md#json-decoded-string):

```
"hi
hello"
```


##### jval example 3

To print only integers:

```sh
jval -Q -t int jparse/test_jparse/test_JSON/good/top_level_array.json
```

will print:

```
21701
1
2
3
4
5
6
```

Care must be taken as to which [JSON values](./json_README.md#json-value) are printed.
Just because a value is an integer value does not mean that value should be printed
under the `-t int` option.

The `struct json_number` elements `is_floating` and `is_e_notation` need to be considered.

Consider this JSON:

```json
[31, 31.0, 3.1e1]
```

The command:

```sh
jval -Q -t int jparse/test_jparse/test_JSON/good/top_level_array.json
```

will only print:

```
31
```

The command:

```sh
jval -Q -t float jparse/test_jparse/test_JSON/good/top_level_array.json
```

will only print:

```
31.0
```

And the command:

```sh
jval -Q -t exp jparse/test_jparse/test_JSON/good/top_level_array.json
```

will only print:

```
3.1e1
```


##### jval example 4

The presence of "`args`" creates match conditions that can further restrict
which [JSON values](./json_README.md#json-value) are printed.  The default match
criteria is when the  "`arg`" is an exact match:

```sh
jval jparse/test_jparse/test_JSON/good/h2g2.json 42
```

This should print 3 lines:

```
42
42
42
```

When we add `-Q`:

```sh
jval -Q jparse/test_jparse/test_JSON/good/h2g2.json 42
```


we see why there are 3 lines:

```
42
"42"
42
```

The second `"42"`, with `-Q`, shows that it is a [JSON
string](./json_README.md#json-string) and not an integer.  This points out that,
by default, "`arg`" matching is based on the strings in the [JSON
document](./json_README.md#json-document).

Combined with `-t type`, it allows for further restriction. For instance:

```sh
jval -Q -t str jparse/test_jparse/test_JSON/good/h2g2.json 42
```

produces:

```
"42"
```


##### jval example 5

Multiple "`arg`"s, by default, setup multiple ways for a match.  One may think
of multiple "`args`" are matching in an "OR"-like way.

```sh
jval -Q -t str jparse/test_jparse/test_JSON/good/h2g2.json 42 number
```

produces:

```
"number"
"42"
```

Or for example:

```sh
jval -Q -t str jparse/test_jparse/test_JSON/good/h2g2.json 42 number name
```

produces:

```
"number"
"42"
"name"
"name"
"name"
```


##### jval example 6

The `jval` offers two to count matches instead of simply printing [JSON
values](./json_README.md#json-value) that match:

 ```sh
 jval -Q -t str -c jparse/test_jparse/test_JSON/good/h2g2.json 42 number name
 ```

 produces:

```
5
```

because there are 5 total matches.

With `-C`, the match count per "`arg`" will be printed:

 ```sh
 jval -Q -t str -C jparse/test_jparse/test_JSON/good/h2g2.json 42 number name foo
 ```

produces:

```
1	"number"
1	"42"
3	"name"
```

Observe that the "`arg`" of "_foo_" had 0 matches and was not listed.

Without any "`args`" using `-c` just counts [JSON values](./json_README.md#json-value):

```sh
jval -c jparse/test_jparse/test_JSON/good/foo.json
```

produces:

```
6
```

Using `-C` with no "`args`" is a special case.  The [JSON
value](./json_README.md#json-value) is printed after a tab.  So with `-C` and no
"`args`" we will see the counts of the individual [JSON
values](./json_README.md#json-value):

```sh
jval -C -Q jparse/test_jparse/test_JSON/good/foo.json
```

```
2	"foo"
4	"bar"
```

Observe in this case, the `-Q` is useful because what is being printed counts as
[JSON values](./json_README.md#json-value), not "`args`".

Consider this case:

```sh
jval -C -Q jparse/test_jparse/test_JSON/good/h2g2.json 42
```

We have two types of "42" to consider, and thus different match counts:

```
2	42
1	"42"
```

Here the `-Q` is useful to distinguish between the integer and the
[JSON string](./json_README.md#json-string) being matched.


##### jval example 7

The `-i` option inverts matches, so `jval` would print [JSON
values](./json_README.md#json-value) that do **not match an "`arg`"**:


```sh
jval -i jparse/test_jparse/test_JSON/good/foo.json bar
```

produces:

```
foo
foo
```

Use of `-i` without any "`args`" does nothing:

```sh
jval -i jparse/test_jparse/test_JSON/good/foo.json
```

produces:

```
```

(no output). The `-t type` restricts the inversion of the match.  So:

```sh
jval -i -t int jparse/test_jparse/test_JSON/good/top_level_array.json 23209 1 2 3
```

produces:

```
4
5
6
```

because those are the [JSON values](./json_README.md#json-value) of type "int"
that does **NOT** match one of the "`args`".


##### jval example 8

Be default, "`arg`" must match the entire [JSON value](./json_README.md#json-value).
With `-s`, substring matches are allowed:

```sh
json -s jparse/test_jparse/test_JSON/good/top_level_array.json e
```

produces:

```
6.023e23
true
false
hello
true
false
6.67430e-11
```

Each of those [JSON values](./json_README.md#json-value) have an `e` in them.

Had the type been restricted to strings:

```sh
json -s -t str jparse/test_jparse/test_JSON/good/top_level_array.json e
```

only the [JSON strings](./json_README.md#json-string) with an `e` would have been printed:

```
hello
```


##### jval example 9

By default, `jval` matches [JSON strings](./json_README.md#json-string)
based on their [JSON encoded string](./json_README.md#json-encoded-string)
values.  With `-d`, such matching is on the [JSON decoded
string](./json_README.md#json-decoded-string) value:

```sh
jval -Q -d jparse/test_jparse/test_JSON/good/hi-hello.json 'hi
hello'
```

This produces:

```
"hi
hello"
```


##### jval example 10

By default, `jval` matches [JSON strings](./json_README.md#json-string) where
the case of the string matters:

```sh
jval -Q -s jparse/test_jparse/test_JSON/good/hello-world.json Hello
```

will print:

```
"Hello, World!
"
```

As far as why there is a `"` on a line by itself: note the newline at the end
of the string in the JSON file.

However:

```sh
jval -Q -s jparse/test_jparse/test_JSON/good/hello-world.json hello
```

produces nothing:

```
```

The use of `-f` causes string matching to be case insensitive:

```sh
jval -Q -s jparse/test_jparse/test_JSON/good/h2g2.json killers
```

produces:

```
"killers"
"Killers"
```


##### jval example 11

The `-g` option causes `jval` to use regular expressions to match:

```sh
jval -g party.json 'B.*Bag'
```

produces:

```
Bag End, Bagshot Row, Hobbiton, Westfarthing, the Shire, Middle-earth
Bilbo Baggins
```

Observe that the regular expression given will match substrings.

With `-f` the regular expression matching becomes case insensitive:

```sh
jval -g -f party.json 'b.*bag'
```

produces:

```
Bag End, Bagshot Row, Hobbiton, Westfarthing, the Shire, Middle-earth
Bilbo Baggins
```

The use of regular expression anchors may force the match of the entire [JSON
value](./json_README.md#json-value):

```sh
jval -g party.json '^P.*t$'
```

produces just:

```
Proudfoot
Proudfeet
```


##### jval example 12

Numerical matching (use of `-eq`, `-lt`, `-le`, `-gt`, `-ge`) solves the problem
of trying to match various ways to encode a [JSON
number](./json_README.md#json-number).  Thus with `-eq 200` matching with the
numerical value of _200_ would match [JSON
numbers](./json_README.md#json-number) such as in
`jparse/test_jparse/test_JSON/good/200.json`:

```
[ 200, 200.0, 2e2, 200.000, 2.0e2, 0.2e3, 20.0e1 ]
```

The command:

```sh
jval -n eq=200 jparse/test_jparse/test_JSON/good/200.json
```

would produce:

```
200
200.0
2e2
200.000
2.0e2
0.2e3
    20.0e1
```

The numerical value may be further restricted by using `-t type`.  Observe that:

```sh
jval -t int -n eq=200 jparse/test_jparse/test_JSON/good/200.json
```

would produce:

```
200
```

Also consider that:

```sh
jval jparse/test_jparse/test_JSON/good/two-array.json 2
```

produces:

```
2
2.0
2e0
2.0e0
2.00
2.00e0
0.2e1
20e-1
20.0e-1
```

whereas:

```sh
jval -t int -n eq=2 jparse/test_jparse/test_JSON/good/two-array.json
```

produces:

```
2
```

And:

```sh
jval -t float -n eq=2 jparse/test_jparse/test_JSON/good/two-array.json
```

produces:

```
2.0
2.00
```

And whereas:

```sh
jval -t int,float -n eq=2 jparse/test_jparse/test_JSON/good/two-array.json
```

produces:

```
2
2.0
2.00
```

Recall that:

```sh
jval -Q jparse/test_jparse/test_JSON/good/h2g2.json 42
```

produces:

```
42
"42"
42
```

With `-n eq=42` the numerical value is considered:

```sh
jval -n eq=42 4parse/test_jparse/test_JSON/good/h2g2.json
```

produces:

```
42
42
```

And for example:

```sh
jval -n eq=2 -n eq=3 -n eq=5 jparse/test_jparse/test_JSON/good/top_level_array.json
```

produces:

```
2
3
5
```

The use of multiple `-n eq=val` tests multiple values.

Only values that can be represented by a machine can be matched.  The following
command:

```sh
jval -n eq=1e10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 jparse/test_jparse/test_JSON/good/googolplex.json
```

will print an error message to standard error and exit with a value of 6 because
a googolplex is too big for C types.


##### jval example 13

When comparing as strings:

```sh
jval -S eq=1e10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 jparse/test_jparse/test_JSON/good/googolplex.json
```

produces:

```
1e10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
```

This is because the `-S` is able to match more than [JSON
strings](./json_README.md#json-string): it is able to match any simple [JSON
value](./json_README.md#json-value) type (i.e., JSON
int,float,exp,bool,str,null).

All such JSON nodes have an `as_str` element which is an allocated string of the
JSON object in question that the `-S` is able match, for example.

Consider this command:

```sh
jval -S eq=200 jparse/test_jparse/test_JSON/good/200.json
```

Because `-S` can, for non-[JSON strings](./json_README.md#json-string),
check the `as_str` element, the above command produces:

```
200
```

The above is not a numeric match, but rather a string match.
For example:


```sh
jval -S eq=200.000 jparse/test_jparse/test_JSON/good/200.json
```

produces:

```
200.000
```

When multiple `-S op` where `op` is one of `lt`, `le`, `gt`, or `ge`, pairs of
`-S op` string matches are combined as an AND-ed pair of string matches.

Consider the following command:

```sh
jval -R gele -S ge=c -S le=b jparse/test_jparse/test_JSON/good/JSON_misfeature_number_7.json
```

The strings that are `>= "a"` _AND_ `<= "c"` are printed:

```
avalue
bvalue
```

With 4 such operations, two `AND`ed match pairs are formed:

```sh
jval -S gt=a -S lt=c -S gt=m -S lt=o jparse/test_jparse/test_JSON/good/JSON_misfeature_number_7.json
```

The strings that are `> "a"` _AND_ `< "c"`, _OR_
strings that are `> "m"` _AND_ `< "o"` produces:

```
avalue
bvalue
name1
name2
```

The presence of `-S eq` does not impact the formation of _AND_-ed pairs.
For example:

```sh
jval -S gt=a -S eq=zvalue -S lt=c -S gt=m -S lt=o jparse/test_jparse/test_JSON/good/JSON_misfeature_number_7.json
```

The strings that are `> "a"` _AND_ `< "c"`, _OR_ strings that are `> "m"` _AND_
`< "o"`, _OR_ strings that are `== "zvalue"` will produce:

```
avalue
bvalue
name1
name2
zvalue
```

Half-open ranges may be formed by using a odd number:

```sh
jval -S lt=name2 jparse/test_jparse/test_JSON/good/JSON_misfeature_number_7.json
```

This produces all strings that are `< "name2"`:

```
name1
avalue
bvalue
```

The `-S op=str` is intended to strictly be a string comparison.  While the use
may be obvious for JSON stings, for JSON non-strings the same `as_str`
comparison applies to other JSON simple types (i.e., numbers, booleans, nulls).

Consider the case of `jparse/test_jparse/test_JSON/good/names.json`:

```sh
jval -S ge=n -S le=o jparse/test_jparse/test_JSON/good/names.json
```

Here we print over the range of strings in the [JSON
document](./json_README.md#json-document) that are ">= n" and "<=
o".  We perform string comparisons on the `as_str` structure element
and produces:

```
name0
name1
name2
null
name3
name4
```

Using `-S` with a half-open range:

```sh
jval -S ge=n jparse/test_jparse/test_JSON/good/names.json
```

produces:

```
name0
name1
name2
null
name3
true
name4
```

If we only wanted [JSON strings](./json_README.md#json-string) then we would need to use:

```sh
jval -S ge=n -S le=o -t str jparse/test_jparse/test_JSON/good/names.json
```

which would only produce:

```
name0
name1
name2
name3
name4
```


##### jval example 14

When multiple `-n op` where `op` is one of `lt`, `le`, `gt`, or `ge`, pairs of
`-n op` numeric matches are combined as an AND-ed pair of numeric matches.

```sh
jval -n ge=1 -n le=3 jparse/test_jparse/test_JSON/good/top_level_array.json
```

produces all numeric values `>= 1` _AND_ values `<= 3`:

```
1
2
2.718
3
```

The presence of `-n eq` does not impact the formation of _AND_-ed pairs.
For example:

```sh
jval -n ge=1 -n eq=5 -n le=3 jparse/test_jparse/test_JSON/good/top_level_array.json
```

produces all numeric values `>= 1` _AND_ values `<= 3` as well as values `== 5`:

```
1
2
2.718
3
5
```

For a half open interval, use an odd number such as for all values `< 1`:


```sh
jval -n lt=1 jparse/test_jparse/test_JSON/good/top_level_array.json
```

produces:

```
6.67430e-11
```


### jnamval

The `jnamval` utility operates only on [JSON
members](./json_README.md#json-member).  This utility is only concerned with
[JSON member names](./json_README.md#json-member-name) and their associated
[JSON member values](./json_README.md#json-member-value).


#### jnamval command line options

The options for the `jnamval` utility are very similar to the [jval
utility](#jval-utility).

In addition to the [Common command line options](#common-command-line-options),
and the [jval command line options](#jval-command-line-options),
we recommend the following additional command line options for `jnamval`:

```
        -t type         Match only JSON values of a given comma-separated type (def: simple)
	...

			These types are only valid with -N, -M or if there are no args after file.json:

				member		JSON members
				object		JSON objects
				array		JSON arrays
				compound	alias for 'member,object,array'
				any		alias for 'simple,compound'

	-N		Match based on JSON member names (def: match JSON member values)
	-H		Match name hierarchies (def: do not match hierarchies)

			Use of -H implies -N.

	-r types	Restrict printing to only the comma-separated types (def: do not restrict):

			int		integer values
			float		floating point values
			exp		exponential notation values
			num		alias for 'int,float,exp'
			bool		boolean values
			str		string values
			null		null values
			member		members
			object		objects
			array		arrays
			compound	alias for 'member,object,array'
			simple		alias for 'int,float,exp,bool,str,null'
			any		alias for 'int,float,exp,bool,str,null,member,object,array' (default)

	-p {n,v,b,j}	Print JSON name, value, name & value, or valid JSON (def: print JSON values)
	-p name		Alias for '-p n'
	-p value	Alias for '-p v'
	-p both		Alias for '-p b'
	-p json		Alias for '-p j'

			Later -p instances in the command override earlier -p instances.
			For -p b, name is separated from value by the same number of whitespaces used to indent one level.
```

The `-t type` applies to the type of the [JSON member
values](./json_README.md#json-member-value) in given [JSON
members](./json_README.md#json-member).  Because of the difficulty in expressing
[JSON member values](./json_README.md#json-member-value) for types member,
object, array, compound, and any, those types can only be used if there are no
"`args`" on the command line.

By default, `jnamval` prints [JSON member
values](./json_README.md#json-member-value), just like the [jval
utility](#jval-utility) only prints [JSON values](./json_README.md#json-value).
With the use of `-p parts` [JSON member
names](./json_README.md#json-member-name) and/or [JSON member
values](./json_README.md#json-member-value) can be printed.  Furthermore, with
`-p j` or `-p json`, the JSON syntax can be printed like the [jfmt
utility](#jfmt-utility) does.

By default, `jnamval` uses "`args`" to search the values, ranges, numerical
values, and numerical ranges.  Multiple "`args`" on the command line, by
default, behave the same way as the [jval utility](#jval-utility).  With `-H`,
the meaning of more than one "`arg`" changes to searching for a given [JSON
members](./json_README.md#json-member) hierarchy.

When `jnamval` looks for a match, it is done in the following general order:

0. Check JSON tree level (`-l` default matches any level).
1. For all JSON nodes that previously matched, check JSON member nodes.
2. For all JSON nodes that previously matched, check for "`arg`" value match (no
args indicates that all values should match).
3. For all JSON nodes that previously matched, check for `-n op=num` and `-S
op=str` matches (def: don't).
4. If `-i` invert all matches (def: don't invert).



#### jnamval exit codes

The exit codes for `jnamval` (that are < 10) are identical to
the [jval exit codes](#jval-exit-codes) which we include here again to help with
development and documentation purposes.

In addition to the [Common exit codes](#common-exit-codes), we recommend the
following exit codes for `jnamval`:

```
...
    3	invalid command line, invalid option, option missing argument, invalid number of args
...
    6	-n op=num: however num cannot be represetned as a numerical value
    7	no matches found
```

Other exit codes probably should fall under the "_internal error_" category
and should fall into the "_>= 10_" range.


#### jnamval examples

Level values are a guess made while looking at the output of `jparse -J 3`.
The actual level values depend on the actual JSON parse tree.

We presume that the same concepts described in [jval examples](#jval-examples)
apply directly or indirectly to `jnamval`.  We will focus on command line
options unique to `jnamval` as well as some variations from how
[jval](#jval) operates.


##### jnamval example 0

Unlike [jval](#jval), `jnamval` operates only on [JSON
members](./json_README.md#json-member):

Because `-t type` defaults to simple, and `-p parts` defaults
to [JSON member values](./json_README.md#json-member-value),
this command:

```sh
jnamval jparse/test_jparse/test_JSON/good/42.json
```

produces:

```
42
```

This is because only 1 of the 3 [JSON members](./json_README.md#json-member) had
a [JSON member value](./json_README.md#json-member-value) that was of a simple
type.


##### jnamval example 1

The `-p type` option can change what `jnamval` prints from the
default [JSON member value](./json_README.md#json-member-value).

For example:

```sh
jnamval -p json jparse/test_jparse/test_JSON/good/42.json
```

produces:

```
{ "bar" : 42 }
```

We recommend that when printing with `-p json`, the resulting JSON
be formatted to fit in a single line with spaces between the JSON
syntax tokens.

And for example:

```sh
jnamval -t name -Q jparse/test_jparse/test_JSON/good/42.json
```

produces:

```
"bar"
```

And for example:

```sh
jnamval -p both -Q jparse/test_jparse/test_JSON/good/42.json
```

produces:

```
"bar" : 42
```

Observe that the `-p both` prints the [JSON member
name](./json_README.md#json-member-name) followed by a space,
followed by a `:`(ASCII colon), followed by the [JSON member
value](./json_README.md#json-member-value).


##### jnamval example 2

The `jnamval` `-t type` option includes compound types such as:

- member for [JSON members](./json_README.md#json-member)
- object for [JSON objects](.json_README.md#json-object)
- array for [JSON arrays](.json_README.md#json-array)
- any for every part of JSON

We recommend that when printing with compound types, the resulting JSON be
formatted to fit in a single line with spaces between the JSON syntax tokens.

Consider this command for example:

```sh
jnamval -t array jparse/test_jparse/test_JSON/good/42.json
```

This should produce:

```
"foo" : [ { "foo" : [ { "bar" : 42 } ] } ]
"foo" : [ { "bar" : 42 } ]
```


##### jnamval example 3

While `jnamval` is able to search for [JSON member
values](./json_README.md#json-member-value) in the same way as [jval](#jval),
the use of "`args`" as match criteria is limited to only JSON simple types.

The use of `-t type` where type is a compound type is allowed ONLY when there
are no "`args`" after the "_file.json_" OR if `-N` is used.

When `-N` is used, we search for [JSON member
names](./json_README.md#json-member-name), which are [JSON
strings](./json_README.md#json-string), so any search "`args`" make `jnamval`
search for the [JSON member names](./json_README.md#json-member-name).  This
allows for a  `-t type` where the type is a compound type with "`args`" after
the "_file.json_".

For example:

```sh
jnamval -N -t array jparse/test_jparse/test_JSON/good/party.json hobbit
```

produces:

```
[ { "name" : "Bilbo Baggins", "age" : 111, "inheritance" : false }, { "name": "Frodo Baggins", "age": 33, "inheritance" : true } ]
```

If one wanted to pretty print the results, they should pipe the output into
[jfmt](#jfmt):

```sh
jnamval -N -t array jparse/test_jparse/test_JSON/good/party.json hobbit | jfmt -
```

produces:

```json
[
    {
	"name" : "Bilbo Baggins",
	"age" : 111,
	"inheritance" : false
    },
    {
	"name" : "Frodo Baggins",
	"age" : 33,
	"inheritance" : true
    }
]
```


##### jnamval example 4

With `-N` and "`args`" one may search for certain [JSON member
names](./json_README.md#json-member-name) and print the matching corresponding
[JSON member names](./json_README.md#json-member-value):

```sh
jnamval -N -s -i -Q jparse/test_jparse/test_JSON/good/party.json name alias
```

produces:

```
"Gandalf"
"Gandalf the Grey"
"Mithrandir"
"Tharkûn"
"Olórin"
"Incánus"
```

With `-N -t any` and no "`args`", one may simply print the list of [JSON member
names](./json_README.md#json-member-value):

```sh
jnamval -N -t any -Q jparse/test_jparse/test_JSON/good/dyfi_geo_10km.geo.json
```

produces:

```
"features"
"geometry"
"coordinates"
"type"
"type"
"properties"
"stddev"
"nresp"
"name"
"cdi"
"dist"
"type"
```

##### jnamval example 5

The `-H` option (which implies `-N`) causes "`args`" to only match a hierarchy
of [JSON members](./json_README.md#json-member).

Consider this command:

```sh
jnamval -H jparse/test_jparse/test_JSON/good/dyfi_geo_10km.geo.json features properties stddev
```

A search for [JSON members](./json_README.md#json-member) with a
[JSON member name](./json_README.md#json-member-name) of "_features_" is performed.

For every [JSON member name](./json_README.md#json-member-name) of "_features_"
that is found, a search under the respective JSON node is then made for [JSON
members](./json_README.md#json-member) with a [JSON member
name](./json_README.md#json-member-name) "_properties_".

Then for every [JSON member name](./json_README.md#json-member-name) of
"_properties_" that is found, a search under the given JSON node is then made
for a [JSON members](./json_README.md#json-member) with a [JSON member
name](./json_README.md#json-member-name) of "_stddev_" is name.

Since "_stddev_" is the last "`arg`", then the [JSON member
value](./json_README.md#json-member-value) is examined to determine
if it is of the correct type.  As `-t type` defaults in this case to simple,
the command produces:

```
0.33
```

The check of the [JSON member value](./json_README.md#json-member-value) is only
performed on the last "`arg`".  All prior "`args`", to be in a hierarchy, had to
be some form of compound type as otherwise their value would not be able to
contain lower level [JSON member](./json_README.md#json-member).

While this command:

```sh
jnamval -Q jparse/test_jparse/test_JSON/good/party.json guest_list name
```

will produce:

```
"Bilbo Baggins"
"Frodo Baggins"
"Gandalf"
"Meriadoc Brandybuck"
"Peregrin Took"
"Gollum"
"Proudfoot"
```

Consider this command:

```sh
jnamval -H -Q jparse/test_jparse/test_JSON/good/party.json guest_list name
```

This will only produce:

```
"Gandalf"
"Meriadoc Brandybuck"
"Peregrin Took"
"Gollum"
"Proudfoot"
"Sackville-Baggins"
```

This is because with `-H`, the "name" was restricted to those being under "guest_list".

By using the `-n` or `-S`, one may further refine the search:

```sh
jnamval -H -n eq=2 -c jparse/test_jparse/test_JSON/good/party.json guest_list guest_number
```

produces:

```
1
```

because there is only one "guest_list guest_number" hierarchy with a numeric
value of 2.
