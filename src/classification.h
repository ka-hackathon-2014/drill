#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include "queue.h"
#include "event.h"

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q);
}

#endif /* end of include guard: CLASSIFICATION_H */
