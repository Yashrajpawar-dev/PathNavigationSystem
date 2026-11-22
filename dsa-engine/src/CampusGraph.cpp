#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include "CampusGraph.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cctype>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace VITNav {

static constexpr double EARTH_RADIUS_M = 6371000.0;
static constexpr double DEG_TO_RAD = M_PI / 180.0;

bool CampusGraph::addLocation(const Location& loc) {
    // Validate duplicate location ID
    if (locationMap.find(loc.id) != locationMap.end()) {
        std::cerr << "[CampusGraph] Warning: Location with ID " << loc.id << " already exists. Skipped.\n";
        return false;
    }
    int idx = static_cast<int>(locations.size());
    locationMap[loc.id] = idx;
    locations.push_back(loc);
    adj[loc.id] = {}; // initialize empty adjacency entry
    return true;
}

bool CampusGraph::addEdge(int fromId, int toId, double distance, double walkingTime,
                          const std::string& pathType, const std::string& description) {
    // 1. Validate endpoint existence
    if (locationMap.find(fromId) == locationMap.end()) {
        std::cerr << "[CampusGraph] Error: Source location ID " << fromId << " does not exist.\n";
        return false;
    }
    if (locationMap.find(toId) == locationMap.end()) {
        std::cerr << "[CampusGraph] Error: Destination location ID " << toId << " does not exist.\n";
        return false;
    }

    // 2. Reject self-loops
    if (fromId == toId) {
        std::cerr << "[CampusGraph] Error: Self-loops are not allowed (ID " << fromId << ").\n";
        return false;
    }

    // 3. Reject negative weights
    if (distance < 0.0) {
        std::cerr << "[CampusGraph] Error: Negative edge distance rejected (" << distance << " m).\n";
        return false;
    }
    if (walkingTime < 0.0) {
        // Automatically calculate walking time assuming average walking speed 5 km/h (approx 83.33 m/min)
        walkingTime = (distance / 1000.0) / 5.0 * 60.0;
    }

    // 4. Check for duplicate edges
    if (edgeExists(fromId, toId)) {
        std::cerr << "[CampusGraph] Warning: Walkway between " << fromId << " and " << toId << " already exists. Skipped.\n";
        return false;
    }

    PathType pType = pathTypeFromString(pathType);

    // 5. Store undirected edge consistently in both directions
    Edge forwardEdge(toId, distance, walkingTime, false, fromId, pType, description);
    Edge backwardEdge(fromId, distance, walkingTime, false, toId, pType, description);

    adj[fromId].push_back(forwardEdge);
    adj[toId].push_back(backwardEdge);

    return true;
}

void CampusGraph::displayLocations() const {
    std::cout << "\n" << std::string(75, '=') << "\n";
    std::cout << "                 CAMPUS LOCATIONS DIRECTORY (" << locations.size() << " Facilities)\n";
    std::cout << std::string(75, '=') << "\n";
    for (const auto& loc : locations) {
        loc.displayLocation();
    }
    std::cout << std::string(75, '=') << "\n";
}

void CampusGraph::displayConnections() const {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "                 CAMPUS WALKWAYS (ADJACENCY LIST)\n";
    std::cout << std::string(80, '=') << "\n";
    for (const auto& loc : locations) {
        std::cout << "\n[" << loc.id << "] " << loc.name << " (" << loc.categoryString() << "):\n";
        auto it = adj.find(loc.id);
        if (it == adj.end() || it->second.empty()) {
            std::cout << "  (No connected walkways)\n";
            continue;
        }
        for (const auto& edge : it->second) {
            edge.displayEdge();
        }
    }
    std::cout << std::string(80, '=') << "\n";
}

const Location* CampusGraph::findLocationByName(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    // Exact match first
    for (const auto& loc : locations) {
        std::string locName = loc.name;
        std::transform(locName.begin(), locName.end(), locName.begin(), ::tolower);
        if (locName == query) return &loc;
        std::string sName = loc.shortName;
        std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
        if (sName == query) return &loc;
    }

    // Substring match
    for (const auto& loc : locations) {
        std::string locName = loc.name;
        std::transform(locName.begin(), locName.end(), locName.begin(), ::tolower);
        if (locName.find(query) != std::string::npos) return &loc;
        std::string sName = loc.shortName;
        std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
        if (sName.find(query) != std::string::npos) return &loc;
    }

    return nullptr;
}

const Location* CampusGraph::getLocationById(int id) const {
    auto it = locationMap.find(id);
    if (it != locationMap.end() && it->second >= 0 && it->second < static_cast<int>(locations.size())) {
        return &locations[it->second];
    }
    return nullptr;
}

bool CampusGraph::blockWalkway(int fromId, int toId) {
    if (!edgeExists(fromId, toId)) return false;

    bool modified = false;
    auto itA = adj.find(fromId);
    if (itA != adj.end()) {
        for (auto& edge : itA->second) {
            if (edge.destinationId == toId) {
                edge.isBlocked = true;
                edge.blocked = true;
                modified = true;
            }
        }
    }

    auto itB = adj.find(toId);
    if (itB != adj.end()) {
        for (auto& edge : itB->second) {
            if (edge.destinationId == fromId) {
                edge.isBlocked = true;
                edge.blocked = true;
                modified = true;
            }
        }
    }

    return modified;
}

