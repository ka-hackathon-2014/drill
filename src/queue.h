#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <deque>
#include <mutex>
#include <list>
#include <iterator>

namespace drill {

// thread safe queue, using the tmp list-splice trick for minimal contention
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


// fast, simple, value-semantic using, bounded queue
template <typename T>
class bounded_queue {
public:
  using queue_type = std::deque<T>;
  using iterator_type = typename queue_type::iterator;
  using size_type = typename queue_type::size_type;

  bounded_queue(std::size_t bound) : bound_{bound}
  {
  }

  void enqueue(T x)
  {
    if (q_.size() >= bound_)
      q_.pop_front();

    q_.push_back(std::move(x));
  }

  size_type size()
  {
    return q_.size();
  }

  bool empty()
  {
    return q_.empty();
  }

  T front()
  {
    return q_.front();
  }

  T back()
  {
    return q_.back();
  }

  iterator_type begin()
  {
    return std::begin(q_);
  }

  iterator_type end()
  {
    return std::end(q_);
  }

private:
  queue_type q_;
  std::size_t bound_;
};
}

#endif
