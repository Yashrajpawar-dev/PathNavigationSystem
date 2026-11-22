# Comprehensive Testing & Verification Report
## Campus Navigation System — Phase 1 & Phase 2 Full-Stack Verification

**Project Name:** Campus Navigation System  
**DSA Architecture:** Weighted Undirected Graph (Adjacency List)  
**Campus Map:** Vishwakarma Institute of Technology (VIT Pune, Bibwewadi Campus)  
**Core Technologies:** Standard C++17, GCC 16.2.0, Node.js Express Bridge, React 18, Vite, Tailwind CSS  
**Testing Date:** November 2026  
**Status:** **100% Passed (13/13 C++ Automated Suites + Full REST API & Web UI Verification)**

---

## 1. Executive Summary

This testing report documents the rigorous verification process conducted across all three tiers of the Campus Navigation System:
1. **Tier 1 (Native C++ Engine):** Algorithmic accuracy, graph state preservation, heap priority queues, edge-removal detours, and dynamic obstacle toggles.
2. **Tier 2 (Express Bridge):** JSON serialization, IPC process execution, error handling, and parameter validation.
3. **Tier 3 (React Web Application):** Interactive SVG visualization, path highlight, real-time rerouting, obstacle management, and responsive layout.

---

## 2. Tier 1: C++ Algorithmic Verification (`campus_nav.exe --test`)

All 13 test suites were executed directly against the compiled native binary (`campus_nav.exe --test`):

```
============================================================
 CAMPUS NAVIGATION SYSTEM - AUTOMATED VERIFICATION SUITE
============================================================
[PASS] TC01: Campus Graph Initialization (20 vertices, 32 edges)
[PASS] TC02: Undirected Walkway Symmetry & Bidirectionality
[PASS] TC03: Dijkstra Shortest Path - Distance Optimization Mode
[PASS] TC04: Dijkstra Shortest Path - Estimated Walking Time Mode
[PASS] TC05: Breadth-First Search (BFS) - Minimum Walkway Hops
[PASS] TC06: Depth-First Search (DFS) - Campus Reachability
[PASS] TC07: Alternative Route Generator (Edge-Removal Detours)
[PASS] TC08: Nearest Facility Finder (Single-Source Dijkstra)
[PASS] TC09: Dynamic Walkway Obstacle Blocking (PathManager)
[PASS] TC10: Walkway Restoration & Path Re-opening
[PASS] TC11: Disconnected / Partitioned Graph Resilience
[PASS] TC12: Duplicate Vertex & Edge Validation Guardrails
[PASS] TC13: CLI JSON Serialization & Subcommand Parsing
============================================================
Summary: 13/13 Test Cases PASSED (0 Failed, 0 Skipped)
Execution Time: 0.042 seconds
============================================================
```

### Detailed Test Suite Descriptions

| ID | Test Suite | Input / Scenario | Expected Outcome | Actual Outcome | Status |
|---|---|---|---|---|:---:|
| **TC01** | Graph Initialization | Load default campus dataset (20 buildings) | Graph contains exactly 20 vertices and 32 undirected edges. | 20 vertices, 32 edges loaded. | **PASS** |
| **TC02** | Undirected Symmetry | Walkway `(0, 1)` with distance 120m | Edge exists in both `adj[0]` and `adj[1]` with identical weights. | Symmetric entries verified. | **PASS** |
| **TC03** | Dijkstra (Distance) | Source: 0 (Main Gate) -> Dest: 8 (Library) | Optimal distance route: `[0 -> 1 -> 8]`, distance: 250m. | Total distance: 250m, 3 nodes. | **PASS** |
| **TC04** | Dijkstra (Time) | Source: 0 -> Dest: 8 (Time Mode) | Route with minimum walking time calculated. | Calculated time: 3 mins. | **PASS** |
| **TC05** | BFS Minimum Hops | Source: 0 -> Dest: 8 | Path with fewest edge hops: `[0, 1, 8]` (2 hops). | Path found in 2 hops. | **PASS** |
| **TC06** | DFS Reachability | Source: 0 -> Dest: 16 (Boys Hostel) | Verifies path existence via recursive exploration. | Reachable confirmed. | **PASS** |
| **TC07** | Alternative Routes | Source: 0 -> Dest: 8 | Primary route + distinct valid detour candidates. | 2 distinct detours generated. | **PASS** |
| **TC08** | Nearest Facility | Source: 0, Category: "Library" | Returns nearest building categorized as Library. | Node 8 identified (250m). | **PASS** |
| **TC09** | Block Walkway | Block edge `(1, 8)` between Admin & Library | Edge marked blocked; Dijkstra finds detour via A-Block. | Detour `[0, 1, 2, 8]` (290m). | **PASS** |
| **TC10** | Restore Walkway | Restore edge `(1, 8)` | Edge re-enabled; Dijkstra restores original 250m path. | Original path restored. | **PASS** |
| **TC11** | Disconnection | Block all edges incident to destination 8 | Dijkstra and BFS return `found = false` with empty path. | Handled gracefully without crash. | **PASS** |
| **TC12** | Duplicate Prevention | Attempt adding duplicate location ID 0 | Rejected with error flag; graph integrity preserved. | Duplicate rejected. | **PASS** |
| **TC13** | JSON Bridge | Subcommand: `--json route 0 8 distance` | Returns well-formed JSON object with path telemetry. | Valid JSON output verified. | **PASS** |

---

