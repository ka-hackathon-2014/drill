#ifndef EVENT_H
#define EVENT_H

namespace drill {

struct EvtMovementChange {
  EvtMovementChange(double x_, double y_) : x{x_}, y{y_}
  {
  }

  double x;
  double y;
};

struct EvtEffect {
};

struct EvtTooFast : EvtEffect {
};

struct EvtTooSlow : EvtEffect {
};

struct EvtHeight : EvtEffect {
};

struct EvtCount : EvtEffect {
  int n;
  EvtCount(int n_) : n(n_)
  {
  }
};

struct EvtReady : EvtEffect {
};
}

#endif /* end of include guard: EVENT_H */
