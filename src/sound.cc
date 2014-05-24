#include "sound.h"
#include "debug.h"
#include "audioxx.h"

#include <map>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

namespace drill {
void run_sound(concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown) try
{
  Audioxx::Player player;
  std::vector<std::pair<std::string, std::vector<std::string>>> setup{
      {"tooSlow", {"cola"}}, {"tooFast", {}}, {"height", {}}, {"count", {}}, {"ready", {}}};
  std::map<std::string, std::vector<Audioxx::Buffer>> buffers;
  for (const auto& type : setup) {
    std::vector<Audioxx::Buffer> variants;
    for (const std::string& name : type.second) {
      std::stringstream ss;
      ss << "sound/" << name << ".ogg";
      variants.emplace_back(Audioxx::Buffer{ss.str()});
    }
    buffers.emplace(type.first, std::move(variants));
  }
  std::random_device rdev;
  std::mt19937 rng{rdev()};

  while (!shutdown) {
    auto lst = classification_q.dequeue();
    for (const auto& evt : lst) {
      std::string id = evt->getID();
      const auto& variants = buffers.at(id);
      out()([&](std::ostream& out) { out << "Effect! (" << id << ")" << std::endl; });

      if (id == "count") {
        const auto& evtCount = dynamic_cast<const EvtCount&>(*evt);
        size_t i = static_cast<size_t>(evtCount.n + 1);
        if (i < variants.size()) {
          player.play(variants.at(i), [&]()->bool { return shutdown; });
        }
      } else if (!variants.empty()) {
        std::uniform_int_distribution<size_t> dist{0, variants.size() - 1};
        const auto& buffer = variants.at(dist(rng));
        player.play(buffer, [&]()->bool { return shutdown; });
      }
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
