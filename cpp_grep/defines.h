#pragma once

// This header contains defines used in multiple files

// nullptr isn't supported in C++ 98, so we have it as a define here to easily change it
#define NULLPTR nullptr

// Regex char classes are usually in square brackets, but some systems (Guardian) interpret those characters on the command line for variable expansion.
#define OPEN_CLASS_STR "["
#define CLOSE_CLASS_STR "]"

#define OPEN_CLASS_C '['
#define CLOSE_CLASS_C ']'
