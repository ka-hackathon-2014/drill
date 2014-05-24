#include "classification.h"

#include <iostream>
#include <thread>

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q,
                        concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown)
{
  auto lastTp = std::chrono::system_clock::now();
  int count = 0;
  bool changed = false;

  while (!shutdown) {
    auto lst = extraction_q.dequeue();
    for (const auto& evt : lst) {
      changed = true;

      // == 1. count ==
      if (evt.sgn > 0) {
        ++count;
        if (count == 10) {
          classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtReady{}});
          shutdown = true;
          return;
        } else {
          classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtCount{count}});
        }
      }

      // TODO 2. height

      // == 3. too fast ==
      auto nowTp = std::chrono::system_clock::now();
      auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowTp - lastTp).count();
      if (deltaMs < 300) {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtTooFast{}});
      }
      lastTp = std::chrono::system_clock::now();
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
