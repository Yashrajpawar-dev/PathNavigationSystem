#pragma once
// ============================================================
// FacilityFinder.h – Campus Navigation System
// Finds the nearest reachable campus facility of a given category
// ============================================================

#ifndef FACILITY_FINDER_H
#define FACILITY_FINDER_H

#include "CampusGraph.h"
#include "Navigation.h"
#include <string>

namespace VITNav {

class FacilityFinder {
public:
    // Required data members per specification
    std::string facilityCategory;
    int         nearestFacilityId{-1};

    // Constructor
    FacilityFinder() = default;

    // Required method per specification
    PathResult findNearestFacility(const CampusGraph& graph, Navigation& nav,
                                  int sourceId, const std::string& category,
                                  OptimizationMode mode = OptimizationMode::DISTANCE,
                                  bool excludeSource = false);
};

} // namespace VITNav

#endif // FACILITY_FINDER_H
