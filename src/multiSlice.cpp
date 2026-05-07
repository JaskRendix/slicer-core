#include "multiSlice.h"
#include <cmath>
#include <limits>

static void computeZBounds(const triangleMesh &mesh, double &minZ,
                           double &maxZ) {
  minZ = std::numeric_limits<double>::infinity();
  maxZ = -std::numeric_limits<double>::infinity();

  for (const auto &tri : mesh.getMesh()) {
    minZ = std::min(minZ, tri.p0().getZ());
    minZ = std::min(minZ, tri.p1().getZ());
    minZ = std::min(minZ, tri.p2().getZ());

    maxZ = std::max(maxZ, tri.p0().getZ());
    maxZ = std::max(maxZ, tri.p1().getZ());
    maxZ = std::max(maxZ, tri.p2().getZ());
  }
}

std::vector<LayerSlice> sliceMeshMultiLayer(const triangleMesh &mesh,
                                            double layerHeight) {
  std::vector<LayerSlice> layers;
  if (mesh.size() == 0 || layerHeight <= 0.0)
    return layers;

  double minZ, maxZ;
  computeZBounds(mesh, minZ, maxZ);

  // Small epsilon to avoid floating point drift
  const double eps = 1e-9;

  for (double z = minZ; z <= maxZ + eps; z += layerHeight) {
    auto segs = mesh.sliceAtZ(z);
    if (segs.empty())
      continue;

    SliceLayer layer(z);
    for (auto &s : segs)
      layer.addSegment(s);

    auto polys = layer.buildPolylines();
    if (!polys.empty()) {
      layers.push_back(LayerSlice{z, std::move(polys)});
    }
  }

  return layers;
}
