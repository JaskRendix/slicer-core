#pragma once

#include "sliceLayer.h"
#include "triangleMesh.h"
#include <vector>

struct LayerSlice {
  double z;
  std::vector<SliceLayer::Polyline> polylines;
};

// Slice the whole mesh into layers from minZ to maxZ with given layer height.
std::vector<LayerSlice> sliceMeshMultiLayer(const triangleMesh &mesh,
                                            double layerHeight);
