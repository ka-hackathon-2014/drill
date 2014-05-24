#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include "monitor.h"
#include "queue.h"
#include "event.h"
#include "cam.h"
#include "classification.h"


using namespace drill;

int main(int argc, char** argv)
{
  std::vector<std::string> args{argv, argv + argc};
  (void)args; // command line args needed?

  std::atomic<bool> shutdown{false};

  // thread-safe cout, used by all stages
  monitor<std::ostream&> sync_cout{std::cout};

  concurrent_queue<EvtMovementChange> extraction_q;
  concurrent_queue<EvtEffect> classification_q;

  std::thread extraction{[&] {
    std::string classifier;

    if (args.size() == 2)
      classifier = args[1];
    else
      classifier = "/usr/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";

    cam cam{extraction_q, shutdown, classifier};
    cam.interact();

    sync_cout([&](std::ostream& out) { out << "[camera]: shutdown" << std::endl; });
  }};

  std::thread classification{[&] {
    run_classification(extraction_q, shutdown);

    sync_cout([&](std::ostream& out) { out << "[classification]: shutdown" << std::endl; });
  }};

  std::thread sound{[&] {
    sync_cout([&](std::ostream& out) { out << "[sound]: shutdown" << std::endl; });
  }};

  sound.join();
  classification.join();
  extraction.join();
}
