#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include <atomic> // for atomic
#include <memory> // for unique_ptr

namespace drill {
struct EvtCamera;
struct EvtEffect;
template <typename T>
class concurrent_queue;

void run_classification(concurrent_queue<std::unique_ptr<EvtCamera>>& extraction_q,
                        concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown,
                        bool verbose);
}

#endif /* end of include guard: CLASSIFICATION_H */
