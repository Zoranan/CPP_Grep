# CPP Grep

A regex library written in C++, Compilable with C++98 (See below)


## Features

This is a WIP. Any known broken features will be noted below.

- Char classes
- Negated char classes
- Branches (|)
- Any char (.) <== Currently always in single-line mode
- Capture (and non capturing) groups
- Standard greedy and non-greedy quantifiers
- Escape chars: \r \n \t \f
- Special classes: \d \w \s \b and \D \W \S \B
- Hex char codes with \x00 -\xFF
- ASCII codes with \u0000 -\u0255 (leading 0's optional unless followed by a literal digit)
- Beginning of line (^)	<== Currently always in multiline mode
- End of line ($) 		<== This isn't working right when used with lazy quantifiers. It forces the lazy quantifier to drag itself out until it reaches its max value or the end of line is reached.


## Capture Groups

Capture groups are enclosed in parenthesis. If you don't want output to be captured, use (?: to start your group

- Each group can capture multiple values if it is quantified
- Groups can be nested and captures will be made for each
- Capture groups are numbered in the order that their left parenthesis appear in


## C++ 98 Compatability

To compile this library for C++ 98, you will need to make sure the "defines.h" has the following two lines uncommented:

```
#define override
#define nullptr 0
```

## TODO

- Mostly this needs a lot of ad-hoc testing to find bugs and correct them. 
- Automated testing should be created as well
- Multiline and Singleline modes need to be implemented
- Better exceptions and exception handling needs to be added to provide useful error messages
- The end of line needs to be fixed so it works properly with lazy quantifiers
- Enable regex to parse over unsigned strings to allow reading of binary files


## Nice to Have Features
- Back-references to previous capture groups (\1, \2, etc...)
