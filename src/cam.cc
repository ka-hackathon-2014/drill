#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>
#include <functional>

#include "debug.h"
#include "cam.h"

namespace drill {

namespace {
  // no signum function in stdlib, so use this instead
  template <typename T>
  int sgn(T val)
  {
    return (T(0) < val) - (val < T(0));
  }
}

void cam::interact(bool ui, std::size_t fps, std::size_t slice_length, double threshold) try
{

  // max number of Points in a sliding window pre-calculatable, assumes fixed fps -- heuristic
  auto max_points = static_cast<std::size_t>(slice_length / 1000.f * fps);

  /*
   * sliding window stores N+M events from a timeslice:
   *
   * | ..    . . | .       ...|
   *       N    t/2     M
   *
   * estimate direction by: sign((sum(M_items) / M) - (sum(N_items) / N))
   */
  bounded_queue<cv::Point> sliding_window{max_points};

  if (ui)
    cv::namedWindow("outputCapture", 1);

  std::vector<cv::Rect> faces;
  int direction = 0;

  // eventloop
  while (!shutdown_) {
    device_ >> frame_;
    faces = get_faces(frame_);

    // no faces detected, no need to evaluate any further here
    if (faces.size() == 0)
      continue;

    const auto& face = faces.front();
    cv::Point center{face.x + face.width / 2, face.y + face.height / 2};

    if (ui) {
      circle(frame_, center, 20, cvScalar(0, 0, 255, 0), -1);
      cv::imshow("outputCapture", frame_);

      if (cv::waitKey(static_cast<int>(fps)) >= 0)
        break;
    }

    // rolling window
    sliding_window.enqueue(std::move(center));

    auto it = std::begin(sliding_window), mid = it + sliding_window.size() / 2, end = std::end(sliding_window);

    auto accu_lhs = std::accumulate(it, mid, cv::Point{0, 0}, std::plus<cv::Point>{});
    auto accu_rhs = std::accumulate(mid, end, cv::Point{0, 0}, std::plus<cv::Point>{});

    auto estimate_y = accu_rhs.y - accu_lhs.y, estimate_x = accu_rhs.x - accu_lhs.x;
    (void)estimate_x; // x direction not needed for now: up, down only

    auto current_direction = sgn(estimate_y);

    // near face requires higher threshold, assume rectangle-shaped face
    auto scaled_threshold = static_cast<double>(face.height) * threshold;

    // check for triggered event; filter jitter
    if (current_direction != direction && static_cast<double>(std::abs(estimate_y)) > scaled_threshold) {
      direction = current_direction;

      auto x = static_cast<double>(face.x), y = static_cast<double>(face.y);
      extraction_q_.enqueue(EvtMovementChange{x, y, direction});
    }

    // next frame please
    faces.clear();
  }

  // if we jumped out of the eventloop, force all stages to shutdown
  throw shutdown_t{};
}
catch (...)
{
  // destructor does the shutdown already
}

std::vector<cv::Rect> cam::get_faces(const cv::Mat& frame, double min_depth, double max_depth)
{
  std::vector<cv::Rect> faces;
  cv::Mat gray;

  cv::cvtColor(frame, gray, CV_BGR2GRAY);
  cv::equalizeHist(gray, gray);

  // constrain depth by heuristic: face has to be of size [min_depth, max_depth] * frame's size
  auto frame_size = gray.size();
  const auto min_face = static_cast<int>(min_depth * (std::min)(frame_size.height, frame_size.width));
  const auto max_face = static_cast<int>(max_depth * (std::min)(frame_size.height, frame_size.width));

  // face-detect
  cascade_.detectMultiScale(gray, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE,
                            cv::Size(min_face, min_face), cv::Size(max_face, max_face));

  // assert single dominant object
  if (faces.size() > 1) {
    out()([&](std::ostream& out) { out << "Error: more than one dominant face" << std::endl; });
    throw shutdown_t{};
  }

  return faces;
}
}
