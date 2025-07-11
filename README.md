# LED

## OVERVIEW

Line EDitor (led) is a simple command line utility wrote in C to edit text files using the well known PCRE2 library (Philip Hazel) for modern REGEX syntax and more.
It aims to cover in one tool common text search/replace/process transformations that often need multiple tools combined like sed, grep, tr, awk, perl, xargs ...

### Features

- simple search on multiple files like grep
- advanced block search based on multiple regex boundaries
- STDIN / STDOUT support for pipes
- line based processing with linear workflow of up to 16 function steps
- up to 50 specific text transform functions
- up to 10 registers for memory store/recall in line processing workflow
- simple function format, always applyable on a regex with zone capture.
- transformed lines can be executed (option)
- UTF-8 full support in content and regex patterns
- group lines as multi-lines (option)
- massive file sets handling
- multiple exit code mode

### Command line syntax

The **led** command line arguments is composed of sections in the folling order:

```
# direct invocation
led [SELECTOR] [PROCESSOR] [-opts...] [-f] [FILES...]

# file content piped invocation
cat [FILES...] | led [SELECTOR] [PROCESSOR] [-opts...]

# file names piped invocation
ls [DIR] | led [SELECTOR] [PROCESSOR] [-opts...] -f
```

- The options (except `-f`) can be everywhere before `-f`
- after `-f` option all args are considered as file names.
- The section recognition (selector, processor) depends on the arguments format and content.

### Invocation modes

There is 4 ways to invoque **led**:

* direct mode with files
* pipe mode with content
* pipe mode with file names (massive changes)
* advanced pipe mode with file names (advanced massive changes)

#### Direct mode with file names

`led ... -f <file> <file> ...`

the `-f` option must be the last option, every subsequent argument is considered as an intput file name.

#### Pipe mode with content

`cat <file> | led ...`

this is the default mode when the `-f` option is not given.

#### Pipe mode with file names

`find <dir> | led ... -f`

the last argument is the `-f` option without any files behind. It tells led to read file names from STDIN.

This allows massive changes on multiple files.

#### Advanced pipe mode with file names

`find <dir> | led ... -F -f | led ... -F -f | ...`

See options -F, -W, -A, -E, -D and examples to ouput filenames instead of content.

This allows multiple led invocations on multiple files and file trees.

## PRINCIPLES

The global Led text processing pipeline is simple:
- For each input file and each line of file:
    - read line ==> select line ==> process line ==> write line

The selector is defined once, the processor is composed of 0 (not defined) to 16 functions.
- Using led without any processor (only selector), is similar to **grep** usage (filter only).

### The selector

The selector filters lines of input text that must be processed.

Example:

`cat <file> | led Test`

is equivalent to:

`cat <file> | grep Test`

but PCRE2 regex engine is used.

#### Selector syntax:

`led [regex_start|line_number [+shift]] [regex_stop|line_count [+shift]]`

Start selection condition
- the input line matches regex_start or the line number match line_number
- an optional shift of positive cout of line after matching line (to be used with regex_start)

Stop selection condition
- the input line matches regex_stop or the line count from current selection starting point
- an optional shift of positive cout of line after matching line (to be used with regex_stop)
- the matching line is not included into the last selected block of lines.

If start and stop conditions are defined it is possible to select several block of lines in the current text. Each time the start condition is met a new block of lines is selected until the stop condition is met. If the stop selection condition is not defined, only the lines matching the start condition are selected.

#### Selector examples:

```
# output the first line only
cat file.txt | led 1

# output the block of lines from line 10 and 3 next ones (including line 10)
cat file.txt | led 10 3

# output all lines containing abc (regex)
cat file.txt | led abc

# for each line containing `abc` output the block of lines until it contains `def` (not included)
cat file.txt | led abc def

# for each line containing `abc` output the block of 5 next ones
cat file.txt | led abc 5

# output the block of lines from line 4 to a line containing `def` (not included)
cat file.txt | led 4 def

# output the block of lines from 1 line after lines containing `abc` to 1 line after line containing `def` (not included)
cat file.txt | led abc +1 def +1

# for each line containing `abc`; output the line that is 2 lines after
cat file.txt | led abc +2

# the block of contiguous selected lines will be processed 1 per 1 (default)
cat file.txt | led abc 10 <processor>

# the block of contiguous selected lines will be packed and processed all together (multi-line processing)
cat file.txt | led abc 10 <processor> -p

```

