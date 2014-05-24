#include "classification.h"
#include "debug.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <thread>

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q,
                        concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown)
{
  auto lastTp = std::chrono::system_clock::now();
  bool calibrated = false;
  std::list<double> caliTopLst;
  std::list<double> caliBottomLst;
  double caliTop = 0;
  double caliBottom = 0;
  int count = 0;
  bool changed = false;
  constexpr double heightThreshold = 0.2;

  classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtCalibrate{}});

  while (!shutdown) {
    auto lst = extraction_q.dequeue();
    for (const auto& evt : lst) {
      out()([&](std::ostream& out) {
        out << "MvtmChange x=" << evt.x << " y=" << evt.y << " sgn=" << evt.sgn << std::endl;
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
        if (ok && evt.sgn < 0) {
          double deltaHeight = std::max(0.0, caliBottom - evt.y);
          out()([&](std::ostream& out) {
            out << "d=" << deltaHeight << " >= " << heightThreshold* std::abs(caliBottom - caliTop) << std::endl;
          });
          if (deltaHeight >= heightThreshold * std::abs(caliBottom - caliTop)) {
            classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtHeight{}});
            ok = false;
          }
        }

        // == 3. count or not? ==
        if (ok && evt.sgn < 0) {
          ++count;
          if (count == 10) {
            classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtReady{}});
            shutdown = true;
            return;
          } else {
            classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtCount{count}});
          }
        }
        if (ok)
          changed = true;
      } else {
        // == 0. calibration ==
        if (evt.sgn < 0) {
          caliBottomLst.push_back(evt.y);
        } else {
          caliTopLst.push_back(evt.y);
        }

        if ((caliTopLst.size() >= 2) && (caliBottomLst.size() >= 2)) {
          caliTop = std::accumulate(caliTopLst.begin(), caliTopLst.end(), 0, std::plus<double>{}) /
                    static_cast<double>(caliTopLst.size());
          caliBottom = std::accumulate(caliBottomLst.begin(), caliBottomLst.end(), 0, std::plus<double>{}) /
                       static_cast<double>(caliBottomLst.size());
          if (caliTop < caliBottom) {
            calibrated = true;
            classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtStart{10, 1}});
          }
        }
      }
    }

    if (changed) {
      // == 4. speed ==
      auto nowTp = std::chrono::system_clock::now();
      auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowTp - lastTp).count();
      if (deltaMs > 1500) {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtTooSlow{}});
        changed = false;
      }
    }

    if (lst.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{20});
    }
  }
}
}
