import React, { useState, useEffect, useCallback } from 'react';
import { Building2, Navigation, Clock, Footprints, AlertCircle, ArrowRight, CheckCircle2 } from 'lucide-react';
import CampusMap from '../components/CampusMap';
import { fetchNearestFacility, fetchRoute } from '../services/api';

const FACILITY_PRESETS = [
  { label: 'Central Library', category: 'Library', icon: '📚' },
  { label: 'Canteen / Food Court', category: 'Facility', icon: '🍽️' },
  { label: 'Medical Centre', category: 'Health', icon: '🏥' },
  { label: 'Academic Departments', category: 'Department', icon: '🏫' },
  { label: 'Computer Laboratories', category: 'Laboratory', icon: '💻' },
  { label: 'Sports & Gymnasium', category: 'Sports', icon: '⚽' },
  { label: 'Vehicle Parking', category: 'Parking', icon: '🚗' },
  { label: 'Student Hostels', category: 'Hostel', icon: '🏠' },
  { label: 'Administrative Block', category: 'Administration', icon: '🏛️' },
];

export default function NearestFacilities({ locations = [], connections = [] }) {
  const [sourceId, setSourceId] = useState(0); // Main Gate default
  const [category, setCategory] = useState('Library');
  const [mode, setMode] = useState('distance'); // distance | time
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState(null);
  const [allMatches, setAllMatches] = useState([]);
  const [error, setError] = useState(null);

  const handleSearch = useCallback(async (src = sourceId, targetCategory = category, metric = mode) => {
    if (src === undefined || src === null) return;
    setLoading(true);
    setError(null);
    try {
      const data = await fetchNearestFacility(src, targetCategory, metric);
      setResult(data);

      if (!data.found) {
        setError(`No reachable facility found in category "${targetCategory}". Walkways may be blocked.`);
        setAllMatches([]);
      } else {
        const matchingLocs = locations.filter(l =>
          l.category?.toLowerCase() === targetCategory.toLowerCase() ||
          l.name?.toLowerCase().includes(targetCategory.toLowerCase())
        );

        const evaluated = await Promise.all(
          matchingLocs.map(async (loc) => {
            try {
              const r = await fetchRoute(src, loc.id, metric);
              return { ...loc, routeInfo: r };
            } catch {
              return { ...loc, routeInfo: null };
            }
          })
        );
        evaluated.sort((a, b) => (a.routeInfo?.total_distance_m || 9999) - (b.routeInfo?.total_distance_m || 9999));
        setAllMatches(evaluated);
      }
    } catch (err) {
      setError(err.message || 'Failed to search nearest facility.');
    } finally {
      setLoading(false);
    }
  }, [sourceId, category, mode, locations]);

  useEffect(() => {
    handleSearch(sourceId, category, mode);
  }, [sourceId, category, mode, handleSearch]);

  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-slate-900">Nearest Campus Facility Finder</h2>
        <p className="text-slate-500 text-sm">
          Uses single-source Dijkstra to scan all facilities matching your requested category and routes you to the closest reachable one.
        </p>
      </div>

      {/* Control Strip */}
      <div className="bg-white rounded-2xl p-6 border border-slate-200 shadow-sm space-y-4">
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4 items-end">
          <div>
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5 flex items-center gap-1.5">
              <span className="w-2.5 h-2.5 rounded-full bg-emerald-500"></span>
              <span>Your Current Location (Start)</span>
            </label>
            <select
              value={sourceId}
              onChange={(e) => setSourceId(parseInt(e.target.value))}
              disabled={locations.length === 0}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:opacity-60"
            >
              {locations.length === 0
                ? <option value="">⏳ Loading locations...</option>
                : locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name} ({loc.category})</option>
                  ))
              }
            </select>
          </div>

          <div>
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5">
              Facility Category
            </label>
            <select
              value={category}
              onChange={(e) => setCategory(e.target.value)}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
            >
              {FACILITY_PRESETS.map((p, idx) => (
                <option key={idx} value={p.category}>{p.icon} {p.label}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-xs font-bold text-slate-700 uppercase tracking-wider mb-1.5">
              Optimization Metric
            </label>
            <select
              value={mode}
              onChange={(e) => setMode(e.target.value)}
              className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3.5 py-2.5 text-sm font-medium text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
            >
              <option value="distance">Shortest Distance</option>
              <option value="time">Fastest Walking Time</option>
            </select>
          </div>
        </div>

        {/* Category Quick Chips */}
        <div className="pt-2 border-t border-slate-100 flex items-center gap-2 overflow-x-auto">
          <span className="text-xs text-slate-400 font-medium shrink-0">Quick Categories:</span>
          {FACILITY_PRESETS.map((p, idx) => (
            <button
              key={idx}
              onClick={() => setCategory(p.category)}
              className={`px-3 py-1 rounded-full text-xs font-medium whitespace-nowrap transition flex items-center gap-1 ${
                category === p.category
                  ? 'bg-blue-600 text-white shadow-sm'
                  : 'bg-slate-100 hover:bg-slate-200 text-slate-700'
              }`}
            >
              <span>{p.icon}</span>
              <span>{p.label}</span>
            </button>
          ))}
        </div>
      </div>

      {/* Results View: Map + Cards */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        {/* Left: Map with path */}
        <div className="lg:col-span-7 bg-white rounded-2xl p-5 border border-slate-200 shadow-sm">
          <h3 className="font-bold text-slate-900 text-sm mb-3">
            {result?.found
              ? `Route to Nearest ${category}: [${result.nearest_facility_id}] ${result.nearest_facility_name}`
              : 'Select starting location and category'}
          </h3>
          <CampusMap
            locations={locations}
            connections={connections}
            activePath={result?.found ? result.path : []}
            sourceId={sourceId}
            destId={result?.nearest_facility_id}
            height="480px"
            onSelectStart={(id) => setSourceId(id)}
          />
        </div>

        {/* Right: Results Breakdown */}
        <div className="lg:col-span-5 space-y-4">
          {error && (
            <div className="bg-rose-50 border border-rose-200 text-rose-800 rounded-2xl p-5 flex items-start gap-3">
              <AlertCircle className="w-5 h-5 text-rose-600 shrink-0 mt-0.5" />
              <div>
                <h4 className="font-bold text-sm">No Facility Reachable</h4>
                <p className="text-xs mt-1 text-rose-700">{error}</p>
              </div>
            </div>
          )}

          {result && result.found && (
            <div className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm space-y-4">
              <div className="p-4 bg-emerald-50 border border-emerald-200 rounded-xl flex items-center justify-between">
                <div>
                  <span className="text-[11px] font-bold text-emerald-700 uppercase tracking-wider block">Nearest Match</span>
                  <h4 className="text-lg font-extrabold text-emerald-950">
                    [{result.nearest_facility_id}] {result.nearest_facility_name}
                  </h4>
                  <span className="text-xs text-emerald-700 font-medium">{result.category}</span>
                </div>
                <div className="text-right">
                  <div className="text-2xl font-black text-emerald-900">{Math.round(result.total_distance_m)}m</div>
                  <div className="text-xs text-emerald-700 font-semibold">{result.total_time_min?.toFixed(1)} min</div>
                </div>
              </div>

              {/* All matching facilities list */}
              {allMatches.length > 0 && (
                <div>
                  <h4 className="font-bold text-xs text-slate-700 uppercase tracking-wider mb-2">
                    All Facilities in Category ({allMatches.length})
                  </h4>
                  <div className="space-y-2 max-h-64 overflow-y-auto pr-1">
                    {allMatches.map((item) => {
                      const isClosest = item.id === result.nearest_facility_id;
                      return (
                        <div
                          key={item.id}
                          className={`p-3 rounded-xl border flex items-center justify-between text-xs transition ${
                            isClosest
                              ? 'bg-blue-50 border-blue-300 ring-1 ring-blue-400'
                              : 'bg-slate-50 border-slate-200 text-slate-700'
                          }`}
                        >
                          <div>
                            <div className="font-bold text-slate-900 flex items-center gap-1.5">
                              <span>[{item.id}] {item.name}</span>
                              {isClosest && (
                                <span className="px-1.5 py-0.5 bg-blue-600 text-white text-[9px] font-bold rounded">
                                  CLOSEST
                                </span>
                              )}
                            </div>
                            <span className="text-[10px] text-slate-400">{item.short_name}</span>
                          </div>

                          <div className="text-right font-medium">
                            {item.routeInfo?.found ? (
                              <div>
                                <span className="font-bold text-slate-900">{Math.round(item.routeInfo.total_distance_m)}m</span>
                                <span className="text-slate-400 block text-[10px]">{item.routeInfo.total_time_min?.toFixed(1)} min</span>
                              </div>
                            ) : (
                              <span className="text-rose-500 font-semibold">Unreachable</span>
                            )}
                          </div>
                        </div>
                      );
                    })}
                  </div>
                </div>
              )}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
