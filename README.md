# Campus Navigation System

A high-performance campus navigation and pathfinding application built with a native C++17 Data Structures and Algorithms (DSA) engine, a Node.js Express REST bridge, and an interactive React 18 frontend with real-time SVG campus mapping.

Designed for university campuses (modeled on the Vishwakarma Institute of Technology campus), this system calculates optimal walking paths, explores alternative detours, identifies nearest facilities, and supports dynamic walkway blocking/unblocking with instantaneous route recalculation.

---

## 🌟 Key Features

1. **Shortest Distance Routing (Dijkstra)**
   - Calculates globally optimal walking paths based on physical walkway distance in meters.
2. **Fastest Walking Time Routing (Dijkstra)**
   - Computes time-optimized paths accounting for walkway surfaces (paved, gravel, indoor, stairs).
3. **Fewest Stops Routing (Breadth-First Search / BFS)**
   - Minimizes the number of intermediate junctions or buildings traversed.
4. **Deep Campus Exploration (Depth-First Search / DFS)**
   - Visualizes reachability analysis and graph traversal patterns.
5. **Alternative Detour Suggestions**
   - Implements edge-penalty heuristic detour generation to provide backup routes when main paths are crowded or closed.
6. **Nearest Facility Locator**
   - Single-source Dijkstra search across categorical hubs (Libraries, Canteens, Health Centres, Laboratories, Hostels, etc.).
7. **Dynamic Walkway Blocking & Restoration**
   - Real-time blockage simulation for campus maintenance or closures with persistent state (`blocked_walkways.json`).
8. **Interactive SVG Campus Map**
   - Responsive vector map with pan, zoom, clickable location pins, animated route glow polyline, and turn-by-turn guidance.

---

## 🛠️ Technology Stack

| Layer | Technology | Role |
|---|---|---|
| **DSA Core Engine** | **C++17** (GCC / MinGW-w64) | Graph representation, Dijkstra, BFS, DFS, route alternatives, facility search, walkway management |
| **Backend Bridge** | **Node.js + Express** | REST API layer executing C++ CLI with JSON serialization via `child_process.execFile` |
| **Frontend UI** | **React 18 + Vite** | Single-page reactive application with state management |
| **Styling & Icons** | **Tailwind CSS + Lucide React** | Clean UI, responsive layout, iconography |
| **Data Format** | **JSON** | Location metadata, connection topologies, blockage persistence |

---

## 📁 Project Structure

```text
PathNavigationSystem/
├── backend/                        # Node.js Express API Bridge
│   ├── node_modules/               # Backend dependencies (express, cors)
│   ├── package.json                # Backend dependency declarations
│   └── server.js                   # REST API server & static frontend host
│
├── dsa-engine/                     # Native C++ Data Structures & Algorithms Core
│   ├── include/                    # Header files
│   │   ├── Algorithms.h            # Dijkstra, BFS, DFS, A* declarations
│   │   ├── CampusData.h            # JSON loader & persistence manager
│   │   ├── CampusGraph.h           # Graph representation & location indexing
│   │   ├── CampusNavigation.h      # Facade orchestrator
│   │   ├── Edge.h                  # Walkway representation
│   │   ├── FacilityFinder.h        # Category-based nearest facility finder
│   │   ├── Graph.h                 # Adjacency list graph base
│   │   ├── Location.h              # Campus vertex / building model
│   │   ├── Navigation.h            # Traversal state & parent pointers
│   │   ├── NavigationController.h  # Controller coordination
│   │   ├── PathManager.h           # Walkway blocking/restoration
│   │   └── RouteManager.h          # Route comparison & metrics
│   ├── src/                        # C++ Source implementations
│   │   ├── Algorithms.cpp
│   │   ├── AStar.cpp
│   │   ├── BFS.cpp
│   │   ├── CampusData.cpp
│   │   ├── CampusGraph.cpp
│   │   ├── CampusNavigation.cpp
│   │   ├── DFS.cpp
│   │   ├── Dijkstra.cpp
│   │   ├── Edge.cpp
│   │   ├── FacilityFinder.cpp
│   │   ├── Graph.cpp
│   │   ├── Location.cpp
│   │   ├── main.cpp                # CLI entry point (interactive & JSON modes)
│   │   ├── Navigation.cpp
│   │   ├── NavigationController.cpp
│   │   ├── PathManager.cpp
│   │   └── RouteManager.cpp
│   ├── data/                       # Campus datasets
│   │   ├── locations.json          # 20 campus landmarks with coordinates & categories
│   │   ├── connections.json        # 32 bidirectional walkway segments
│   │   └── blocked_walkways.json   # Blockage state persistence
│   └── CMakeLists.txt              # CMake configuration
│
├── frontend/                       # Modern React 18 + Vite web interface
│   ├── public/                     # Public assets & favicons
│   ├── src/
│   │   ├── components/             # Reusable UI widgets
│   │   │   ├── CampusMap.jsx       # Interactive SVG map visualizer
│   │   │   └── Sidebar.jsx         # Navigation sidebar
│   │   ├── pages/                  # Application views
│   │   │   ├── AlternativeRoutes.jsx
│   │   │   ├── CampusLocations.jsx
│   │   │   ├── Dashboard.jsx
│   │   │   ├── ExploreCampus.jsx
│   │   │   ├── FindRoute.jsx
│   │   │   ├── ManageWalkways.jsx
│   │   │   └── NearestFacilities.jsx
│   │   ├── services/
│   │   │   └── api.js              # REST client consuming backend endpoints
│   │   ├── App.jsx                 # Top-level state & routing
│   │   ├── index.css               # Tailwind CSS entry
│   │   └── main.jsx                # DOM mounting entry
│   ├── dist/                       # Production frontend build
│   ├── package.json
│   ├── tailwind.config.js
│   └── vite.config.js
│
├── docs/                           # Architecture docs & specifications
│   ├── API_DESIGN.md               # REST API specification
│   ├── IMPLEMENTATION_STATUS.md    # Feature verification tracking
│   ├── PROJECT_AUDIT.md            # Architecture audit report
│   ├── TESTING_REPORT.md           # 13/13 DSA test case verification report
│   ├── Campus Navigation System Architecture.png
│   └── Campus_Navigation_System_Project_Plan.pdf
│
├── .gitignore                      # Git exclusion rules
├── build.bat                       # C++ engine build script (g++ C++17)
├── run.bat                         # One-click full system launcher (Node + Browser)
├── run_terminal.bat                # Standalone C++ terminal launcher
└── README.md                       # Complete documentation
```

