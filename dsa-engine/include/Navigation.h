#pragma once
// ============================================================
// Navigation.h – Campus Navigation System
// Implements graph traversal and shortest-path algorithms (BFS, DFS, Dijkstra)
// ============================================================

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "CampusGraph.h"
#include "Algorithms.h" // For PathResult structure
#include <vector>
#include <string>
#include <unordered_map>

namespace VITNav {

enum class OptimizationMode {
    DISTANCE,   // Minimize total walking distance (metres)
    TIME        // Minimize total estimated walking time (minutes)
};

inline std::string modeToString(OptimizationMode mode) {
    return (mode == OptimizationMode::DISTANCE) ? "Shortest Distance" : "Fastest Estimated Time";
}

class Navigation {
public:
    // Required data members per specification
    std::vector<int>    parent;
    std::vector<bool>   visited;
    std::vector<double> distance; // Represents cost: distance (m) or time (min)

    // Algorithm execution telemetry
    std::vector<int>              visitOrder;
    std::vector<std::pair<int,int>> exploredEdges;
    int    nodesVisited{0};
    int    edgesExplored{0};
    double executionTimeMs{0.0};

    // Constructor
    Navigation() = default;

    // Required methods per specification
    PathResult bfs(const CampusGraph& graph, int sourceId, int destId);
    PathResult dfs(const CampusGraph& graph, int sourceId, int destId);
    PathResult dijkstra(const CampusGraph& graph, int sourceId, int destId,
                        OptimizationMode mode = OptimizationMode::DISTANCE);
    std::vector<int> reconstructPath(int sourceId, int destId,
                                     const std::unordered_map<int, int>& idIdx) const;

private:
    void resetState(int n);
};

} // namespace VITNav

#endif // NAVIGATION_H
