#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include "queue.h"
#include "event.h"
#include "cam.h"
#include "classification.h"


using namespace drill;

int main(int argc, char** argv)
{
  std::vector<std::string> args{argv, argv + argc};
  (void)args; // command line args needed?

  std::atomic<bool> shutdown{0};

  concurrent_queue<EvtMovementChange> extraction_q;
  concurrent_queue<EvtEffect> classification_q;

  std::thread extraction{[&] {
    cam cam{extraction_q, shutdown};
    cam.interact();
  }};

  std::thread classification{[&] {
    run_classification(extraction_q);
  }};

  std::thread sound{[&] {}};

  sound.join();
  classification.join();
  extraction.join();
}