bool CampusGraph::restoreWalkway(int fromId, int toId) {
    if (!edgeExists(fromId, toId)) return false;

    bool modified = false;
    auto itA = adj.find(fromId);
    if (itA != adj.end()) {
        for (auto& edge : itA->second) {
            if (edge.destinationId == toId) {
                edge.isBlocked = false;
                edge.blocked = false;
                modified = true;
            }
        }
    }

    auto itB = adj.find(toId);
    if (itB != adj.end()) {
        for (auto& edge : itB->second) {
            if (edge.destinationId == fromId) {
                edge.isBlocked = false;
                edge.blocked = false;
                modified = true;
            }
        }
    }

    return modified;
}

bool CampusGraph::isWalkwayAvailable(int fromId, int toId) const {
    auto it = adj.find(fromId);
    if (it == adj.end()) return false;
    for (const auto& edge : it->second) {
        if (edge.destinationId == toId) {
            return !edge.isBlocked;
        }
    }
    return false;
}

bool CampusGraph::edgeExists(int fromId, int toId) const {
    auto it = adj.find(fromId);
    if (it == adj.end()) return false;
    for (const auto& edge : it->second) {
        if (edge.destinationId == toId) return true;
    }
    return false;
}

std::vector<Edge> CampusGraph::getNeighbors(int id, bool includeBlocked) const {
    std::vector<Edge> result;
    auto it = adj.find(id);
    if (it != adj.end()) {
        for (const auto& edge : it->second) {
            if (includeBlocked || !edge.isBlocked) {
                result.push_back(edge);
            }
        }
    }
    return result;
}

std::optional<Edge> CampusGraph::getEdge(int fromId, int toId) const {
    auto it = adj.find(fromId);
    if (it != adj.end()) {
        for (const auto& edge : it->second) {
            if (edge.destinationId == toId) return edge;
        }
    }
    return std::nullopt;
}

int CampusGraph::numEdges() const {
    int total = 0;
    for (const auto& [id, edges] : adj) {
        total += static_cast<int>(edges.size());
    }
    return total / 2; // Each undirected edge is stored twice
}

void CampusGraph::clear() {
    locations.clear();
    locationMap.clear();
    adj.clear();
}

double CampusGraph::haversineDistance(int idA, int idB) const {
    const Location* a = getLocationById(idA);
    const Location* b = getLocationById(idB);
    if (!a || !b) return 0.0;
    return haversine(a->lat, a->lon, b->lat, b->lon);
}

double CampusGraph::haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * DEG_TO_RAD) * std::cos(lat2 * DEG_TO_RAD) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    return EARTH_RADIUS_M * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

std::vector<Edge> CampusGraph::getBlockedEdges() const {
    std::vector<Edge> result;
    for (const auto& [fromId, edgeList] : adj) {
        for (const auto& edge : edgeList) {
            if (fromId < edge.destinationId && edge.isBlocked) {
                result.push_back(edge);
            }
        }
    }
    return result;
}

std::vector<Edge> CampusGraph::getActiveEdges() const {
    std::vector<Edge> result;
    for (const auto& [fromId, edgeList] : adj) {
        for (const auto& edge : edgeList) {
            if (fromId < edge.destinationId && !edge.isBlocked) {
                result.push_back(edge);
            }
        }
    }
    return result;
}

void CampusGraph::displayBlockedWalkways() const {
    auto blocked = getBlockedEdges();
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "                 CURRENTLY BLOCKED WALKWAYS (" << blocked.size() << ")\n";
    std::cout << std::string(80, '=') << "\n";
    if (blocked.empty()) {
        std::cout << "  (All campus walkways are currently active and open)\n";
    } else {
        for (const auto& edge : blocked) {
            const Location* u = getLocationById(edge.sourceId);
            const Location* v = getLocationById(edge.destinationId);
            std::cout << "  [" << edge.sourceId << "] " << (u ? u->shortName : "Loc")
                      << " <---> [" << edge.destinationId << "] " << (v ? v->shortName : "Loc")
                      << " | " << edge.distance << " m | " << edge.pathTypeString()
                      << " | [STATUS: BLOCKED]\n";
        }
    }
    std::cout << std::string(80, '=') << "\n";
}

void CampusGraph::displayActiveWalkways() const {
    auto active = getActiveEdges();
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "                 ACTIVE CAMPUS WALKWAYS (" << active.size() << ")\n";
    std::cout << std::string(80, '=') << "\n";
    for (const auto& edge : active) {
        const Location* u = getLocationById(edge.sourceId);
        const Location* v = getLocationById(edge.destinationId);
        std::cout << "  [" << edge.sourceId << "] " << (u ? u->shortName : "Loc")
                  << " <---> [" << edge.destinationId << "] " << (v ? v->shortName : "Loc")
                  << " | " << edge.distance << " m | " << edge.walkingTime << " min"
                  << " | " << edge.pathTypeString() << "\n";
    }
    std::cout << std::string(80, '=') << "\n";
}

int CampusGraph::indexFor(int id) const {
    auto it = locationMap.find(id);
    return (it != locationMap.end()) ? it->second : -1;
}

} // namespace VITNav
