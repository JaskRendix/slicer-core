#include "perimeters.h"
#include "offset.h"
#include "winding_normalize.h"

std::vector<std::vector<SliceLayer::Polyline>>
generatePerimeters(const Island &island, int count, double width) {
  std::vector<std::vector<SliceLayer::Polyline>> result;

  if (island.outer.points.size() < 4)
    return result;

  SliceLayer::Polyline basePoly = island.outer;
  winding_normalize::toCCW(basePoly);

  std::vector<SliceLayer::Polyline> currentPolys = {basePoly};

  for (int i = 0; i < count; ++i) {
    auto shells = offset::offsetLayerPolylines(currentPolys, -width);
    if (shells.empty())
      break;

    bool validFound = false;
    for (const auto &sh : shells) {
      if (sh.points.size() >= 4) {
        validFound = true;
        break;
      }
    }
    if (!validFound)
      break;

    result.push_back(shells);
    currentPolys = std::move(shells);
  }
  return result;
}
