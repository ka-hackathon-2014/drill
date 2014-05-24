#include <iostream>
#include <string>
#include <vector>
#include <iterator>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


int main(int argc, char** argv)
{
  std::vector<std::string> args{argv, argv + argc};
  if (args.size() != 2) return -1;

  cv::CascadeClassifier face_cascade;
  face_cascade.load(args[1]);

  cv::VideoCapture captureDevice;
  captureDevice.open(0);

  cv::Mat captureFrame;
  cv::Mat grayscaleFrame;

  cv::namedWindow("outputCapture", 1);

  while (true) {
    captureDevice >> captureFrame;

    cv::cvtColor(captureFrame, grayscaleFrame, CV_BGR2GRAY);
    cv::equalizeHist(grayscaleFrame, grayscaleFrame);

    std::vector<cv::Rect> faces;

    face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE,
                                  cv::Size(30, 30));

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
