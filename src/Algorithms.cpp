// ============================================================
// Algorithms.cpp – VIT Pune Campus Navigator
// Shared helper: buildResult + Floyd-Warshall implementation
// ============================================================

#include "Algorithms.h"
#include <chrono>
#include <limits>
#include <unordered_map>

namespace VITNav {

// ---- buildResult ----
PathResult buildResult(const Graph& g,
                       const std::string& algName,
                       int sourceId, int destId,
                       const std::vector<int>& parent,
                       const std::vector<double>& dist,
                       int nodesVisited, int edgesExplored,
                       double execMs,
                       const std::vector<int>& visitOrder,
                       const std::vector<std::pair<int,int>>& exploredEdges) {
    PathResult result;
    result.algorithmName  = algName;
    result.sourceId       = sourceId;
    result.destId         = destId;
    result.nodesVisited   = nodesVisited;
    result.edgesExplored  = edgesExplored;
    result.executionTimeMs = execMs;
    result.visitOrder     = visitOrder;
    result.exploredEdges  = exploredEdges;

    // Map ID → index
    const auto& locs = g.allLocations();
    std::unordered_map<int,int> idIdx;
    for (int i = 0; i < (int)locs.size(); ++i) idIdx[locs[i].id] = i;

    int dstIdx = idIdx.count(destId) ? idIdx[destId] : -1;
    if (dstIdx < 0) return result;

    const double INF = std::numeric_limits<double>::infinity();
    if (dist[dstIdx] >= INF) return result; // no path found

    // Reconstruct path by following parent pointers
    std::vector<int> path;
    for (int cur = destId; cur != -1; ) {
        path.push_back(cur);
        int idx = idIdx.count(cur) ? idIdx[cur] : -1;
        if (idx < 0 || parent[idx] == cur) break; // source reached
        if (cur == sourceId) break;
        cur = parent[idx];
        if (cur == sourceId) { path.push_back(sourceId); break; }
    }
    // Ensure source is included
    if (path.empty() || path.back() != sourceId) path.push_back(sourceId);
    std::reverse(path.begin(), path.end());

    if (path.empty() || path.front() != sourceId || path.back() != destId)
        return result;

    result.found        = true;
    result.path         = path;
    result.totalDistance = dist[dstIdx];
    result.totalTimeMin  = (result.totalDistance / 1000.0) / 5.0 * 60.0;

    return result;
}

// ---- Floyd-Warshall ----
FloydResult floydWarshall(const Graph& g) {
    auto startTime = std::chrono::high_resolution_clock::now();

    FloydResult fr;
    const auto& locs = g.allLocations();
    int n = static_cast<int>(locs.size());
    fr.numLocations = n;

    const double INF = std::numeric_limits<double>::infinity();

    // Initialise distance and next matrices
    fr.dist.assign(n, std::vector<double>(n, INF));
    fr.next.assign(n, std::vector<int>(n, -1));

    // Map location id → matrix index
    std::unordered_map<int,int> idIdx;
    for (int i = 0; i < n; ++i) {
        idIdx[locs[i].id] = i;
        fr.dist[i][i] = 0.0;
    }

    // Fill in direct edges
    for (int i = 0; i < n; ++i) {
        for (const auto& [nbId, weight] : g.neighbours(locs[i].id)) {
            int j = idIdx.count(nbId) ? idIdx[nbId] : -1;
            if (j < 0) continue;
            if (weight < fr.dist[i][j]) {
                fr.dist[i][j] = weight;
                fr.dist[j][i] = weight;
                fr.next[i][j] = j;
                fr.next[j][i] = i;
            }
        }
    }

    // Floyd-Warshall triple loop: O(V^3)
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (fr.dist[i][k] == INF) continue;
            for (int j = 0; j < n; ++j) {
                if (fr.dist[k][j] == INF) continue;
                double via = fr.dist[i][k] + fr.dist[k][j];
                if (via < fr.dist[i][j]) {
                    fr.dist[i][j] = via;
                    fr.next[i][j] = fr.next[i][k];
                }
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    fr.executionTimeMs = std::chrono::duration<double, std::milli>(
                             endTime - startTime).count();
    fr.computed = true;
    return fr;
}

std::vector<int> FloydResult::getPath(int fromIdx, int toIdx) const {
    if (!computed || next[fromIdx][toIdx] == -1) return {};
    std::vector<int> path;
    int cur = fromIdx;
    while (cur != toIdx) {
        path.push_back(cur);
        cur = next[cur][toIdx];
        if (cur == -1) return {}; // disconnected
    }
    path.push_back(toIdx);
    return path;
}

} // namespace VITNav
