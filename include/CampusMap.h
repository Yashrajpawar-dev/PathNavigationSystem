#ifndef CampusMap_H
#define CampusMap_H

#include <string>
#include <unordered_map>
#include <vector>

struct Edge {
    std::string destination;
    int distance;
};

struct CampusLocation {
    std::string name;
    int x;
    int y;
};

struct PathResult {
    int distance;
    std::vector<std::string> path;
};

#endif 