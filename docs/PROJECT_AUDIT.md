# Project Audit: Campus Navigation System (Phase 1)

**Audit Date:** November 2026  
**Auditor:** Senior C++ DSA Engineer & Software Architect  
**Project Workspace:** `D:\Data Structure1`  
**Target Specification:** Campus Navigation System (DSA Course Project)

---

## 1. Executive Summary

An exhaustive inspection of the entire project repository was conducted. The project contains substantial existing work originally developed for an interactive campus navigation system for **Vishwakarma Institute of Technology (VIT Pune, Bibwewadi Campus)**. 

The existing implementation was split between:
1. A C++ core library (`Location`, `Edge`, `Graph`, `Algorithms`, `NavigationController`) in `DSA project/DSA project/VIT-Campus-Navigator`.
2. A Qt6-based desktop UI (`ui/MainWindow`, `ui/MapWidget`, `src/main.cpp`).
3. A Python/PyQt6 prototype (`python_app/main.py`).
4. Design documents and specifications (`Campus Navigation System_Classes&Function&DataMembers.docx`, `Campus Navigation System claude Input structure.docx`, `Campus_Navigation_System_Project_Plan.pdf`).

However, neither **Qt6** nor **PyQt6** is installed in the target environment, causing the existing graphical application to be unbuildable. Crucially, the DSA course requires a **menu-driven console application** implementing the exact 8-component class architecture (`Location`, `Edge`, `CampusGraph`, `Navigation`, `RouteManager`, `FacilityFinder`, `PathManager`, `CampusNavigation`). Furthermore, critical compilation errors and logical bugs in graph traversal and edge unblocking were discovered in the friend's code.

---

## 2. Inventory of Existing Files and Components

### A. Root Directory Files
- `Campus Navigation System Architecture.png`: Architectural diagram illustrating system component layout.
- `Campus Navigation System claude Input structure.docx`: Project planning document outlining algorithm logic, time complexities, test cases, and suggested menus.
- `Campus Navigation System_Classes&Function&DataMembers.docx`: Final class and function design chart defining the 8 core classes and required members.
- `Campus_Navigation_System_Project_Plan.pdf`: High-level course project plan.
- `CourseProject.docx`: Reference project brief.
- `DSA project.zip`: Zip archive containing the friend's code.
- Standalone lab files: `fast.cpp`, `merge.cpp`, `quick.cpp`, and directories `1Assignment/`, `Algorithms_Problem_Dsa/`, `Binary Search Tree/`, `Graph_/`, `Hashing/`, `Linked List/`, `Queue/`, `stack/` (unrelated standard course homework).

### B. Core Navigation System (`DSA project/DSA project/VIT-Campus-Navigator/`)
- **`include/`**:
  - `Location.h`: Node representation with geographical coordinates and categories.
  - `Edge.h`: Edge representation with distance, path type, and blocked status.
  - `Graph.h`: Adjacency-list based weighted graph.
  - `Algorithms.h`: Declarations for BFS, DFS, Dijkstra, A*, Floyd-Warshall, and `PathResult`.
  - `NavigationController.h`: High-level controller parsing JSON data and coordinating algorithms.
- **`src/`**:
  - `Location.cpp`: Category string mappings.
  - `Edge.cpp`: Path type string mappings.
  - `Graph.cpp`: Adjacency list management, haversine calculations.
  - `Algorithms.cpp`: `buildResult()` path reconstruction and Floyd-Warshall.
  - `BFS.cpp`: Queue-based breadth-first search.
  - `DFS.cpp`: Stack-based depth-first search.
  - `Dijkstra.cpp`: Min-heap priority queue Dijkstra (distance-only).
  - `AStar.cpp`: A* with Haversine heuristic.
  - `NavigationController.cpp`: Manual JSON parser and facade methods.
  - `main.cpp`: Qt6 GUI entry point (`QApplication`).
