// ============================================================
// DFS.cpp – VIT Pune Campus Navigator
// Depth-First Search: graph traversal (NOT guaranteed shortest)
//
// Time Complexity:  O(V + E)
// Space Complexity: O(V)
//
// NOTE: DFS does NOT guarantee the shortest or minimum-distance
//       path. It demonstrates deep graph traversal and is
//       useful for reachability analysis and cycle detection.
// ============================================================

#include "Algorithms.h"
#include <stack>
#include <unordered_map>
#include <chrono>

namespace VITNav {

PathResult dfs(const Graph& g, int sourceId, int destId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = g.numLocations();
    const auto& locs = g.allLocations();

    std::unordered_map<int,int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    std::vector<int>    parent(n, -1);
    std::vector<bool>   visited(n, false);
    std::vector<double> dist(n, 0.0);

    std::vector<int>              visitOrder;
    std::vector<std::pair<int,int>> exploredEdges;

    PathResult result;
    result.algorithmName = "DFS (Depth-First Search)";
    result.sourceId = sourceId;
    result.destId   = destId;

    int srcIdx = idIdx.count(sourceId) ? idIdx[sourceId] : -1;
    int dstIdx = idIdx.count(destId)   ? idIdx[destId]   : -1;
    if (srcIdx < 0 || dstIdx < 0) return result;

    // Iterative DFS using an explicit stack
    // Stack stores {locationId, distanceSoFar}
    std::stack<std::pair<int, double>> stk;
    stk.push({sourceId, 0.0});
    visited[srcIdx] = true;
    visitOrder.push_back(sourceId);

    int nodesVisited = 1, edgesExplored = 0;
    bool reachedDest = false;

    while (!stk.empty() && !reachedDest) {
        auto [cur, curDist] = stk.top(); stk.pop();
        int curIdx = idIdx[cur];

        if (cur == destId) {
            reachedDest = true;
            break;
        }

        for (const auto& [nbId, weight] : g.neighbours(cur)) {
            int nbIdx = idIdx.count(nbId) ? idIdx[nbId] : -1;
            if (nbIdx < 0) continue;
            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});
            if (!visited[nbIdx]) {
                visited[nbIdx] = true;
                parent[nbIdx]  = cur;
                dist[nbIdx]    = curDist + weight;
                stk.push({nbId, dist[nbIdx]});
                visitOrder.push_back(nbId);
                ++nodesVisited;
                if (nbId == destId) { reachedDest = true; break; }
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return buildResult(g,
        "DFS (Depth-First Search) [⚠ Not Guaranteed Shortest]",
        sourceId, destId,
        parent, dist,
        nodesVisited, edgesExplored, ms,
        visitOrder, exploredEdges);
}

} // namespace VITNav
