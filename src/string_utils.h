#ifndef _DART_JVM_STRING_UTILS_H
#define _DART_JVM_STRING_UTILS_H

#include <iostream>
#include <sstream>

#define STRING_STREAM(args) \
    ((std::ostringstream&)(std::ostringstream() << args)).str()

#endif  // _DART_JVM_STRING_UTILS_H
