#include "triangle.h"

int triangle::intersectPlane(const Plane &pl, lineSegment &out) const {
  double d0 = pl.distance(p0_);
  double d1 = pl.distance(p1_);
  double d2 = pl.distance(p2_);

  bool s0 = d0 >= 0;
  bool s1 = d1 >= 0;
  bool s2 = d2 >= 0;

  // All on same side → no intersection
  if ((s0 == s1) && (s1 == s2))
    return 1;

  auto interp = [&](const v3 &a, const v3 &b, double da, double db) {
    double t = da / (da - db);
    return a + (b - a) * t;
  };

  v3 pts[2];
  int count = 0;

  if (s0 != s1)
    pts[count++] = interp(p0_, p1_, d0, d1);
  if (s1 != s2)
    pts[count++] = interp(p1_, p2_, d1, d2);
  if (count < 2 && s2 != s0)
    pts[count++] = interp(p2_, p0_, d2, d0);

  if (count == 2) {
    out = lineSegment(pts[0], pts[1]);
    return 0;
  }

  return 1;
}
