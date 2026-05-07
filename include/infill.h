#pragma once
#include "islands.h"
#include <vector>

struct InfillSegment {
  v3 a, b;
};

std::vector<InfillSegment>
generateLineInfill(const Island &island, double spacing, double angleDegrees);
std::vector<InfillSegment> generateGridInfill(const Island &island,
                                              double spacing);
std::vector<InfillSegment> generateHexInfill(const Island &island,
                                             double spacing);