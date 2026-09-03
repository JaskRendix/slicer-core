#pragma once
#include "Plane.h"
#include "lineSegment.h"
#include "v3.h"
#include <optional>

class triangle {
  v3 normal_;
  v3 p0_, p1_, p2_;

public:
  triangle(const v3 &normal, const v3 &a, const v3 &b, const v3 &c) noexcept
      : normal_(normal), p0_(a), p1_(b), p2_(c) {}

  const v3 &normal() const noexcept { return normal_; }
  const v3 &p0() const noexcept { return p0_; }
  const v3 &p1() const noexcept { return p1_; }
  const v3 &p2() const noexcept { return p2_; }

  triangle &operator-=(const v3 &v) noexcept {
    p0_ = p0_ - v;
    p1_ = p1_ - v;
    p2_ = p2_ - v;
    return *this;
  }

  std::optional<lineSegment> intersectPlane(const Plane &pl,
                                            double eps = 1e-9) const;
};
