#include "classification.h"

#include <iostream>
#include <thread>

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q,
                        concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown)
{
  auto lastTp = std::chrono::system_clock::now();
  int count = 0;

  while (!shutdown) {
    bool changed = false;
    auto lst = extraction_q.dequeue();
    for (const auto& evt : lst) {
      changed = true;
      lastTp = std::chrono::system_clock::now();

      // == 1. count ==
      ++count;
      if (count == 10) {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtReady{}});
        shutdown = true;
        return;
      } else {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtCount{count}});
      }

      // TODO 2. height
    }

    if (changed) {
      // == 3. speed ==
      auto nowTp = std::chrono::system_clock::now();
      auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowTp - lastTp).count();
      if (deltaMs > 500) {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtTooSlow{}});
      } else if (deltaMs < 200) {
        classification_q.enqueue(std::unique_ptr<EvtEffect>{new EvtTooFast{}});
      }
    }

    if (lst.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{20});
    }
  }
}
}
