// ============================================================
// NavigationController.cpp – VIT Pune Campus Navigator
// Loads JSON data, owns Graph, dispatches algorithm calls
// ============================================================

#include "NavigationController.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace VITNav {

// ---- Minimal JSON parser helpers (no external library) ----
// Finds the value string for a given key in a simple JSON object string.
static std::string jsonValue(const std::string& obj, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = obj.find(searchKey);
    if (pos == std::string::npos) return "";
    pos = obj.find(':', pos + searchKey.size());
    if (pos == std::string::npos) return "";
    ++pos;
    while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t' ||
                                 obj[pos] == '\n' || obj[pos] == '\r')) ++pos;
    if (pos >= obj.size()) return "";
    if (obj[pos] == '"') {
        // String value
        ++pos;
        std::string val;
        while (pos < obj.size() && obj[pos] != '"') {
            if (obj[pos] == '\\') { ++pos; }
            val += obj[pos++];
        }
        return val;
    } else {
        // Number or boolean
        std::string val;
        while (pos < obj.size() && obj[pos] != ',' && obj[pos] != '}' &&
               obj[pos] != '\n' && obj[pos] != '\r') {
            val += obj[pos++];
        }
        // trim whitespace
        while (!val.empty() && std::isspace((unsigned char)val.back())) val.pop_back();
        return val;
    }
}

// Split a JSON array string into individual object strings
static std::vector<std::string> splitJsonArray(const std::string& json,
                                               const std::string& arrayKey) {
    std::vector<std::string> items;
    std::string searchKey = "\"" + arrayKey + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return items;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return items;
    ++pos;

    int depth = 0;
    std::string cur;
    for (; pos < json.size(); ++pos) {
        char c = json[pos];
        if (c == '{') { depth++; cur += c; }
        else if (c == '}') {
            cur += c; --depth;
            if (depth == 0) { items.push_back(cur); cur.clear(); }
        } else if (c == ']' && depth == 0) {
            break;
        } else {
            cur += c;
        }
    }
    return items;
}

// ---- NavigationController implementation ----

NavigationController::NavigationController() = default;

