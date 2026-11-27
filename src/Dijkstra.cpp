// ============================================================
// Dijkstra.cpp – VIT Pune Campus Navigator
// Dijkstra's Algorithm: guaranteed minimum-distance path
//
// Time Complexity:  O((V + E) log V)  with binary-heap priority queue
// Space Complexity: O(V)
//
// Use case: Finding the shortest walking-distance route between
//           two campus locations when edge weights represent metres.
// ============================================================

#include "Algorithms.h"
#include <queue>
#include <unordered_map>
#include <chrono>
#include <limits>

namespace VITNav {

PathResult dijkstra(const Graph& g, int sourceId, int destId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = g.numLocations();
    const auto& locs = g.allLocations();

    std::unordered_map<int,int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);
    std::vector<int>    parent(n, -1);
    std::vector<bool>   settled(n, false);

    std::vector<int>              visitOrder;
    std::vector<std::pair<int,int>> exploredEdges;

    PathResult result;
    result.algorithmName = "Dijkstra's Algorithm";
    result.sourceId = sourceId;
    result.destId   = destId;

    int srcIdx = idIdx.count(sourceId) ? idIdx[sourceId] : -1;
    int dstIdx = idIdx.count(destId)   ? idIdx[destId]   : -1;
    if (srcIdx < 0 || dstIdx < 0) return result;

    // Min-heap: {distance, locationId}
    using PQEntry = std::pair<double, int>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;

    dist[srcIdx] = 0.0;
    pq.push({0.0, sourceId});

    int nodesVisited = 0, edgesExplored = 0;

    while (!pq.empty()) {
        auto [d, cur] = pq.top(); pq.pop();
        int curIdx = idIdx.count(cur) ? idIdx[cur] : -1;
        if (curIdx < 0 || settled[curIdx]) continue;

        settled[curIdx] = true;
        visitOrder.push_back(cur);
        ++nodesVisited;

        if (cur == destId) break; // early exit

        for (const auto& [nbId, weight] : g.neighbours(cur)) {
            int nbIdx = idIdx.count(nbId) ? idIdx[nbId] : -1;
            if (nbIdx < 0 || settled[nbIdx]) continue;

            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});

            double newDist = dist[curIdx] + weight;
            if (newDist < dist[nbIdx]) {
                dist[nbIdx]   = newDist;
                parent[nbIdx] = cur;
                pq.push({newDist, nbId});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return buildResult(g, "Dijkstra's Algorithm",
                       sourceId, destId,
                       parent, dist,
                       nodesVisited, edgesExplored, ms,
                       visitOrder, exploredEdges);
}

} // namespace VITNav
