#include "perimeters.h"
#include "offset.h"
#include "winding_normalize.h"

std::vector<std::vector<SliceLayer::Polyline>>
generatePerimeters(const Island &island, int count, double width) {
  std::vector<std::vector<SliceLayer::Polyline>> result;

  if (island.outer.points.size() < 4 && island.holes.empty())
    return result;

  // Prepare outer boundary
  std::vector<SliceLayer::Polyline> currentOuter;
  if (island.outer.points.size() >= 4) {
    SliceLayer::Polyline basePoly = island.outer;
    winding_normalize::toCCW(basePoly);
    currentOuter.push_back(basePoly);
  }

  // Prepare hole boundaries (CW orientation)
  std::vector<SliceLayer::Polyline> currentHoles;
  for (const auto &hole : island.holes) {
    if (hole.points.size() >= 4) {
      SliceLayer::Polyline hPoly = hole;
      winding_normalize::toCW(hPoly);
      currentHoles.push_back(hPoly);
    }
  }

  for (int i = 0; i < count; ++i) {
    std::vector<SliceLayer::Polyline> layerShells;

    // Offset outer perimeters inward (-width)
    if (!currentOuter.empty()) {
      auto outerShells = offset::offsetLayerPolylines(currentOuter, -width);
      if (!outerShells.empty()) {
        layerShells.insert(layerShells.end(), outerShells.begin(), outerShells.end());
        currentOuter = std::move(outerShells);
      } else {
        currentOuter.clear();
      }
    }

    // Offset hole perimeters outward/inward depending on inner cavity rules (+width or -width)
    if (!currentHoles.empty()) {
      auto holeShells = offset::offsetLayerPolylines(currentHoles, width); // positive to shrink hole cavity inward
      if (!holeShells.empty()) {
        layerShells.insert(layerShells.end(), holeShells.begin(), holeShells.end());
        currentHoles = std::move(holeShells);
      } else {
        currentHoles.clear();
      }
    }

    if (layerShells.empty())
      break;

    result.push_back(std::move(layerShells));
  }

  return result;
}