std::string NavigationController::readFile(const std::string& path) const {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool NavigationController::loadAll(const std::string& dataDir) {
    std::string sep = dataDir.back() == '/' || dataDir.back() == '\\' ? "" : "/";
    bool ok = loadLocations(dataDir + sep + "locations.json");
    if (!ok) return false;
    ok = loadConnections(dataDir + sep + "connections.json");
    loaded_ = ok;
    return ok;
}

bool NavigationController::loadLocations(const std::string& jsonPath) {
    std::string content = readFile(jsonPath);
    if (content.empty()) {
        lastError_ = "Cannot open: " + jsonPath;
        return false;
    }
    return parseLocationsJson(content);
}

bool NavigationController::loadConnections(const std::string& jsonPath) {
    std::string content = readFile(jsonPath);
    if (content.empty()) {
        lastError_ = "Cannot open: " + jsonPath;
        return false;
    }
    return parseConnectionsJson(content);
}

bool NavigationController::parseLocationsJson(const std::string& content) {
    auto items = splitJsonArray(content, "locations");
    if (items.empty()) {
        lastError_ = "No locations array found in JSON";
        return false;
    }
    for (const auto& item : items) {
        Location loc;
        try {
            loc.id          = std::stoi(jsonValue(item, "id"));
            loc.name        = jsonValue(item, "name");
            loc.shortName   = jsonValue(item, "short_name");
            loc.category    = categoryFromString(jsonValue(item, "category"));
            loc.lat         = std::stod(jsonValue(item, "lat"));
            loc.lon         = std::stod(jsonValue(item, "lon"));
            loc.description = jsonValue(item, "description");
            loc.source      = jsonValue(item, "source");
            std::string acc = jsonValue(item, "accuracy");
            if (acc == "VERIFIED")    loc.accuracy = CoordinateAccuracy::VERIFIED;
            else if (acc == "PLACEHOLDER") loc.accuracy = CoordinateAccuracy::PLACEHOLDER;
            else                      loc.accuracy = CoordinateAccuracy::APPROXIMATE;
        } catch (...) {
            lastError_ = "Failed to parse a location entry";
            continue;
        }
        graph_.addLocation(loc);
    }
    return true;
}

bool NavigationController::parseConnectionsJson(const std::string& content) {
    auto items = splitJsonArray(content, "connections");
    if (items.empty()) {
        lastError_ = "No connections array found in JSON";
        return false;
    }
    for (const auto& item : items) {
        Edge edge;
        try {
            edge.fromId           = std::stoi(jsonValue(item, "from_id"));
            edge.toId             = std::stoi(jsonValue(item, "to_id"));
            edge.distanceM        = std::stod(jsonValue(item, "distance_m"));
            edge.pathType         = pathTypeFromString(jsonValue(item, "path_type"));
            std::string blk       = jsonValue(item, "blocked");
            edge.blocked          = (blk == "true");
            std::string ver       = jsonValue(item, "verified_distance");
            edge.verifiedDistance = (ver == "true");
            edge.description      = jsonValue(item, "description");
        } catch (...) {
            lastError_ = "Failed to parse a connection entry";
            continue;
        }
        graph_.addEdge(edge);
    }
    return true;
}

PathResult NavigationController::findRoute(int sourceId, int destId, Algorithm alg) {
    switch (alg) {
        case Algorithm::BFS:          return bfs(graph_, sourceId, destId);
        case Algorithm::DFS:          return dfs(graph_, sourceId, destId);
        case Algorithm::Dijkstra:     return dijkstra(graph_, sourceId, destId);
        case Algorithm::AStar:        return aStar(graph_, sourceId, destId);
        case Algorithm::FloydWarshall: {
            // Run Floyd-Warshall and extract path for the requested pair
            const FloydResult& fr = getFloydResult();
            PathResult pr;
            pr.algorithmName = "Floyd-Warshall";
            pr.sourceId = sourceId;
            pr.destId   = destId;
            if (!fr.computed) return pr;
            const auto& locs = graph_.allLocations();
            int srcIdx = -1, dstIdx = -1;
            for (int i = 0; i < (int)locs.size(); ++i) {
                if (locs[i].id == sourceId) srcIdx = i;
                if (locs[i].id == destId)   dstIdx = i;
            }
            if (srcIdx < 0 || dstIdx < 0) return pr;
            if (fr.dist[srcIdx][dstIdx] == std::numeric_limits<double>::infinity()) {
                return pr;
            }
            pr.found = true;
            auto pathIdx = fr.getPath(srcIdx, dstIdx);
            for (int idx : pathIdx) pr.path.push_back(locs[idx].id);
            pr.totalDistance = fr.dist[srcIdx][dstIdx];
            pr.totalTimeMin  = (pr.totalDistance / 1000.0) / 5.0 * 60.0;
            pr.executionTimeMs = fr.executionTimeMs;
            return pr;
        }
        default: return dijkstra(graph_, sourceId, destId);
    }
}

const FloydResult& NavigationController::getFloydResult() {
    if (!floydResult_.computed) {
        floydResult_ = floydWarshall(graph_);
    }
    return floydResult_;
}

std::vector<const Location*> NavigationController::searchLocations(
    const std::string& query) const {
    std::string q = query;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    std::vector<const Location*> results;
    for (const auto& loc : graph_.allLocations()) {
        std::string name = loc.name;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::string sname = loc.shortName;
        std::transform(sname.begin(), sname.end(), sname.begin(), ::tolower);
        if (name.find(q) != std::string::npos ||
            sname.find(q) != std::string::npos ||
            loc.categoryString().find(q) != std::string::npos) {
            results.push_back(&loc);
        }
    }
    return results;
}

void NavigationController::blockEdge(int fromId, int toId) {
    graph_.setEdgeBlocked(fromId, toId, true);
    blockedEdges_.insert({std::min(fromId, toId), std::max(fromId, toId)});
    floydResult_.computed = false; // invalidate
}

void NavigationController::unblockEdge(int fromId, int toId) {
    graph_.setEdgeBlocked(fromId, toId, false);
    auto key = std::make_pair(std::min(fromId, toId), std::max(fromId, toId));
    blockedEdges_.erase(key);
    floydResult_.computed = false;
}

bool NavigationController::isEdgeBlocked(int fromId, int toId) const {
    return graph_.edgeBlocked(fromId, toId);
}

void NavigationController::clearAllBlocks() {
    for (const auto& [a, b] : blockedEdges_) {
        graph_.setEdgeBlocked(a, b, false);
    }
    blockedEdges_.clear();
    floydResult_.computed = false;
}

NavigationController::ComparisonResult
NavigationController::compareAlgorithms(int sourceId, int destId) {
    ComparisonResult cr;
    cr.bfsResult      = bfs(graph_, sourceId, destId);
    cr.dfsResult      = dfs(graph_, sourceId, destId);
    cr.dijkstraResult = dijkstra(graph_, sourceId, destId);
    cr.aStarResult    = aStar(graph_, sourceId, destId);
    return cr;
}

std::string algorithmName(Algorithm alg) {
    switch (alg) {
        case Algorithm::BFS:          return "BFS";
        case Algorithm::DFS:          return "DFS";
        case Algorithm::Dijkstra:     return "Dijkstra";
        case Algorithm::AStar:        return "A*";
        case Algorithm::FloydWarshall:return "Floyd-Warshall";
        default:                      return "Unknown";
    }
}

} // namespace VITNav
