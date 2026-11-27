# VIT Pune Campus Navigator

**Smart Campus Navigation System**  
Vishwakarma Institute of Technology, Bibwewadi, Pune 411037

---

## Project Overview

This is a **DSA (Data Structures and Algorithms) project** that demonstrates how graph algorithms solve real-world campus navigation problems.

The application provides:
- An interactive map of the **VIT Pune Bibwewadi campus**
- Multiple graph traversal and shortest-path algorithms
- Route visualization with animation
- Algorithm comparison dashboard
- Path blocking/unblocking for dynamic rerouting

---

## ⚠ Data Accuracy Notice

Building positions and walking paths are **APPROXIMATE**, estimated from OpenStreetMap data for the VIT Bibwewadi campus area. They have NOT been verified by official campus survey. The system is designed so verified coordinates can easily replace approximate ones by editing `data/locations.json`.

---

## Project Structure

```
VIT-Campus-Navigator/
├── include/                   # C++ header files
│   ├── Location.h             # Campus location class
│   ├── Edge.h                 # Walking path edge class
│   ├── Graph.h                # Weighted undirected graph
│   ├── Algorithms.h           # Algorithm declarations
│   └── NavigationController.h # Data loading & dispatch
│
├── src/                       # C++ source files
│   ├── main.cpp               # Qt6 application entry point
│   ├── Location.cpp
│   ├── Edge.cpp
│   ├── Graph.cpp
│   ├── Algorithms.cpp         # Shared helpers + Floyd-Warshall
│   ├── BFS.cpp                # Breadth-First Search
│   ├── DFS.cpp                # Depth-First Search
│   ├── Dijkstra.cpp           # Dijkstra's Algorithm
│   ├── AStar.cpp              # A* Search
│   └── NavigationController.cpp
│
├── ui/                        # Qt6 UI files
│   ├── MainWindow.h/.cpp      # Main application window
│   └── MapWidget.h/.cpp       # Custom campus map renderer
│
├── python_app/
│   └── main.py                # ★ Immediately runnable PyQt6 app
│
├── data/
│   ├── locations.json         # Campus location records
│   └── connections.json       # Walking path connections
│
├── resources/
│   └── resources.qrc
│
├── CMakeLists.txt             # CMake build configuration
└── README.md
```

---

## Algorithms Implemented

| Algorithm | Type | Guarantee | Complexity |
|-----------|------|-----------|------------|
| **BFS** | Min-hop traversal | Fewest stops | O(V+E) |
| **DFS** | Graph traversal | Reachability only | O(V+E) |
| **Dijkstra** | Shortest weighted path | Minimum distance ✓ | O((V+E) log V) |
| **A\*** | Informed shortest path | Minimum distance ✓ | O((V+E) log V) |
| **Floyd-Warshall** | All-pairs shortest paths | All pairs ✓ | O(V³) |

> **Note:** BFS guarantees the fewest edges, not the shortest distance. DFS guarantees neither. Always use Dijkstra or A\* for minimum-distance navigation.

### Heuristic Used in A\*
The A\* heuristic is the **Haversine formula** – the great-circle distance between node and destination using GPS coordinates. This is admissible (never overestimates) because walking distance ≥ straight-line distance.

---

## Campus Locations (20 nodes)

| ID | Location | Category |
|----|----------|----------|
| 0 | Main Gate | Gate |
| 1 | Administrative Block | Administration |
| 2 | Main Building (A-Block) | Academic |
| 3 | Computer Engineering Dept | Department |
| 4 | Information Technology Dept | Department |
| 5 | Electronics & TC Dept | Department |
| 6 | Mechanical Engineering Dept | Department |
| 7 | Civil Engineering Dept | Department |
| 8 | Library | Library |
| 9 | Canteen / Food Court | Facility |
| 10 | Auditorium | Facility |
| 11 | Computer Lab Complex | Laboratory |
| 12 | Sports Ground & Gymnasium | Sports |
| 13 | Parking Area | Parking |
| 14 | Workshop & Manufacturing Lab | Laboratory |
| 15 | Research & Innovation Centre | Research |
| 16 | Boys Hostel | Hostel |
| 17 | Girls Hostel | Hostel |
| 18 | Medical Centre | Health |
| 19 | Staff Quarters / Faculty Block | Faculty |

