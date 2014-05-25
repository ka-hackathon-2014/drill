#include <cstdlib>
#include <thread>
#include <string>
#include <atomic>
#include <map>

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
  concurrent_queue<std::unique_ptr<EvtCamera>> extraction_q;
  concurrent_queue<std::unique_ptr<EvtEffect>> classification_q;


  std::string classifier{args.size() > 1 ? args[1] : "classifier/haarcascade_frontalface_alt.xml"};
  bool ui = true;
  bool verbose = true;


  // Extraction
  std::thread extraction{[&] {
    const lifetime sentry{"extraction", verbose};

    cam cam{extraction_q, shutdown, classifier};
    cam.interact(ui);
  }};


  // Classification
  std::thread classification{[&] {
    const lifetime sentry{"classification", verbose};

    run_classification(extraction_q, classification_q, shutdown);
  }};


  // Sound
  std::thread sound{[&] {
    const lifetime sentry{"sound", verbose};

    run_sound(classification_q, shutdown);
  }};


  // Wait for all
  sound.join();
  classification.join();
  extraction.join();
}
