#include "sliceLayer.h"
#include "lineSegment.h"
#include "v3.h"
#include <vector>

static inline bool pointsEqual(const v3 &a, const v3 &b, double eps) {
  v3 diff = a - b;
  double d2 = diff.getX() * diff.getX() + diff.getY() * diff.getY() +
              diff.getZ() * diff.getZ();
  return d2 < (eps * eps);
}

void SliceLayer::addSegment(const lineSegment &seg) {
  segments_.push_back(seg);
}

std::vector<SliceLayer::Polyline>
SliceLayer::buildPolylines(double epsilon) const {
  std::vector<Polyline> result;

  std::vector<lineSegment> segs = segments_;
  std::vector<bool> used(segs.size(), false);

  for (std::size_t i = 0; i < segs.size(); ++i) {
    if (used[i])
      continue;

    Polyline poly;
    poly.points.push_back(segs[i].start());
    poly.points.push_back(segs[i].end());
    used[i] = true;

    bool extended = true;
    while (extended) {
      extended = false;

      v3 &front = poly.points.front();
      v3 &back = poly.points.back();

      // Closed loop with at least 3 points
      if (pointsEqual(front, back, epsilon) && poly.points.size() > 2) {
        break;
      }

      for (std::size_t j = 0; j < segs.size(); ++j) {
        if (used[j])
          continue;

        const v3 &s = segs[j].start();
        const v3 &e = segs[j].end();

        if (pointsEqual(back, s, epsilon)) {
          poly.points.push_back(e);
          used[j] = true;
          extended = true;
        } else if (pointsEqual(back, e, epsilon)) {
          poly.points.push_back(s);
          used[j] = true;
          extended = true;
        } else if (pointsEqual(front, e, epsilon)) {
          poly.points.insert(poly.points.begin(), s);
          used[j] = true;
          extended = true;
        } else if (pointsEqual(front, s, epsilon)) {
          poly.points.insert(poly.points.begin(), e);
          used[j] = true;
          extended = true;
        }

        if (extended)
          break;
      }

      // Safety: if nothing extended, we’re done with this polyline
      if (!extended)
        break;
    }

    result.push_back(std::move(poly));
  }

  return result;
}
