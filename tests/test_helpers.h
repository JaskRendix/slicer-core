#pragma once
#include "islands.h"
#include "sliceLayer.h"
#include "v3.h"
#include <algorithm>



inline SliceLayer::Polyline makeSquare(double half, double z = 0.0) {
    SliceLayer::Polyline p;
    p.points = {
        v3(-half, -half, z),
        v3( half, -half, z),
        v3( half,  half, z),
        v3(-half,  half, z),
        v3(-half, -half, z)
    };
    return p;
}

inline SliceLayer::Polyline makeReversed(const SliceLayer::Polyline &poly) {
    SliceLayer::Polyline out = poly;
    std::reverse(out.points.begin(), out.points.end());
    return out;
}



inline Island makeSquareIsland(double half, double z = 0.0) {
    Island isl;
    isl.outer = makeSquare(half, z);
    return isl;
}

inline Island makeDonutIsland(double outerHalf, double innerHalf, double z = 0.0) {
    Island isl;
    isl.outer = makeSquare(outerHalf, z);

    SliceLayer::Polyline hole = makeReversed(makeSquare(innerHalf, z));
    isl.holes.push_back(hole);

    return isl;
}
