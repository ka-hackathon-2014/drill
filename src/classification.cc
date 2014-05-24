#include "classification.h"

#include <iostream>

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q)
{
  auto lastTp = std::chrono::system_clock::now();
  int count = 0;

  while (true) {
    bool changed = false;
    auto lst = extraction_q.dequeue();
    for (const auto& evt : lst) {
      changed = true;
      lastTp = std::chrono::system_clock::now();

      // == 1. count ==
      ++count;
      if (count == 10) {
        std::cout << "Rdy" << std::endl;
      } else {
        std::cout << "Count: " << count << std::endl;
      }

      // TODO 2. height
    }

    if (changed) {
      // == 3. speed ==
      auto nowTp = std::chrono::system_clock::now();
      auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowTp - lastTp).count();
      if (deltaMs > 500) {
        std::cout << "to slow!" << std::endl;
      } else if (deltaMs < 200) {
        std::cout << "to fast!" << std::endl;
      }
    }
  }
}
}
