#pragma once
#include "sliceLayer.h"

namespace winding_normalize {

void toCCW(SliceLayer::Polyline &poly);
void toCW(SliceLayer::Polyline &poly);

} // namespace winding_normalize
