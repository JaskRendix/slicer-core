#pragma once

#include "clipper2/clipper.h"
#include "sliceLayer.h"
#include <vector>

namespace offset {

// Offsets a single closed polyline by `distance` (positive = outward).
// Returns a vector since an offset can split a shape into multiple fragments.
std::vector<SliceLayer::Polyline>
offsetPolyline(const SliceLayer::Polyline &poly, double distance,
               Clipper2Lib::JoinType joinType = Clipper2Lib::JoinType::Miter,
               double miterLimit = 2.0);

// Offsets all polylines in a layer simultaneously for correct multi-path
// handling.
std::vector<SliceLayer::Polyline> offsetLayerPolylines(
    const std::vector<SliceLayer::Polyline> &polys, double distance,
    Clipper2Lib::JoinType joinType = Clipper2Lib::JoinType::Miter,
    double miterLimit = 2.0);

} // namespace offset
