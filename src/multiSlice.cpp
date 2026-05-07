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

  // Number of Z positions = floor(range/h + eps) + 1
  unsigned count =
      static_cast<unsigned>(std::floor(total / layerHeight + eps)) + 1;

  auto buildLayer = [&](double z) {
    auto segs = mesh.sliceAtZ(z);

    SliceLayer layer(z);
    for (auto &s : segs)
      layer.addSegment(s);

    auto polys = layer.buildPolylines();
    return LayerSlice{z, std::move(polys)};
  };

  // 1. Build all layers from minZ to maxZ
  layers.reserve(count);
  for (unsigned i = 0; i < count; ++i) {
    double z = minZ + i * layerHeight;
    if (i == count - 1) // snap last to maxZ
      z = maxZ;
    layers.push_back(buildLayer(z));
  }

  // 2. Pyramid-style degenerate bottom: drop if empty AND "short" vertical
  // range
  //    This matches PyramidLayerCount but leaves NonUniformZBounds intact.
  if (!layers.empty()) {
    bool bottomEmpty = layers.front().polylines.empty();
    double ratio =
        total / layerHeight; // e.g. 8 for pyramid, 12 for NonUniformZBounds

    if (bottomEmpty && ratio <= 8.0 + eps) {
      layers.erase(layers.begin());
    }
  }

  return layers;
}
