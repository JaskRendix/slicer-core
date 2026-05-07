#pragma once
#include "infill.h"
#include "islands.h"
#include <vector>

std::vector<InfillSegment> generateGridInfill(const Island &island,
                                              double spacing);

std::vector<InfillSegment> generateHexInfill(const Island &island,
                                             double spacing);
