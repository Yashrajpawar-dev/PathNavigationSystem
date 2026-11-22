// ============================================================
// backend/server.js - Campus Navigation System Express Bridge
// Connects React Frontend to Native C++ DSA Engine (campus_nav.exe)
// ============================================================

const express = require('express');
const cors = require('cors');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
app.use(cors());
app.use(express.json());

// Locate compiled C++ executable
const CPP_BIN_CANDIDATES = [
    path.join(__dirname, '..', 'campus_nav.exe'),
    path.join(__dirname, 'campus_nav.exe'),
    path.join(process.cwd(), 'campus_nav.exe')
];

let CPP_BIN = CPP_BIN_CANDIDATES.find(p => fs.existsSync(p)) || CPP_BIN_CANDIDATES[0];

console.log(`[Bridge] Using C++ Binary at: ${CPP_BIN}`);

// Execute C++ CLI command with --json flag
function runCpp(args) {
    return new Promise((resolve, reject) => {
        execFile(CPP_BIN, ['--json', ...args], { cwd: path.join(__dirname, '..') }, (err, stdout, stderr) => {
            if (err && !stdout) {
                console.error(`[Bridge Error] Command failed: ${args.join(' ')}`, stderr || err);
                return reject(stderr || err.message || 'Execution failed');
            }
            try {
                // Parse stdout JSON (find first '{' or '[')
                const text = stdout.trim();
                const jsonStart = text.search(/[\[{]/);
                if (jsonStart === -1) {
                    return reject(`No JSON found in C++ output: ${text}`);
                }
                const parsed = JSON.parse(text.slice(jsonStart));
                resolve(parsed);
            } catch (e) {
                console.error('[Bridge JSON Parse Error]', stdout);
                reject(`Failed to parse C++ JSON: ${e.message}`);
            }
        });
    });
}

// -------------------------------------------------------------
// REST API Endpoints
// -------------------------------------------------------------

// 1. Health check
app.get('/api/health', async (req, res) => {
    try {
        const data = await runCpp(['health']);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 2. Locations list
app.get('/api/locations', async (req, res) => {
    try {
        const data = await runCpp(['locations']);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 3. Add Location
app.post('/api/locations', async (req, res) => {
    try {
        const { id, name, category, short_name, lat, lon, description } = req.body;
        if (id === undefined || !name || !category) {
            return res.status(400).json({ error: 'id, name, and category are required' });
        }
        const data = await runCpp([
            'add_location',
            id.toString(),
            name,
            category,
            short_name || name,
            (lat || 18.464).toString(),
            (lon || 73.868).toString(),
            description || ''
        ]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 4. Walkways (Connections) list
app.get('/api/connections', async (req, res) => {
    try {
        const data = await runCpp(['connections']);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 5. Add Walkway
app.post('/api/connections', async (req, res) => {
    try {
        const { from_id, to_id, distance_m, walking_time_min, path_type, description } = req.body;
        if (from_id === undefined || to_id === undefined || !distance_m) {
            return res.status(400).json({ error: 'from_id, to_id, and distance_m are required' });
        }
        const data = await runCpp([
            'add_walkway',
            from_id.toString(),
            to_id.toString(),
            distance_m.toString(),
            (walking_time_min || -1).toString(),
            path_type || 'paved_path',
            description || ''
        ]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 6. Route search
app.get('/api/route', async (req, res) => {
    try {
        const { from, to, mode = 'distance' } = req.query;
        if (from === undefined || to === undefined) {
            return res.status(400).json({ error: 'Query parameters from and to are required' });
        }
        const data = await runCpp(['route', from.toString(), to.toString(), mode.toLowerCase()]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 7. Alternative routes
app.get('/api/alternatives', async (req, res) => {
    try {
        const { from, to, mode = 'distance' } = req.query;
        if (from === undefined || to === undefined) {
            return res.status(400).json({ error: 'Query parameters from and to are required' });
        }
        const data = await runCpp(['alternatives', from.toString(), to.toString(), mode.toLowerCase()]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 8. Nearest facility
app.get('/api/nearest', async (req, res) => {
    try {
        const { from, category, mode = 'distance' } = req.query;
        if (from === undefined || !category) {
            return res.status(400).json({ error: 'Query parameters from and category are required' });
        }
        const data = await runCpp(['nearest', from.toString(), category, mode.toLowerCase()]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 9. Block walkway
app.post('/api/walkway/block', async (req, res) => {
    try {
        const { from, to } = req.body;
        if (from === undefined || to === undefined) {
            return res.status(400).json({ error: 'Body fields from and to are required' });
        }
        const data = await runCpp(['block', from.toString(), to.toString()]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 10. Restore walkway
app.post('/api/walkway/restore', async (req, res) => {
    try {
        const { from, to } = req.body;
        if (from === undefined || to === undefined) {
            return res.status(400).json({ error: 'Body fields from and to are required' });
        }
        const data = await runCpp(['restore', from.toString(), to.toString()]);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 11. Restore all walkways
app.post('/api/walkway/restore-all', async (req, res) => {
    try {
        const data = await runCpp(['restore_all']);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 12. Blocked walkways list
app.get('/api/walkways/blocked', async (req, res) => {
    try {
        const data = await runCpp(['blocked_walkways']);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 13. Active walkways list
app.get('/api/walkways/active', async (req, res) => {
    try {
        const data = await runCpp(['active_walkways']);
        res.json(data);
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 14. Dashboard summary
app.get('/api/dashboard', async (req, res) => {
    try {
        const [health, locations, connections, blocked] = await Promise.all([
            runCpp(['health']),
            runCpp(['locations']),
            runCpp(['connections']),
            runCpp(['blocked_walkways'])
        ]);

        const totalLocations = locations.length;
        const totalWalkways = connections.length;
        const blockedWalkways = blocked.length;
        const activeWalkways = totalWalkways - blockedWalkways;

        // Categories breakdown
        const categories = {};
        locations.forEach(loc => {
            const cat = loc.category || 'Other';
            categories[cat] = (categories[cat] || 0) + 1;
        });

        res.json({
            status: 'ok',
            totalLocations,
            totalWalkways,
            activeWalkways,
            blockedWalkways,
            categories,
            campusName: 'VIT Pune (Bibwewadi Campus)',
            algorithmEngine: 'Native C++17 DSA Core'
        });
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// 15. Multi-Algorithm comparison
app.get('/api/compare', async (req, res) => {
    try {
        const { from, to } = req.query;
        if (from === undefined || to === undefined) {
            return res.status(400).json({ error: 'Query parameters from and to are required' });
        }
        const [dijkstraDist, dijkstraTime, bfsRoute, dfsRoute] = await Promise.all([
            runCpp(['route', from.toString(), to.toString(), 'distance']),
            runCpp(['route', from.toString(), to.toString(), 'time']),
            runCpp(['route', from.toString(), to.toString(), 'bfs']),
            runCpp(['route', from.toString(), to.toString(), 'dfs'])
        ]);

        res.json({
            source_id: parseInt(from),
            dest_id: parseInt(to),
            results: [
                { algorithm: "Dijkstra (Shortest Distance)", ...dijkstraDist },
                { algorithm: "Dijkstra (Fastest Time)", ...dijkstraTime },
                { algorithm: "BFS (Fewest Hops)", ...bfsRoute },
                { algorithm: "DFS (Exploration)", ...dfsRoute }
            ]
        });
    } catch (e) {
        res.status(500).json({ error: e.toString() });
    }
});

// Serve static frontend assets if built
const frontendDist = path.join(__dirname, '..', 'frontend', 'dist');
if (fs.existsSync(frontendDist)) {
    console.log(`[Bridge] Serving production frontend build from: ${frontendDist}`);
    app.use(express.static(frontendDist));
    app.get('*', (req, res, next) => {
        if (req.path.startsWith('/api')) return next();
        res.sendFile(path.join(frontendDist, 'index.html'));
    });
}

const PORT = process.env.PORT || 5000;
app.listen(PORT, () => {
    console.log(`=======================================================`);
    console.log(` Campus Navigation API Bridge running on port ${PORT}`);
    console.log(` Health: http://localhost:${PORT}/api/health`);
    console.log(` Dashboard: http://localhost:${PORT}/api/dashboard`);
    console.log(`=======================================================`);
});
