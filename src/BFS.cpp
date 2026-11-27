// ============================================================
// BFS.cpp – VIT Pune Campus Navigator
// Breadth-First Search: minimum-hop path (ignores weights)
//
// Time Complexity:  O(V + E)
// Space Complexity: O(V)
//
// Use case: Finding the route with fewest intermediate stops,
//           regardless of walking distance.
// ============================================================

#include "Algorithms.h"
#include <queue>
#include <unordered_map>
#include <chrono>

namespace VITNav {

PathResult bfs(const Graph& g, int sourceId, int destId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = g.numLocations();
    std::vector<int>    parent(n, -1);
    std::vector<bool>   visited(n, false);
    std::vector<double> dist(n, 0.0);     // accumulated distance (for display)

    std::vector<int>              visitOrder;
    std::vector<std::pair<int,int>> exploredEdges;

    // Map location IDs to [0..n) indices for arrays
    const auto& locs = g.allLocations();
    std::unordered_map<int,int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    int srcIdx = idIdx.count(sourceId) ? idIdx[sourceId] : -1;
    int dstIdx = idIdx.count(destId)   ? idIdx[destId]   : -1;

    PathResult result;
    result.algorithmName = "BFS (Breadth-First Search)";
    result.sourceId = sourceId;
    result.destId   = destId;

    if (srcIdx < 0 || dstIdx < 0) return result;

    std::queue<int> q;
    visited[srcIdx] = true;
    q.push(sourceId);
    visitOrder.push_back(sourceId);

    int nodesVisited = 1, edgesExplored = 0;

    while (!q.empty()) {
        int cur = q.front(); q.pop();
        int curIdx = idIdx[cur];

        if (cur == destId) break; // found destination

        for (const auto& [nbId, weight] : g.neighbours(cur)) {
            int nbIdx = idIdx.count(nbId) ? idIdx[nbId] : -1;
            if (nbIdx < 0) continue;
            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});
            if (!visited[nbIdx]) {
                visited[nbIdx] = true;
                parent[nbIdx] = cur;
                dist[nbIdx]   = dist[curIdx] + weight;
                q.push(nbId);
                visitOrder.push_back(nbId);
                ++nodesVisited;
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return buildResult(g, "BFS (Breadth-First Search)",
                       sourceId, destId,
                       parent, dist,
                       nodesVisited, edgesExplored, ms,
                       visitOrder, exploredEdges);
}

} // namespace VITNav
