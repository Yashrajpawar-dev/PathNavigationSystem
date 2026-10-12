#ifndef EDGE_H
#define EDGE_H

#include <string>

using namespace std;

class Edge {
private:
    int destinationId;
    float distance;
    float walkingTime;
    bool isBlocked;
    string name;
    string category;

public:
    Edge();

    void addLocation();
    void addEdge();
    void displayLocations();
    void displayConnections();
    void findLocationByName();
    int getLocationById();
  
};

#endif
