#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <chrono>

#include "monitor.h"

namespace drill {

// thread-safe singletonc
static monitor<std::ostream&>& out()
{
  static monitor<std::ostream&> sync_cout{std::cout};
  return sync_cout;
}

// RAII debug lifetime messages
class lifetime {
public:
  lifetime(std::string name) : name_{std::move(name)}
  {
    out()([&](std::ostream& out) {
      out << "[" << now() << "]"
          << "[" << name_ << "]: initialized" << std::endl;
    });
  }

  ~lifetime()
  {
    out()([&](std::ostream& out) {
      out << "[" << now() << "]"
          << "[" << name_ << "]: shutdown" << std::endl;
    });
  }

private:
  std::string name_;

private:
  // return type is kind of ugly -- let the type system infer it!
  auto now() -> decltype(std::chrono::high_resolution_clock::now().time_since_epoch().count())
  {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
  }
};
// thread-safe cout, used by all stages
}

#endif
