#include "triangle.h"
#include <cmath>

int triangle::intersectPlane(const Plane &pl, lineSegment &out) const {
  double d0 = pl.distance(p0_);
  double d1 = pl.distance(p1_);
  double d2 = pl.distance(p2_);

  const double eps = 1e-9;

  auto classify = [&](double d) {
    if (std::abs(d) < eps)
      return 0;              // on plane
    return d > 0.0 ? 1 : -1; // above / below
  };

  int c0 = classify(d0);
  int c1 = classify(d1);
  int c2 = classify(d2);

  // All strictly on one side → no intersection
  if ((c0 > 0 && c1 > 0 && c2 > 0) || (c0 < 0 && c1 < 0 && c2 < 0)) {
    return 1;
  }

  // Helper: linear interpolation
  auto interp = [&](const v3 &a, const v3 &b, double da, double db) {
    double t = da / (da - db);
    return a + (b - a) * t;
  };

  v3 pts[3];
  int count = 0;

  // For each edge, if it crosses or touches the plane, add intersection
  auto addEdge = [&](const v3 &a, const v3 &b, double da, double db, int ca,
                     int cb) {
    // Both on plane → skip here (coplanar case not handled as segment)
    if (ca == 0 && cb == 0)
      return;

    // One endpoint on plane → use that endpoint
    if (ca == 0 && cb != 0) {
      pts[count++] = a;
    } else if (cb == 0 && ca != 0) {
      pts[count++] = b;
    }
    // Opposite signs → proper crossing
    else if (ca * cb < 0) {
      pts[count++] = interp(a, b, da, db);
    }
  };

  addEdge(p0_, p1_, d0, d1, c0, c1);
  addEdge(p1_, p2_, d1, d2, c1, c2);
  addEdge(p2_, p0_, d2, d0, c2, c0);

  // Collapse duplicates (can happen when a vertex is shared by two edges)
  auto samePoint = [&](const v3 &a, const v3 &b) {
    v3 d = a - b;
    return d.getX() * d.getX() + d.getY() * d.getY() + d.getZ() * d.getZ() <
           eps * eps;
  };

  int uniqueCount = 0;
  for (int i = 0; i < count; ++i) {
    bool dup = false;
    for (int j = 0; j < uniqueCount; ++j) {
      if (samePoint(pts[i], pts[j])) {
        dup = true;
        break;
      }
    }
    if (!dup) {
      pts[uniqueCount++] = pts[i];
    }
  }

  if (uniqueCount == 2) {
    out = lineSegment(pts[0], pts[1]);
    return 0;
  }

  // 0, 1, or 3 unique points → treat as no usable segment
  return 1;
}
