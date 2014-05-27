#include <atomic> // for atomic, atomic_bool
#include <memory> // for unique_ptr
#include <string> // for string
#include <thread> // for thread
#include <vector> // for vector

#include "cam.h"            // for cam
#include "classification.h" // for run_classification
#include "debug.h"          // for lifetime
#include "queue.h"          // for concurrent_queue
#include "sound.h"          // for run_sound
#include "event.h"


#ifdef WIN32
// no ctrl-c
#else
#include <signal.h> // for sigaction, SIGINT, etc
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
    struct sigaction act {
    };
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
  bool verbose = false;


  // Extraction
  std::thread extraction{[&] {
    const lifetime sentry{"extraction", verbose};

    cam cam{extraction_q, shutdown, classifier};
    cam.interact(ui);
  }};


  // Classification
  std::thread classification{[&] {
    const lifetime sentry{"classification", verbose};

    run_classification(extraction_q, classification_q, shutdown, verbose);
  }};


  // Sound
  std::thread sound{[&] {
    const lifetime sentry{"sound", verbose};

    run_sound(classification_q, shutdown, verbose);
  }};


  // Wait for all
  sound.join();
  classification.join();
  extraction.join();
}
