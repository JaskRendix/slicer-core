#include "multiSlice.h"
#include <algorithm>
#include <cmath>
#include <limits>

static inline double triMinZ(const triangle &t) {
  return std::min({t.p0().getZ(), t.p1().getZ(), t.p2().getZ()});
}

static inline double triMaxZ(const triangle &t) {
  return std::max({t.p0().getZ(), t.p1().getZ(), t.p2().getZ()});
}

std::vector<LayerSlice> sliceMeshMultiLayer(const triangleMesh &mesh,
                                            double layerHeight) {
  std::vector<LayerSlice> layers;
  if (mesh.size() == 0 || layerHeight <= 0.0)
    return layers;

  // 1. Copy and sort triangles by their minimum Z
  auto allTriangles = mesh.getMesh();
  std::sort(allTriangles.begin(), allTriangles.end(),
            [](const triangle &a, const triangle &b) {
              return triMinZ(a) < triMinZ(b);
            });

  // 2. Determine global bounds
  double minZ = triMinZ(allTriangles.front());
  double maxZ = -std::numeric_limits<double>::infinity();
  for (const auto &tri : allTriangles) {
    maxZ = std::max(maxZ, triMaxZ(tri));
  }

  const double eps = 1e-9;
  double total = maxZ - minZ;
  unsigned count =
      static_cast<unsigned>(std::floor(total / layerHeight + eps)) + 1;

  layers.reserve(count);

  std::size_t triIdx = 0;
  std::vector<triangle> activePool;
  activePool.reserve(allTriangles.size());

  for (unsigned i = 0; i < count; ++i) {
    double z = minZ + i * layerHeight;
    if (i == count - 1) {
      // Snap last layer exactly to maxZ
      z = maxZ;
    }

    // Add triangles whose minZ is now reached
    while (triIdx < allTriangles.size()) {
      double tMin = triMinZ(allTriangles[triIdx]);
      if (tMin > z)
        break;
      activePool.push_back(allTriangles[triIdx]);
      ++triIdx;
    }

    // Remove triangles that are completely below the plane
    activePool.erase(
        std::remove_if(activePool.begin(), activePool.end(),
                       [z](const triangle &t) { return triMaxZ(t) < z; }),
        activePool.end());

    // Slice only active triangles
    SliceLayer layer(z);
    Plane slicePlane(v3(0, 0, 1), z);
    for (const auto &tri : activePool) {
      lineSegment seg;
      if (tri.intersectPlane(slicePlane, seg) == 0) {
        layer.addSegment(seg);
      }
    }

    // Build polylines with a stitching epsilon
    layers.push_back(LayerSlice{z, layer.buildPolylines(1e-4)});
  }

  return layers;
}
