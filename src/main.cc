#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <thread>

#include "queue.h"
#include "event.h"


using namespace drill;

int main(int argc, char** argv)
{
  concurrent_queue<EvtMovementChange> extraction_q;
  concurrent_queue<EvtEffect> classification_q;

  std::thread extraction{[&] {
    extraction_q.enqueue(EvtMovementChange{0, 0});
    extraction_q.enqueue(EvtMovementChange{0, 0});
    extraction_q.enqueue(EvtMovementChange{0, 0});
  }};

  std::thread classification{[&] {
    while (true) {
      auto lst = extraction_q.dequeue();
      if (lst.size())
        std::cout << "foo" << std::endl;
    }
  }};

  std::thread sound{[&] {}};

  sound.join();
  classification.join();
  extraction.join();
}
