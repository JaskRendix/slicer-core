#include "offset.h"
#include <cassert>
#include <cmath>

namespace {

double signedAreaXY(const SliceLayer::Polyline &poly) {
  const auto &pts = poly.points;
  if (pts.size() < 3)
    return 0.0;

  double area = 0.0;
  const std::size_t n = pts.size();

  for (std::size_t i = 0; i + 1 < n; ++i) {
    const auto &p = pts[i];
    const auto &q = pts[i + 1];
    area += p.getX() * q.getY() - q.getX() * p.getY();
  }

  return 0.5 * area;
}

struct Vec2 {
  double x, y;
};

Vec2 sub(const Vec2 &a, const Vec2 &b) { return {a.x - b.x, a.y - b.y}; }

Vec2 add(const Vec2 &a, const Vec2 &b) { return {a.x + b.x, a.y + b.y}; }

Vec2 mul(const Vec2 &a, double s) { return {a.x * s, a.y * s}; }

double dot(const Vec2 &a, const Vec2 &b) { return a.x * b.x + a.y * b.y; }

double length(const Vec2 &a) { return std::sqrt(dot(a, a)); }

Vec2 normalize(const Vec2 &a) {
  double len = length(a);
  if (len == 0.0)
    return {0.0, 0.0};
  return {a.x / len, a.y / len};
}

// Rotate 90° CCW
Vec2 normalCCW(const Vec2 &e) { return {-e.y, e.x}; }

} // namespace

namespace offset {

SliceLayer::Polyline offsetPolyline(const SliceLayer::Polyline &poly,
                                    double distance) {
  SliceLayer::Polyline out;
  const auto &pts = poly.points;
  const std::size_t n = pts.size();

  if (n < 3) {
    out.points = pts;
    return out;
  }

  // Ensure closed (last == first)
  bool isClosed = (pts.front().getX() == pts.back().getX() &&
                   pts.front().getY() == pts.back().getY() &&
                   pts.front().getZ() == pts.back().getZ());

  // Work on indices [0, m)
  std::size_t m = n;
  if (isClosed)
    m = n - 1;

  if (m < 3) {
    out.points = pts;
    return out;
  }

  // Determine winding: area > 0 => CCW
  double area = signedAreaXY(poly);
  int winding = (area > 0.0) ? 1 : -1;

  out.points.reserve(m + 1);

  for (std::size_t i = 0; i < m; ++i) {
    const auto &pPrev = pts[(i + m - 1) % m];
    const auto &pCurr = pts[i];
    const auto &pNext = pts[(i + 1) % m];

    Vec2 prev{pPrev.getX(), pPrev.getY()};
    Vec2 curr{pCurr.getX(), pCurr.getY()};
    Vec2 next{pNext.getX(), pNext.getY()};

    Vec2 e1 = normalize(sub(curr, prev));
    Vec2 e2 = normalize(sub(next, curr));

    // Always use CCW normals for "outward"
    Vec2 n1 = normalCCW(e1);
    Vec2 n2 = normalCCW(e2);

    // If polygon is CW, flip the distance so positive still means "grow"
    double localDistance = distance;
    if (winding < 0)
      localDistance = -distance;

    Vec2 nSum = add(n1, n2);
    double nSumLen = length(nSum);

    Vec2 dir;
    double scale = 1.0;

    if (nSumLen == 0.0) {
      // 180° turn: just use one normal
      dir = normalize(n1);
    } else {
      dir = {nSum.x / nSumLen, nSum.y / nSumLen};
      // Miter length correction: distance / cos(theta/2)
      double cosTheta = dot(dir, n1);
      if (std::abs(cosTheta) > 1e-6)
        scale = 1.0 / cosTheta;
    }

    double dz = pCurr.getZ();
    Vec2 offsetVec = mul(dir, localDistance * scale);

    out.points.emplace_back(pCurr.getX() + offsetVec.x,
                            pCurr.getY() + offsetVec.y, dz);
  }

  // Close the loop
  out.points.push_back(out.points.front());
  return out;
}

std::vector<SliceLayer::Polyline>
offsetLayerPolylines(const std::vector<SliceLayer::Polyline> &polys,
                     double distance) {
  std::vector<SliceLayer::Polyline> result;
  result.reserve(polys.size());
  for (const auto &p : polys)
    result.push_back(offsetPolyline(p, distance));
  return result;
}

} // namespace offset