### The processor

The processor is composed of 1 to a maximum of 16 functions applied sequentlially on each line. Each function is a shell argument. If space or some specific shell char is used in a function definition, it must be quoted or escaped.

- `function/args... 'function/a r g s...' function/args...`

If the processor is not defined (i.e. no function defined), led will just output lines selected by the selector

Each function process the current line and transform it inplace.

#### Function syntax:

A function allways starts by the function name and the argument separator: `function/`. The argument separator can be replaced by `:` instead of `/` to facilitate the usage of `/` as char data.

The global format is `function/[regex][/arg1][/arg2]...`

Each function has a short name and a long name
- example:  `s|substitute`

The first argment (arg0) is allways a `regex`. It is used to identify a matching zone of the current line where the function is applied.

- if the regex is NOT defined (empty)
    - `function/`
    - `function//arg1`
    - the default regex is used (`.*`)
    - the function is applied on the whole line

- if the regex is defined
    - `function/Abc.+/`
    - `function/Abc.+/arg1`
    - the function is applied on the mathing zone only.

- if the regex is defined with capture block (...)
    - `function/Abc(\w+)/`
    - `function/Abc(\w+)/arg1`
    - the function is applied on the first capture group only (allows improved zone definition)

*Depending on the function, in some few cases, the regex can define multiple capture groups or can be not used*.

## PROCESSOR FUNCTION REFERENCE

### Substitute function

The `s|substitute` function allows to substitute string from a regex.

