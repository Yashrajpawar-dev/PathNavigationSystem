#include "FacilityFinder.h"
#include <algorithm>
#include <limits>
#include <iostream>

namespace VITNav {

PathResult FacilityFinder::findNearestFacility(const CampusGraph& graph, Navigation& nav,
                                              int sourceId, const std::string& category,
                                              OptimizationMode mode,
                                              bool excludeSource) {
    facilityCategory = category;
    nearestFacilityId = -1;

    PathResult emptyResult;
    emptyResult.algorithmName = "Nearest Facility Search";
    emptyResult.sourceId = sourceId;
    emptyResult.destId = -1;
    emptyResult.found = false;

    if (!graph.hasLocation(sourceId)) {
        return emptyResult;
    }

    // 1. Identify all locations matching category (case-insensitive)
    std::string qCat = category;
    std::transform(qCat.begin(), qCat.end(), qCat.begin(), ::tolower);

    std::vector<int> matchingIds;
    for (const auto& loc : graph.allLocations()) {
        std::string locCat = loc.categoryString();
        std::transform(locCat.begin(), locCat.end(), locCat.begin(), ::tolower);

        std::string locName = loc.name;
        std::transform(locName.begin(), locName.end(), locName.begin(), ::tolower);

        if (locCat.find(qCat) != std::string::npos ||
            locName.find(qCat) != std::string::npos ||
            loc.categoryName.find(qCat) != std::string::npos) {
            if (excludeSource && loc.id == sourceId) continue;
            matchingIds.push_back(loc.id);
        }
    }

    if (matchingIds.empty()) {
        return emptyResult;
    }

    // 2. Evaluate shortest route to each matching facility
    int bestFacilityId = -1;
    double bestCost = std::numeric_limits<double>::infinity();
    PathResult bestResult;

    for (int targetId : matchingIds) {
        PathResult res = nav.dijkstra(graph, sourceId, targetId, mode);
        if (res.found) {
            double cost = (mode == OptimizationMode::DISTANCE) ? res.totalDistance : res.totalTimeMin;
            if (cost < bestCost) {
                bestCost = cost;
                bestFacilityId = targetId;
                bestResult = res;
            }
        }
    }

    if (bestFacilityId != -1) {
        nearestFacilityId = bestFacilityId;
        bestResult.algorithmName = "Nearest Facility (" + category + ") via " + modeToString(mode);
        return bestResult;
    }

    return emptyResult;
}

} // namespace VITNav
