# Campus Navigation System — API & Integration Architecture Design
## Phase 1 C++ Core to Phase 2 Web Integration

**Document Version:** 1.0.0  
**Phase:** 1 (Foundation for Phase 2)  
**Target Frontend:** React + Vite + Tailwind CSS / Canvas Map  
**Bridge Layer:** Node.js (Express / Fastify) Bridge communicating with native C++ Binary  

---

## 1. Architectural Strategy

The project architecture strictly adheres to the course requirement:
- **All actual graph representations, traversal algorithms (BFS, DFS), shortest-path calculations (Dijkstra distance/time modes), alternative route evaluations, and dynamic path availability MUST run in C++**.
- JavaScript / TypeScript in Phase 2 will **never re-implement graph algorithms**. It serves purely as the UI layer and HTTP bridge.

```
+-----------------------------------------------------------+
|               Phase 2: React + Vite Frontend              |
|  - Interactive Schematic Map Canvas                       |
|  - Route Finder UI (Source/Destination/Mode)              |
|  - Live Rerouting Dashboard (Block/Restore Walkways)      |
|  - Multi-Algorithm Visualizer & Comparison Cards          |
+-----------------------------------------------------------+
                             |
                   HTTP REST API (JSON)
                             |
+-----------------------------------------------------------+
|          Phase 2: Lightweight Node.js Bridge Server        |
|  - Spawns C++ binary (`campus_nav.exe --json <command>`)  |
|  - Or communicates via stdio / child_process              |
|  - Ultra-fast native execution (<5ms per route query)     |
+-----------------------------------------------------------+
                             |
                     JSON stdio / IPC
                             |
+-----------------------------------------------------------+
|               C++ DSA Core Engine (Phase 1)               |
|  - CampusGraph: Adjacency list representation             |
|  - Navigation: BFS, DFS, Dijkstra (Distance & Time modes) |
|  - RouteManager: Multi-algorithm comparison & alternatives|
|  - FacilityFinder: Category-based Dijkstra locator        |
|  - PathManager: Dynamic edge blocking & restoration       |
+-----------------------------------------------------------+
```

---

## 2. CLI Command Specification (JSON API Contract)

The compiled C++ executable (`campus_nav.exe`) supports direct JSON commands via the `--json` flag.

### A. Health Check
- **Command:** `campus_nav.exe --json health`
- **Response Format:**
```json
{
  "status": "ok",
  "locations_count": 20,
  "edges_count": 32,
  "version": "1.0.0"
}
```

### B. Get All Campus Locations
- **Command:** `campus_nav.exe --json locations`
- **Response Format:**
```json
[
  {
    "id": 0,
    "name": "Main Gate (Entry/Exit)",
    "short_name": "Main Gate",
    "category": "Gate",
    "lat": 18.463100,
    "lon": 73.867600,
    "description": "Primary entrance/exit on main road"
  },
  {
    "id": 8,
    "name": "Central Campus Library",
    "short_name": "Library",
    "category": "Library",
    "lat": 18.464200,
    "lon": 73.868500,
    "description": "Multi-floor library & reading rooms"
  }
]
```

### C. Get All Walkways / Connections
- **Command:** `campus_nav.exe --json connections`
- **Response Format:**
```json
[
  {
    "from_id": 0,
    "to_id": 1,
    "distance_m": 120.0,
    "walking_time_min": 1.4,
    "path_type": "Paved Path",
    "blocked": false,
    "description": "Main Gate to Admin Block"
  },
  {
    "from_id": 1,
    "to_id": 8,
    "distance_m": 130.0,
    "walking_time_min": 1.6,
    "path_type": "Paved Path",
    "blocked": false,
    "description": "Admin Block to Library"
  }
]
```

