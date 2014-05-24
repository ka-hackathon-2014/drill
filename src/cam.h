#ifndef CAM_H
#define CAM_H

#include <cstddef>
#include <string>
#include <atomic>
#include <chrono>

#include "queue.h"
#include "event.h"

namespace drill {

class cam {

public:
  cam(concurrent_queue<EvtMovementChange>& q, std::atomic<bool>& shutdown, std::string classifier)
      : extraction_q_{q}, shutdown_{shutdown}, classifier_{std::move(classifier)}
  {
  }

  ~cam()
  {
    shutdown_ = true;
  }

  void interact(bool ui = false, std::size_t fps = 30, std::size_t slice_length = 200, double threshold = 0.75);

private:
  concurrent_queue<EvtMovementChange>& extraction_q_;
  std::atomic<bool>& shutdown_;
  std::string classifier_;

  using clock_t_ = std::chrono::high_resolution_clock;
  using epoch_t_ = decltype(std::chrono::high_resolution_clock::now().time_since_epoch().count());
};
}

#endif
