// ============================================================
// Graph.cpp – VIT Pune Campus Navigator
// Weighted undirected graph – adjacency list implementation
// ============================================================

#include "Graph.h"
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace VITNav {

static constexpr double EARTH_RADIUS_M = 6371000.0;
static constexpr double DEG_TO_RAD = M_PI / 180.0;

Graph::Graph(int numNodes) {
    adj_.resize(numNodes);
}

void Graph::addLocation(const Location& loc) {
    if (idToIndex_.count(loc.id)) return; // already added
    int idx = static_cast<int>(locations_.size());
    idToIndex_[loc.id] = idx;
    locations_.push_back(loc);
    adj_.push_back({});
}

void Graph::addEdge(const Edge& edge) {
    edges_.push_back(edge);
    if (!edge.blocked) {
        int ia = indexFor(edge.fromId);
        int ib = indexFor(edge.toId);
        if (ia < 0 || ib < 0) return;
        adj_[ia].push_back({edge.toId,   edge.distanceM});
        adj_[ib].push_back({edge.fromId, edge.distanceM});
    }
}

void Graph::removeEdge(int fromId, int toId) {
    // Remove from adj list
    int ia = indexFor(fromId);
    int ib = indexFor(toId);
    if (ia >= 0) {
        auto& a = adj_[ia];
        a.erase(std::remove_if(a.begin(), a.end(),
            [toId](const AdjEntry& e){ return e.first == toId; }), a.end());
    }
    if (ib >= 0) {
        auto& b = adj_[ib];
        b.erase(std::remove_if(b.begin(), b.end(),
            [fromId](const AdjEntry& e){ return e.first == fromId; }), b.end());
    }
    // Remove from edge store
    edges_.erase(std::remove_if(edges_.begin(), edges_.end(),
        [fromId, toId](const Edge& e){
            return (e.fromId == fromId && e.toId == toId) ||
                   (e.fromId == toId   && e.toId == fromId);
        }), edges_.end());
}

void Graph::setEdgeBlocked(int fromId, int toId, bool blocked) {
    // Update edge metadata
    for (auto& e : edges_) {
        if ((e.fromId == fromId && e.toId == toId) ||
            (e.fromId == toId   && e.toId == fromId)) {
            e.blocked = blocked;
        }
    }
    if (blocked) {
        // Remove from adjacency list
        int ia = indexFor(fromId), ib = indexFor(toId);
        if (ia >= 0) {
            auto& a = adj_[ia];
            a.erase(std::remove_if(a.begin(), a.end(),
                [toId](const AdjEntry& e){ return e.first == toId; }), a.end());
        }
        if (ib >= 0) {
            auto& b = adj_[ib];
            b.erase(std::remove_if(b.begin(), b.end(),
                [fromId](const AdjEntry& e){ return e.first == fromId; }), b.end());
        }
    } else {
        // Re-add to adjacency list
        for (const auto& e : edges_) {
            if ((e.fromId == fromId && e.toId == toId) ||
                (e.fromId == toId   && e.toId == fromId)) {
                int ia = indexFor(e.fromId), ib = indexFor(e.toId);
                if (ia >= 0 && ib >= 0) {
                    adj_[ia].push_back({e.toId,   e.distanceM});
                    adj_[ib].push_back({e.fromId, e.distanceM});
                }
            }
        }
    }
}

int Graph::numEdges() const { return static_cast<int>(edges_.size()); }

bool Graph::hasLocation(int id) const { return idToIndex_.count(id) > 0; }

bool Graph::edgeExists(int fromId, int toId) const {
    for (const auto& e : edges_) {
        if ((e.fromId == fromId && e.toId == toId) ||
            (e.fromId == toId   && e.toId == fromId)) return true;
    }
    return false;
}

bool Graph::edgeBlocked(int fromId, int toId) const {
    for (const auto& e : edges_) {
        if ((e.fromId == fromId && e.toId == toId) ||
            (e.fromId == toId   && e.toId == fromId)) return e.blocked;
    }
    return false;
}

const Location& Graph::location(int id) const {
    int idx = indexFor(id);
    if (idx < 0) throw std::out_of_range("Location id not found: " + std::to_string(id));
    return locations_[idx];
}

std::vector<AdjEntry> Graph::neighbours(int id) const {
    int idx = indexFor(id);
    if (idx < 0) return {};
    return adj_[idx];
}

std::vector<AdjEntry> Graph::allNeighbours(int id) const {
    // Returns neighbours from edge store (includes blocked)
    std::vector<AdjEntry> result;
    for (const auto& e : edges_) {
        if (e.fromId == id) result.push_back({e.toId, e.distanceM});
        if (e.toId == id)   result.push_back({e.fromId, e.distanceM});
    }
    return result;
}

std::optional<Edge> Graph::getEdge(int fromId, int toId) const {
    for (const auto& e : edges_) {
        if ((e.fromId == fromId && e.toId == toId) ||
            (e.fromId == toId   && e.toId == fromId)) return e;
    }
    return std::nullopt;
}

void Graph::clear() {
    locations_.clear();
    idToIndex_.clear();
    adj_.clear();
    edges_.clear();
}

void Graph::printAdjacencyList() const {
    std::cout << "\n=== VIT Campus Graph – Adjacency List ===\n";
    for (const auto& loc : locations_) {
        std::cout << "[" << loc.id << "] " << loc.name << " → ";
        int idx = indexFor(loc.id);
        for (const auto& [nbId, dist] : adj_[idx]) {
            std::cout << locations_[indexFor(nbId)].shortName
                      << "(" << dist << "m) ";
        }
        std::cout << "\n";
    }
}

double Graph::haversineDistance(int idA, int idB) const {
    const auto& a = location(idA);
    const auto& b = location(idB);
    return haversine(a.lat, a.lon, b.lat, b.lon);
}

double Graph::haversine(double lat1, double lon1,
                        double lat2, double lon2) {
    double dLat = (lat2 - lat1) * DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * DEG_TO_RAD) * std::cos(lat2 * DEG_TO_RAD) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    return EARTH_RADIUS_M * 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
}

int Graph::indexFor(int id) const {
    auto it = idToIndex_.find(id);
    return (it != idToIndex_.end()) ? it->second : -1;
}

} // namespace VITNav
