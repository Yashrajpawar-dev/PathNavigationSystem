import React, { useState } from 'react';
import {
  MapPin,
  GitCommit,
  CheckCircle2,
  AlertOctagon,
  ArrowRight,
  Navigation,
  Compass,
  Building2,
  ShieldAlert
} from 'lucide-react';
import CampusMap from '../components/CampusMap';

export default function Dashboard({
  dashboardData,
  locations,
  connections,
  onNavigate,
  onQuickRoute
}) {
  const [quickSrc, setQuickSrc] = useState(0);
  const [quickDst, setQuickDst] = useState(8);

  const stats = [
    {
      label: 'Campus Facilities',
      value: dashboardData?.totalLocations ?? locations.length,
      icon: MapPin,
      color: 'text-blue-600',
      bg: 'bg-blue-50',
      desc: 'Vertices in Graph'
    },
    {
      label: 'Total Walkways',
      value: dashboardData?.totalWalkways ?? (connections.length / 2),
      icon: GitCommit,
      color: 'text-indigo-600',
      bg: 'bg-indigo-50',
      desc: 'Undirected Edges'
    },
    {
      label: 'Active Walkways',
      value: dashboardData?.activeWalkways ?? (connections.filter(c => !c.blocked).length / 2),
      icon: CheckCircle2,
      color: 'text-emerald-600',
      bg: 'bg-emerald-50',
      desc: 'Available for Routing'
    },
    {
      label: 'Blocked Walkways',
      value: dashboardData?.blockedWalkways ?? (connections.filter(c => c.blocked).length / 2),
      icon: AlertOctagon,
      color: 'text-rose-600',
      bg: 'bg-rose-50',
      desc: 'Temporarily Closed'
    }
  ];

  const popularLocations = [
    { id: 0, name: 'Main Gate', category: 'Gate' },
    { id: 8, name: 'Central Library', category: 'Library' },
    { id: 9, name: 'Canteen / Food Court', category: 'Facility' },
    { id: 18, name: 'Medical Centre', category: 'Health' },
    { id: 3, name: 'Computer Engg Dept', category: 'Department' },
    { id: 11, name: 'Computer Labs', category: 'Laboratory' },
  ];

  return (
    <div className="space-y-6">
      {/* Top Banner */}
      <div className="bg-gradient-to-r from-blue-900 to-indigo-900 rounded-2xl p-6 text-white shadow-xl flex flex-col md:flex-row justify-between items-start md:items-center gap-4">
        <div>
          <span className="inline-block px-3 py-1 bg-blue-500/20 text-blue-300 rounded-full text-xs font-semibold uppercase tracking-wider mb-2">
            VIT Pune • Campus Navigation Engine
          </span>
          <h2 className="text-2xl font-bold">Smart Campus Navigation Dashboard</h2>
          <p className="text-blue-200 text-sm mt-1 max-w-xl">
            Real-time graph navigation powered by native C++17 DSA algorithms (Dijkstra, BFS, DFS, and edge-removal rerouting).
          </p>
        </div>
        <div className="flex gap-2">
          <button
            onClick={() => onNavigate('find-route')}
            className="px-4 py-2.5 bg-blue-600 hover:bg-blue-500 text-white font-medium rounded-xl text-sm transition flex items-center gap-2 shadow-md shadow-blue-600/30"
          >
            <Navigation className="w-4 h-4" />
            <span>Find Route</span>
          </button>
          <button
            onClick={() => onNavigate('manage-walkways')}
            className="px-4 py-2.5 bg-white/10 hover:bg-white/20 text-white font-medium rounded-xl text-sm transition flex items-center gap-2 backdrop-blur"
          >
            <ShieldAlert className="w-4 h-4" />
            <span>Manage Walkways</span>
          </button>
        </div>
      </div>

      {/* Metric Summary Cards */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        {stats.map((s, idx) => {
          const Icon = s.icon;
          return (
            <div key={idx} className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm hover:shadow transition">
              <div className="flex items-center justify-between mb-3">
                <span className="text-xs font-medium text-slate-500 uppercase tracking-wider">{s.label}</span>
                <div className={`p-2 rounded-xl ${s.bg} ${s.color}`}>
                  <Icon className="w-5 h-5" />
                </div>
              </div>
              <div className="text-3xl font-extrabold text-slate-900">{s.value}</div>
              <div className="text-xs text-slate-400 mt-1">{s.desc}</div>
            </div>
          );
        })}
      </div>

      {/* Main Graph Preview + Quick Route Finder */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Quick Navigation Widget & Shortcuts */}
        <div className="space-y-6">
          {/* Quick Route Card */}
          <div className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm">
            <h3 className="font-bold text-slate-900 text-base mb-4 flex items-center gap-2">
              <Navigation className="w-5 h-5 text-blue-600" />
              <span>Quick Route Search</span>
            </h3>

            <div className="space-y-3">
              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Starting Point</label>
                <select
                  value={quickSrc}
                  onChange={(e) => setQuickSrc(parseInt(e.target.value))}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
                >
                  {locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))}
                </select>
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Destination</label>
                <select
                  value={quickDst}
                  onChange={(e) => setQuickDst(parseInt(e.target.value))}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
                >
                  {locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))}
                </select>
              </div>

              <button
                onClick={() => onQuickRoute(quickSrc, quickDst)}
                className="w-full py-2.5 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-xl text-sm transition flex items-center justify-center gap-2 shadow-sm"
              >
                <span>Compute Optimal Route</span>
                <ArrowRight className="w-4 h-4" />
              </button>
            </div>
          </div>

          {/* Popular Locations */}
          <div className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm">
            <h3 className="font-bold text-slate-900 text-base mb-3 flex items-center gap-2">
              <MapPin className="w-5 h-5 text-indigo-600" />
              <span>Key Campus Hubs</span>
            </h3>
            <div className="grid grid-cols-2 gap-2">
              {popularLocations.map((hub) => (
                <button
                  key={hub.id}
                  onClick={() => onQuickRoute(0, hub.id)}
                  className="text-left p-2.5 bg-slate-50 hover:bg-blue-50 border border-slate-200/80 hover:border-blue-300 rounded-xl transition text-xs group"
                >
                  <div className="font-semibold text-slate-800 group-hover:text-blue-700 truncate">{hub.name}</div>
                  <div className="text-[10px] text-slate-400 mt-0.5">{hub.category}</div>
                </button>
              ))}
            </div>
          </div>
        </div>

        {/* Right: Interactive Graph Preview */}
        <div className="lg:col-span-2 bg-white rounded-2xl p-5 border border-slate-200 shadow-sm flex flex-col justify-between">
          <div className="flex items-center justify-between mb-4">
            <div>
              <h3 className="font-bold text-slate-900 text-base">Campus Graph Overview</h3>
              <p className="text-xs text-slate-500">Interactive SVG topology of VIT Pune Bibwewadi campus</p>
            </div>
            <button
              onClick={() => onNavigate('explore')}
              className="text-xs text-blue-600 hover:text-blue-800 font-semibold flex items-center gap-1"
            >
              <span>Full Visualizer</span>
              <ArrowRight className="w-3.5 h-3.5" />
            </button>
          </div>

          <div className="w-full flex-1">
            <CampusMap
              locations={locations}
              connections={connections}
              height="420px"
              sourceId={quickSrc}
              destId={quickDst}
              onSelectNode={(id) => {
                if (quickSrc === null || quickSrc === id) {
                  setQuickSrc(id);
                } else {
                  setQuickDst(id);
                }
              }}
            />
          </div>
        </div>
      </div>
    </div>
  );
}