- **`ui/`**:
  - `MainWindow.h`, `MainWindow.cpp`: Qt6 widget interface.
  - `MapWidget.h`, `MapWidget.cpp`: Custom canvas rendering campus map.
- **`data/`**:
  - `locations.json`: 20 realistic campus locations for VIT Pune.
  - `connections.json`: 43 campus walkways with distances and metadata.
- **`python_app/`**:
  - `main.py`: PyQt6 prototype application.
  - `test_algorithms.py`: Python algorithm test runner (fails because PyQt6 is not installed).

---

## 3. Compilation Errors and Logical Bugs Identified

### Compilation Errors (C++ Core)
When compiling the existing C++ files with GCC 16.2.0 (`g++ -std=c++17`):
1. **`Graph.cpp:15`**: `error: 'M_PI' was not declared in this scope`. On Windows/MinGW/UCRT, `M_PI` is not defined unless `_USE_MATH_DEFINES` is defined prior to `<cmath>`, or a constant is explicitly defined.
2. **`Graph.cpp:46, 51, 55, 75, 80`**: `error: 'remove_if' is not a member of 'std'`. Missing `#include <algorithm>`.
3. **`Algorithms.cpp:56`**: `error: 'reverse' is not a member of 'std'`. Missing `#include <algorithm>`.
4. **`src/main.cpp`**: Depends on Qt6 headers (`<QApplication>`, `<QDir>`, etc.) which are not present on the host system.

### Severe Logical Bugs
1. **Unreachable Destination Bug in `buildResult` (`Algorithms.cpp`)**:
   - In `BFS.cpp` and `DFS.cpp`, distances are initialized to `0.0`, and parents to `-1`.
   - If a destination is disconnected/unreachable, `parent[dstIdx]` remains `-1`, and `dist[dstIdx]` remains `0.0`.
   - In `buildResult()`, the check is `if (dist[dstIdx] >= INF) return result;`. Since `0.0 < INF`, this check passes!
   - Path reconstruction pushes `destId`, finds `parent[dstIdx] == -1`, breaks, appends `sourceId`, reverses the vector to `[sourceId, destId]`, and checks `path.front() == sourceId && path.back() == destId` which evaluates to **TRUE**!
   - Result: **BFS and DFS falsely report a path found for disconnected/unreachable destinations!**
2. **Adjacency Duplication Bug in `Graph::setEdgeBlocked` (`Graph.cpp`)**:
   - When unblocking an edge (`blocked = false`), it loops over `edges_` and appends to `adj_[ia]` and `adj_[ib]` without checking if the edge is already present in `adj_`. If unblocking is executed multiple times, duplicate parallel edges accumulate in the adjacency list.
3. **Missing Walking Time Optimization**:
   - `adj_` in `Graph.h` only stores `std::pair<int, double>` where the weight is distance in metres.
   - Walking time is never stored in the adjacency list.
   - `dijkstra()` has no parameter for mode (`DISTANCE` vs `TIME`). It only optimizes distance.
4. **Lack of Validation for Negative Weights & Invalid IDs**:
   - Negative distances and times are not rejected when edges are added.
   - Graph does not validate duplicate vertices or check for valid endpoints before adding edges.

---

## 4. Status of Required Features

