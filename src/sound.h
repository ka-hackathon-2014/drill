#ifndef SOUND_H
#define SOUND_H

#include "event.h"
#include "queue.h"

#include <atomic>

namespace drill {

void run_sound(concurrent_queue<EvtEffect>& classification_q, std::atomic<bool>& shutdown);
}

#endif /* end of include guard: SOUND_H */
