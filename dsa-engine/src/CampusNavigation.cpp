#include "CampusNavigation.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>

namespace VITNav {

CampusNavigation::CampusNavigation() {
    initializeData();
}

void CampusNavigation::initializeData(const std::string& dataDir) {
    bool loaded = false;
    if (!dataDir.empty()) {
        loaded = CampusData::loadFromJsonFiles(graph, dataDir);
    }
    if (!loaded) {
        // Try default relative data directories
        loaded = CampusData::loadFromJsonFiles(graph, "data") ||
                 CampusData::loadFromJsonFiles(graph, "../data") ||
                 CampusData::loadFromJsonFiles(graph, "DSA project/DSA project/VIT-Campus-Navigator/data");
    }
    if (!loaded) {
        // Fallback to embedded centralized realistic sample data
        CampusData::loadCentralizedSampleData(graph);
    }
    // Load persisted dynamic walkway blocks
    CampusData::loadBlockedWalkways(graph, dataDir);
}

void CampusNavigation::printHeader() const {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "               CAMPUS NAVIGATION SYSTEM — PHASE 1\n";
    std::cout << "         Data Structures & Algorithms Course Project\n";
    std::cout << "      Campus: Vishwakarma Institute of Technology (VIT Pune)\n";
    std::cout << std::string(70, '=') << "\n";
}

void CampusNavigation::displayMenu() const {
    printHeader();
    std::cout << " 1. Display All Campus Locations (" << graph.numLocations() << " Facilities)\n";
    std::cout << " 2. Display Campus Walkways (Adjacency List Network)\n";
    std::cout << " 3. Search Campus Location by Name or Keyword\n";
    std::cout << " 4. Navigate Route: Shortest Distance (Dijkstra Distance Mode)\n";
    std::cout << " 5. Navigate Route: Fastest Estimated Time (Dijkstra Time Mode)\n";
    std::cout << " 6. Navigate Route: Fewest Walkway Segments (BFS Mode)\n";
    std::cout << " 7. Explore Campus Reachability (DFS Traversal)\n";
    std::cout << " 8. Find Nearest Facility (Library, Canteen, Medical, Labs...)\n";
    std::cout << " 9. Manage Walkways (Simulate Blocked / Restored Paths)\n";
    std::cout << "10. Find Alternative Routes\n";
    std::cout << "11. Compare All Routing Algorithms (Dashboard)\n";
    std::cout << "12. Run Comprehensive Automated Test Suite (13 Test Cases)\n";
    std::cout << "13. Display Currently Blocked Walkways\n";
    std::cout << "14. Display Currently Active Walkways\n";
    std::cout << "15. Add Campus Location (Graph Management)\n";
    std::cout << "16. Add Campus Walkway (Graph Management)\n";
    std::cout << "17. Restore All Walkways to Active Status\n";
    std::cout << " 0. Exit Application\n";
    std::cout << std::string(70, '-') << "\n";
}

int CampusNavigation::getValidInt(const std::string& prompt, int minVal, int maxVal) const {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) {
            if (val >= minVal && val <= maxVal) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return val;
            }
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter an integer between " << minVal << " and " << maxVal << ".\n";
    }
}

