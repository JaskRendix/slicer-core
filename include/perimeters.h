#pragma once
#include "islands.h"
#include <vector>

std::vector<std::vector<SliceLayer::Polyline>>
generatePerimeters(const Island &island, int count, double width);
