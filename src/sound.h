#ifndef SOUND_H
#define SOUND_H

#include "event.h"
#include "queue.h"

#include <atomic>
#include <memory>

namespace drill {

void run_sound(concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown,
               bool verbose);
}

#endif /* end of include guard: SOUND_H */