| Feature / Component | Status | Details |
|---|---|---|
| **Location Class** | Partially Implemented | Has data members, missing `displayLocation()`. |
| **Edge Class** | Partially Implemented | Has data members, missing `displayEdge()`, `walkingTime` in edge struct. |
| **CampusGraph Class** | Partially Implemented | Underlying `Graph` exists; missing required methods `displayLocations()`, `displayConnections()`, `findLocationByName()`, `getLocationById()`. |
| **Navigation Class** | Not Implemented | Standalone functions exist, but no OOP encapsulation with state (`parent`, `visited`, `distance/cost`). |
| **RouteManager Class** | Not Implemented | Alternative routes (`findAlternativeRoutes`), metric comparison (`compareRoutes`) missing. |
| **FacilityFinder Class** | Not Implemented | Nearest facility search missing. |
| **PathManager Class** | Not Implemented | Blocking/restoring walkways exists as raw methods on `Graph`, not in `PathManager`. |
| **CampusNavigation System Class** | Not Implemented | Orchestrator and interactive console menu completely missing. |
| **BFS Algorithm** | Partially Implemented | Logic exists, but has unreachable destination bug and lacks segment-count output. |
| **DFS Algorithm** | Partially Implemented | Traversal logic exists, but has unreachable destination bug and lacks traversal order output. |
| **Dijkstra's Algorithm** | Partially Implemented | Distance mode works; time optimization mode is completely missing. |
| **Distance & Time Modes** | Not Implemented | Single Dijkstra implementation parameterized by mode is missing. |
| **Dynamic Walkway Blocking** | Partially Implemented | Edge blocking flag exists, but suffers from duplication bug on unblock. |
| **Backend Integration Foundation** | Not Implemented | Need CLI JSON bridge or server endpoint for Phase 2 React frontend. |

---

## 5. Architectural Remediation & Engineering Decisions

1. **Preserve Friend's Core Logic & Data**:
   - Retain the 20 realistic campus locations (`locations.json`) and 43 walkways (`connections.json`) for VIT Pune.
   - Retain the efficient adjacency list representation and min-heap Dijkstra.
2. **Fix Syntax & Standard Library Issues**:
   - Add `#define _USE_MATH_DEFINES`, `#include <cmath>`, and `#include <algorithm>` where required.
3. **Implement Full Final Class Architecture (Section 3)**:
   - Provide standard-compliant, clean classes: `Location`, `Edge`, `CampusGraph`, `Navigation`, `RouteManager`, `FacilityFinder`, `PathManager`, and `CampusNavigation`.
   - Ensure clean OOP separation, beginner-friendly readability, and full compatibility with existing code.
4. **Fix Algorithmic Defects**:
   - Fix the unreachable check in `buildResult` (check `visited` or `parent` validity).
   - Parameterize `dijkstra` with `OptimizationMode::DISTANCE` vs `OptimizationMode::TIME`.
   - Prevent duplicate edges when restoring walkways.
   - Validate negative edge weights and invalid IDs.
5. **Console Menu & Backend Bridge**:
   - Implement the complete interactive menu in `CampusNavigation::run()`.
   - Implement CLI mode (`--json <command> [args...]`) or JSON API output so a Node.js / React backend in Phase 2 can seamlessly query graph data, compute routes, search locations, and toggle blocked paths without rewriting any C++ DSA logic!

---


## 6. Phase 2 Completion & Full-Stack Audit

Following the completion of the Phase 1 C++ core remediation, Phase 2 was successfully executed:

1. **Alternative Routes Implementation (`RouteManager::findAlternativeRoutes`)**:
   - Implemented edge-removal heuristic: primary Dijkstra path edges are iteratively blocked, alternative routes recalculated, and edges restored non-destructively.
   - De-duplication and rank ordering by distance or walking time.
2. **Nearest Facility Finder (`FacilityFinder::findNearestFacility`)**:
   - Single-source Dijkstra scans all vertices matching target category (e.g., Library, Canteen, Health, Sports, Department).
   - Reconstructs optimal route and turn-by-turn path metrics.
3. **Dynamic Walkway Obstacle Persistence**:
   - Blocked walkways persist automatically across runs to `data/blocked_walkways.json`.
   - Dynamic synchronization in both directions of undirected edges.
4. **Node.js Express REST Bridge (`backend/server.js`)**:
   - Fast asynchronous child process execution of native C++ binary with `--json` subcommands.
   - 12 comprehensive REST endpoints.
5. **Modern React 18 Web UI (`frontend/`)**:
   - Vite 6.4.6 + Tailwind CSS + Lucide React.
   - Responsive university portal with interactive SVG campus visualizer.
   - Static distribution bundle (`frontend/dist`) served directly by the Express bridge at `http://localhost:5000`.