---

## 🚀 Getting Started

### Prerequisites

1. **C++ Compiler**: GCC 9+ or MinGW-w64 with C++17 support (e.g. `C:\msys64\ucrt64\bin\g++.exe` or `g++` on PATH).
2. **Node.js**: v18.0.0 or higher.
3. **npm**: v9.0.0 or higher.

---

### 1. Compile the C++ Engine

Run the build script from the root `PathNavigationSystem/` directory:

```powershell
.\build.bat
```

Or compile manually using `g++`:

```powershell
g++ -std=c++17 -O2 -I"dsa-engine/include" ^
    dsa-engine/src/*.cpp ^
    -o campus_nav.exe
```

Run test suite to verify all 13 core algorithms:

```powershell
.\campus_nav.exe --test
```

---

### 2. Run the Full Application (One-Click)

From `PathNavigationSystem/`:

```powershell
.\run.bat
```

This will automatically:
1. Start the Express API bridge on `http://localhost:5000`.
2. Serve the production frontend bundle.
3. Open your default web browser to `http://localhost:5000`.

---

### 3. Running Services Individually

#### Backend Server
```powershell
cd backend
npm install
node server.js
```
The server will run on `http://localhost:5000`.

#### Frontend (Development Server with Hot Reload)
```powershell
cd frontend
npm install
npm run dev
```
Accessible at `http://localhost:5173` (proxies API requests to port 5000).

#### Frontend Production Build
```powershell
cd frontend
npm run build
```
Generates optimized static assets into `frontend/dist/`.

#### Standalone C++ Terminal Console
```powershell
.\run_terminal.bat
```
Launches the interactive menu-driven console interface for manual route queries without a browser.

---

## 📡 API Endpoints Reference

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/health` | Health check & C++ binary status |
| `GET` | `/api/dashboard` | Graph metrics, total locations, active/blocked edges |
| `GET` | `/api/locations` | List of all 20 campus locations |
| `GET` | `/api/connections` | List of all 32 walkway segments |
| `GET` | `/api/route?from={id}&to={id}&mode={distance\|time\|bfs\|dfs}` | Calculate optimal route between two points |
| `GET` | `/api/alternatives?from={id}&to={id}&mode={distance\|time}` | Detour route recommendations |
| `GET` | `/api/nearest?from={id}&category={category}&mode={mode}` | Nearest facility locator |
| `POST` | `/api/walkway/block` | Dynamically block a walkway (`{from, to}`) |
| `POST` | `/api/walkway/restore` | Restore a blocked walkway (`{from, to}`) |
| `POST` | `/api/walkway/restore-all` | Unblock all blocked walkways |

---

## 🧪 Verification & Test Results

The native C++ core features a built-in automated test suite verifying 13 core DSA requirements:

- **Test 1**: Location loading & validation (20 locations verified)
- **Test 2**: Walkway connection topology (32 undirected edges verified)
- **Test 3**: Keyword location search (case-insensitive substring match)
- **Test 4**: Non-existent location safety
- **Test 5**: BFS min-hop pathfinding
- **Test 6**: DFS deep graph exploration
- **Test 7**: Dijkstra shortest distance optimization
- **Test 8**: Dijkstra fastest walking time optimization
- **Test 9**: Zero-distance loop handling (source == destination)
- **Test 10**: Unreachable destination safety (disconnected components)
- **Test 11**: Boundary & invalid node ID validation
- **Test 12**: Negative weight rejection (preserving graph invariant)
- **Test 13**: Dynamic walkway blocking and rerouting

All 13 tests execute with **100% PASS** rate.
