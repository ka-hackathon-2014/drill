#ifndef CAM_H
#define CAM_H

#include <cstddef>
#include <string>
#include <atomic>
#include <chrono>
#include <memory>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "queue.h"
#include "event.h"
#include "debug.h"

namespace drill {

class cam {

public:
  cam(concurrent_queue<std::unique_ptr<EvtCamera>>& q, std::atomic<bool>& shutdown, std::string classifier)
      : extraction_q_{q}, shutdown_{shutdown}, classifier_{std::move(classifier)}
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

  void interact(bool ui = false, std::size_t fps = 30, std::size_t slice_length = 200, double threshold = 0.75);

private:
  concurrent_queue<std::unique_ptr<EvtCamera>>& extraction_q_;
  std::atomic<bool>& shutdown_;
  std::string classifier_;

  cv::Mat frame_;
  cv::VideoCapture device_;
  cv::CascadeClassifier cascade_;

  std::vector<cv::Rect> get_faces(const cv::Mat& frame, double min_depth = 0.15, double max_depth = 0.5);

  using clock_t_ = std::chrono::high_resolution_clock;
  using epoch_t_ = decltype(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  struct shutdown_t {
  };
};
}

#endif
