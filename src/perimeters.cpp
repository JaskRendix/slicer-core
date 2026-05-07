#include "perimeters.h"
#include "offset.h"
#include "winding_normalize.h"

std::vector<std::vector<SliceLayer::Polyline>>
generatePerimeters(const Island &island, int count, double width) {
  std::vector<std::vector<SliceLayer::Polyline>> result;

  if (island.outer.points.size() < 4)
    return result;

  SliceLayer::Polyline current = island.outer;
  winding_normalize::toCCW(current);

  for (int i = 0; i < count; ++i) {
    auto shell = offset::offsetPolyline(current, -width);
    if (shell.points.size() < 4)
      break;
    result.push_back({shell});
    current = shell;
  }
  return result;
}
