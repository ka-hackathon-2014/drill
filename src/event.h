#ifndef EVENT_H
#define EVENT_H

#include <string>

namespace drill {

struct EvtMovementChange {
  EvtMovementChange(double x_, double y_, int sgn_ = 0) : x{x_}, y{y_}, sgn(sgn_)
  {
  }

  double x;
  double y;
  int sgn;
};

struct EvtEffect {
  virtual ~EvtEffect() = default;
  virtual std::string getID() const = 0;
};

struct EvtStart : EvtEffect {
  size_t reps;
  size_t sets;
  EvtStart(size_t reps_, size_t sets_) : reps(reps_), sets(sets_)
  {
  }
  virtual std::string getID() const override
  {
    return "start";
  }
};

struct EvtCalibrate : EvtEffect {
  virtual std::string getID() const override
  {
    return "calibrate";
  }
};

struct EvtTooFast : EvtEffect {
  virtual std::string getID() const override
  {
    return "tooFast";
  }
};

struct EvtTooSlow : EvtEffect {
  virtual std::string getID() const override
  {
    return "tooSlow";
  }
};

struct EvtHeight : EvtEffect {
  virtual std::string getID() const override
  {
    return "height";
  }
};

struct EvtCount : EvtEffect {
  int n;
  EvtCount(int n_) : n(n_)
  {
  }
  virtual std::string getID() const override
  {
    return "count";
  }
};

struct EvtReady : EvtEffect {
  virtual std::string getID() const override
  {
    return "ready";
  }
};
}

#endif /* end of include guard: EVENT_H */
