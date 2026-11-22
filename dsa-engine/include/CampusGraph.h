#pragma once
// ============================================================
// CampusGraph.h – Campus Navigation System
// Manages the weighted campus graph using an adjacency list
// ============================================================

#ifndef CAMPUS_GRAPH_H
#define CAMPUS_GRAPH_H

#include "Location.h"
#include "Edge.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

namespace VITNav {

class CampusGraph {
public:
    // Required data members per specification
    std::vector<Location>                   locations;
    std::unordered_map<int, std::vector<Edge>> adj;
    std::unordered_map<int, int>            locationMap; // ID -> index in locations vector

    // Constructor
    CampusGraph() = default;

    // Required methods per specification
    bool addLocation(const Location& loc);
    bool addEdge(int fromId, int toId, double distance, double walkingTime = -1.0,
                 const std::string& pathType = "paved_path", const std::string& description = "");
    void displayLocations() const;
    void displayConnections() const;
    const Location* findLocationByName(const std::string& name) const;
    const Location* getLocationById(int id) const;

    // Walkway availability management (for PathManager)
    bool blockWalkway(int fromId, int toId);
    bool restoreWalkway(int fromId, int toId);
    bool isWalkwayAvailable(int fromId, int toId) const;
    bool edgeExists(int fromId, int toId) const;
    void displayBlockedWalkways() const;
    void displayActiveWalkways() const;
    std::vector<Edge> getBlockedEdges() const;
    std::vector<Edge> getActiveEdges() const;

    // Edge queries
    std::vector<Edge> getNeighbors(int id, bool includeBlocked = false) const;
    std::optional<Edge> getEdge(int fromId, int toId) const;

    // Graph metrics and lifecycle
    int  numLocations() const { return static_cast<int>(locations.size()); }
    int  numEdges()     const;
    bool hasLocation(int id) const { return locationMap.find(id) != locationMap.end(); }
    const std::vector<Location>& allLocations() const { return locations; }
    void clear();

    // Haversine geographic distance calculation (metres)
    double haversineDistance(int idA, int idB) const;
    static double haversine(double lat1, double lon1, double lat2, double lon2);

private:
    int indexFor(int id) const;
};

} // namespace VITNav

#endif // CAMPUS_GRAPH_H
