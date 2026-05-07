#pragma once
#include "v3.h"

class Plane {
  v3 normal_;
  double d_;

public:
  Plane(const v3 &normal, double d) : normal_(normal), d_(d) {}

  const v3 &normal() const { return normal_; }
  double d() const { return d_; }

  double distance(const v3 &p) const { return normal_.dot(p) - d_; }
};
