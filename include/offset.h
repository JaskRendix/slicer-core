#pragma once

#include "sliceLayer.h"
#include <vector>

namespace offset {

// Offsets a single closed polyline by `distance` (positive = outward).
// Assumes all points share the same Z.
SliceLayer::Polyline offsetPolyline(const SliceLayer::Polyline &poly,
                                    double distance);

// Offsets all polylines in a layer.
std::vector<SliceLayer::Polyline>
offsetLayerPolylines(const std::vector<SliceLayer::Polyline> &polys,
                     double distance);

} // namespace offset
