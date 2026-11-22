#include "Navigation.h"
#include <queue>
#include <stack>
#include <limits>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace VITNav {

void Navigation::resetState(int n) {
    parent.assign(n, -1);
    visited.assign(n, false);
    distance.assign(n, std::numeric_limits<double>::infinity());
    visitOrder.clear();
    exploredEdges.clear();
    nodesVisited = 0;
    edgesExplored = 0;
    executionTimeMs = 0.0;
}

std::vector<int> Navigation::reconstructPath(int sourceId, int destId,
                                            const std::unordered_map<int, int>& idIdx) const {
    if (sourceId == destId) return {sourceId};

    auto dstIt = idIdx.find(destId);
    if (dstIt == idIdx.end()) return {};

    int dstIdx = dstIt->second;
    if (parent[dstIdx] == -1) return {}; // unreachable

    std::vector<int> path;
    int cur = destId;
    while (cur != -1) {
        path.push_back(cur);
        if (cur == sourceId) break;
        auto it = idIdx.find(cur);
        if (it == idIdx.end()) break;
        cur = parent[it->second];
    }

    if (path.empty() || path.back() != sourceId) return {};
    std::reverse(path.begin(), path.end());
    return path;
}

PathResult Navigation::bfs(const CampusGraph& graph, int sourceId, int destId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = graph.numLocations();
    resetState(n);

    // Map location ID -> internal array index [0..n)
    const auto& locs = graph.allLocations();
    std::unordered_map<int, int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    PathResult result;
    result.algorithmName = "BFS (Fewest Walkway Segments)";
    result.sourceId = sourceId;
    result.destId = destId;

    if (!graph.hasLocation(sourceId) || !graph.hasLocation(destId)) {
        return result;
    }

    if (sourceId == destId) {
        result.found = true;
        result.path = {sourceId};
        result.totalDistance = 0.0;
        result.totalTimeMin = 0.0;
        return result;
    }

    int srcIdx = idIdx[sourceId];
    std::queue<int> q;
    visited[srcIdx] = true;
    distance[srcIdx] = 0.0; // Distance represents hop count in BFS
    q.push(sourceId);
    visitOrder.push_back(sourceId);
    nodesVisited = 1;

    bool reached = false;

    while (!q.empty()) {
        int cur = q.front();
        q.pop();

        if (cur == destId) {
            reached = true;
            break;
        }

        int curIdx = idIdx[cur];

        // Examine available (unblocked) walkways
        for (const auto& edge : graph.getNeighbors(cur, false)) {
            int nbId = edge.destinationId;
            auto nbIt = idIdx.find(nbId);
            if (nbIt == idIdx.end()) continue;
            int nbIdx = nbIt->second;

            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});

            if (!visited[nbIdx]) {
                visited[nbIdx] = true;
                parent[nbIdx] = cur;
                distance[nbIdx] = distance[curIdx] + 1.0;
                q.push(nbId);
                visitOrder.push_back(nbId);
                ++nodesVisited;

                if (nbId == destId) {
                    reached = true;
                    break;
                }
            }
        }
        if (reached) break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    if (!reached) {
        result.found = false;
        result.nodesVisited = nodesVisited;
        result.edgesExplored = edgesExplored;
        result.executionTimeMs = executionTimeMs;
        return result;
    }

    std::vector<int> path = reconstructPath(sourceId, destId, idIdx);
    if (path.empty()) {
        result.found = false;
        return result;
    }

    // Compute actual distance and walking time along reconstructed route
    double totalDist = 0.0;
    double totalTime = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        auto edgeOpt = graph.getEdge(path[i], path[i+1]);
        if (edgeOpt) {
            totalDist += edgeOpt->distance;
            totalTime += edgeOpt->walkingTime;
        }
    }

    result.found = true;
    result.path = path;
    result.totalDistance = totalDist;
    result.totalTimeMin = totalTime;
    result.nodesVisited = nodesVisited;
    result.edgesExplored = edgesExplored;
    result.executionTimeMs = executionTimeMs;
    result.visitOrder = visitOrder;
    result.exploredEdges = exploredEdges;

    return result;
}

