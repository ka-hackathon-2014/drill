#include "sound.h"
#include "debug.h"
#include "audioxx.h"

#include <map>
#include <random>
#include <sstream>
#include <thread>
#include <vector>
#include <algorithm>

namespace drill {
void play_variant(Audioxx::Player& player, const std::map<std::string, std::vector<Audioxx::Buffer>>& buffers,
                  const std::string& id, std::mt19937& rng,
                  std::map<std::string, std::vector<std::size_t>>& random_state, bool verbose);

void play_variant(Audioxx::Player& player, const std::map<std::string, std::vector<Audioxx::Buffer>>& buffers,
                  const std::string& id, std::mt19937& rng,
                  std::map<std::string, std::vector<std::size_t>>& random_state, bool verbose)
{

  const auto& variantsIt = buffers.find(id);
  if (variantsIt != buffers.end()) {

    if (verbose)
      out()([&](std::ostream& out) { out << "Play " << id << std::endl; });

    const auto& variants = buffers.at(id);
    if (!variants.empty()) {
      // pre-shuffled index available?
      if ((random_state.find(id) == random_state.end()) || (random_state.at(id).empty())) {
        // create index list without direct repetitions
        // length is just set to 5*number_of_variants
        // if there is only 1 variant, we have to repeet :(
        std::vector<std::size_t> idxs;

        for (std::size_t i = 0; i < variants.size() * 5; ++i) {
          if (variants.size() == 1) {
            idxs.push_back(0);
          } else {
            std::vector<size_t> choices;
            for (std::size_t j = 0; j < variants.size(); ++j) {
              if (i != j) {
                choices.push_back(j);
              }
            }
            idxs.push_back(choices.at(std::uniform_int_distribution<std::size_t> { 0, choices.size() - 1 }(rng)));
          }
        }

        random_state.emplace(id, std::move(idxs));
      }

      // play a random element
      const auto& buffer = variants.at(random_state.at(id).back());
      player.play(buffer, [&]()->bool { return false; });
      random_state.at(id).pop_back();
    } else {
      if (verbose)
        out()([&](std::ostream& out) { out << "Warning: No variant for effect " << id << std::endl; });
    }
  } else {
    if (verbose)
      out()([&](std::ostream& out) { out << "Warning: Unknown effect " << id << std::endl; });
  }
}

void run_sound(concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown,
               bool verbose) try
{
  Audioxx::Player player;
  std::vector<std::pair<std::string, std::vector<std::string>>> setup{
      {"calibrate", {"anfang_merged"}},                                                     //
      {"out", {"ausdembild1", "ausdembild2", "ausdembild3", "ausdembild4", "ausdembild5"}}, //
      {"start_step_0_oneway", {"anfang7"}},                                                 //
      {"start_step_1_intro", {"uebung_intro_1_1", "uebung_intro_1_2"}},                     //
      {"start_step_2_sep1", {"uebung_intro_2_1"}},                                          //
      {"start_step_3_type_only", {"kniebeugen"}},                                           //
      {"start_step_4_sep2", {"uebung_intro_3_1"}},                                          //
      {"start_step_5_sets_1", {"1"}},                                                       //
      {"start_step_5_sets_2", {"2"}},                                                       //
      {"start_step_5_sets_3", {"3"}},                                                       //
      {"start_step_5_sets_4", {"4"}},                                                       //
      {"start_step_5_sets_5", {"5"}},                                                       //
      {"start_step_6_sep3", {"uebung_intro_4_1"}},                                          //
      {"start_step_7_reps_5", {"5"}},                                                       //
      {"start_step_7_reps_10", {"10"}},                                                     //
      {"start_step_7_reps_15", {"15"}},                                                     //
      {"start_step_7_reps_20", {"20"}},                                                     //
      {"start_step_8_sep4", {"uebung_intro_5_1"}},                                          //
      {"tooSlow",
       {"schneller1", "schneller2", "schneller3", "schneller4", "schneller5", "schneller6", "schneller7"}}, //
      {"tooFast", {"verscheissern1"}},                                                                      //
      {"height", {"tiefer1", "tiefer2", "tiefer3"}},                                                        //
      {"count_-5", {"count_-5_pissed"}},                                                                    //
      {"count_-4", {"count_-4_pissed"}},                                                                    //
      {"count_-3", {"count_-3_pissed"}},                                                                    //
      {"count_-2", {"count_-2_pissed"}},                                                                    //
      {"count_-1", {"count_-1_pissed"}},                                                                    //
      {"count_0", {"count_0_pissed"}},                                                                      //
      {"count_1", {"count_1_normal", "count_1_passiv_aggr", "count_1_aggr"}},                               //
      {"count_2", {"count_2_normal", "count_2_passiv_aggr", "count_2_aggr"}},                               //
      {"count_3", {"count_3_normal", "count_3_passiv_aggr", "count_3_aggr"}},                               //
      {"count_4", {"count_4_normal", "count_4_passiv_aggr", "count_4_aggr"}},                               //
      {"count_5", {"count_5_normal", "count_5_passiv_aggr", "count_5_aggr"}},                               //
      {"count_6", {"count_6_normal", "count_6_passiv_aggr", "count_6_aggr"}},                               //
      {"count_7", {"count_7_normal", "count_7_passiv_aggr", "count_7_aggr"}},                               //
      {"count_8", {"count_8_normal", "count_8_passiv_aggr", "count_8_aggr"}},                               //
      {"count_9", {"count_9_normal", "count_9_passiv_aggr", "count_9_aggr"}},                               //
      {"count_10", {"count_10_normal", "count_10_passiv_aggr", "count_10_aggr"}},                           //
      {"count_11", {"count_11_normal", "count_11_passiv_aggr", "count_11_aggr"}},                           //
      {"count_12", {"count_12_normal", "count_12_passiv_aggr", "count_12_aggr"}},                           //
      {"count_13", {"count_13_normal", "count_13_passiv_aggr", "count_13_aggr"}},                           //
      {"count_14", {"count_14_normal", "count_14_passiv_aggr", "count_14_aggr"}},                           //
      {"count_15", {"count_15_normal", "count_15_passiv_aggr", "count_15_aggr"}},                           //
      {"count_16", {"count_16_normal", "count_16_passiv_aggr", "count_16_aggr"}},                           //
      {"count_17", {"count_17_normal", "count_17_passiv_aggr", "count_17_aggr"}},                           //
      {"count_18", {"count_18_normal", "count_18_passiv_aggr", "count_18_aggr"}},                           //
      {"count_19", {"count_19_normal", "count_19_passiv_aggr", "count_19_aggr"}},                           //
      {"count_20", {"count_20_normal", "count_20_passiv_aggr", "count_20_aggr"}},                           //
      {"nextSet", {"pause1", "pause2"}},                                                                    //
      {"mot", {"mot_beweg_dich1",  "mot_beweg_dich2",     "mot_bund1",              "mot_lieber_gott1",
               "mot_mach_weiter1", "mot_mama1",           "mot_noch_nicht_fertig1", "mot_noch_nicht_fertig2",
               "mot_schlecht1",    "mot_streng_dich_an1", "mot_traenen1",           "mot_weichei1"}}, //
      {"ready", {"ende1", "ende2", "ende3", "ende4", "ende5", "ende6"}}};
  std::map<std::string, std::vector<Audioxx::Buffer>> buffers;
  for (const auto& type : setup) {
    std::vector<Audioxx::Buffer> variants;
    for (const std::string& name : type.second) {
      std::string path{"sound/" + name + ".ogg"};

      if (verbose)
        out()([&](std::ostream& out) { out << "Load " << path << std::endl; });

      variants.emplace_back(Audioxx::Buffer{path});
    }
    buffers.emplace(type.first, std::move(variants));
  }
  std::random_device rdev;
  std::mt19937 rng{rdev()};
  std::map<std::string, std::vector<std::size_t>> random_state;

  int wasEmpty = false;
  while (!(wasEmpty && shutdown)) {
    bool setWasEmpty = shutdown;
    auto lst = classification_q.dequeue();
    for (const auto& evt : lst) {
      std::string id = evt->getID();
      if (id == "start") {
        const auto& evtStart = dynamic_cast<const EvtStart&>(*evt);

        play_variant(player, buffers, "start_step_0_oneway", rng, random_state, verbose);
        play_variant(player, buffers, "start_step_1_intro", rng, random_state, verbose);
        play_variant(player, buffers, "start_step_2_sep1", rng, random_state, verbose);
        play_variant(player, buffers, "start_step_3_type_only", rng, random_state, verbose);
        play_variant(player, buffers, "start_step_4_sep2", rng, random_state, verbose);

        std::stringstream ss1;
        ss1 << "start_step_5_sets_" << evtStart.sets;
        play_variant(player, buffers, ss1.str(), rng, random_state, verbose);
        play_variant(player, buffers, "start_step_6_sep3", rng, random_state, verbose);

        std::stringstream ss2;
        ss2 << "start_step_7_reps_" << evtStart.reps;
        play_variant(player, buffers, ss2.str(), rng, random_state, verbose);

        play_variant(player, buffers, "start_step_8_sep4", rng, random_state, verbose);
      } else {
        if (id == "count") {
          const auto& evtCount = dynamic_cast<const EvtCount&>(*evt);
          std::stringstream ss;
          ss << "count_" << evtCount.n;
          play_variant(player, buffers, ss.str(), rng, random_state, verbose);
        } else {
          play_variant(player, buffers, id, rng, random_state, verbose);
        }
      }
    }

    if (lst.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{20});
      if (setWasEmpty) {
        wasEmpty = true;
      }
    }
  }
}
catch (const std::runtime_error& e)
{
  // XXX: verbose flag, only run_* function gets it, though!
  out()([&](std::ostream& out) { out << e.what() << std::endl; });
  shutdown = true;
}
catch (...)
{
  shutdown = true;
}
}
