#include <vector>
#include <stdexcept>

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

    face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE,
                                  cv::Size(30, 30));

    if (ui) {
      for (const auto& face : faces) {
        cv::Point pt1(face.x + face.width, face.y + face.height);
        cv::Point pt2(face.x, face.y);

        rectangle(captureFrame, pt1, pt2, cvScalar(0, 255, 0, 0), 1, 8, 0);
      }

      cv::imshow("outputCapture", captureFrame);

      if (cv::waitKey(30) >= 0)
        break;
    }
  }
}
catch (...)
{
  // destructor does the shutdown already
}
}
