#pragma once
#include "sliceLayer.h"
#include <vector>

struct Island {
  SliceLayer::Polyline outer;
  std::vector<SliceLayer::Polyline> holes;
};

// Given a set of polylines (mixed CCW/CW), group them into islands.
std::vector<Island>
buildIslands(const std::vector<SliceLayer::Polyline> &polys);
