#ifndef _DART_JVM_UTILS_H
#define _DART_JVM_UTILS_H

#include <iostream>
#include <sstream>

#define STRING_STREAM(args) \
    ((std::ostringstream&)(std::ostringstream() << args)).str()

class AutoEnterIsolate {
 public:
  AutoEnterIsolate(Dart_Isolate isolate) {
    Dart_EnterIsolate(isolate);
  }
  ~AutoEnterIsolate() {
    Dart_ExitIsolate();
  }
};

class AutoEnterScope {
 public:
  AutoEnterScope() {
    Dart_EnterScope();
  }
  ~AutoEnterScope() {
    Dart_ExitScope();
  }
};

#endif  // _DART_JVM_UTILS_H
