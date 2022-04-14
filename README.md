# CPP Grep

A regex library written in C++


## Features

This is a WIP. Many features are likely broken. Any know broken features will be noted below.

- Char classes (negated as well)
- Branches (|)
- Any char (.) <== Currently always in single-line mode
- Capture (and non capturing) groups
- Standard greedy and non-greedy quantifiers
- Escape chars: \r \n \t \f
- Special classes: \d \w \s
- Beginning of line (^) <== Currently always in multiline mode
- End of line ($) <== This isn't working right, specifically when used with lazy quantifiers. It forces the lazy quantifier to drag itself out until it reaches its max value or the end of line is reached.


## Capture Groups

Capture groups are enclosed in parenthesis. If you don't want output to be captured, use (?: to start your group

There is currently an issue when quantifiying capture groups with greedy quantifiers where the match will succeed, but output will not be captured.

- Each group can capture multiple values if it is quantified
- Groups can be nested
- Capture groups are numbered in the order that their left parenthesis appear in


## TODO

- Mostly this needs a lot of ad-hoc testing to find the glaring bugs and correct them. 
- Automated testing should be created as well
- Multiline and Singleline modes need to be implemented and tested
- Case insensitive mode needs to be fixed for CharRange