PathResult Navigation::dfs(const CampusGraph& graph, int sourceId, int destId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = graph.numLocations();
    resetState(n);

    const auto& locs = graph.allLocations();
    std::unordered_map<int, int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    PathResult result;
    result.algorithmName = "DFS (Exploratory Campus Traversal)";
    result.sourceId = sourceId;
    result.destId = destId;

    if (!graph.hasLocation(sourceId) || !graph.hasLocation(destId)) {
        return result;
    }

    if (sourceId == destId) {
        result.found = true;
        result.path = {sourceId};
        result.totalDistance = 0.0;
        result.totalTimeMin = 0.0;
        return result;
    }

    int srcIdx = idIdx[sourceId];
    std::stack<int> stk;
    stk.push(sourceId);
    visited[srcIdx] = true;
    visitOrder.push_back(sourceId);
    nodesVisited = 1;

    bool reached = false;

    while (!stk.empty()) {
        int cur = stk.top();
        stk.pop();

        if (cur == destId) {
            reached = true;
            break;
        }

        for (const auto& edge : graph.getNeighbors(cur, false)) {
            int nbId = edge.destinationId;
            auto nbIt = idIdx.find(nbId);
            if (nbIt == idIdx.end()) continue;
            int nbIdx = nbIt->second;

            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});

            if (!visited[nbIdx]) {
                visited[nbIdx] = true;
                parent[nbIdx] = cur;
                stk.push(nbId);
                visitOrder.push_back(nbId);
                ++nodesVisited;

                if (nbId == destId) {
                    reached = true;
                    break;
                }
            }
        }
        if (reached) break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    if (!reached) {
        result.found = false;
        result.nodesVisited = nodesVisited;
        result.edgesExplored = edgesExplored;
        result.executionTimeMs = executionTimeMs;
        result.visitOrder = visitOrder;
        return result;
    }

    std::vector<int> path = reconstructPath(sourceId, destId, idIdx);
    double totalDist = 0.0;
    double totalTime = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        auto edgeOpt = graph.getEdge(path[i], path[i+1]);
        if (edgeOpt) {
            totalDist += edgeOpt->distance;
            totalTime += edgeOpt->walkingTime;
        }
    }

    result.found = true;
    result.path = path;
    result.totalDistance = totalDist;
    result.totalTimeMin = totalTime;
    result.nodesVisited = nodesVisited;
    result.edgesExplored = edgesExplored;
    result.executionTimeMs = executionTimeMs;
    result.visitOrder = visitOrder;
    result.exploredEdges = exploredEdges;

    return result;
}

PathResult Navigation::dijkstra(const CampusGraph& graph, int sourceId, int destId,
                               OptimizationMode mode) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int n = graph.numLocations();
    resetState(n);

    const auto& locs = graph.allLocations();
    std::unordered_map<int, int> idIdx;
    for (int i = 0; i < n; ++i) idIdx[locs[i].id] = i;

    PathResult result;
    result.algorithmName = (mode == OptimizationMode::DISTANCE)
                           ? "Dijkstra (Shortest Distance)"
                           : "Dijkstra (Fastest Estimated Time)";
    result.sourceId = sourceId;
    result.destId = destId;

    if (!graph.hasLocation(sourceId) || !graph.hasLocation(destId)) {
        return result;
    }

    if (sourceId == destId) {
        result.found = true;
        result.path = {sourceId};
        result.totalDistance = 0.0;
        result.totalTimeMin = 0.0;
        return result;
    }

    int srcIdx = idIdx[sourceId];
    int dstIdx = idIdx[destId];

    // Priority queue of pairs: {cost, locationId} (min-heap)
    using PQEntry = std::pair<double, int>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;

    distance[srcIdx] = 0.0;
    pq.push({0.0, sourceId});

    std::vector<bool> settled(n, false);

    while (!pq.empty()) {
        auto [d, cur] = pq.top();
        pq.pop();

        int curIdx = idIdx[cur];
        if (settled[curIdx]) continue;

        settled[curIdx] = true;
        visited[curIdx] = true;
        visitOrder.push_back(cur);
        ++nodesVisited;

        if (cur == destId) {
            break; // Destination settled with optimal cost
        }

        // Explore available (unblocked) outgoing edges
        for (const auto& edge : graph.getNeighbors(cur, false)) {
            int nbId = edge.destinationId;
            auto nbIt = idIdx.find(nbId);
            if (nbIt == idIdx.end()) continue;
            int nbIdx = nbIt->second;

            if (settled[nbIdx]) continue;

            ++edgesExplored;
            exploredEdges.push_back({cur, nbId});

            // Select cost metric based on optimization mode
            double weight = (mode == OptimizationMode::DISTANCE) ? edge.distance : edge.walkingTime;

            if (weight < 0.0) {
                std::cerr << "[Navigation::dijkstra] Error: Negative edge weight detected! Dijkstra requires non-negative weights.\n";
                continue;
            }

            double newCost = distance[curIdx] + weight;
            if (newCost < distance[nbIdx]) {
                distance[nbIdx] = newCost;
                parent[nbIdx] = cur;
                pq.push({newCost, nbId});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Check if destination was reached
    if (!settled[dstIdx] || parent[dstIdx] == -1) {
        result.found = false;
        result.nodesVisited = nodesVisited;
        result.edgesExplored = edgesExplored;
        result.executionTimeMs = executionTimeMs;
        result.visitOrder = visitOrder;
        return result;
    }

    std::vector<int> path = reconstructPath(sourceId, destId, idIdx);
    if (path.empty()) {
        result.found = false;
        return result;
    }

    // Accurately compute BOTH physical distance and estimated walking time
    double totalDist = 0.0;
    double totalTime = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        auto edgeOpt = graph.getEdge(path[i], path[i+1]);
        if (edgeOpt) {
            totalDist += edgeOpt->distance;
            totalTime += edgeOpt->walkingTime;
        }
    }

    result.found = true;
    result.path = path;
    result.totalDistance = totalDist;
    result.totalTimeMin = totalTime;
    result.nodesVisited = nodesVisited;
    result.edgesExplored = edgesExplored;
    result.executionTimeMs = executionTimeMs;
    result.visitOrder = visitOrder;
    result.exploredEdges = exploredEdges;

    return result;
}

} // namespace VITNav
