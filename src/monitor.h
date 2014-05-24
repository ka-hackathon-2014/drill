#ifndef MONITOR_H
#define MONITOR_H

#include <mutex>

namespace drill {

template <typename T>
class monitor {
private:
  /* mutable */ T t;
  mutable std::mutex m;

public:
  monitor(T t_ = T{}) : t(t_)
  {
  }

  template <typename F>
  auto operator()(F f) const -> decltype(f(t))
  {
    std::lock_guard<std::mutex> _{m};
    return f(t);
  }
};
};

#endif
