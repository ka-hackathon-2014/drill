#ifndef CAM_H
#define CAM_H

#include <string>
#include <atomic>

#include "queue.h"
#include "event.h"

namespace drill {

class cam {

public:
  cam(concurrent_queue<EvtMovementChange>& q, std::atomic<bool>& shutdown,
      std::string classifier = "/usr/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml")
      : extraction_q_{q}, shutdown_{shutdown}, classifier_{std::move(classifier)}
  {
  }

  ~cam()
  {
    shutdown_ = true;
  }

  void interact();

private:
  concurrent_queue<EvtMovementChange>& extraction_q_;
  std::atomic<bool>& shutdown_;
  std::string classifier_;
};
}

#endif
