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

namespace {
// signal shutdown for all stages
std::atomic<bool> shutdown{false};

// merge environment variables with commandline arguments
std::map<std::string, std::string> get_env(int argc, char** argv);
}


int main(int argc, char** argv)
{
  const auto env = get_env(argc, argv);

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


  bool ui = env.at("DRILL_SHOWSELF") != "";
  bool verbose = env.at("DRILL_VERBOSE") != "";

  std::size_t fps = 30;
  std::size_t slice_length = 200;
  double threshold = 0.75;

  try
  {
    fps = static_cast<std::size_t>(std::stoull(env.at("DRILL_GUIFPS")));
    slice_length = static_cast<std::size_t>(std::stoull(env.at("DRILL_CAMSLICELENGTH")));
    threshold = std::stod(env.at("DRILL_CAMTHRESHOLD"));
  }
  catch (...)
  {
    // conversion failed, use default values
    out()([&](std::ostream& out) {
      out << "Warning: some environment variables could not be read, using default" << std::endl;
    });
  }


  // Extraction
  std::thread extraction{[&] {
    const lifetime sentry{"extraction", verbose};

    cam cam{extraction_q, shutdown, env.at("DRILL_CLASSIFIER")};
    cam.interact(ui, fps, slice_length, threshold);
  }};


  // Classification
  std::thread classification{[&] {
    const lifetime sentry
    {
      "classification", verbose
    }

    run_classification(extraction_q, classification_q, shutdown);
  }};


  // Sound
  std::thread sound{[&] {
    const lifetime sentry
    {
      "sound", verbose
    }

    run_sound(classification_q, shutdown);
  }};


  // Wait for all
  sound.join();
  classification.join();
  extraction.join();
}

namespace {

std::map<std::string, std::string> get_env(int argc, char** argv)
{
  std::vector<std::string> args{argv, argv + argc};
  std::string classifier;

  if (args.size() > 1)
    classifier = args[1];
  else
    classifier = "classifier/haarcascade_frontalface_alt.xml";

  // recognizing the following hidden environment variables
  std::map<std::string, std::string> env{ //
      {"DRILL_VERBOSE", {}},              //
      {"DRILL_SHOWSELF", {}},             //
      {"DRILL_GUIFPS", {}},               //
      {"DRILL_CAMTHRESHOLD", {}},         //
      {"DRILL_CAMSLICELENGTH", {}},       //
  };

  // merge with args
  env.emplace("DRILL_CLASSIFIER", std::move(classifier));

  for (auto& kv : env) {
    // string(nullptr) is undefined behavior, convert: nullptr -> ""
    const char* e = std::getenv(kv.first.c_str());
    std::string v{e ? e : ""};

    // use default if not user-specified
    if (v != "")
      kv.second = std::move(v);
  }

  return env;
}
}
