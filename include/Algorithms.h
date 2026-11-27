#pragma once
// ============================================================
// Algorithms.h – VIT Pune Campus Navigator
// All graph algorithm declarations (manually implemented)
// ============================================================

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Graph.h"
#include <vector>
#include <string>
#include <chrono>
#include <functional>

namespace VITNav {

// ---- Result returned by every algorithm ----
struct PathResult {
    bool   found{false};
    int    sourceId{-1};
    int    destId{-1};
    std::vector<int> path;          // sequence of location IDs
    double totalDistance{0.0};      // sum of edge weights (metres)
    double totalTimeMin{0.0};       // estimated walking time (minutes)

    // Algorithm statistics
    std::string algorithmName;
    int    nodesVisited{0};
    int    edgesExplored{0};
    double executionTimeMs{0.0};

    // For step-by-step visualisation
    std::vector<int>              visitOrder;
    std::vector<std::pair<int,int>> exploredEdges;

    bool empty() const { return path.empty(); }
};

// ---- BFS ----
// Finds the minimum-hop path (ignores edge weights)
// Time:  O(V + E)   Space: O(V)
PathResult bfs(const Graph& g, int sourceId, int destId);

// ---- DFS ----
// Demonstrates depth-first traversal; does NOT guarantee shortest path
// Time:  O(V + E)   Space: O(V)
PathResult dfs(const Graph& g, int sourceId, int destId);

// ---- Dijkstra ----
// Finds the minimum-weight (shortest-distance) path
// Time:  O((V + E) log V)   Space: O(V)
PathResult dijkstra(const Graph& g, int sourceId, int destId);

// ---- A* Search ----
// Uses Haversine heuristic for geographic distance estimation
// Time:  O((V + E) log V)   Space: O(V)
PathResult aStar(const Graph& g, int sourceId, int destId);

// ---- Floyd-Warshall ----
// All-pairs shortest paths; returns matrix of distances
// Time:  O(V^3)   Space: O(V^2)
struct FloydResult {
    std::vector<std::vector<double>> dist;   // dist[i][j] metres
    std::vector<std::vector<int>>    next;   // for path reconstruction
    int    numLocations{0};
    double executionTimeMs{0.0};
    bool   computed{false};

    // Reconstruct path from floydResult.next
    std::vector<int> getPath(int fromIdx, int toIdx) const;
};
FloydResult floydWarshall(const Graph& g);

// ---- Helper: build PathResult from parent array ----
PathResult buildResult(const Graph& g,
                       const std::string& algName,
                       int sourceId, int destId,
                       const std::vector<int>& parent,
                       const std::vector<double>& dist,
                       int nodesVisited, int edgesExplored,
                       double execMs,
                       const std::vector<int>& visitOrder,
                       const std::vector<std::pair<int,int>>& exploredEdges);

} // namespace VITNav

#endif // ALGORITHMS_H
