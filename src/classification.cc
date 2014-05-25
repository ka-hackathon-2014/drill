#include "classification.h"
#include "debug.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <thread>
#include <random>
#include <vector>

namespace drill {

template <typename T>
struct TopBottom {
  T top;
  T bottom;
};

template <typename T>
struct RepsSets {
  T reps;
  T sets;

  RepsSets() = default;
  RepsSets(T reps_, T sets_) : reps(reps_), sets(sets_)
  {
  }
};

std::pair<std::size_t, std::size_t> chooseSetsReps();

std::pair<std::size_t, std::size_t> chooseSetsReps()
{
  std::vector<std::pair<std::size_t, std::vector<std::size_t>>> styles{{1, {20}},         //
                                                                       {2, {10, 15, 20}}, //
                                                                       {3, {10, 15}},     //
                                                                       {4, {10, 15}},     //
                                                                       {5, {5, 10}}};

  std::random_device rdev;
  std::mt19937 rng{rdev()};

  auto dist_style = std::uniform_int_distribution<std::size_t>{0, styles.size() - 1};
  const auto& style = styles.at(dist_style(rng));

  auto dist_reps = std::uniform_int_distribution<std::size_t>{0, style.second.size() - 1};
  std::size_t reps = style.second.at(dist_reps(rng));

  return std::make_pair(style.first, reps);
}

void run_classification(concurrent_queue<std::unique_ptr<EvtCamera>>& extraction_q,
                        concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown,
                        bool verbose)
{
  auto lastTp = std::chrono::system_clock::now();
  bool calibrated = false;
  TopBottom<std::list<double>> caliLst;
  TopBottom<double> cali;
  RepsSets<int> count{0, 0};
  bool changed = false;
  constexpr double heightThreshold = 0.2;
  int rating = 0;

  RepsSets<std::size_t> cfg;
  std::tie(cfg.sets, cfg.reps) = chooseSetsReps();

  classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtCalibrate{}});

  while (!shutdown) {
    // #################### event parsing ####################
    auto lst = extraction_q.dequeue();
    for (const auto& evt : lst) {
      if (evt->getID() == "movement") {
        const auto& evtMovement = dynamic_cast<const EvtMovementChange&>(*evt);

        if (verbose)
          out()([&](std::ostream& out) {
            out << "MvtmChange x=" << evtMovement.x << " y=" << evtMovement.y << " sgn=" << evtMovement.sgn
                << std::endl;
          });

        if (calibrated) {
          bool ok = true;
          // == 1. too fast? ==
          auto nowTp = std::chrono::system_clock::now();
          auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowTp - lastTp).count();
          if (deltaMs < 300) {
            classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtTooFast{}});
            ok = false;
          }
          lastTp = std::chrono::system_clock::now();


          // == 2. height? ==
          if (ok && evtMovement.sgn < 0) {
            double deltaHeight = std::max(0.0, cali.bottom - evtMovement.y);

            if (verbose)
              out()([&](std::ostream& out) {
                out << "d=" << deltaHeight << " >= " << heightThreshold* std::abs(cali.bottom - cali.top) << std::endl;
              });

            if (deltaHeight >= heightThreshold * std::abs(cali.bottom - cali.top)) {
              classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtHeight{}});
              ok = false;
            }
          }

          // == 3. countReps or not? ==
          if (ok && evtMovement.sgn < 0) {
            ++count.reps;

            if (count.reps >= static_cast<int>(cfg.reps)) {
              ++count.sets;

              if (count.sets >= static_cast<int>(cfg.sets)) {
                classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtReady{}});
                shutdown = true;
                return;
              } else {
                classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtNextSet{}});
                count.reps = 0;
                changed = false;
              }
            } else {
              classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtCount{count.reps}});
            }
          }

          // check and rate
          if (ok) {
            if (count.reps > 0)
              changed = true;
            rating += 1;
          } else {
            rating -= 5;
          }
        } else {
          // == 0. calibration ==
          if (evtMovement.sgn < 0) {
            caliLst.bottom.push_back(evtMovement.y);
          } else {
            caliLst.top.push_back(evtMovement.y);
          }

          if ((caliLst.top.size() >= 2) && (caliLst.bottom.size() >= 2)) {
            cali.top = std::accumulate(caliLst.top.begin(), caliLst.top.end(), 0, std::plus<double>{}) /
                       static_cast<double>(caliLst.top.size());
            cali.bottom = std::accumulate(caliLst.bottom.begin(), caliLst.bottom.end(), 0, std::plus<double>{}) /
                          static_cast<double>(caliLst.bottom.size());
            if (cali.top < cali.bottom) {
              calibrated = true;
              classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtStart{cfg.reps, cfg.sets}});
            }
          }
        }
      } else if (evt->getID() == "lost") {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtOut{}});
      }
    }

    // #################### time bounds ####################
    if (changed) {
      // == 4. speed ==
      auto nowTp = std::chrono::system_clock::now();
      auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowTp - lastTp).count();
      if (deltaMs > 1500) {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtTooSlow{}});
        changed = false;
      }
    }

    // #################### rating ####################
    if (rating < -5) {
      rating = 0;
      classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtMotivation{}});
    }

    if (lst.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{20});
    }
  }
}
}
