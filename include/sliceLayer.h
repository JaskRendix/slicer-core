#pragma once
#include "lineSegment.h"
#include "v3.h"
#include <vector>

class SliceLayer {
public:
  struct Polyline {
    std::vector<v3> points;
  };

  explicit SliceLayer(double z) : z_(z) {}

  void addSegment(const lineSegment &seg);

  // Build polylines from unordered segments
  std::vector<Polyline> buildPolylines(double epsilon = 1e-6) const;

private:
  double z_;
  std::vector<lineSegment> segments_;
};
