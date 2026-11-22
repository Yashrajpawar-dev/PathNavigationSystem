#pragma once
// ============================================================
// CampusNavigation.h – Campus Navigation System
// System orchestrator: connects Graph, Navigation, RouteManager,
// FacilityFinder, PathManager, interactive menu, and JSON API bridge.
// ============================================================

#ifndef CAMPUS_NAVIGATION_H
#define CAMPUS_NAVIGATION_H

#include "CampusGraph.h"
#include "Navigation.h"
#include "RouteManager.h"
#include "FacilityFinder.h"
#include "PathManager.h"
#include "CampusData.h"
#include <string>

namespace VITNav {

class CampusNavigation {
public:
    // Required data members per specification
    CampusGraph    graph;
    Navigation     navigation;
    RouteManager   routeManager;
    FacilityFinder facilityFinder;
    PathManager    pathManager;

    // Constructor
    CampusNavigation();

    // Required methods per specification
    void run();
    void displayMenu() const;
    void searchLocation();
    void findRoute();
    void findFacility();
    void manageWalkways();

    // Additional core capabilities
    void findAlternativeRoutesMenu();
    void compareAlgorithmsMenu();
    bool runAutomatedTests();

    // Backend CLI & JSON API integration mode (for Phase 2 React web frontend)
    int handleCli(int argc, char* argv[]);

    // Campus data initialization
    void initializeData(const std::string& dataDir = "");

private:
    void printHeader() const;
    int  getValidInt(const std::string& prompt, int minVal = -1, int maxVal = 999999) const;
    std::string getLine(const std::string& prompt) const;
    void pause() const;
};

} // namespace VITNav

#endif // CAMPUS_NAVIGATION_H
