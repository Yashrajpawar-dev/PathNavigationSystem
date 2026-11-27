// ============================================================
// AStar.cpp – VIT Pune Campus Navigator
// A* Search: informed shortest path using Haversine heuristic
//
// Time Complexity:  O((V + E) log V)  — typically faster than
//                   Dijkstra due to informed heuristic
// Space Complexity: O(V)
//
// Heuristic h(n): Haversine distance from node n to destination.
// h(n) is admissible because straight-line distance ≤ actual
// walking distance (never overestimates).
//
// Use case: Same as Dijkstra but explores fewer nodes using
//           geographic knowledge, making it faster in practice.
// ============================================================

#include "Algorithms.h"
#include <queue>
#include <unordered_map>
#include <chrono>
#include <limits>

namespace VITNav {

PathResult aStar(const Graph& g, int sourceId, int destId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = g.numLocations();
    const auto& locs = g.allLocations();

    std::unordered_map<int,int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> gCost(n, INF);  // actual cost from source
    std::vector<double> fCost(n, INF);  // f = g + h
    std::vector<int>    parent(n, -1);
    std::vector<bool>   closed(n, false);

    std::vector<int>              visitOrder;
    std::vector<std::pair<int,int>> exploredEdges;

    PathResult result;
    result.algorithmName = "A* Search";
    result.sourceId = sourceId;
    result.destId   = destId;

    int srcIdx = idIdx.count(sourceId) ? idIdx[sourceId] : -1;
    int dstIdx = idIdx.count(destId)   ? idIdx[destId]   : -1;
    if (srcIdx < 0 || dstIdx < 0) return result;

    // Heuristic: Haversine distance to destination
    auto heuristic = [&](int id) -> double {
        return g.haversineDistance(id, destId);
    };

    // Min-heap: {fCost, locationId}
    using PQEntry = std::pair<double, int>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> openSet;

    gCost[srcIdx] = 0.0;
    fCost[srcIdx] = heuristic(sourceId);
    openSet.push({fCost[srcIdx], sourceId});

    int nodesVisited = 0, edgesExplored = 0;

    while (!openSet.empty()) {
        auto [f, cur] = openSet.top(); openSet.pop();
        int curIdx = idIdx.count(cur) ? idIdx[cur] : -1;
        if (curIdx < 0 || closed[curIdx]) continue;

        closed[curIdx] = true;
        visitOrder.push_back(cur);
        ++nodesVisited;

        if (cur == destId) break;

        for (const auto& [nbId, weight] : g.neighbours(cur)) {
            int nbIdx = idIdx.count(nbId) ? idIdx[nbId] : -1;
            if (nbIdx < 0 || closed[nbIdx]) continue;

            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});

            double tentativeG = gCost[curIdx] + weight;
            if (tentativeG < gCost[nbIdx]) {
                gCost[nbIdx]  = tentativeG;
                fCost[nbIdx]  = tentativeG + heuristic(nbId);
                parent[nbIdx] = cur;
                openSet.push({fCost[nbIdx], nbId});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return buildResult(g, "A* Search (Haversine Heuristic)",
                       sourceId, destId,
                       parent, gCost,
                       nodesVisited, edgesExplored, ms,
                       visitOrder, exploredEdges);
}

} // namespace VITNav
