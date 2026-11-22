#pragma once
// ============================================================
// PathManager.h – Campus Navigation System
// Manages walkway availability, blocking and restoration
// ============================================================

#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include "CampusGraph.h"

namespace VITNav {

class PathManager {
public:
    // Required data members per specification
    int sourceId{-1};
    int destinationId{-1};

    // Constructor
    PathManager() = default;

    // Required methods per specification
    bool blockWalkway(CampusGraph& graph, int fromId, int toId, bool verbose = true);
    bool restoreWalkway(CampusGraph& graph, int fromId, int toId, bool verbose = true);
    bool restoreAllWalkways(CampusGraph& graph, bool verbose = true);
    bool isWalkwayAvailable(const CampusGraph& graph, int fromId, int toId) const;
};

} // namespace VITNav

#endif // PATH_MANAGER_H
