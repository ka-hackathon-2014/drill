#ifndef SOUND_H
#define SOUND_H

#include <atomic> // for atomic
#include <memory> // for unique_ptr

namespace drill {

struct EvtEffect;
template <typename T>
class concurrent_queue;

void run_sound(concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown,
               bool verbose);
}

#endif /* end of include guard: SOUND_H */
