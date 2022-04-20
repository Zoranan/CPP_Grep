#pragma once

// This header contains defines used in multiple files

// nullptr and override arent supported in C++ 98, so we have it as a define here to easily change

//#define override
//#define nullptr 0


// Regex char classes are usually in square brackets, but some systems (Guardian) interpret those characters on the command line for variable expansion.
#define OPEN_CLASS_STR "["
#define CLOSE_CLASS_STR "]"

#define OPEN_CLASS_C '['
#define CLOSE_CLASS_C ']'

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFF
#endif

