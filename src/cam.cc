#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>
#include <functional>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "debug.h"
#include "cam.h"

namespace drill {

struct shutdown_t {
};

void cam::interact(bool ui, std::size_t fps, std::size_t slice_length, double threshold) try
{
  cv::CascadeClassifier face_cascade;
  if (!face_cascade.load(classifier_)) {
    out()([&](std::ostream& out) { out << "Error: bad classifier file" << std::endl; });
    throw shutdown_t{};
  }

  cv::VideoCapture captureDevice;
  captureDevice.open(0); // default device

  cv::Mat captureFrame;
  cv::Mat grayscaleFrame;

  if (ui)
    cv::namedWindow("outputCapture", 1);

  std::vector<cv::Rect> faces;

  /*
   * sliding window stores N+M events from a timeslice:
   *
   * | ..    . . | .       ...|
   *       N    t/2     M
   *
   * estimate direction by: sign((sum(M_items) / M) - (sum(N_items) / N))
   */

  // max number of Points in a sliding window
  bounded_queue<cv::Point> sliding_window{static_cast<std::size_t>(slice_length / 1000.f * fps)};

  // no signum function in stdlib, so use this instead
  auto sgn = [](int val) { return (0 < val) - (val < 0); };

  int old_direction = 0;


  // eventloop
  while (!shutdown_) {
    captureDevice >> captureFrame;

    cv::cvtColor(captureFrame, grayscaleFrame, CV_BGR2GRAY);
    cv::equalizeHist(grayscaleFrame, grayscaleFrame);

    // constrain distance by heuristic: face has to be of size [5%, 50%] * frame's size
    auto frame_size = grayscaleFrame.size();
    const auto min_face = static_cast<int>(0.15f * (std::min)(frame_size.height, frame_size.width)); // 15% min
    const auto max_face = static_cast<int>(0.5f * (std::min)(frame_size.height, frame_size.width));  // 50% max

    // face-detect
    face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE,
                                  cv::Size(min_face, min_face), cv::Size(max_face, max_face));

    // assert single biggest object
    if (faces.size() > 1) {
      out()([&](std::ostream& out) { out << "Error: more than one dominant face" << std::endl; });
      throw shutdown_t{};
    }

    // no faces detected, no need to evaluate any further here
    if (faces.size() == 0)
      continue;

    const auto& face = faces.front();
    cv::Point center{face.x + face.width / 2, face.y + face.height / 2};

    if (ui) {
      circle(captureFrame, center, 20, cvScalar(0, 0, 255, 0), -1);
      cv::imshow("outputCapture", captureFrame);

      if (cv::waitKey(static_cast<int>(fps)) >= 0)
        break;
    }

    sliding_window.enqueue(std::move(center));

    auto it = std::begin(sliding_window);
    auto mid = it + sliding_window.size() / 2;
    auto end = std::end(sliding_window);

    auto accu_lhs = std::accumulate(it, mid, cv::Point{0, 0}, std::plus<cv::Point>{});
    auto accu_rhs = std::accumulate(mid, end, cv::Point{0, 0}, std::plus<cv::Point>{});

    // x direction not needed for now: up, down only
    auto estimate_y = accu_rhs.y - accu_lhs.y;
    // auto estimate_x = accu_rhs.x - accu_lhs.x;

    auto direction = sgn(estimate_y);

    // near face requires higher threshold, assume rectangle-shaped face
    auto scaled_threshold = static_cast<double>(face.height) * threshold;

    // check for triggered event; filter jitter
    if (direction != old_direction && static_cast<double>(std::abs(estimate_y)) > scaled_threshold) {
      old_direction = direction;

      auto x = static_cast<double>(face.x);
      auto y = static_cast<double>(face.y);
      auto event = EvtMovementChange{x, y, direction};

      extraction_q_.enqueue(event);
    }

    // next frame please
    faces.clear();
  }

  // if we jumped out of the eventloop, force a shutdown
  throw shutdown_t{};
}
catch (...)
{
  // destructor does the shutdown already
}
}
