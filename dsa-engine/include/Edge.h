#pragma once
// ============================================================
// Edge.h – VIT Pune Campus Navigator
// Represents a walking connection between two campus locations
// ============================================================

#ifndef EDGE_H
#define EDGE_H

#include <string>

namespace VITNav {

enum class PathType {
    PavedRoad,
    PavedPath,
    Corridor,
    StairCase,
    Unknown
};

class Edge {
public:
    // Required data members per course specification
    int    sourceId{0};
    int    destinationId{0};
    double distance{0.0};       // Walking distance in metres
    double walkingTime{0.0};    // Walking time in minutes
    bool   isBlocked{false};    // Blocked status

    // Original fields for backward compatibility
    int    fromId{0};
    int    toId{0};
    double distanceM{0.0};
    PathType pathType{PathType::PavedPath};
    bool   blocked{false};
    bool   verifiedDistance{false};
    std::string description;

    Edge();
    Edge(int from, int to, double dist,
         PathType type = PathType::PavedPath,
         bool blocked = false,
         bool verified = false,
         const std::string& desc = "");

    // Constructor matching course specification: destinationId, distance, walkingTime, isBlocked
    Edge(int destId, double dist, double time = -1.0, bool blockedStatus = false,
         int srcId = 0, PathType type = PathType::PavedPath, const std::string& desc = "");

    std::string pathTypeString() const;
    void displayEdge() const;

    // Estimated walking time (minutes) at 5 km/h average walking speed
    double walkingTimeMin() const { return walkingTime; }
};

PathType pathTypeFromString(const std::string& s);

} // namespace VITNav

#endif // EDGE_H
