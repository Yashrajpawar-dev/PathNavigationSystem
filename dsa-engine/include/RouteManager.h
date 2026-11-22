#pragma once
// ============================================================
// RouteManager.h – Campus Navigation System
// Manages route candidates, alternative routes, metrics and comparison
// ============================================================

#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include "CampusGraph.h"
#include "Navigation.h"
#include <vector>

namespace VITNav {

class RouteManager {
public:
    // Required data members per specification
    std::vector<PathResult> routes;
    std::vector<double>     routeCosts;

    // Constructor
    RouteManager() = default;

    // Required methods per specification
    std::vector<PathResult> findAlternativeRoutes(CampusGraph& graph, Navigation& nav,
                                                 int sourceId, int destId,
                                                 OptimizationMode mode = OptimizationMode::DISTANCE,
                                                 int maxAlternatives = 3);

    double calculateDistance(const CampusGraph& graph, const std::vector<int>& path) const;
    double calculateTime(const CampusGraph& graph, const std::vector<int>& path) const;
    void   displayRoute(const CampusGraph& graph, const PathResult& route) const;
    void   compareRoutes(const CampusGraph& graph, Navigation& nav, int sourceId, int destId);

    void clear() {
        routes.clear();
        routeCosts.clear();
    }
};

} // namespace VITNav

#endif // ROUTE_MANAGER_H
