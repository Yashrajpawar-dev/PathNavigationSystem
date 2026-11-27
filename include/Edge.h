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
    int    fromId;
    int    toId;
    double distanceM;       // Walking distance in metres
    PathType pathType;
    bool   blocked;         // If true, path is temporarily closed
    bool   verifiedDistance; // Whether distance is field-measured
    std::string description;

    Edge() = default;
    Edge(int from, int to, double dist,
         PathType type = PathType::PavedPath,
         bool blocked = false,
         bool verified = false,
         const std::string& desc = "");

    std::string pathTypeString() const;

    // Estimated walking time (minutes) at 5 km/h average walking speed
    double walkingTimeMin() const { return (distanceM / 1000.0) / 5.0 * 60.0; }
};

PathType pathTypeFromString(const std::string& s);

} // namespace VITNav

#endif // EDGE_H
