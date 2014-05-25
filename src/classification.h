#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include "queue.h"
#include "event.h"

#include <atomic>
#include <memory>

namespace drill {
void run_classification(concurrent_queue<std::unique_ptr<EvtCamera>>& extraction_q,
                        concurrent_queue<std::unique_ptr<EvtEffect>>& classification_q, std::atomic<bool>& shutdown);
}

#endif /* end of include guard: CLASSIFICATION_H */
