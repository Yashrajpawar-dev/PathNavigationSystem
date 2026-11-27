// ============================================================
// Edge.cpp – VIT Pune Campus Navigator
// ============================================================

#include "Edge.h"
#include <algorithm>
#include <cctype>

namespace VITNav {

Edge::Edge(int from, int to, double dist, PathType type,
           bool blocked, bool verified, const std::string& desc)
    : fromId(from), toId(to), distanceM(dist), pathType(type),
      blocked(blocked), verifiedDistance(verified), description(desc)
{}

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
