#include "PathManager.h"
#include "CampusData.h"
#include <iostream>

namespace VITNav {

bool PathManager::blockWalkway(CampusGraph& graph, int fromId, int toId, bool verbose) {
    sourceId = fromId;
    destinationId = toId;

    if (!graph.edgeExists(fromId, toId)) {
        if (verbose) std::cerr << "[PathManager] Error: Walkway between [" << fromId << "] and [" << toId << "] does not exist.\n";
        return false;
    }

    if (!graph.isWalkwayAvailable(fromId, toId)) {
        if (verbose) std::cout << "[PathManager] Notice: Walkway between [" << fromId << "] and [" << toId << "] is already blocked.\n";
        return true;
    }

    bool success = graph.blockWalkway(fromId, toId);
    if (success) {
        CampusData::saveBlockedWalkways(graph);
        if (verbose) {
            std::cout << "[PathManager] Success: Walkway between [" << fromId << "] and [" << toId << "] is now BLOCKED.\n";
        }
    }
    return success;
}

bool PathManager::restoreWalkway(CampusGraph& graph, int fromId, int toId, bool verbose) {
    sourceId = fromId;
    destinationId = toId;

    if (!graph.edgeExists(fromId, toId)) {
        if (verbose) std::cerr << "[PathManager] Error: Walkway between [" << fromId << "] and [" << toId << "] does not exist.\n";
        return false;
    }

    if (graph.isWalkwayAvailable(fromId, toId)) {
        if (verbose) std::cout << "[PathManager] Notice: Walkway between [" << fromId << "] and [" << toId << "] is already available.\n";
        return true;
    }

    bool success = graph.restoreWalkway(fromId, toId);
    if (success) {
        CampusData::saveBlockedWalkways(graph);
        if (verbose) {
            std::cout << "[PathManager] Success: Walkway between [" << fromId << "] and [" << toId << "] has been RESTORED.\n";
        }
    }
    return success;
}

bool PathManager::restoreAllWalkways(CampusGraph& graph, bool verbose) {
    auto blocked = graph.getBlockedEdges();
    for (const auto& edge : blocked) {
        graph.restoreWalkway(edge.sourceId, edge.destinationId);
    }
    CampusData::saveBlockedWalkways(graph);
    if (verbose) {
        std::cout << "[PathManager] Success: All " << blocked.size() << " blocked walkway(s) have been RESTORED.\n";
    }
    return true;
}

bool PathManager::isWalkwayAvailable(const CampusGraph& graph, int fromId, int toId) const {
    return graph.isWalkwayAvailable(fromId, toId);
}

} // namespace VITNav
