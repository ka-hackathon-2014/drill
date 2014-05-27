#ifndef CAM_H
#define CAM_H

#include <opencv2/core/core.hpp>           // for Mat
#include <opencv2/core/mat.hpp>            // for Mat::~Mat
#include <opencv2/highgui/highgui.hpp>     // for VideoCapture
#include <opencv2/objdetect/objdetect.hpp> // for CascadeClassifier
#include <algorithm>                       // for move
#include <atomic>                          // for atomic, atomic_bool
#include <chrono>                          // for high_resolution_clock
#include <cstddef>                         // for size_t
#include <memory>                          // for unique_ptr
#include <ostream>                         // for operator<<, ostream, etc
#include <string>                          // for string
#include <vector>                          // for vector

#include "debug.h"   // for out
#include "monitor.h" // for monitor

namespace drill {

struct EvtCamera;
template <typename T>
class concurrent_queue;

class cam {

public:
  cam(concurrent_queue<std::unique_ptr<EvtCamera>>& q, std::atomic<bool>& shutdown, std::string classifier)
      : extraction_q_(q), shutdown_(shutdown), classifier_{std::move(classifier)}
  {
    // default device
    device_.open(0);

    // train classifier
    if (!cascade_.load(classifier_)) {
      out()([&](std::ostream& out) { out << "Error: bad classifier file" << std::endl; });
      shutdown_ = true;
    }
  }

  ~cam()
  {
    shutdown_ = true;
  }

  void interact(bool ui = false, std::size_t fps = 30, std::size_t slice_length = 1000, double threshold = 0.25);

private:
  concurrent_queue<std::unique_ptr<EvtCamera>>& extraction_q_;
  std::atomic<bool>& shutdown_;
  std::string classifier_;

  cv::Mat frame_;
  cv::VideoCapture device_;
  cv::CascadeClassifier cascade_;

  std::vector<cv::Rect> get_faces(const cv::Mat& frame, double min_depth = 0.15, double max_depth = 0.5);

  using clock_t_ = std::chrono::high_resolution_clock;

  struct shutdown_t {
  };
};
}

#endif
