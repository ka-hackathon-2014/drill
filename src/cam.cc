#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <chrono>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "debug.h"
#include "cam.h"

namespace drill {

struct shutdown_t {
};

void cam::interact(bool ui) try
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
   * accumulator stores N+M events from a timeslice:
   *
   * | ..    . . | .       ...|
   *       N    t/2     M
   *
   * estimate direction by: sign((sum(N_items) / N) - (sum(M_items) / M))
   */
  std::vector<cv::Point> accumulator_lhs;
  std::vector<cv::Point> accumulator_rhs;

  // heuristic timeslice length
  auto slice_length = std::chrono::milliseconds(200);

  std::chrono::time_point<std::chrono::high_resolution_clock> slice_start;
  std::chrono::time_point<std::chrono::high_resolution_clock> slice_now;

  // eventloop
  while (!shutdown_) {
    slice_start = std::chrono::high_resolution_clock::now();

    captureDevice >> captureFrame;

    cv::cvtColor(captureFrame, grayscaleFrame, CV_BGR2GRAY);
    cv::equalizeHist(grayscaleFrame, grayscaleFrame);

    // constrain distance by heuristic: face has to be of size [5%, 50%] * frame's size
    auto frame_size = grayscaleFrame.size();
    auto min_face = static_cast<int>(0.15f * (std::min)(frame_size.height, frame_size.width)); // 15% min
    auto max_face = static_cast<int>(0.5f * (std::min)(frame_size.height, frame_size.width));  // 50% max

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

      if (cv::waitKey(30) >= 0)
        break;
    }

    // XXX: dummy events for now
    auto e = EvtMovementChange{static_cast<double>(face.x), static_cast<double>(face.y)};
    extraction_q_.enqueue(e);

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
