import React, { useState, useEffect, useCallback } from 'react';
import { GitFork, Navigation, CheckCircle2, AlertCircle, Clock, Footprints, Layers, ArrowRight, ArrowLeftRight } from 'lucide-react';
import CampusMap from '../components/CampusMap';
import { fetchAlternatives } from '../services/api';

export default function AlternativeRoutes({ locations = [], connections = [] }) {
  const [sourceId, setSourceId] = useState(0);
  const [destId, setDestId] = useState(8);
  const [mode, setMode] = useState('distance');
  const [data, setData] = useState(null);
  const [selectedPath, setSelectedPath] = useState([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);

  const handleSearch = useCallback(async (src = sourceId, dst = destId, metric = mode) => {
    if (src === undefined || dst === undefined || src === null || dst === null) return;
    if (src === dst) {
      setError('Source and destination cannot be identical for alternative routing.');
      setData(null);
      setSelectedPath([]);
      return;
    }
    setLoading(true);
    setError(null);
    try {
      const res = await fetchAlternatives(src, dst, metric);
      setData(res);
      if (res.primary_route?.found) {
        setSelectedPath(res.primary_route.path);
      } else {
        setSelectedPath([]);
        setError('No available routes between these locations. Walkways may be blocked.');
      }
    } catch (err) {
      setError(err.message || 'Failed to fetch alternative routes.');
    } finally {
      setLoading(false);
    }
  }, [sourceId, destId, mode]);

  useEffect(() => {
    handleSearch(sourceId, destId, mode);
  }, [sourceId, destId, mode, handleSearch]);

  const handleSwap = () => {
    const s = destId;
    const d = sourceId;
    setSourceId(s);
    setDestId(d);
  };

  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-slate-900">Alternative Route Suggestions</h2>
        <p className="text-slate-500 text-sm">
          Calculates distinct detour candidates using the C++ edge-removal method to provide alternative paths when primary routes are congested or closed.
        </p>
      </div>

      {/* Control Strip */}
      <div className="bg-white rounded-2xl p-6 border border-slate-200 shadow-sm">
        <div className="grid grid-cols-1 md:grid-cols-12 gap-4 items-end">
          <div className="md:col-span-4">
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5 flex items-center gap-1.5">
              <span className="w-2.5 h-2.5 rounded-full bg-emerald-500"></span>
              <span>Start Location</span>
            </label>
            <select
              value={sourceId}
              onChange={(e) => setSourceId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading locations...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))
              }
            </select>
          </div>

          <div className="md:col-span-1 flex justify-center pb-1">
            <button
              onClick={handleSwap}
              title="Swap"
              className="p-2.5 bg-slate-100 hover:bg-blue-50 border border-slate-200 rounded-xl transition text-slate-600"
            >
              <ArrowLeftRight className="w-4 h-4" />
            </button>
          </div>

          <div className="md:col-span-4">
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5 flex items-center gap-1.5">
              <span className="w-2.5 h-2.5 rounded-full bg-rose-500"></span>
              <span>Destination Location</span>
            </label>
            <select
              value={destId}
              onChange={(e) => setDestId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading locations...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))
              }
            </select>
          </div>

          <div className="md:col-span-3">
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5">Optimization Metric</label>
            <select
              value={mode}
              onChange={(e) => setMode(e.target.value)}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800"
            >
              <option value="distance">Shortest Distance</option>
              <option value="time">Fastest Walking Time</option>
            </select>
          </div>
        </div>
      </div>

      {/* Main Grid: Interactive Map + Alternatives Cards */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        {/* Map View */}
        <div className="lg:col-span-7 bg-white rounded-2xl p-5 border border-slate-200 shadow-sm flex flex-col">
          <div className="flex items-center justify-between mb-3">
            <h3 className="font-bold text-slate-900 text-sm">Visual Path Comparison</h3>
            <span className="text-xs text-slate-500">Click any card on the right to preview on the map</span>
          </div>
          <CampusMap
            locations={locations}
            connections={connections}
            activePath={selectedPath}
            sourceId={sourceId}
            destId={destId}
            height="500px"
            onSelectStart={(id) => setSourceId(id)}
            onSelectDest={(id) => setDestId(id)}
          />
        </div>

        {/* Alternatives List */}
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

          {data && data.primary_route?.found && (
            <div className="space-y-3">
              {/* Primary Optimal Route */}
              <div
                onClick={() => setSelectedPath(data.primary_route.path)}
                className={`p-4 rounded-2xl border cursor-pointer transition ${
                  JSON.stringify(selectedPath) === JSON.stringify(data.primary_route.path)
                    ? 'bg-blue-50/80 border-blue-400 ring-2 ring-blue-500/20'
                    : 'bg-white border-slate-200 hover:border-slate-300'
                }`}
              >
                <div className="flex items-center justify-between mb-2">
                  <span className="px-2.5 py-0.5 bg-emerald-600 text-white text-[10px] font-bold uppercase tracking-wider rounded-full flex items-center gap-1">
                    <CheckCircle2 className="w-3 h-3" />
                    <span>Primary Route</span>
                  </span>
                  <span className="text-xs font-bold text-slate-900 font-mono">
                    {Math.round(data.primary_route.total_distance_m)}m • {data.primary_route.total_time_min?.toFixed(1)} min
                  </span>
                </div>

                <div className="text-xs text-slate-700 mb-2 font-medium">
                  {data.primary_route.path_names?.join(' → ')}
                </div>

                <div className="text-[11px] text-slate-400">
                  {data.primary_route.segments} walkway segments
                </div>
              </div>

              {/* Alternative Route Candidates */}
              {data.alternatives && data.alternatives.length > 0 ? (
                data.alternatives.map((alt, idx) => {
                  const isSelected = JSON.stringify(selectedPath) === JSON.stringify(alt.path);
                  const distDiff = alt.total_distance_m - data.primary_route.total_distance_m;
                  const timeDiff = alt.total_time_min - data.primary_route.total_time_min;

                  return (
                    <div
                      key={idx}
                      onClick={() => setSelectedPath(alt.path)}
                      className={`p-4 rounded-2xl border cursor-pointer transition ${
                        isSelected
                          ? 'bg-blue-50/80 border-blue-400 ring-2 ring-blue-500/20'
                          : 'bg-white border-slate-200 hover:border-slate-300'
                      }`}
                    >
                      <div className="flex items-center justify-between mb-2">
                        <span className="px-2 py-0.5 bg-blue-100 text-blue-700 text-[10px] font-bold rounded-md">
                          Alternative #{alt.rank}
                        </span>
                        <div className="text-right">
                          <span className="text-xs font-bold text-slate-900 font-mono">
                            {Math.round(alt.total_distance_m)}m ({distDiff > 0 ? `+${Math.round(distDiff)}m` : '0m'})
                          </span>
                          <span className="text-[10px] text-slate-400 block font-mono">
                            {alt.total_time_min?.toFixed(1)} min (+{timeDiff.toFixed(1)}m)
                          </span>
                        </div>
                      </div>

                      <div className="text-xs text-slate-700 mb-1.5 font-medium">
                        {alt.path_names?.join(' → ')}
                      </div>

                      <div className="text-[11px] text-slate-400">
                        {alt.segments} walkway segments
                      </div>
                    </div>
                  );
                })
              ) : (
                <div className="p-4 bg-slate-50 border border-slate-200 rounded-xl text-xs text-slate-500 text-center">
                  No distinct secondary alternatives found for this pair.
                </div>
              )}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
