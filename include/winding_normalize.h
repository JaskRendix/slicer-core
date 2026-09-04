#pragma once
#include "sliceLayer.h"
#include <vector>

namespace winding_normalize {

double computeSignedArea(const SliceLayer::Polyline &poly);

void toCCW(SliceLayer::Polyline &poly);
void toCW(SliceLayer::Polyline &poly);
void normalizeCanonical(SliceLayer::Polyline &poly);
void normalizeAll(std::vector<SliceLayer::Polyline> &polys);

} // namespace winding_normalize
