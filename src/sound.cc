#include "sound.h"

namespace drill {
void run_sound(concurrent_queue<EvtEffect>& classification_q, std::atomic<bool>& shutdown)
{
  while (!shutdown) {
    auto lst = classification_q.dequeue();
    for (const auto& evt : lst) {
      // TODO stub
    }
  }
}
}
