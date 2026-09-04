#include "winding_normalize.h"
#include "winding.h"
#include <algorithm>

namespace winding_normalize {

double computeSignedArea(const SliceLayer::Polyline &poly) {
  if (poly.points.size() < 3)
    return 0.0;
  double area = 0.0;
  size_t n = poly.points.size();
  for (size_t i = 0; i < n; ++i) {
    const auto &p1 = poly.points[i];
    const auto &p2 = poly.points[(i + 1) % n];
    area += (static_cast<double>(p1.getX()) * p2.getY()) -
            (static_cast<double>(p2.getX()) * p1.getY());
  }
  return area * 0.5;
}

void toCCW(SliceLayer::Polyline &poly) {
  if (!poly.is_closed || poly.points.size() < 3)
    return;
  if (computeSignedArea(poly) < 0.0) {
    std::reverse(poly.points.begin(), poly.points.end());
  }
}

void toCW(SliceLayer::Polyline &poly) {
  if (!poly.is_closed || poly.points.size() < 3)
    return;
  if (computeSignedArea(poly) > 0.0) {
    std::reverse(poly.points.begin(), poly.points.end());
  }
}

void normalizeCanonical(SliceLayer::Polyline &poly) {
  if (!poly.is_closed || poly.points.size() < 3)
    return;

  auto it = std::min_element(poly.points.begin(), poly.points.end(),
                             [](const v3 &a, const v3 &b) {
                               if (a.getX() != b.getX())
                                 return a.getX() < b.getX();
                               return a.getY() < b.getY();
                             });

  if (it != poly.points.begin()) {
    std::rotate(poly.points.begin(), it, poly.points.end());
  }
}

void normalizeAll(std::vector<SliceLayer::Polyline> &polys) {
  for (auto &poly : polys) {
    if (!poly.is_closed)
      continue;
    toCCW(poly);
    normalizeCanonical(poly);
  }
}

} // namespace winding_normalize
