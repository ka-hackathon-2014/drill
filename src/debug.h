#ifndef DEBUG_H
#define DEBUG_H

#include <chrono>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

#include "monitor.h"

namespace drill {

// thread-safe cout, used by all stages
static monitor<std::ostream&>& out()
{
  static monitor<std::ostream&> sync_cout{std::cout};
  return sync_cout;
}

// RAII-style debug lifetime messages
class lifetime {
public:
  lifetime(std::string name, bool verbose) : name_{std::move(name)}, verbose_{verbose}
  {
    if (verbose_)
      out()([&](std::ostream& out) { out << name_ << ": initialized" << std::endl; });
  }

  ~lifetime()
  {
    if (verbose_)
      out()([&](std::ostream& out) { out << name_ << ": shutdown" << std::endl; });
  }

private:
  std::string name_;
  bool verbose_;
};
}

#endif
