#include "sliceLayer.h"
#include "lineSegment.h"
#include "v3.h"
#include <cmath>
#include <vector>

static inline double dist2(const v3 &a, const v3 &b) {
  double dx = a.getX() - b.getX();
  double dy = a.getY() - b.getY();
  double dz = a.getZ() - b.getZ();
  return dx * dx + dy * dy + dz * dz;
}

static inline bool pointsEqual(const v3 &a, const v3 &b, double eps) {
  return dist2(a, b) < eps * eps;
}

void SliceLayer::addSegment(const lineSegment &seg) {
  segments_.push_back(seg);
}

std::vector<SliceLayer::Polyline>
SliceLayer::buildPolylines(double epsilon) const {
  std::vector<Polyline> result;

  // Copy segments so we can mark them as used
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

      // --- IMPORTANT: stop extending if loop is closed ---
      if (pointsEqual(front, back, epsilon)) {
        break;
      }

      for (std::size_t j = 0; j < segs.size(); ++j) {
        if (used[j])
          continue;

        const v3 &s = segs[j].start();
        const v3 &e = segs[j].end();

        // Extend at the back
        if (pointsEqual(back, s, epsilon)) {
          poly.points.push_back(e);
          used[j] = true;
          extended = true;
        } else if (pointsEqual(back, e, epsilon)) {
          poly.points.push_back(s);
          used[j] = true;
          extended = true;
        }
        // Extend at the front
        else if (pointsEqual(front, e, epsilon)) {
          poly.points.insert(poly.points.begin(), s);
          used[j] = true;
          extended = true;
        } else if (pointsEqual(front, s, epsilon)) {
          poly.points.insert(poly.points.begin(), e);
          used[j] = true;
          extended = true;
        }
      }
    }

    result.push_back(std::move(poly));
  }

  return result;
}
