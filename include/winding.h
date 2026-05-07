#pragma once
#include "sliceLayer.h"
#include <cmath>

namespace winding {

inline double signedArea(const SliceLayer::Polyline &poly) {
  const auto &pts = poly.points;
  if (pts.size() < 3)
    return 0.0;

  double area = 0.0;
  for (size_t i = 0; i + 1 < pts.size(); ++i) {
    const auto &p = pts[i];
    const auto &q = pts[i + 1];
    area += p.getX() * q.getY() - q.getX() * p.getY();
  }
  return 0.5 * area;
}

inline bool isCCW(const SliceLayer::Polyline &poly) {
  return signedArea(poly) > 0.0;
}

inline bool isCW(const SliceLayer::Polyline &poly) {
  return signedArea(poly) < 0.0;
}

} // namespace winding