std::string CampusNavigation::getLine(const std::string& prompt) const {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void CampusNavigation::pause() const {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void CampusNavigation::searchLocation() {
    std::cout << "\n--- Search Campus Locations ---\n";
    std::string query = getLine("Enter location name or keyword: ");
    if (query.empty()) return;

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<const Location*> matches;
    for (const auto& loc : graph.allLocations()) {
        std::string n = loc.name;
        std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        std::string sn = loc.shortName;
        std::transform(sn.begin(), sn.end(), sn.begin(), ::tolower);
        std::string c = loc.categoryString();
        std::transform(c.begin(), c.end(), c.begin(), ::tolower);

        if (n.find(qLower) != std::string::npos ||
            sn.find(qLower) != std::string::npos ||
            c.find(qLower) != std::string::npos) {
            matches.push_back(&loc);
        }
    }

    if (matches.empty()) {
        std::cout << "No matching campus locations found for \"" << query << "\".\n";
    } else {
        std::cout << "\nFound " << matches.size() << " matching facility/location(s):\n";
        for (const auto* loc : matches) {
            loc->displayLocation();
        }
    }
}

void CampusNavigation::findRoute() {
    std::cout << "\n--- Plan a Route ---\n";
    int src = getValidInt("Enter Source Location ID: ", 0, 9999);
    int dst = getValidInt("Enter Destination Location ID: ", 0, 9999);

    if (!graph.hasLocation(src)) {
        std::cout << "Error: Source ID " << src << " does not exist in the campus graph.\n";
        return;
    }
    if (!graph.hasLocation(dst)) {
        std::cout << "Error: Destination ID " << dst << " does not exist in the campus graph.\n";
        return;
    }

    std::cout << "\nSelect Optimization Mode:\n";
    std::cout << " 1. Shortest Walking Distance (Dijkstra - metres)\n";
    std::cout << " 2. Fastest Estimated Walking Time (Dijkstra - minutes)\n";
    std::cout << " 3. Fewest Walkway Segments / Intermediate Stops (BFS)\n";
    int modeChoice = getValidInt("Choice (1-3): ", 1, 3);

    PathResult route;
    if (modeChoice == 1) {
        route = navigation.dijkstra(graph, src, dst, OptimizationMode::DISTANCE);
    } else if (modeChoice == 2) {
        route = navigation.dijkstra(graph, src, dst, OptimizationMode::TIME);
    } else {
        route = navigation.bfs(graph, src, dst);
    }

    routeManager.displayRoute(graph, route);
}

void CampusNavigation::findFacility() {
    std::cout << "\n--- Nearest Campus Facility Finder ---\n";
    int src = getValidInt("Enter your current starting Location ID: ", 0, 9999);
    if (!graph.hasLocation(src)) {
        std::cout << "Error: Location ID " << src << " does not exist.\n";
        return;
    }

    std::cout << "\nCommon Facility Categories:\n";
    std::cout << "  Canteen, Library, Medical, Department, Laboratory, Sports, Parking, Hostel, Administration\n";
    std::string cat = getLine("Enter category name to search: ");
    if (cat.empty()) return;

    std::cout << "\nOptimization Metric:\n";
    std::cout << " 1. Shortest Distance\n";
    std::cout << " 2. Fastest Walking Time\n";
    int modeChoice = getValidInt("Choice (1-2): ", 1, 2);
    OptimizationMode mode = (modeChoice == 1) ? OptimizationMode::DISTANCE : OptimizationMode::TIME;

    PathResult route = facilityFinder.findNearestFacility(graph, navigation, src, cat, mode);
    if (!route.found) {
        std::cout << "\nNo reachable facilities found for category \"" << cat << "\".\n";
    } else {
        std::cout << "\nNearest facility identified: [" << facilityFinder.nearestFacilityId << "]\n";
        routeManager.displayRoute(graph, route);
    }
}

void CampusNavigation::manageWalkways() {
    std::cout << "\n--- Dynamic Walkway Management ---\n";
    std::cout << " 1. Block a Walkway (Simulate construction / closure)\n";
    std::cout << " 2. Restore a Walkway (Reopen blocked path)\n";
    std::cout << " 3. Check Walkway Availability Status\n";
    int choice = getValidInt("Choice (1-3): ", 1, 3);

    int u = getValidInt("Enter First Location ID: ", 0, 9999);
    int v = getValidInt("Enter Second Location ID: ", 0, 9999);

    if (choice == 1) {
        pathManager.blockWalkway(graph, u, v);
    } else if (choice == 2) {
        pathManager.restoreWalkway(graph, u, v);
    } else if (choice == 3) {
        if (!graph.edgeExists(u, v)) {
            std::cout << "Walkway between [" << u << "] and [" << v << "] does not exist.\n";
        } else {
            bool avail = pathManager.isWalkwayAvailable(graph, u, v);
            std::cout << "Walkway between [" << u << "] and [" << v << "] is currently: "
                      << (avail ? "AVAILABLE" : "BLOCKED") << "\n";
        }
    }
}

void CampusNavigation::findAlternativeRoutesMenu() {
    std::cout << "\n--- Find Alternative Routes ---\n";
    int src = getValidInt("Enter Source Location ID: ", 0, 9999);
    int dst = getValidInt("Enter Destination Location ID: ", 0, 9999);

    if (!graph.hasLocation(src) || !graph.hasLocation(dst)) {
        std::cout << "Invalid location ID specified.\n";
        return;
    }

    std::cout << "\nOptimization Metric:\n";
    std::cout << " 1. Shortest Distance\n";
    std::cout << " 2. Fastest Time\n";
    int m = getValidInt("Choice (1-2): ", 1, 2);
    OptimizationMode mode = (m == 1) ? OptimizationMode::DISTANCE : OptimizationMode::TIME;

    // Display primary route first
    PathResult primary = navigation.dijkstra(graph, src, dst, mode);
    std::cout << "\n>>> Primary Optimal Route <<<\n";
    routeManager.displayRoute(graph, primary);

    // Find alternatives
    auto alts = routeManager.findAlternativeRoutes(graph, navigation, src, dst, mode, 3);
    if (alts.empty()) {
        std::cout << "\nNo distinct alternative routes found for this path.\n";
    } else {
        std::cout << "\nFound " << alts.size() << " distinct alternative route candidate(s):\n";
        for (size_t i = 0; i < alts.size(); ++i) {
            std::cout << "\n>>> Alternative Route #" << (i + 1) << " <<<\n";
            routeManager.displayRoute(graph, alts[i]);
        }
    }
}

void CampusNavigation::compareAlgorithmsMenu() {
    std::cout << "\n--- Multi-Algorithm Performance Comparison ---\n";
    int src = getValidInt("Enter Source Location ID: ", 0, 9999);
    int dst = getValidInt("Enter Destination Location ID: ", 0, 9999);

    if (!graph.hasLocation(src) || !graph.hasLocation(dst)) {
        std::cout << "Invalid location ID specified.\n";
        return;
    }

    routeManager.compareRoutes(graph, navigation, src, dst);
}

bool CampusNavigation::runAutomatedTests() {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "           CAMPUS NAVIGATION SYSTEM — COMPREHENSIVE TEST SUITE\n";
    std::cout << "         Verifying 13 Mandatory Test Cases from Project Specification\n";
    std::cout << std::string(80, '=') << "\n";

    int testsPassed = 0;
    const int totalTests = 13;

    // Test 1: Display all locations
    std::cout << "[Test 1/13] Display all locations: ";
    if (graph.numLocations() >= 10) {
        std::cout << "PASSED (" << graph.numLocations() << " locations loaded)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED (Insufficient locations: " << graph.numLocations() << ")\n";
    }

    // Test 2: Display graph connections
    std::cout << "[Test 2/13] Display graph connections: ";
    if (graph.numEdges() >= 15) {
        std::cout << "PASSED (" << graph.numEdges() << " undirected walkways present)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED (Insufficient edges: " << graph.numEdges() << ")\n";
    }

    // Test 3: Search an existing location
    std::cout << "[Test 3/13] Search existing location ('Library'): ";
    const Location* locLib = graph.findLocationByName("Library");
    if (locLib != nullptr && locLib->id == 8) {
        std::cout << "PASSED (Found ID " << locLib->id << ": " << locLib->name << ")\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 4: Search a nonexistent location
    std::cout << "[Test 4/13] Search nonexistent location ('Spaceport 99'): ";
    const Location* locNone = graph.findLocationByName("Spaceport 99");
    if (locNone == nullptr) {
        std::cout << "PASSED (Returned null safely)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 5: BFS traversal (Guarantees fewest segments)
    std::cout << "[Test 5/13] BFS traversal (Main Gate [0] -> Library [8]): ";
    PathResult bfsRes = navigation.bfs(graph, 0, 8);
    if (bfsRes.found && !bfsRes.path.empty() && bfsRes.path.front() == 0 && bfsRes.path.back() == 8) {
        std::cout << "PASSED (Path found with " << (bfsRes.path.size() - 1) << " segments, "
                  << bfsRes.totalDistance << " m)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 6: DFS traversal
    std::cout << "[Test 6/13] DFS traversal (Main Gate [0] -> Library [8]): ";
    PathResult dfsRes = navigation.dfs(graph, 0, 8);
    if (dfsRes.found && !dfsRes.path.empty() && dfsRes.path.front() == 0 && dfsRes.path.back() == 8) {
        std::cout << "PASSED (Explored order nodes: " << dfsRes.nodesVisited << ")\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 7: Dijkstra shortest-distance route
    std::cout << "[Test 7/13] Dijkstra shortest-distance route (0 -> 8): ";
    PathResult dijDist = navigation.dijkstra(graph, 0, 8, OptimizationMode::DISTANCE);
    if (dijDist.found && dijDist.totalDistance > 0.0) {
        std::cout << "PASSED (Optimal distance: " << dijDist.totalDistance << " m, "
                  << dijDist.totalTimeMin << " min)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 8: Dijkstra fastest-time route
    std::cout << "[Test 8/13] Dijkstra fastest-time route (0 -> 8): ";
    PathResult dijTime = navigation.dijkstra(graph, 0, 8, OptimizationMode::TIME);
    if (dijTime.found && dijTime.totalTimeMin > 0.0) {
        std::cout << "PASSED (Optimal time: " << dijTime.totalTimeMin << " min, "
                  << dijTime.totalDistance << " m)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 9: Source equals destination
    std::cout << "[Test 9/13] Source equals destination (5 -> 5): ";
    PathResult sameRes = navigation.dijkstra(graph, 5, 5, OptimizationMode::DISTANCE);
    if (sameRes.found && sameRes.path.size() == 1 && sameRes.totalDistance == 0.0 && sameRes.totalTimeMin == 0.0) {
        std::cout << "PASSED (Distance = 0, Time = 0, Path = [5])\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 10: Unreachable destination handling
    std::cout << "[Test 10/13] Unreachable destination handling: ";
    // Temporarily isolate node 999 (create temporary test graph)
    CampusGraph testG;
    testG.addLocation(Location(100, "Component A1", "Academic"));
    testG.addLocation(Location(101, "Component A2", "Academic"));
    testG.addLocation(Location(200, "Island Facility", "Facility"));
    testG.addEdge(100, 101, 50.0);
    // Destination 200 is completely disconnected from 100
    PathResult unreachBfs = navigation.bfs(testG, 100, 200);
    PathResult unreachDij = navigation.dijkstra(testG, 100, 200, OptimizationMode::DISTANCE);
    if (!unreachBfs.found && !unreachDij.found && unreachBfs.path.empty() && unreachDij.path.empty()) {
        std::cout << "PASSED (Both BFS and Dijkstra correctly return found=false for unreachable node)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED (Unreachable node incorrectly marked found)\n";
    }

    // Test 11: Invalid location IDs
    std::cout << "[Test 11/13] Invalid location IDs: ";
    PathResult invRes = navigation.dijkstra(graph, 999, 888, OptimizationMode::DISTANCE);
    if (!invRes.found && invRes.path.empty()) {
        std::cout << "PASSED (Handled safely without crash)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 12: Negative edge weights rejected
    std::cout << "[Test 12/13] Negative edge weights rejected: ";
    CampusGraph negG;
    negG.addLocation(Location(1, "Node 1", "Academic"));
    negG.addLocation(Location(2, "Node 2", "Academic"));
    bool addedNeg = negG.addEdge(1, 2, -50.0);
    if (!addedNeg && negG.numEdges() == 0) {
        std::cout << "PASSED (Negative edge rejected, graph integrity preserved)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    // Test 13: Blocked edges ignored by route calculations
    std::cout << "[Test 13/13] Blocked edges ignored by route calculations: ";
    // In our VIT campus graph: 0 -> 1 -> 8 is a direct route to Library
    pathManager.blockWalkway(graph, 1, 8);
    PathResult blockedReroute = navigation.dijkstra(graph, 0, 8, OptimizationMode::DISTANCE);
    bool bypassSuccess = false;
    if (blockedReroute.found) {
        // Verify edge (1, 8) is NOT in the path
        bool usedBlocked = false;
        for (size_t i = 0; i + 1 < blockedReroute.path.size(); ++i) {
            if ((blockedReroute.path[i] == 1 && blockedReroute.path[i+1] == 8) ||
                (blockedReroute.path[i] == 8 && blockedReroute.path[i+1] == 1)) {
                usedBlocked = true;
                break;
            }
        }
        if (!usedBlocked) bypassSuccess = true;
    }
    // Restore the walkway
    pathManager.restoreWalkway(graph, 1, 8);
    PathResult restoredRoute = navigation.dijkstra(graph, 0, 8, OptimizationMode::DISTANCE);
    if (bypassSuccess && restoredRoute.found) {
        std::cout << "PASSED (Rerouted around blocked edge; restored successfully)\n";
        ++testsPassed;
    } else {
        std::cout << "FAILED\n";
    }

    std::cout << std::string(80, '=') << "\n";
    std::cout << "TEST RESULTS: " << testsPassed << " / " << totalTests << " PASSED ("
              << (testsPassed * 100 / totalTests) << "%)\n";
    std::cout << std::string(80, '=') << "\n";

    return (testsPassed == totalTests);
}

void CampusNavigation::run() {
    int choice = -1;
    while (choice != 0) {
        displayMenu();
        choice = getValidInt("Enter option (0-17): ", 0, 17);

        switch (choice) {
            case 1:
                graph.displayLocations();
                pause();
                break;
            case 2:
                graph.displayConnections();
                pause();
                break;
            case 3:
                searchLocation();
                pause();
                break;
            case 4:
            case 5:
            case 6: {
                int src = getValidInt("Enter Source Location ID: ", 0, 9999);
                int dst = getValidInt("Enter Destination Location ID: ", 0, 9999);
                PathResult r;
                if (choice == 4) r = navigation.dijkstra(graph, src, dst, OptimizationMode::DISTANCE);
                else if (choice == 5) r = navigation.dijkstra(graph, src, dst, OptimizationMode::TIME);
                else r = navigation.bfs(graph, src, dst);
                routeManager.displayRoute(graph, r);
                pause();
                break;
            }
            case 7: {
                int src = getValidInt("Enter Starting Location ID for DFS exploration: ", 0, 9999);
                int dst = getValidInt("Enter Target Location ID: ", 0, 9999);
                PathResult r = navigation.dfs(graph, src, dst);
                routeManager.displayRoute(graph, r);
                std::cout << "\nExploration Traversal Visit Order:\n";
                for (size_t i = 0; i < r.visitOrder.size(); ++i) {
                    const Location* loc = graph.getLocationById(r.visitOrder[i]);
                    std::cout << "[" << r.visitOrder[i] << "] " << (loc ? loc->shortName : "Loc");
                    if (i + 1 < r.visitOrder.size()) std::cout << " -> ";
                }
                std::cout << "\n";
                pause();
                break;
            }
            case 8:
                findFacility();
                pause();
                break;
            case 9:
                manageWalkways();
                pause();
                break;
            case 10:
                findAlternativeRoutesMenu();
                pause();
                break;
            case 11:
                compareAlgorithmsMenu();
                pause();
                break;
            case 12:
                runAutomatedTests();
                pause();
                break;
            case 13:
                graph.displayBlockedWalkways();
                pause();
                break;
            case 14:
                graph.displayActiveWalkways();
                pause();
                break;
            case 15: {
                std::cout << "\n--- Add Campus Location ---\n";
                int id = getValidInt("Enter Location ID (unique integer): ", 0, 999999);
                std::string name = getLine("Enter Location Name: ");
                std::string cat = getLine("Enter Category (e.g. Department, Facility, Library): ");
                std::string sname = getLine("Enter Short Name: ");
                Location loc(id, name, cat, sname.empty() ? name : sname);
                if (graph.addLocation(loc)) {
                    std::cout << "Location added successfully!\n";
                }
                pause();
                break;
            }
            case 16: {
                std::cout << "\n--- Add Campus Walkway ---\n";
                int u = getValidInt("Enter Source Location ID: ", 0, 9999);
                int v = getValidInt("Enter Destination Location ID: ", 0, 9999);
                std::cout << "Enter Walking Distance in metres: ";
                double dist;
                std::cin >> dist;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::string ptype = getLine("Enter Path Type (paved_path, paved_road, corridor): ");
                if (graph.addEdge(u, v, dist, -1.0, ptype.empty() ? "paved_path" : ptype)) {
                    std::cout << "Walkway added successfully!\n";
                }
                pause();
                break;
            }
            case 17:
                pathManager.restoreAllWalkways(graph, true);
                pause();
                break;
            case 0:
                std::cout << "\nThank you for using the Campus Navigation System. Goodbye!\n";
                break;
            default:
                break;
        }
    }
}

// -----------------------------------------------------------------------------
// Backend CLI & JSON API Bridge (Phase 2 Foundation)
// -----------------------------------------------------------------------------
int CampusNavigation::handleCli(int argc, char* argv[]) {
    if (argc < 2) return -1; // Not CLI mode, run interactive menu

    std::string arg1 = argv[1];

    if (arg1 == "--test") {
        bool ok = runAutomatedTests();
        return ok ? 0 : 1;
    }

    if (arg1 == "--help" || arg1 == "-h") {
        std::cout << "Campus Navigation System - CLI Commands:\n";
        std::cout << "  (no args)             Launch interactive console menu\n";
        std::cout << "  --test                Run automated test suite (13 test cases)\n";
        std::cout << "  --json locations      Output all campus locations as JSON\n";
        std::cout << "  --json connections    Output all walkways as JSON\n";
        std::cout << "  --json route <src> <dst> [distance|time|bfs|dfs]  Compute route and return JSON\n";
        std::cout << "  --json nearest <src> <category> [distance|time]   Find nearest facility as JSON\n";
        std::cout << "  --json block <from> <to>    Block walkway and return JSON\n";
        std::cout << "  --json restore <from> <to>  Restore walkway and return JSON\n";
        std::cout << "  --json health         Return health check JSON\n";
        return 0;
    }

    if (arg1 == "--json") {
        if (argc < 3) {
            std::cout << "{\"error\": \"Missing JSON subcommand. Use: locations, connections, route, nearest, block, restore, health\"}\n";
            return 1;
        }
        std::string sub = argv[2];

        if (sub == "health") {
            std::cout << "{\"status\": \"ok\", \"locations_count\": " << graph.numLocations()
                      << ", \"edges_count\": " << graph.numEdges()
                      << ", \"version\": \"1.0.0\"}\n";
            return 0;
        }

        if (sub == "locations") {
            std::cout << "[\n";
            const auto& locs = graph.allLocations();
            for (size_t i = 0; i < locs.size(); ++i) {
                const auto& loc = locs[i];
                std::cout << "  {\"id\": " << loc.id
                          << ", \"name\": \"" << loc.name << "\""
                          << ", \"short_name\": \"" << loc.shortName << "\""
                          << ", \"category\": \"" << loc.categoryString() << "\""
                          << ", \"lat\": " << std::fixed << std::setprecision(6) << loc.lat
                          << ", \"lon\": " << loc.lon
                          << ", \"description\": \"" << loc.description << "\"}";
                if (i + 1 < locs.size()) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << "]\n";
            return 0;
        }

        if (sub == "connections") {
            std::cout << "[\n";
            bool first = true;
            for (const auto& [fromId, edgeList] : graph.adj) {
                for (const auto& edge : edgeList) {
                    if (fromId < edge.destinationId) { // output each undirected edge once
                        if (!first) std::cout << ",\n";
                        first = false;
                        std::cout << "  {\"from_id\": " << fromId
                                  << ", \"to_id\": " << edge.destinationId
                                  << ", \"distance_m\": " << std::fixed << std::setprecision(1) << edge.distance
                                  << ", \"walking_time_min\": " << edge.walkingTime
                                  << ", \"path_type\": \"" << edge.pathTypeString() << "\""
                                  << ", \"blocked\": " << (edge.isBlocked ? "true" : "false")
                                  << ", \"description\": \"" << edge.description << "\"}";
                    }
                }
            }
            std::cout << "\n]\n";
            return 0;
        }

        if (sub == "route") {
            if (argc < 5) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json route <sourceId> <destId> [mode]\"}\n";
                return 1;
            }
            int src = std::stoi(argv[3]);
            int dst = std::stoi(argv[4]);
            std::string modeStr = (argc >= 6) ? argv[5] : "distance";

            PathResult r;
            if (modeStr == "time") {
                r = navigation.dijkstra(graph, src, dst, OptimizationMode::TIME);
            } else if (modeStr == "bfs") {
                r = navigation.bfs(graph, src, dst);
            } else if (modeStr == "dfs") {
                r = navigation.dfs(graph, src, dst);
            } else {
                r = navigation.dijkstra(graph, src, dst, OptimizationMode::DISTANCE);
            }

            std::cout << "{\n";
            std::cout << "  \"found\": " << (r.found ? "true" : "false") << ",\n";
            std::cout << "  \"source_id\": " << r.sourceId << ",\n";
            std::cout << "  \"dest_id\": " << r.destId << ",\n";
            std::cout << "  \"algorithm\": \"" << r.algorithmName << "\",\n";
            std::cout << "  \"total_distance_m\": " << std::fixed << std::setprecision(1) << r.totalDistance << ",\n";
            std::cout << "  \"total_time_min\": " << r.totalTimeMin << ",\n";
            std::cout << "  \"nodes_visited\": " << r.nodesVisited << ",\n";
            std::cout << "  \"edges_explored\": " << r.edgesExplored << ",\n";
            std::cout << "  \"execution_time_ms\": " << std::setprecision(3) << r.executionTimeMs << ",\n";
            std::cout << "  \"path\": [";
            for (size_t i = 0; i < r.path.size(); ++i) {
                std::cout << r.path[i];
                if (i + 1 < r.path.size()) std::cout << ", ";
            }
            std::cout << "],\n";
            std::cout << "  \"path_names\": [";
            for (size_t i = 0; i < r.path.size(); ++i) {
                const Location* loc = graph.getLocationById(r.path[i]);
                std::cout << "\"" << (loc ? loc->shortName : std::to_string(r.path[i])) << "\"";
                if (i + 1 < r.path.size()) std::cout << ", ";
            }
            std::cout << "]\n";
            std::cout << "}\n";
            return 0;
        }

        if (sub == "nearest") {
            if (argc < 5) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json nearest <sourceId> <category> [mode]\"}\n";
                return 1;
            }
            int src = std::stoi(argv[3]);
            std::string cat = argv[4];
            std::string modeStr = (argc >= 6) ? argv[5] : "distance";
            OptimizationMode mode = (modeStr == "time") ? OptimizationMode::TIME : OptimizationMode::DISTANCE;

            PathResult r = facilityFinder.findNearestFacility(graph, navigation, src, cat, mode);
            const Location* fLoc = graph.getLocationById(facilityFinder.nearestFacilityId);

            std::cout << "{\n";
            std::cout << "  \"found\": " << (r.found ? "true" : "false") << ",\n";
            std::cout << "  \"source_id\": " << src << ",\n";
            std::cout << "  \"category\": \"" << cat << "\",\n";
            std::cout << "  \"nearest_facility_id\": " << facilityFinder.nearestFacilityId << ",\n";
            std::cout << "  \"nearest_facility_name\": \"" << (fLoc ? fLoc->name : "") << "\",\n";
            std::cout << "  \"total_distance_m\": " << std::fixed << std::setprecision(1) << r.totalDistance << ",\n";
            std::cout << "  \"total_time_min\": " << r.totalTimeMin << ",\n";
            std::cout << "  \"path\": [";
            for (size_t i = 0; i < r.path.size(); ++i) {
                std::cout << r.path[i];
                if (i + 1 < r.path.size()) std::cout << ", ";
            }
            std::cout << "]\n";
            std::cout << "}\n";
            return 0;
        }

        if (sub == "block") {
            if (argc < 5) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json block <fromId> <toId>\"}\n";
                return 1;
            }
            int u = std::stoi(argv[3]);
            int v = std::stoi(argv[4]);
            bool ok = pathManager.blockWalkway(graph, u, v, false);
            std::cout << "{\"success\": " << (ok ? "true" : "false")
                      << ", \"from_id\": " << u
                      << ", \"to_id\": " << v
                      << ", \"is_blocked\": " << (!pathManager.isWalkwayAvailable(graph, u, v) ? "true" : "false")
                      << "}\n";
            return ok ? 0 : 1;
        }

        if (sub == "restore") {
            if (argc < 5) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json restore <fromId> <toId>\"}\n";
                return 1;
            }
            int u = std::stoi(argv[3]);
            int v = std::stoi(argv[4]);
            bool ok = pathManager.restoreWalkway(graph, u, v, false);
            std::cout << "{\"success\": " << (ok ? "true" : "false")
                      << ", \"from_id\": " << u
                      << ", \"to_id\": " << v
                      << ", \"is_blocked\": " << (!pathManager.isWalkwayAvailable(graph, u, v) ? "true" : "false")
                      << "}\n";
            return ok ? 0 : 1;
        }

        if (sub == "restore_all") {
            pathManager.restoreAllWalkways(graph, false);
            std::cout << "{\"success\": true, \"message\": \"All walkways restored to active status\"}\n";
            return 0;
        }

        if (sub == "blocked_walkways") {
            std::cout << "[\n";
            auto blocked = graph.getBlockedEdges();
            for (size_t i = 0; i < blocked.size(); ++i) {
                const auto& edge = blocked[i];
                std::cout << "  {\"from_id\": " << edge.sourceId
                          << ", \"to_id\": " << edge.destinationId
                          << ", \"distance_m\": " << std::fixed << std::setprecision(1) << edge.distance
                          << ", \"walking_time_min\": " << edge.walkingTime
                          << ", \"path_type\": \"" << edge.pathTypeString() << "\"}";
                if (i + 1 < blocked.size()) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << "]\n";
            return 0;
        }

        if (sub == "active_walkways") {
            std::cout << "[\n";
            auto active = graph.getActiveEdges();
            for (size_t i = 0; i < active.size(); ++i) {
                const auto& edge = active[i];
                std::cout << "  {\"from_id\": " << edge.sourceId
                          << ", \"to_id\": " << edge.destinationId
                          << ", \"distance_m\": " << std::fixed << std::setprecision(1) << edge.distance
                          << ", \"walking_time_min\": " << edge.walkingTime
                          << ", \"path_type\": \"" << edge.pathTypeString() << "\"}";
                if (i + 1 < active.size()) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << "]\n";
            return 0;
        }

        if (sub == "alternatives") {
            if (argc < 5) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json alternatives <sourceId> <destId> [mode]\"}\n";
                return 1;
            }
            int src = std::stoi(argv[3]);
            int dst = std::stoi(argv[4]);
            std::string modeStr = (argc >= 6) ? argv[5] : "distance";
            OptimizationMode mode = (modeStr == "time") ? OptimizationMode::TIME : OptimizationMode::DISTANCE;

            PathResult primary = navigation.dijkstra(graph, src, dst, mode);
            auto alts = routeManager.findAlternativeRoutes(graph, navigation, src, dst, mode, 3);

            std::cout << "{\n";
            std::cout << "  \"source_id\": " << src << ",\n";
            std::cout << "  \"dest_id\": " << dst << ",\n";
            std::cout << "  \"optimization_mode\": \"" << modeToString(mode) << "\",\n";
            std::cout << "  \"primary_route\": {\n";
            std::cout << "    \"found\": " << (primary.found ? "true" : "false") << ",\n";
            std::cout << "    \"total_distance_m\": " << std::fixed << std::setprecision(1) << primary.totalDistance << ",\n";
            std::cout << "    \"total_time_min\": " << primary.totalTimeMin << ",\n";
            std::cout << "    \"segments\": " << (primary.path.empty() ? 0 : (int)primary.path.size() - 1) << ",\n";
            std::cout << "    \"path\": [";
            for (size_t i = 0; i < primary.path.size(); ++i) {
                std::cout << primary.path[i];
                if (i + 1 < primary.path.size()) std::cout << ", ";
            }
            std::cout << "],\n";
            std::cout << "    \"path_names\": [";
            for (size_t i = 0; i < primary.path.size(); ++i) {
                const Location* loc = graph.getLocationById(primary.path[i]);
                std::cout << "\"" << (loc ? loc->shortName : std::to_string(primary.path[i])) << "\"";
                if (i + 1 < primary.path.size()) std::cout << ", ";
            }
            std::cout << "]\n";
            std::cout << "  },\n";
            std::cout << "  \"alternatives\": [\n";
            for (size_t a = 0; a < alts.size(); ++a) {
                const auto& r = alts[a];
                std::cout << "    {\n";
                std::cout << "      \"rank\": " << (a + 1) << ",\n";
                std::cout << "      \"total_distance_m\": " << std::fixed << std::setprecision(1) << r.totalDistance << ",\n";
                std::cout << "      \"total_time_min\": " << r.totalTimeMin << ",\n";
                std::cout << "      \"segments\": " << (r.path.empty() ? 0 : (int)r.path.size() - 1) << ",\n";
                std::cout << "      \"path\": [";
                for (size_t i = 0; i < r.path.size(); ++i) {
                    std::cout << r.path[i];
                    if (i + 1 < r.path.size()) std::cout << ", ";
                }
                std::cout << "],\n";
                std::cout << "      \"path_names\": [";
                for (size_t i = 0; i < r.path.size(); ++i) {
                    const Location* loc = graph.getLocationById(r.path[i]);
                    std::cout << "\"" << (loc ? loc->shortName : std::to_string(r.path[i])) << "\"";
                    if (i + 1 < r.path.size()) std::cout << ", ";
                }
                std::cout << "]\n";
                std::cout << "    }";
                if (a + 1 < alts.size()) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << "  ]\n";
            std::cout << "}\n";
            return 0;
        }

        if (sub == "add_location") {
            if (argc < 6) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json add_location <id> <name> <category> [short_name] [lat] [lon] [desc]\"}\n";
                return 1;
            }
            int id = std::stoi(argv[3]);
            std::string name = argv[4];
            std::string cat = argv[5];
            std::string sname = (argc >= 7) ? argv[6] : name;
            double lat = (argc >= 8) ? std::stod(argv[7]) : 18.4640;
            double lon = (argc >= 9) ? std::stod(argv[8]) : 73.8680;
            std::string desc = (argc >= 10) ? argv[9] : "";

            Location loc(id, name, cat, sname, lat, lon, desc);
            bool ok = graph.addLocation(loc);
            std::cout << "{\"success\": " << (ok ? "true" : "false")
                      << ", \"location_id\": " << id
                      << ", \"name\": \"" << name << "\"}\n";
            return ok ? 0 : 1;
        }

        if (sub == "add_walkway") {
            if (argc < 6) {
                std::cout << "{\"error\": \"Missing arguments. Expected: --json add_walkway <fromId> <toId> <dist> [time] [type] [desc]\"}\n";
                return 1;
            }
            int u = std::stoi(argv[3]);
            int v = std::stoi(argv[4]);
            double dist = std::stod(argv[5]);
            double time = (argc >= 7) ? std::stod(argv[6]) : -1.0;
            std::string ptype = (argc >= 8) ? argv[7] : "paved_path";
            std::string desc = (argc >= 9) ? argv[8] : "";

            bool ok = graph.addEdge(u, v, dist, time, ptype, desc);
            std::cout << "{\"success\": " << (ok ? "true" : "false")
                      << ", \"from_id\": " << u
                      << ", \"to_id\": " << v
                      << ", \"distance_m\": " << std::fixed << std::setprecision(1) << dist
                      << "}\n";
            return ok ? 0 : 1;
        }
    }

    return -1;
}

} // namespace VITNav
