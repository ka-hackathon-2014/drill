#include "classification.h"

#include <iostream>

namespace drill {
void run_classification(concurrent_queue<EvtMovementChange>& extraction_q) {
    while (true) {
      auto lst = extraction_q.dequeue();
      if (lst.size())
        std::cout << "foo" << std::endl;
    }
}

}