PCRE2 library substitution feature is used (see https://www.pcre.org/current/doc/html/pcre2_substitute.html).
`PCRE2_SUBSTITUTE_EXTENDED` option is used in order to have more substitution flexibility (see https://www.pcre.org/current/doc/html/pcre2api.html#SEC36).

`s|substitute/[regex]/replace[/opts]`

- regex: the search regex string
- replace: the replace string
- opts: character sequence of PCRE2 options
    - g: PCRE2_SUBSTITUTE_GLOBAL
    - e: PCRE2_SUBSTITUTE_EXTENDED
    - l: PCRE2_SUBSTITUTE_LITERAL

### Delete function

Delete line

`d|delete/`

### Insert function

Insert replaced line(s)

`i|insert/[regex]/<replace>[/N]`

- replace: the replaced string to insert before line
- N: number of line inserted, default 1

### Append function

Append replaced line(s)

`a|append/[regex]/<replace>[/N]`

- replace: the replaced string to append after line
- N: number of line appended, default 1

### Join function

 Join lines.
 This function needs selector `pack` mode to transmit all lines in the same buffer.

`j|join/`

### Delete blank function

 Delete blank lines.

`db|delete_blank/`

### Translate function

Translate characters string of a matching regex.

`tr|translate/[regex]/<src_chars>/<dst_chars>`

- src_chars: a sequence of source characters to be replaced by dest characters
- dst_chars: a sequence of dest characters

### Case functions

Convert to various case

`csl|case_lower/[regex]`

`csu|case_upper/[regex]`

`csf|case_first/[regex]`

`csc|case_camel/[regex]`

`css|case_snake/[regex]`

### Quote functions

Quote and unquote if needed.

`qt|quote_simple/[regex]`

`qtd|quote_double/[regex]`

`qtb|quote_back/[regex]`

`qtr|quote_remove/[regex]`

### Trim functions

`tm|trim/[regex]`

`tml|trim_left/[regex]`

`tmr|trim_right/[regex]`

### Split functions

 split line with given separators

`sp|split/[regex]/<sep_chars>`

split line with comma separators

`spc|split_csv/[regex]`

split line with space separators

`sps|split_space/[regex]`

split line with space and comma separators

`spm|split_mixed/[regex]`

### Revert function

revert the char order of the line

`rv|revert/[regex]`

### Field functions

 Extract fields of a line.

`fl|field/[regex]/<N>/<sep_chars>`

`flc|field_csv/[regex]/<N>`

`fls|field_space/[regex]/<N>`

`flm|field_mixed/[regex]/<N>`

- N: extract the Nth field, by default the first one.
- sep: separator chars

### Base64 encoding functions

 encode/decode lines.
 This function can work with selector `block` mode to encrypt a block of lines or a whole file.

`b64e|base64_encode/[regex]`

`b64d|base64_decode/[regex]`

### Url encoding function

 URL encode line or part of line.

`urle|url_encode/[regex]`

### Shell escaping functions

 Escape chars for shell executions.

`she|shell_escape/[regex]`

`shu|shell_unescape/[regex]`

### Path functions

Modify path in a line.

`rp|realpath/[regex]`

`dn|dirname/[regex]`

`bn|basename/[regex]`

### File name functions

Modify file name in a line, path prefix is not modified.

`fnl|fname_lower/[regex]`

`fnu|fname_upper/[regex]`

`fnf|fname_first/[regex]`

`fnc|fname_camel/[regex]`

`fns|fname_snake/[regex]`

### Randomize functions

Generate randomized characters

`rzn|randomize_num/[regex]`

`rza|randomize_alpha/[regex]`

`rzan|randomize_alnum/[regex]`

`rzh|randomize_hexa/[regex]`

`rzm|randomize_mixed/[regex]`

### Generate chars function

Generate randomized characters

`gen|generate/[regex]/<char>[/N]`

- char: the character to repeat
- N: char count, default is 1

### Range function

Extract a range of characters in the line

`rn|range_sel/[regex]/N[/C]`

Extract all characters in the line except for the range given

`rnu|range_unsel/[regex]/N[/C]`

- N: first column (if N is negative, it is relative to the end of line)
- C: character count, 1 by default

### Register functions

Registering line or part of line into one or more temporary registers (0 to 9)
A register can be used into a substitue replace string with using `$R[N]` notation.

`r|register/[regex][/N]`

- without N: each capture of the regex is copied into subsequent register IDs
- N: the regex capture is copied in the given register ID

`r/` => all the line into R0

`r/=(\w+)/` => first catched group of line into R0

`r/=(\w+)/2` => first catched group of line into R2

`r//1` => all the line into R1

`r/(\w+),(\w+)/` => all the matching zone into R0, first capture into R1, second capture into R2

Copy a register value (or part of the value) to line

`rr|register_recall/[regex][/N]`

- `[regex]`: regex to copy only a part of the register value
- `N`: the register ID to be copied, 0 if not defined.

`rr/` => R0 copied to the current line

`rr//1` => R1 copied to the current line

`rr/=(\w+)/1` => first catched group of R1 copied to the current line

## OPTIONS

### Selector options

- `-n` invert selection
- `-b` selected lines as blocks.
- `-s` output only selected

### File options

- `-f` read file names (paths) from STDIN instead of content, or from command line if followed by arguments as file names (file section)

following file options write filenames to STDOUT instead of file content. It allows advanced pipe mode on chained led invocations on multiple given files from STDIN. `-f` option is mandatory to use them.

- `-F` change each input file inplace.
- `-W<path>` write content to a fixed file
- `-A<path>` append content to a fixed file
- `-E<ext>`  write content to <file>.ext
- `-D<dir>`  write to same file names in a given target dir.

### Execution option

- `-X` execute each line (after processing) instead of output.

### Global options

- `-v` verbose to STDERR
- `-r` report to STDERR
- `-q` quiet, do not ouptut anything (exit code only)
- `-e` exit code on value

## EXIT CODES

Standard:
- `0` = match/change
- `1` = no match
- `2` = internal error

On value (see `-e` option):
- `0` = output not empty
- `1` = no match/change
- `2` = internal error

## EXAMPLES

### "grep" like

`cat file.txt | led <regex>`

`led <regex> -f file.txt`

### "sed" like for simple substitute

`led s/<regex>/<replace> -f file.txt`

`cat file.txt | led s/<regex>/<replace> -Wfile-changed.txt`

### change in-place:

`led s/<regex>/<replace> -F -f file.txt`

### massive multi change inplace:

`ls -1 *.txt | led s/<regex>/<replace> s/<regex>/<replace>  ... -F -f`

### massive command execution (rename files in camel case)

`find /path/to/dir -type f | led she/ r/ shu/ fnc/ 's//mv $R $0' -X`

## STATUS

- Documented features (README) are all implemented.
- Tests coverage is not fully completed (in progress).

Global current status is unstable.

## FUTURE PLANS

- Add man pages.
- Add hash and encryption functions
- Add negative shift on line selection
- re-write **led** in Rust !
