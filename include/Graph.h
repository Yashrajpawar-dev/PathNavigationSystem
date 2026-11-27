#pragma once
// ============================================================
// Graph.h – VIT Pune Campus Navigator
// Weighted undirected graph using adjacency list representation
// ============================================================

#ifndef GRAPH_H
#define GRAPH_H

#include "Location.h"
#include "Edge.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

namespace VITNav {

// Adjacency list entry: {neighbour_id, edge_distance_m}
using AdjEntry = std::pair<int, double>;

class Graph {
public:
    // ---- Construction ----
    Graph() = default;
    explicit Graph(int numNodes);

    void addLocation(const Location& loc);
    void addEdge(const Edge& edge);
    void removeEdge(int fromId, int toId);
    void setEdgeBlocked(int fromId, int toId, bool blocked);

    // ---- Queries ----
    int  numLocations()  const { return static_cast<int>(locations_.size()); }
    int  numEdges()      const;
    bool hasLocation(int id) const;
    bool edgeExists(int fromId, int toId) const;
    bool edgeBlocked(int fromId, int toId) const;

    const Location& location(int id) const;
    const std::vector<Location>& allLocations() const { return locations_; }

    // Returns active (non-blocked) neighbours of node id
    std::vector<AdjEntry> neighbours(int id) const;
    // Returns all neighbours including blocked ones
    std::vector<AdjEntry> allNeighbours(int id) const;

    std::optional<Edge> getEdge(int fromId, int toId) const;

    // ---- Utility ----
    void clear();
    void printAdjacencyList() const;

    // Haversine distance between two location IDs (metres)
    double haversineDistance(int idA, int idB) const;
    // Haversine from lat/lon
    static double haversine(double lat1, double lon1,
                            double lat2, double lon2);

private:
    std::vector<Location>            locations_;
    std::unordered_map<int, int>     idToIndex_;     // location id → vector index
    // Adjacency list: adj_[locIndex] = list of (neighbourIndex, weight)
    std::vector<std::vector<AdjEntry>> adj_;
    // Edge store for metadata (blocked flag, etc.)
    std::vector<Edge>                edges_;

    int indexFor(int id) const;
};

} // namespace VITNav

#endif // GRAPH_H
