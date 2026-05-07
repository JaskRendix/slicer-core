#pragma once
#include "Plane.h"
#include "lineSegment.h"
#include "v3.h"

class triangle {
  v3 normal_;
  v3 p0_, p1_, p2_;

public:
  triangle(const v3 &normal, const v3 &a, const v3 &b, const v3 &c)
      : normal_(normal), p0_(a), p1_(b), p2_(c) {}

  const v3 &normal() const { return normal_; }
  const v3 &p0() const { return p0_; }
  const v3 &p1() const { return p1_; }
  const v3 &p2() const { return p2_; }

  // Needed by triangleMesh::normalize()
  triangle &operator-=(const v3 &v) {
    p0_ = p0_ - v;
    p1_ = p1_ - v;
    p2_ = p2_ - v;
    return *this;
  }

  // Returns 0 if intersects, 1 if no intersection
  int intersectPlane(const Plane &pl, lineSegment &out) const;
};
