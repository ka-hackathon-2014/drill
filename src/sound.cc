#include "sound.h"
#include "debug.h"
#include "audioxx.h"

#include <map>
#include <sstream>
#include <thread>

namespace drill {
void run_sound(concurrent_queue<EvtEffect>& classification_q, std::atomic<bool>& shutdown) try
{
  Audioxx::Player player;
  std::map<std::string, Audioxx::Buffer> buffers;
  for (const auto& name : {"cola"}) {
    std::stringstream ss;
    ss << "sound/" << name << ".ogg";
    buffers.emplace(name, Audioxx::Buffer{ss.str()});
  }
  player.play(buffers.at("cola"));

  while (!shutdown) {
    auto lst = classification_q.dequeue();
    for (const auto& evt : lst) {
      out()([&](std::ostream& out) { out << "Effect! Wohoo!" << std::endl; });
    }

    if (lst.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{20});
    }
  }
}
catch (const std::runtime_error& e)
{
  out()([&](std::ostream& out) { out << e.what() << std::endl; });
  shutdown = true;
}
catch (...)
{
  shutdown = true;
}
}
