#ifndef QUEUE_H
#define QUEUE_H

#include <mutex>
#include <list>
#include <iterator>

namespace drill {

template <typename T>
class concurrent_queue {
public:
  using queue_type = std::list<T>;

  bool enqueue(T x)
  {
    bool was_dry;

    queue_type tmp;
    tmp.push_back(std::move(x));

    {
      std::lock_guard<std::mutex> lock(mutex_);
      was_dry = was_dry_;
      was_dry_ = false;
      q_.splice(std::end(q_), tmp);
    }

    return was_dry;
  }

  queue_type dequeue()
  {
    queue_type result;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (q_.empty())
        was_dry_ = true;
      else
        result.splice(std::end(result), q_, std::begin(q_));
    }

    return result;
  }

private:
  std::mutex mutex_;
  queue_type q_;
  bool was_dry_ = true;
};
}

#endif
