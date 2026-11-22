#pragma once
// ============================================================
// NavigationController.h – VIT Pune Campus Navigator
// Loads campus data, owns Graph, dispatches algorithm calls
// ============================================================

#ifndef NAVIGATIONCONTROLLER_H
#define NAVIGATIONCONTROLLER_H

#include "Graph.h"
#include "Algorithms.h"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <set>

namespace VITNav {

enum class Algorithm {
    BFS,
    DFS,
    Dijkstra,
    AStar,
    FloydWarshall
};

std::string algorithmName(Algorithm alg);

class NavigationController {
public:
    NavigationController();

    // ---- Data loading ----
    bool loadLocations(const std::string& jsonPath);
    bool loadConnections(const std::string& jsonPath);
    bool loadAll(const std::string& dataDir);  // loads locations.json + connections.json

    // ---- Navigation ----
    PathResult findRoute(int sourceId, int destId, Algorithm alg);

    // ---- Floyd-Warshall (all-pairs) ----
    const FloydResult& getFloydResult();
    bool isFloydComputed() const { return floydResult_.computed; }

    // ---- Graph access ----
    const Graph& graph() const { return graph_; }
    Graph&       graph()       { return graph_; }

    // ---- Search ----
    std::vector<const Location*> searchLocations(const std::string& query) const;

    // ---- Edge control ----
    void blockEdge(int fromId, int toId);
    void unblockEdge(int fromId, int toId);
    bool isEdgeBlocked(int fromId, int toId) const;
    void clearAllBlocks();

    // ---- Algorithm comparison ----
    struct ComparisonResult {
        PathResult bfsResult;
        PathResult dfsResult;
        PathResult dijkstraResult;
        PathResult aStarResult;
    };
    ComparisonResult compareAlgorithms(int sourceId, int destId);

    // ---- Utility ----
    bool isLoaded() const { return loaded_; }
    std::string lastError() const { return lastError_; }

private:
    Graph  graph_;
    bool   loaded_{false};
    std::string lastError_;
    FloydResult floydResult_;

    std::set<std::pair<int,int>> blockedEdges_;

    // JSON parsing helpers (manual, no external library)
    bool parseLocationsJson(const std::string& content);
    bool parseConnectionsJson(const std::string& content);
    std::string readFile(const std::string& path) const;
};

} // namespace VITNav

#endif // NAVIGATIONCONTROLLER_H
