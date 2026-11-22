#include "RouteManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace VITNav {

double RouteManager::calculateDistance(const CampusGraph& graph, const std::vector<int>& path) const {
    double total = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        auto edgeOpt = graph.getEdge(path[i], path[i+1]);
        if (edgeOpt) total += edgeOpt->distance;
    }
    return total;
}

double RouteManager::calculateTime(const CampusGraph& graph, const std::vector<int>& path) const {
    double total = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        auto edgeOpt = graph.getEdge(path[i], path[i+1]);
        if (edgeOpt) total += edgeOpt->walkingTime;
    }
    return total;
}

std::vector<PathResult> RouteManager::findAlternativeRoutes(CampusGraph& graph, Navigation& nav,
                                                           int sourceId, int destId,
                                                           OptimizationMode mode,
                                                           int maxAlternatives) {
    routes.clear();
    routeCosts.clear();

    if (sourceId == destId) return routes;

    // 1. Find primary route
    PathResult primary = nav.dijkstra(graph, sourceId, destId, mode);
    if (!primary.found || primary.path.size() < 2) {
        return routes;
    }

    std::vector<PathResult> candidates;

    // 2. Temporarily block each edge along the primary route
    for (size_t i = 0; i + 1 < primary.path.size(); ++i) {
        int u = primary.path[i];
        int v = primary.path[i+1];

        // Ensure we only touch edges that were actually available initially
        if (!graph.isWalkwayAvailable(u, v)) continue;

        // Temporarily block
        graph.blockWalkway(u, v);

        // Rerun Dijkstra
        PathResult alt = nav.dijkstra(graph, sourceId, destId, mode);
        alt.algorithmName = "Alternative Route";

        if (alt.found && !alt.path.empty()) {
            // Check if distinct from primary path and already recorded candidates
            bool isDuplicate = (alt.path == primary.path);
            for (const auto& cand : candidates) {
                if (cand.path == alt.path) {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate) {
                candidates.push_back(alt);
            }
        }

        // Restore walkway
        graph.restoreWalkway(u, v);
    }

    // 3. Sort candidates by cost ascending
    std::sort(candidates.begin(), candidates.end(), [mode](const PathResult& a, const PathResult& b) {
        if (mode == OptimizationMode::DISTANCE) {
            return a.totalDistance < b.totalDistance;
        } else {
            return a.totalTimeMin < b.totalTimeMin;
        }
    });

    // 4. Retain up to maxAlternatives
    for (size_t i = 0; i < candidates.size() && static_cast<int>(routes.size()) < maxAlternatives; ++i) {
        routes.push_back(candidates[i]);
        double cost = (mode == OptimizationMode::DISTANCE) ? candidates[i].totalDistance : candidates[i].totalTimeMin;
        routeCosts.push_back(cost);
    }

    return routes;
}

void RouteManager::displayRoute(const CampusGraph& graph, const PathResult& route) const {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "                         ROUTE NAVIGATION REPORT\n";
    std::cout << std::string(80, '=') << "\n";
    std::cout << "Algorithm / Mode : " << route.algorithmName << "\n";

    const Location* srcLoc = graph.getLocationById(route.sourceId);
    const Location* dstLoc = graph.getLocationById(route.destId);

    std::cout << "Source           : [" << route.sourceId << "] "
              << (srcLoc ? srcLoc->name : "Unknown") << "\n";
    std::cout << "Destination      : [" << route.destId << "] "
              << (dstLoc ? dstLoc->name : "Unknown") << "\n";

    if (!route.found) {
        std::cout << "\nStatus: NO ROUTE AVAILABLE (Destination unreachable or connecting paths blocked)\n";
        std::cout << std::string(80, '=') << "\n";
        return;
    }

    if (route.sourceId == route.destId) {
        std::cout << "\nStatus: SOURCE EQUALS DESTINATION (Already at destination)\n";
        std::cout << "Total Distance   : 0.0 metres\n";
        std::cout << "Estimated Time   : 0.0 minutes\n";
        std::cout << std::string(80, '=') << "\n";
        return;
    }

    std::cout << "\n--- Route Overview ---\n";
    for (size_t i = 0; i < route.path.size(); ++i) {
        const Location* loc = graph.getLocationById(route.path[i]);
        std::cout << "[" << route.path[i] << "] " << (loc ? loc->shortName : "Loc");
        if (i + 1 < route.path.size()) std::cout << "  -->  ";
    }
    std::cout << "\n\n--- Turn-by-Turn Walkway Breakdown ---\n";
    std::cout << std::left << std::setw(6)  << "Step"
              << std::setw(30) << "From"
              << std::setw(30) << "To"
              << std::setw(12) << "Distance"
              << std::setw(12) << "Est. Time"
              << "Path Type\n";
    std::cout << std::string(98, '-') << "\n";

    for (size_t i = 0; i + 1 < route.path.size(); ++i) {
        int u = route.path[i];
        int v = route.path[i+1];
        const Location* locU = graph.getLocationById(u);
        const Location* locV = graph.getLocationById(v);
        auto edgeOpt = graph.getEdge(u, v);

        std::string uName = locU ? locU->shortName : std::to_string(u);
        std::string vName = locV ? locV->shortName : std::to_string(v);
        double dist = edgeOpt ? edgeOpt->distance : 0.0;
        double time = edgeOpt ? edgeOpt->walkingTime : 0.0;
        std::string pType = edgeOpt ? edgeOpt->pathTypeString() : "Walkway";

        std::cout << std::left << std::setw(6)  << (i + 1)
                  << std::setw(30) << uName
                  << std::setw(30) << vName
                  << std::fixed << std::setprecision(1)
                  << std::setw(12) << (std::to_string((int)dist) + " m")
                  << std::setw(12) << (std::to_string(time).substr(0, 4) + " min")
                  << pType << "\n";
    }

    std::cout << std::string(98, '-') << "\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Total Walking Distance   : " << route.totalDistance << " metres\n";
    std::cout << "Estimated Walking Time   : " << route.totalTimeMin << " minutes (at 5 km/h)\n";
    std::cout << "Walkway Segments (Hops)  : " << (route.path.size() - 1) << " segments\n";
    std::cout << "Algorithm Telemetry      : " << route.nodesVisited << " nodes visited, "
              << route.edgesExplored << " edges examined, "
              << std::setprecision(3) << route.executionTimeMs << " ms computation\n";
    std::cout << std::string(80, '=') << "\n";
}

void RouteManager::compareRoutes(const CampusGraph& graph, Navigation& nav,
                                int sourceId, int destId) {
    std::cout << "\n" << std::string(105, '=') << "\n";
    std::cout << "                           MULTI-ALGORITHM ROUTE COMPARISON DASHBOARD\n";
    std::cout << std::string(105, '=') << "\n";

    const Location* s = graph.getLocationById(sourceId);
    const Location* d = graph.getLocationById(destId);
    std::cout << "Route Query: [" << sourceId << "] " << (s ? s->name : "?")
              << "  -->  [" << destId << "] " << (d ? d->name : "?") << "\n\n";

    PathResult rDist = nav.dijkstra(graph, sourceId, destId, OptimizationMode::DISTANCE);
    PathResult rTime = nav.dijkstra(graph, sourceId, destId, OptimizationMode::TIME);
    PathResult rBfs  = nav.bfs(graph, sourceId, destId);
    PathResult rDfs  = nav.dfs(graph, sourceId, destId);

    std::cout << std::left
              << std::setw(28) << "Algorithm"
              << std::setw(10) << "Found?"
              << std::setw(8)  << "Hops"
              << std::setw(16) << "Distance"
              << std::setw(14) << "Est. Time"
              << std::setw(14) << "Nodes Visited"
              << "Exec Time\n";
    std::cout << std::string(105, '-') << "\n";

    auto printRow = [](const std::string& name, const PathResult& r) {
        std::cout << std::left << std::setw(28) << name
                  << std::setw(10) << (r.found ? "YES" : "NO");
        if (r.found) {
            int hops = r.path.empty() ? 0 : static_cast<int>(r.path.size() - 1);
            std::cout << std::setw(8)  << hops
                      << std::fixed << std::setprecision(1)
                      << std::setw(16) << (std::to_string((int)r.totalDistance) + " m")
                      << std::setw(14) << (std::to_string(r.totalTimeMin).substr(0, 4) + " min")
                      << std::setw(14) << r.nodesVisited
                      << std::fixed << std::setprecision(3) << r.executionTimeMs << " ms\n";
        } else {
            std::cout << std::setw(8)  << "-"
                      << std::setw(16) << "-"
                      << std::setw(14) << "-"
                      << std::setw(14) << r.nodesVisited
                      << std::fixed << std::setprecision(3) << r.executionTimeMs << " ms\n";
        }
    };

    printRow("Dijkstra (Shortest Dist)", rDist);
    printRow("Dijkstra (Fastest Time)", rTime);
    printRow("BFS (Fewest Hops)", rBfs);
    printRow("DFS (Depth Traversal)", rDfs);

    std::cout << std::string(105, '=') << "\n";
    std::cout << "* Note: Dijkstra optimizes for minimal weight (distance or time).\n";
    std::cout << "* Note: BFS guarantees the minimum number of walkway segments (hops).\n";
    std::cout << "* Note: DFS demonstrates graph traversal and does not guarantee optimality.\n";
    std::cout << std::string(105, '=') << "\n";
}

} // namespace VITNav
