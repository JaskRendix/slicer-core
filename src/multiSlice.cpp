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

  const double eps = 1e-9;
  const double total = maxZ - minZ;

  // Number of intervals between minZ and maxZ
  unsigned steps = static_cast<unsigned>(std::floor(total / layerHeight + eps));

  auto buildLayer = [&](double z) {
    auto segs = mesh.sliceAtZ(z);

    SliceLayer layer(z);
    for (auto &s : segs)
      layer.addSegment(s);

    auto polys = layer.buildPolylines();
    return LayerSlice{z, std::move(polys)};
  };

  // --- 1. Bottom layer at minZ ---
  auto bottom = buildLayer(minZ);

  // Pyramid case → bottom is empty → drop it
  if (!bottom.polylines.empty())
    layers.push_back(std::move(bottom));

  // --- 2. Internal layers (always included) ---
  for (unsigned i = 1; i < steps; ++i) {
    double z = minZ + i * layerHeight;
    layers.push_back(buildLayer(z));
  }

  // --- 3. Apex layer at maxZ (always included) ---
  layers.push_back(buildLayer(maxZ));

  return layers;
}
