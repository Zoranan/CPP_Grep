# CPP Grep

A regex library written in C++


## Features

This is a WIP. Any known broken features will be noted below.

- Char classes
- Negated char classes
- Branches (|)
- Any char (.) <== Currently always in single-line mode
- Capture (and non capturing) groups
- Standard greedy and non-greedy quantifiers
- Escape chars: \r \n \t \f
- Special classes: \d \w \s and \D \W \S
- Beginning of line (^)	<== Currently always in multiline mode
- End of line ($) 		<== This isn't working right when used with lazy quantifiers. It forces the lazy quantifier to drag itself out until it reaches its max value or the end of line is reached.


## Capture Groups

Capture groups are enclosed in parenthesis. If you don't want output to be captured, use (?: to start your group

- Each group can capture multiple values if it is quantified
- Groups can be nested and captures will be made for each
- Capture groups are numbered in the order that their left parenthesis appear in


## TODO

- Mostly this needs a lot of ad-hoc testing to find bugs and correct them. 
- Automated testing should be created as well
- Multiline and Singleline modes need to be implemented
- Case insensitive mode needs to be implemented for CharRange
- Better exceptions and exception handling needs to be added to provide useful error messages
- The end of line needs to be fixed so it works properly with lazy quantifiers


## Nice to Have Features
- Back-references to previous capture groups (\1, \2, etc...)
- Support for substitution operator similar to perl for outputting a custom string from captured groups
