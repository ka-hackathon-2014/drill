#include <vector>
#include <stdexcept>
#include <algorithm>

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

  while (true) {
    if (shutdown_)
      throw shutdown_t{};

    faces.clear();

    captureDevice >> captureFrame;

    cv::cvtColor(captureFrame, grayscaleFrame, CV_BGR2GRAY);
    cv::equalizeHist(grayscaleFrame, grayscaleFrame);

    // constrain distance by heuristic: face has to be of size [5%, 50%] * frame's size
    auto frame_size = grayscaleFrame.size();
    auto min_face = static_cast<int>(0.05f * (std::min)(frame_size.height, frame_size.width)); // 5% min
    auto max_face = static_cast<int>(0.5f * (std::min)(frame_size.height, frame_size.width));  // 50% max

    face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE,
                                  cv::Size(min_face, min_face), cv::Size(max_face, max_face));

    // assert single biggest object
    if (faces.size() > 1) {
      out()([&](std::ostream& out) { out << "Error: more than one dominant face" << std::endl; });
      throw shutdown_t{};
    }

    const auto& face = faces.front();

    if (ui) {
      cv::Point pt1(face.x + face.width, face.y + face.height);
      cv::Point pt2(face.x, face.y);

      rectangle(captureFrame, pt1, pt2, cvScalar(0, 255, 0, 0), 1, 8, 0);

      cv::imshow("outputCapture", captureFrame);

      if (cv::waitKey(30) >= 0)
        break;
    }

    // XXX: dummy events for now
    auto e = EvtMovementChange{static_cast<double>(face.x), static_cast<double>(face.y)};
    extraction_q_.enqueue(e);
  }
}
catch (...)
{
  // destructor does the shutdown already
}
}
