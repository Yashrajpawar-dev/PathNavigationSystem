#ifndef CampusMap_H
#define CampusMap_H

#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct Edge {
    string destination;
    int distance;
};

struct CampusLocation {
    string name;
    int x;
    int y;
};

struct PathResult {
    int distance;
    vector<std::string> path;
};

#endif 