---

## Running the Application

### Option 1: Python App (Runs Immediately ★)

**Requirements:** Python 3.8+ with PyQt6

```bash
# Install dependency (one time)
pip install PyQt6

# Run from project root
python python_app/main.py
```

This is a complete, fully functional application with all features.

### Option 2: C++ Qt6 Application (Requires Qt6 Installation)

**Requirements:**
- Qt 6.5+ with Widgets module
- CMake 3.16+
- MSVC 2019+ or GCC 10+ or Clang 12+

**Windows (MSVC + Qt6 installed):**
```bash
# Configure
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"

# Build
cmake --build build --config Release

# Run
build/Release/VITCampusNavigator.exe
```

**Linux/macOS:**
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x.x/gcc_64"
cmake --build build
./build/VITCampusNavigator
```

---

## UI Features

### Navigation Panel (Left)
- 🔍 **Search** – type to filter campus locations
- 🧭 **Route Selection** – choose source and destination
- ⚙ **Algorithm** – select which algorithm to run
- **Find Route** – compute and display the route
- **Compare All** – run all 4 algorithms and compare results
- **Reset / Clear Blocks** – reset state

### Map (Centre)
- Rendered campus map with geographic positioning
- Colour-coded location nodes by category
- Edge distance labels
- Animated route with directional arrows
- S (Start) / D (Destination) markers
- Pan (drag), Zoom (scroll wheel)
- Click nodes to select source/destination
- Blocked paths shown as dashed red lines

### Statistics Panel (Right)
- Route steps with distances
- Total walking distance and estimated time
- Algorithm performance: nodes visited, edges explored, execution time
- Algorithm information cards

---

## Graph Representation

```cpp
// Adjacency list (undirected weighted graph)
vector<vector<pair<int, double>>> adj;
// adj[locIndex] = { (neighbourIndex, distanceMetres), ... }

// Key classes:
class Location   // id, name, category, lat, lon, accuracy
class Edge       // fromId, toId, distanceM, pathType, blocked
class Graph      // locations, adj, edges, haversine()
class PathResult // path[], totalDistance, nodesVisited, ...
```

---

## Extending the Data

### Add a New Location
Edit `data/locations.json`:
```json
{
  "id": 20,
  "name": "New Building Name",
  "short_name": "New Bldg",
  "category": "Academic",
  "lat": 18.4648,
  "lon": 73.8686,
  "accuracy": "APPROXIMATE",
  "description": "Description here.",
  "source": "Your data source"
}
```

### Add a Connection
Edit `data/connections.json`:
```json
{
  "from_id": 20,
  "to_id": 2,
  "distance_m": 85,
  "path_type": "paved_path",
  "blocked": false,
  "verified_distance": false,
  "description": "New Building to A-Block"
}
```

### Replace Approximate Coordinates with Verified Data
When you have verified GPS coordinates from official sources:
1. Update `lat` and `lon` in `data/locations.json`
2. Change `"accuracy": "VERIFIED"`
3. Update `"source"` to cite the official measurement

---

## Accuracy Categories

| Accuracy | Meaning |
|----------|---------|
| `VERIFIED` | Confirmed by official campus survey |
| `APPROXIMATE` | Estimated from OpenStreetMap / satellite imagery |
| `PLACEHOLDER` | Completely unverified – demonstration only |

---

## Known Limitations

1. All building positions are **approximate** – for academic demonstration only
2. Walking distances are straight-line estimates × routing factor, not measured paths
3. The map does not load live tile imagery (uses a custom-rendered schematic map)
4. Floyd-Warshall is computed on demand and cached; it recomputes when edges are blocked

---

## Author & Course

**Course:** Data Structures and Algorithms  
**Institution:** Vishwakarma Institute of Technology, Bibwewadi, Pune  
**Technology:** C++17 / Qt6 Widgets / PyQt6  
**Map Data:** OpenStreetMap (approximate positions)  

---

## References

- OpenStreetMap contributors – campus area coordinates
- VIT Pune official website – facility listing
- CLRS – *Introduction to Algorithms* (Dijkstra, BFS, DFS, Floyd-Warshall)
- A\* Search: Hart, Nilsson, Raphael (1968)
- Qt6 Documentation – https://doc.qt.io/qt-6/
