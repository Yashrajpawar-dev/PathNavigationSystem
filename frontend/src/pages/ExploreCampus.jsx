import React, { useState, useEffect, useCallback } from 'react';
import { Compass, GitMerge, Search, Layers, Play, CheckCircle2 } from 'lucide-react';
import CampusMap from '../components/CampusMap';
import { fetchRoute } from '../services/api';

export default function ExploreCampus({ locations = [], connections = [] }) {
  const [startId, setStartId] = useState(0);
  const [targetId, setTargetId] = useState(16); // Boys Hostel
  const [traversalType, setTraversalType] = useState('dfs'); // dfs | bfs
  const [result, setResult] = useState(null);
  const [loading, setLoading] = useState(false);
  const [selectedCategory, setSelectedCategory] = useState('ALL');

  const categories = ['ALL', ...new Set(locations.map(l => l.category).filter(Boolean))];

  const handleExplore = useCallback(async (src = startId, dst = targetId, type = traversalType) => {
    setLoading(true);
    try {
      const data = await fetchRoute(src, dst, type);
      setResult(data);
    } catch (e) {
      console.error(e);
    } finally {
      setLoading(false);
    }
  }, [startId, targetId, traversalType]);

  useEffect(() => {
    handleExplore(startId, targetId, traversalType);
  }, [startId, targetId, traversalType, handleExplore]);

  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-slate-900">Explore Campus Reachability & Traversal</h2>
        <p className="text-slate-500 text-sm">
          Examine graph exploration using Depth-First Search (DFS) for reachable component tracking and Breadth-First Search (BFS) for minimal hop traversal.
        </p>
      </div>

      {/* Control Strip */}
      <div className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm flex flex-col md:flex-row items-center gap-4 justify-between">
        <div className="flex flex-wrap items-center gap-3 w-full md:w-auto">
          <div>
            <label className="block text-[11px] font-bold text-slate-500 uppercase tracking-wider mb-1">Starting Hub</label>
            <select
              value={startId}
              onChange={(e) => setStartId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-xs font-medium text-slate-800 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))
              }
            </select>
          </div>

          <div>
            <label className="block text-[11px] font-bold text-slate-500 uppercase tracking-wider mb-1">Target Hub</label>
            <select
              value={targetId}
              onChange={(e) => setTargetId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-xs font-medium text-slate-800 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))
              }
            </select>
          </div>

          <div>
            <label className="block text-[11px] font-bold text-slate-500 uppercase tracking-wider mb-1">Traversal Algorithm</label>
            <select
              value={traversalType}
              onChange={(e) => setTraversalType(e.target.value)}
              className="bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-xs font-medium text-slate-800"
            >
              <option value="dfs">Depth-First Search (DFS - Deep Exploration)</option>
              <option value="bfs">Breadth-First Search (BFS - Min Walkways)</option>
            </select>
          </div>
        </div>

        <button
          onClick={() => handleExplore(startId, targetId, traversalType)}
          disabled={loading}
          className="w-full md:w-auto px-5 py-2.5 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-xl text-sm transition flex items-center justify-center gap-2 shadow-sm"
        >
          <Play className="w-4 h-4" />
          <span>{loading ? 'Traversing...' : 'Run Traversal'}</span>
        </button>
      </div>

      {/* Main Grid: Interactive Map + Traversal Details */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        <div className="lg:col-span-8 bg-white rounded-2xl p-5 border border-slate-200 shadow-sm">
          <div className="flex items-center justify-between mb-3">
            <h3 className="font-bold text-slate-900 text-sm">Interactive Campus Graph</h3>
            {/* Category Filter Pills */}
            <div className="flex gap-1 overflow-x-auto max-w-md pb-1 text-[11px]">
              {categories.map(cat => (
                <button
                  key={cat}
                  onClick={() => setSelectedCategory(cat)}
                  className={`px-2 py-0.5 rounded-full font-medium transition ${
                    selectedCategory === cat ? 'bg-blue-600 text-white' : 'bg-slate-100 text-slate-600 hover:bg-slate-200'
                  }`}
                >
                  {cat}
                </button>
              ))}
            </div>
          </div>

          <CampusMap
            locations={locations}
            connections={connections}
            activePath={result?.found ? result.path : []}
            sourceId={startId}
            destId={targetId}
            highlightCategory={selectedCategory}
            height="480px"
            onSelectStart={(id) => setStartId(id)}
            onSelectDest={(id) => setTargetId(id)}
          />
        </div>

        {/* Traversal telemetry */}
        <div className="lg:col-span-4 space-y-4">
          <div className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm space-y-4">
            <h3 className="font-bold text-slate-900 text-sm flex items-center gap-2">
              <Compass className="w-4 h-4 text-blue-600" />
              <span>Algorithm Telemetry</span>
            </h3>

            {result ? (
              <div className="space-y-4">
                <div className="p-3 bg-slate-50 border border-slate-200/80 rounded-xl text-xs space-y-2">
                  <div className="flex justify-between">
                    <span className="text-slate-500">Algorithm:</span>
                    <span className="font-bold text-slate-800">{result.algorithm}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-slate-500">Reachability:</span>
                    <span className={`font-bold ${result.found ? 'text-emerald-600' : 'text-rose-600'}`}>
                      {result.found ? 'REACHABLE' : 'UNREACHABLE'}
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-slate-500">Nodes Explored:</span>
                    <span className="font-bold text-slate-800">{result.nodes_visited} vertices</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-slate-500">Edges Examined:</span>
                    <span className="font-bold text-slate-800">{result.edges_explored} edges</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-slate-500">Execution Time:</span>
                    <span className="font-mono text-slate-800">{result.execution_time_ms} ms</span>
                  </div>
                </div>

                <div>
                  <h4 className="font-bold text-xs text-slate-700 mb-2">Reconstructed Sequence:</h4>
                  <div className="flex flex-wrap gap-1.5 max-h-48 overflow-y-auto">
                    {result.path && result.path.map((nodeId, idx) => {
                      const loc = locations.find(l => l.id === nodeId);
                      return (
                        <span
                          key={idx}
                          className="px-2 py-1 bg-blue-50 border border-blue-200 text-blue-800 text-[11px] font-semibold rounded-lg flex items-center gap-1"
                        >
                          <span>[{nodeId}] {loc?.short_name || loc?.name}</span>
                          {idx < result.path.length - 1 && <span className="text-blue-400">→</span>}
                        </span>
                      );
                    })}
                  </div>
                </div>
              </div>
            ) : (
              <div className="text-center py-10 text-slate-400 text-xs">
                Run DFS or BFS traversal to view node visitation telemetry.
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
