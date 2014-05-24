#include <cstdlib>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include "debug.h"
#include "queue.h"
#include "event.h"
#include "cam.h"
#include "classification.h"


using namespace drill;

int main(int argc, char** argv)
{
  std::vector<std::string> args{argv, argv + argc};

  std::atomic<bool> shutdown{false};

  concurrent_queue<EvtMovementChange> extraction_q;
  concurrent_queue<EvtEffect> classification_q;


  std::thread extraction{[&] {
    lifetime sentry{"extraction"};

    std::string classifier;

    if (args.size() == 2)
      classifier = args[1];
    else
      classifier = "/usr/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";

    cam cam{extraction_q, shutdown, classifier};
    cam.interact();
  }};


  std::thread classification{[&] {
    lifetime sentry{"classification"};

    run_classification(extraction_q, classification_q, shutdown);
  }};


  std::thread sound{[&] {
    lifetime sentry{"sound"};

    while (!shutdown)
      ; /* spin */
  }};


  sound.join();
  classification.join();
  extraction.join();
}
