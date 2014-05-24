#include <cstdlib>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include "signal.h"
#include "debug.h"
#include "queue.h"
#include "event.h"
#include "cam.h"
#include "classification.h"
#include "sound.h"

#ifdef WIN32
// no ctrl-c
#else
#include <signal.h>
#endif

using namespace drill;

// signal shutdown for all stages
static std::atomic<bool> shutdown{false};


int main(int argc, char** argv)
{
  std::vector<std::string> args{argv, argv + argc};

#ifdef WIN32
// no ctrl-c
#else
  {
    struct sigaction act;
    act.sa_handler = [](int) { shutdown = true; };
    ::sigaction(SIGINT, &act, nullptr);
  }
#endif

  /*
   * Staged architecture: passing events through queues
   * [extraction] --> [classification] --> [sound]
   */
  concurrent_queue<EvtMovementChange> extraction_q;
  concurrent_queue<EvtEffect> classification_q;


  // Extraction
  std::thread extraction{[&] {
    lifetime sentry{"extraction"};

    std::string classifier;
    bool ui = true; // XXX: from args

    if (args.size() == 2)
      classifier = args[1];
    else
      classifier = "classifier/haarcascade_frontalface_alt.xml";

    cam cam{extraction_q, shutdown, classifier};
    cam.interact(ui);
  }};


  // Classification
  std::thread classification{[&] {
    lifetime sentry{"classification"};

    run_classification(extraction_q, classification_q, shutdown);
  }};


  // Sound
  std::thread sound{[&] {
    lifetime sentry{"sound"};
    run_sound(classification_q, shutdown);
  }};


  // Wait for all
  sound.join();
  classification.join();
  extraction.join();
}
