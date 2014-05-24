#include "sound.h"
#include "debug.h"

#include <thread>

namespace drill {
void run_sound(concurrent_queue<EvtEffect>& classification_q, std::atomic<bool>& shutdown)
{
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
}
