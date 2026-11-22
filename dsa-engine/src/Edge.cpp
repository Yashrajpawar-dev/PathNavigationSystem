// ============================================================
// Edge.cpp – VIT Pune Campus Navigator
// ============================================================

#include "Edge.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>

namespace VITNav {

Edge::Edge()
    : sourceId(0), destinationId(0), distance(0.0), walkingTime(0.0), isBlocked(false),
      fromId(0), toId(0), distanceM(0.0), pathType(PathType::PavedPath),
      blocked(false), verifiedDistance(false)
{}

Edge::Edge(int from, int to, double dist, PathType type,
           bool blocked, bool verified, const std::string& desc)
    : sourceId(from), destinationId(to), distance(dist),
      walkingTime((dist / 1000.0) / 5.0 * 60.0), isBlocked(blocked),
      fromId(from), toId(to), distanceM(dist), pathType(type),
      blocked(blocked), verifiedDistance(verified), description(desc)
{}

Edge::Edge(int destId, double dist, double time, bool blockedStatus,
           int srcId, PathType type, const std::string& desc)
    : sourceId(srcId), destinationId(destId), distance(dist),
      walkingTime(time < 0 ? (dist / 1000.0) / 5.0 * 60.0 : time),
      isBlocked(blockedStatus), fromId(srcId), toId(destId),
      distanceM(dist), pathType(type), blocked(blockedStatus),
      verifiedDistance(false), description(desc)
{}

void Edge::displayEdge() const {
    std::cout << "  --> To Node [ID: " << std::setw(2) << destinationId << "] | "
              << std::fixed << std::setprecision(1)
              << "Dist: " << std::setw(5) << distance << " m | "
              << "Time: " << std::setw(4) << walkingTime << " min | "
              << "Type: " << std::setw(12) << pathTypeString() << " | "
              << "Status: " << (isBlocked ? "[BLOCKED]" : "[Available]");
    if (!description.empty()) {
        std::cout << " (" << description << ")";
    }
    std::cout << "\n";
}

std::string Edge::pathTypeString() const {
    switch (pathType) {
        case PathType::PavedRoad:  return "Paved Road";
        case PathType::PavedPath:  return "Paved Path";
        case PathType::Corridor:   return "Indoor Corridor";
        case PathType::StairCase:  return "Staircase";
        default:                   return "Unknown";
    }
}

PathType pathTypeFromString(const std::string& s) {
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "paved_road")  return PathType::PavedRoad;
    if (lower == "paved_path")  return PathType::PavedPath;
    if (lower == "corridor")    return PathType::Corridor;
    if (lower == "staircase")   return PathType::StairCase;
    return PathType::Unknown;
}

} // namespace VITNav
