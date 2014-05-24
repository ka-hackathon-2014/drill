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
      {"tooSlow", {"cola"}},
      {"tooFast", {}},
      {"height", {}},
      {"count_1", {"count_1_normal", "count_1_passiv_aggr", "count_1_aggr"}},
      {"count_2", {"count_2_normal", "count_2_passiv_aggr", "count_2_aggr"}},
      {"count_3", {"count_3_normal", "count_3_passiv_aggr", "count_3_aggr"}},
      {"count_4", {"count_4_normal", "count_4_passiv_aggr", "count_4_aggr"}},
      {"count_5", {"count_5_normal", "count_5_passiv_aggr", "count_5_aggr"}},
      {"count_6", {"count_6_normal", "count_6_passiv_aggr", "count_6_aggr"}},
      {"count_7", {"count_7_normal", "count_7_passiv_aggr", "count_7_aggr"}},
      {"count_8", {"count_8_normal", "count_8_passiv_aggr", "count_8_aggr"}},
      {"count_9", {"count_9_normal", "count_9_passiv_aggr", "count_9_aggr"}},
      {"count_10", {"count_10_normal", "count_10_passiv_aggr", "count_10_aggr"}},
      {"count_11", {"count_11_normal", "count_11_passiv_aggr", "count_11_aggr"}},
      {"count_12", {"count_12_normal", "count_12_passiv_aggr", "count_12_aggr"}},
      {"count_13", {"count_13_normal", "count_13_passiv_aggr", "count_13_aggr"}},
      {"count_14", {"count_14_normal", "count_14_passiv_aggr", "count_14_aggr"}},
      {"count_15", {"count_15_normal", "count_15_passiv_aggr", "count_15_aggr"}},
      {"count_16", {"count_16_normal", "count_16_passiv_aggr", "count_16_aggr"}},
      {"count_17", {"count_17_normal", "count_17_passiv_aggr", "count_17_aggr"}},
      {"count_18", {"count_18_normal", "count_18_passiv_aggr", "count_18_aggr"}},
      {"count_19", {"count_19_normal", "count_19_passiv_aggr", "count_19_aggr"}},
      {"count_20", {"count_20_normal", "count_20_passiv_aggr", "count_20_aggr"}},
      {"ready", {}}};
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
      if (id == "count") {
        const auto& evtCount = dynamic_cast<const EvtCount&>(*evt);
        std::stringstream ss;
        ss << "count_" << evtCount.n;
        id = ss.str();
      }

      out()([&](std::ostream& out) { out << "Effect! (" << id << ")" << std::endl; });

      const auto& variants = buffers.at(id);
      if (!variants.empty()) {
        std::uniform_int_distribution<size_t> dist{0, variants.size() - 1};
        const auto& buffer = variants.at(dist(rng));
        out()([&](std::ostream& out) { out << "foo" << std::endl; });
        player.play(buffer, [&]()->bool { return false; });
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
