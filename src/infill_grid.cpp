#include "infill_grid.h"
#include "infill.h"
#include <vector>

std::vector<InfillSegment> generateGridInfill(const Island &island,
                                              double spacing) {
  auto a = generateLineInfill(island, spacing, 0.0);
  auto b = generateLineInfill(island, spacing, 90.0);

  a.insert(a.end(), b.begin(), b.end());
  return a;
}

std::vector<InfillSegment> generateHexInfill(const Island &island,
                                             double spacing) {
  auto a = generateLineInfill(island, spacing, 0.0);
  auto b = generateLineInfill(island, spacing, 60.0);
  auto c = generateLineInfill(island, spacing, 120.0);

  a.insert(a.end(), b.begin(), b.end());
  a.insert(a.end(), c.begin(), c.end());
  return a;
}
