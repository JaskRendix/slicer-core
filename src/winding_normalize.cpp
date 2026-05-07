#include "winding_normalize.h"
#include "winding.h"
#include <algorithm>

namespace winding_normalize {

void toCCW(SliceLayer::Polyline &poly) {
  if (winding::isCW(poly))
    std::reverse(poly.points.begin(), poly.points.end());
}

void toCW(SliceLayer::Polyline &poly) {
  if (winding::isCCW(poly))
    std::reverse(poly.points.begin(), poly.points.end());
}

} // namespace winding_normalize
