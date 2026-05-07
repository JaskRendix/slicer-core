#pragma once
#include "islands.h"
#include <vector>

struct InfillSegment {
  v3 a, b;
};

// Generate simple line infill for one island
std::vector<InfillSegment>
generateLineInfill(const Island &island, double spacing, double angleDegrees);
