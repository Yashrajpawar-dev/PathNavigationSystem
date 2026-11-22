import React, { useState, useEffect, useCallback } from 'react';
import {
  Navigation,
  Clock,
  Compass,
  ArrowRight,
  AlertCircle,
  Footprints,
  Cpu,
  Layers,
  ArrowLeftRight,
  CheckCircle2,
  MapPin,
  Sparkles
} from 'lucide-react';
import CampusMap from '../components/CampusMap';
import { fetchRoute } from '../services/api';

const QUICK_PRESETS = [
  { label: 'Gate → Library', src: 0, dst: 8 },
  { label: 'Gate → Canteen', src: 0, dst: 9 },
  { label: 'Admin → CE Dept', src: 1, dst: 3 },
  { label: 'Hostel → Comp Labs', src: 16, dst: 11 },
  { label: 'A-Block → Sports', src: 2, dst: 12 },
];

export default function FindRoute({ locations = [], connections = [], initialSrc = 0, initialDst = 8 }) {
  const [sourceId, setSourceId] = useState(initialSrc ?? 0);
  const [destId, setDestId] = useState(initialDst ?? 8);
  const [mode, setMode] = useState('distance'); // distance | time | bfs | dfs
  const [loading, setLoading] = useState(false);
  const [routeData, setRouteData] = useState(null);
  const [error, setError] = useState(null);

  // Sync if initial props change
  useEffect(() => {
    if (initialSrc !== undefined && initialSrc !== null) setSourceId(initialSrc);
    if (initialDst !== undefined && initialDst !== null) setDestId(initialDst);
  }, [initialSrc, initialDst]);

  // Ensure valid selection once locations are loaded
  useEffect(() => {
    if (locations && locations.length > 0) {
      if (sourceId === undefined || sourceId === null || !locations.some(l => l.id === sourceId)) {
        setSourceId(locations[0].id);
      }
      if (destId === undefined || destId === null || !locations.some(l => l.id === destId)) {
        const defaultDest = locations.find(l => l.id === 8) || locations[Math.min(1, locations.length - 1)];
        setDestId(defaultDest.id);
      }
    }
  }, [locations]);

  const calculateRoute = useCallback(async (src = sourceId, dst = destId, routingMode = mode) => {
    if (src === undefined || dst === undefined || src === null || dst === null) return;

    if (src === dst) {
      setRouteData({
        found: true,
        source_id: src,
        dest_id: dst,
        algorithm: 'Direct Identification',
        total_distance_m: 0,
        total_time_min: 0,
        path: [src],
        path_names: [locations.find(l => l.id === src)?.name || 'Current Location'],
        nodes_visited: 1,
        edges_explored: 0,
        execution_time_ms: 0.001
      });
      setError(null);
      return;
    }

    setLoading(true);
    setError(null);
    try {
      const data = await fetchRoute(src, dst, routingMode);
      setRouteData(data);
      if (!data.found) {
        setError(`No accessible route found between [${src}] and [${dst}]. Walkways may be blocked.`);
      }
    } catch (err) {
      setError(err.message || 'Failed to calculate route from C++ engine.');
    } finally {
      setLoading(false);
    }
  }, [locations, sourceId, destId, mode]);

  // Recalculate whenever source, destination, or mode changes
  useEffect(() => {
    calculateRoute(sourceId, destId, mode);
  }, [sourceId, destId, mode, calculateRoute]);

  const handleSwap = () => {
    const nextSrc = destId;
    const nextDst = sourceId;
    setSourceId(nextSrc);
    setDestId(nextDst);
  };

  const handlePreset = (preset) => {
    setSourceId(preset.src);
    setDestId(preset.dst);
  };

  // Helper to find walkway edge data between consecutive nodes
  const getLegInfo = (u, v) => {
    const edge = connections.find(c => 
      (c.from_id === u && c.to_id === v) || 
      (c.from_id === v && c.to_id === u)
    );
    return edge || null;
  };

  return (
    <div className="space-y-6">
      {/* Page Header */}
      <div className="flex flex-col md:flex-row md:items-center justify-between gap-2">
        <div>
          <h2 className="text-2xl font-bold text-slate-900">Campus Route Navigation</h2>
          <p className="text-slate-500 text-sm">
            Select Start and Destination on the interactive campus map or dropdowns to compute optimal paths.
          </p>
        </div>
        {/* Preset quick routes */}
        <div className="flex flex-wrap items-center gap-1.5">
          <span className="text-xs text-slate-400 font-medium flex items-center gap-1 mr-1">
            <Sparkles className="w-3.5 h-3.5 text-blue-500" /> Presets:
          </span>
          {QUICK_PRESETS.map((p, idx) => (
            <button
              key={idx}
              onClick={() => handlePreset(p)}
              className={`px-2.5 py-1 text-xs rounded-lg font-medium transition ${
                sourceId === p.src && destId === p.dst
                  ? 'bg-blue-600 text-white shadow-sm'
                  : 'bg-white hover:bg-slate-100 text-slate-700 border border-slate-200'
              }`}
            >
              {p.label}
            </button>
          ))}
        </div>
      </div>

      {/* Control Panel */}
      <div className="bg-white rounded-2xl p-6 border border-slate-200 shadow-sm space-y-4">
        <div className="grid grid-cols-1 md:grid-cols-12 gap-4 items-end">
          {/* Source Select */}
          <div className="md:col-span-4">
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5 flex items-center gap-1.5">
              <span className="w-2.5 h-2.5 rounded-full bg-emerald-500 ring-2 ring-emerald-300"></span>
              <span>Start Location (Source)</span>
            </label>
            <select
              value={sourceId}
              onChange={(e) => setSourceId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 focus:outline-none focus:ring-2 focus:ring-emerald-500 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading locations...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name} ({loc.category})</option>
                  ))
              }
            </select>
          </div>

          {/* Swap Button */}
          <div className="md:col-span-1 flex justify-center pb-1">
            <button
              onClick={handleSwap}
              title="Swap Start and Destination"
              className="p-2.5 bg-slate-100 hover:bg-blue-50 hover:text-blue-600 border border-slate-200 rounded-xl transition text-slate-600"
            >
              <ArrowLeftRight className="w-4 h-4" />
            </button>
          </div>

          {/* Destination Select */}
          <div className="md:col-span-4">
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5 flex items-center gap-1.5">
              <span className="w-2.5 h-2.5 rounded-full bg-rose-500 ring-2 ring-rose-300"></span>
              <span>Goal Location (Destination)</span>
            </label>
            <select
              value={destId}
              onChange={(e) => setDestId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 focus:outline-none focus:ring-2 focus:ring-rose-500 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading locations...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name} ({loc.category})</option>
                  ))
              }
            </select>
          </div>

          {/* Mode Selector */}
          <div className="md:col-span-3">
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5">
              Routing Algorithm / Mode
            </label>
            <select
              value={mode}
              onChange={(e) => setMode(e.target.value)}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
            >
              <option value="distance">Shortest Distance (Dijkstra)</option>
              <option value="time">Fastest Walking Time (Dijkstra)</option>
              <option value="bfs">Fewest Stops (BFS Min-Hops)</option>
              <option value="dfs">Exploratory Route (DFS)</option>
            </select>
          </div>
        </div>

        {/* Algorithm indicator badge */}
        <div className="pt-2 border-t border-slate-100 flex flex-wrap items-center justify-between text-xs text-slate-500">
          <div className="flex items-center gap-2">
            <span className="font-semibold text-slate-700">Map Click Instruction:</span>
            <span>Click any building on the map below and choose <strong>"Set Start"</strong> or <strong>"Set Goal"</strong>.</span>
          </div>
          <div className="text-slate-400">
            Active Mode: <strong className="text-blue-600 uppercase">{mode}</strong>
          </div>
        </div>
      </div>

      {/* Main Content: Map Visualizer + Route Instructions Panel */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        {/* Left 7 Columns: Interactive Campus Map */}
        <div className="lg:col-span-7 bg-white rounded-2xl p-5 border border-slate-200 shadow-sm flex flex-col">
          <div className="flex items-center justify-between mb-3">
            <h3 className="font-bold text-slate-900 text-sm flex items-center gap-2">
              <Compass className="w-4 h-4 text-blue-600" />
              <span>Campus Map & Route Overlay</span>
            </h3>
            {routeData?.found && (
              <span className="text-xs px-2.5 py-1 bg-emerald-50 text-emerald-700 font-semibold rounded-full border border-emerald-200">
                Route Active: {routeData.path.length} Stops ({Math.round(routeData.total_distance_m)}m)
              </span>
            )}
          </div>
          <CampusMap
            locations={locations}
            connections={connections}
            activePath={routeData?.found ? routeData.path : []}
            sourceId={sourceId}
            destId={destId}
            height="520px"
            onSelectStart={(id) => setSourceId(id)}
            onSelectDest={(id) => setDestId(id)}
          />
        </div>

        {/* Right 5 Columns: Route Details Panel */}
        <div className="lg:col-span-5 space-y-4">
          {error && (
            <div className="bg-rose-50 border border-rose-200 text-rose-800 rounded-2xl p-5 flex items-start gap-3">
              <AlertCircle className="w-5 h-5 text-rose-600 shrink-0 mt-0.5" />
              <div>
                <h4 className="font-bold text-sm">Navigation Notice</h4>
                <p className="text-xs mt-1 text-rose-700">{error}</p>
              </div>
            </div>
          )}

          {routeData && routeData.found ? (
            <div className="bg-white rounded-2xl p-6 border border-slate-200 shadow-sm space-y-5">
              {/* Route Metric Cards */}
              <div className="grid grid-cols-2 gap-3">
                <div className="bg-slate-50 border border-slate-200/80 rounded-xl p-3.5">
                  <div className="flex items-center gap-1.5 text-xs text-slate-500 font-medium mb-1">
                    <Footprints className="w-4 h-4 text-blue-600" />
                    <span>Total Distance</span>
                  </div>
                  <div className="text-2xl font-extrabold text-slate-900">
                    {Math.round(routeData.total_distance_m)} <span className="text-sm font-semibold text-slate-500">m</span>
                  </div>
                </div>

                <div className="bg-slate-50 border border-slate-200/80 rounded-xl p-3.5">
                  <div className="flex items-center gap-1.5 text-xs text-slate-500 font-medium mb-1">
                    <Clock className="w-4 h-4 text-indigo-600" />
                    <span>Walking Time</span>
                  </div>
                  <div className="text-2xl font-extrabold text-slate-900">
                    {routeData.total_time_min?.toFixed(1)} <span className="text-sm font-semibold text-slate-500">min</span>
                  </div>
                </div>
              </div>

              {/* Algorithm Details Bar */}
              <div className="p-3 bg-blue-50/70 border border-blue-100 rounded-xl flex items-center justify-between text-xs text-blue-900">
                <div className="flex items-center gap-2">
                  <Cpu className="w-4 h-4 text-blue-600" />
                  <span className="font-semibold">{routeData.algorithm}</span>
                </div>
                <div className="text-slate-500 font-mono">
                  {routeData.execution_time_ms} ms computation
                </div>
              </div>

              {/* Turn-by-Turn Walkway Directions */}
              <div>
                <h4 className="font-bold text-slate-900 text-sm mb-3 flex items-center gap-2">
                  <Layers className="w-4 h-4 text-slate-600" />
                  <span>Turn-by-Turn Directions</span>
                </h4>

                <div className="space-y-2 max-h-72 overflow-y-auto pr-1">
                  {routeData.path.map((nodeId, idx) => {
                    const loc = locations.find(l => l.id === nodeId);
                    const isFirst = idx === 0;
                    const isLast = idx === routeData.path.length - 1;
                    const nextNodeId = !isLast ? routeData.path[idx + 1] : null;
                    const leg = nextNodeId !== null ? getLegInfo(nodeId, nextNodeId) : null;

                    return (
                      <div
                        key={idx}
                        className={`p-3 rounded-xl border flex flex-col gap-1.5 text-xs transition ${
                          isFirst
                            ? 'bg-emerald-50 border-emerald-200 text-emerald-900'
                            : isLast
                            ? 'bg-rose-50 border-rose-200 text-rose-900'
                            : 'bg-slate-50 border-slate-200 text-slate-800'
                        }`}
                      >
                        <div className="flex items-center justify-between">
                          <div className="flex items-center gap-2.5">
                            <span className={`w-5 h-5 rounded-full flex items-center justify-center font-bold text-[10px] ${
                              isFirst ? 'bg-emerald-600 text-white' : isLast ? 'bg-rose-600 text-white' : 'bg-slate-300 text-slate-800'
                            }`}>
                              {idx + 1}
                            </span>
                            <div>
                              <span className="font-bold">[{nodeId}] {loc?.name || 'Campus Location'}</span>
                              <span className="text-[10px] text-slate-400 block">{loc?.category}</span>
                            </div>
                          </div>

                          {isFirst && (
                            <span className="px-2 py-0.5 bg-emerald-600 text-white text-[10px] font-bold rounded-full">
                              START
                            </span>
                          )}
                          {isLast && (
                            <span className="px-2 py-0.5 bg-rose-600 text-white text-[10px] font-bold rounded-full">
                              GOAL
                            </span>
                          )}
                        </div>

                        {/* Step segment metrics if not last stop */}
                        {leg && (
                          <div className="ml-7 pt-1 border-t border-slate-200/50 flex items-center justify-between text-[11px] text-slate-500">
                            <span>Walk along {leg.path_type ? leg.path_type.replace('_', ' ') : 'walkway'}</span>
                            <span className="font-mono font-semibold text-slate-700">
                              {Math.round(leg.distance_m)}m (~{leg.walking_time_min} min)
                            </span>
                          </div>
                        )}
                      </div>
                    );
                  })}
                </div>
              </div>

              {/* Telemetry Footer */}
              <div className="border-t border-slate-100 pt-3 text-[11px] text-slate-400 flex justify-between">
                <span>Visited {routeData.nodes_visited} nodes</span>
                <span>Explored {routeData.edges_explored} edges</span>
                <span>{routeData.path.length - 1} walkway segments</span>
              </div>
            </div>
          ) : !error && (
            <div className="bg-white rounded-2xl p-8 border border-slate-200 shadow-sm text-center text-slate-400">
              <Navigation className="w-10 h-10 mx-auto text-slate-300 mb-2" />
              <p className="text-sm font-medium">Select source and destination to view route details</p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