### D. Route Calculation
- **Command:** `campus_nav.exe --json route <sourceId> <destId> [mode]`
- **Mode Options:** `distance` (default), `time`, `bfs`, `dfs`
- **Example:** `campus_nav.exe --json route 0 8 distance`
- **Response Format:**
```json
{
  "found": true,
  "source_id": 0,
  "dest_id": 8,
  "algorithm": "Dijkstra (Shortest Distance)",
  "total_distance_m": 250.0,
  "total_time_min": 3.0,
  "nodes_visited": 3,
  "edges_explored": 7,
  "execution_time_ms": 0.045,
  "path": [0, 1, 8],
  "path_names": ["Main Gate", "Admin Block", "Library"]
}
```

### E. Nearest Facility Search
- **Command:** `campus_nav.exe --json nearest <sourceId> <category> [mode]`
- **Example:** `campus_nav.exe --json nearest 0 Library distance`
- **Response Format:**
```json
{
  "found": true,
  "source_id": 0,
  "category": "Library",
  "nearest_facility_id": 8,
  "nearest_facility_name": "Central Campus Library",
  "total_distance_m": 250.0,
  "total_time_min": 3.0,
  "path": [0, 1, 8]
}
```

### F. Dynamic Walkway Blocking
- **Command:** `campus_nav.exe --json block <fromId> <toId>`
- **Example:** `campus_nav.exe --json block 1 8`
- **Response Format:**
```json
{
  "success": true,
  "from_id": 1,
  "to_id": 8,
  "is_blocked": true
}
```

### G. Dynamic Walkway Restoration
- **Command:** `campus_nav.exe --json restore <fromId> <toId>`
- **Example:** `campus_nav.exe --json restore 1 8`
- **Response Format:**
```json
{
  "success": true,
  "from_id": 1,
  "to_id": 8,
  "is_blocked": false
}
```

---

## 3. Node.js Express Reference Implementation for Phase 2

Below is the complete reference Express.js server router (`server.js`) that directly executes the C++ backend and exposes clean REST endpoints:

```javascript
// server.js - Node.js Express API Bridge for Phase 2
const express = require('express');
const cors = require('cors');
const { execFile } = require('child_process');
const path = require('path');

const app = express();
app.use(cors());
app.use(express.json());

const CPP_BIN = path.join(__dirname, 'campus_nav.exe');

function runCpp(args) {
    return new Promise((resolve, reject) => {
        execFile(CPP_BIN, ['--json', ...args], (err, stdout, stderr) => {
            if (err && !stdout) return reject(stderr || err);
            try {
                resolve(JSON.parse(stdout));
            } catch (e) {
                reject('Failed to parse C++ JSON output: ' + stdout);
            }
        });
    });
}

// REST Endpoints
app.get('/api/health', async (req, res) => {
    try { res.json(await runCpp(['health'])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

app.get('/api/locations', async (req, res) => {
    try { res.json(await runCpp(['locations'])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

app.get('/api/connections', async (req, res) => {
    try { res.json(await runCpp(['connections'])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

app.get('/api/route', async (req, res) => {
    const { from, to, mode = 'distance' } = req.query;
    try { res.json(await runCpp(['route', from, to, mode])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

app.get('/api/nearest', async (req, res) => {
    const { from, category, mode = 'distance' } = req.query;
    try { res.json(await runCpp(['nearest', from, category, mode])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

app.post('/api/walkway/block', async (req, res) => {
    const { from, to } = req.body;
    try { res.json(await runCpp(['block', from, to])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

app.post('/api/walkway/restore', async (req, res) => {
    const { from, to } = req.body;
    try { res.json(await runCpp(['restore', from, to])); }
    catch (e) { res.status(500).json({ error: e.toString() }); }
});

const PORT = 5000;
app.listen(PORT, () => {
    console.log(`Campus Navigation API Bridge listening on http://localhost:${PORT}`);
});
```

---

## 4. Phase 2 Frontend Architecture Recommendations

- **Framework:** React 19 + Vite + TypeScript / JavaScript.
- **Styling:** Tailwind CSS + Lucide Icons.
- **Mapping:** HTML5 Canvas or SVG renderer plotting node coordinates (`lat`, `lon` mapped to 2D canvas coordinates) with interactive zoom/pan, animated route line drawing, and clickable node selection.
- **State Management:** React hooks / Context managing selected source/destination nodes, blocked walkways, and calculated route responses.
