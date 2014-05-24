#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include "queue.h"
#include "event.h"

#include <atomic>

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q, concurrent_queue<EvtEffect>& classification_q, std::atomic<bool>& shutdown);
}

#endif /* end of include guard: CLASSIFICATION_H */