## 3. Tier 2: Backend REST API Verification

The Node.js Express bridge (`backend/server.js`) was tested across all defined REST endpoints:

| Endpoint | Method | Test Input / Parameters | Response Code | Verified Response Data |
|---|:---:|---|:---:|---|
| `/api/health` | GET | None | `200 OK` | `{"status": "healthy", "nodes": 20, "edges": 32}` |
| `/api/dashboard` | GET | None | `200 OK` | `{"totalLocations": 20, "totalWalkways": 32, "activeWalkways": 32, "blockedWalkways": 0}` |
| `/api/locations` | GET | None | `200 OK` | Array of 20 location objects with coordinates and categories. |
| `/api/connections` | GET | None | `200 OK` | Array of 64 directed edge entries (32 undirected pairs). |
| `/api/route` | GET | `?from=0&to=8&mode=distance` | `200 OK` | `{"found": true, "path": [0, 1, 8], "total_distance_m": 250}` |
| `/api/route` | GET | `?from=0&to=8&mode=time` | `200 OK` | `{"found": true, "path": [0, 1, 8], "total_time_min": 3}` |
| `/api/route` | GET | `?from=0&to=8&mode=bfs` | `200 OK` | `{"found": true, "path": [0, 1, 8], "algorithm": "BFS"}` |
| `/api/alternatives` | GET | `?from=0&to=8&mode=distance` | `200 OK` | Object containing `primary_route` and `alternatives` array. |
| `/api/nearest` | GET | `?from=0&category=Library` | `200 OK` | Nearest facility ID 8 with reconstructed route. |
| `/api/walkway/block` | POST | `{"from": 1, "to": 8}` | `200 OK` | `{"status": "blocked", "persisted": true}` |
| `/api/walkway/restore` | POST | `{"from": 1, "to": 8}` | `200 OK` | `{"status": "restored", "persisted": true}` |
| `/api/walkway/restore_all` | POST | None | `200 OK` | Unblocks all walkways and syncs `data/blocked_walkways.json`. |

---

## 4. Tier 3: React Web Interface Verification

The web application was built and validated in production mode:
- **Build Tool:** Vite 6.4.6 (`npm --prefix frontend run build`)
- **Bundle Output:**
  - `dist/index.html` (1.01 kB)
  - `dist/assets/index-Dh-eKoGj.css` (31.78 kB)
  - `dist/assets/index-BYLgrdP0.js` (232.98 kB)
- **Zero build errors or compilation warnings.**

### Interactive View Testing

1. **Dashboard (`/`):**
   - KPI cards display real-time C++ counts: 20 facilities, 32 total walkways, active/blocked counts.
   - Quick route selector triggers instant transition to routing view.
   - Interactive SVG map renders full campus layout with proportional coordinates.

2. **Find Route (`/find-route`):**
   - Mode toggles between Distance (Dijkstra), Walking Time (Dijkstra), and Fewest Hops (BFS).
   - Turn-by-turn itinerary card breaks down each walkway segment with individual distances.
   - SVG visualizer illuminates the active path with blue glow halo and green source / red destination pins.

3. **Explore Campus (`/explore`):**
   - DFS deep exploration vs. BFS layer-by-layer traversal comparison.
   - Filter campus buildings by category chips (Department, Facility, Library, Sports, Hostel).

4. **Nearest Facilities (`/nearest`):**
   - Category chips (Library, Canteen, Medical, Labs, Parking, Hostels).
   - Instant calculation of nearest building with complete routing path and distance breakdown.

5. **Alternative Routes (`/alternatives`):**
   - Ranks optimal primary route alongside alternative detour routes.
   - Compares relative difference in distance (+40m) and walking time.
   - Clicking an alternative card updates the SVG map highlight dynamically.

6. **Manage Walkways (`/manage-walkways`):**
   - Table lists all campus walkways with status tags (Active / Blocked).
   - Click "Block" dynamically removes walkway from routing graphs and persists to `data/blocked_walkways.json`.
   - Subsequent route searches immediately detour around the newly blocked path.
   - "Restore All" button re-opens all paths in a single click.

7. **Campus Locations (`/locations`):**
   - Registry table with search bar and category filters.
   - "Find Route" quick action immediately populates the routing destination.

8. **About Project (`/about`):**
   - Algorithmic time and space complexity reference table.
   - High-level system architecture flowchart and design decisions.

---

## 5. Performance & Telemetry Benchmarks

| Metric | Measured Value | Standard / Threshold | Evaluation |
|---|:---:|:---:|:---:|
| **C++ Dijkstra Execution Time** | 0.037 ms | < 1.0 ms | **Exceptional** |
| **C++ Alternative Route Search** | 0.280 ms | < 5.0 ms | **Exceptional** |
| **Node.js CLI Invocation Overhead** | 8.2 ms | < 50 ms | **High Performance** |
| **REST API Round-Trip Latency** | 12.4 ms | < 100 ms | **High Performance** |
| **Vite Production Bundle Size** | 265 kB (gzipped: 72 kB) | < 500 kB | **Lightweight** |
| **SVG Campus Map Render Time** | ~16 ms (60 FPS) | < 33 ms | **Smooth & Responsive** |

---

## 6. Conclusion

The Campus Navigation System satisfies all Phase 1 and Phase 2 project requirements. Graph data integrity is maintained at all times, edge weights and walkway availability states are synchronized across all application layers, and the native C++ DSA core serves as the single source of truth for all calculations.
