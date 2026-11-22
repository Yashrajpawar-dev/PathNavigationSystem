import React from 'react';
import { 
  BookOpen, 
  Cpu, 
  GitBranch, 
  Layers, 
  ShieldCheck, 
  Terminal, 
  CheckCircle2, 
  Network, 
  Zap, 
  Clock, 
  Compass,
  FileCode2,
  Server
} from 'lucide-react';

export default function AboutProject() {
  const algorithms = [
    {
      name: "Dijkstra's Shortest Path",
      complexity: "O((V + E) log V)",
      space: "O(V)",
      purpose: "Calculates the globally optimal path between any two campus facilities based on either physical distance (meters) or estimated walking time (seconds).",
      dataStructure: "std::priority_queue (Min-Heap), Adjacency List",
      badge: "Optimal Cost"
    },
    {
      name: "Breadth-First Search (BFS)",
      complexity: "O(V + E)",
      space: "O(V)",
      purpose: "Traverses the campus graph level-by-level to determine the minimum number of walkway hops between buildings, regardless of distance.",
      dataStructure: "std::queue, Visited Array, Parent Map",
      badge: "Fewest Walkways"
    },
    {
      name: "Depth-First Search (DFS)",
      complexity: "O(V + E)",
      space: "O(V)",
      purpose: "Explores paths deeply to verify reachability, connected components, and detect alternative campus routes.",
      dataStructure: "Call Stack / Recursion, Visited Boolean Vector",
      badge: "Reachability"
    },
    {
      name: "Alternative Routes (Edge-Removal)",
      complexity: "O(K · (V + E) log V)",
      space: "O(V + E)",
      purpose: "Generates distinct detour routes by iteratively blocking edges along the primary shortest path, recalculating Dijkstra, and restoring original states.",
      dataStructure: "Edge Restoration Stack, Path De-duplication Set",
      badge: "Detour Candidates"
    },
    {
      name: "Nearest Facility Locator",
      complexity: "O((V + E) log V)",
      space: "O(V)",
      purpose: "Runs single-source Dijkstra from the user's location to scan all locations matching a requested facility category (e.g. Canteen, Library, Lab, Medical).",
      dataStructure: "Single-Source Min-Heap Dijkstra with Target Filtering",
      badge: "Targeted Search"
    },
    {
      name: "Dynamic Obstacle Management",
      complexity: "O(deg(u) + deg(v))",
      space: "O(1) runtime, JSON disk persistence",
      purpose: "Dynamically blocks or restores walkways during campus events, construction, or emergencies. Automatically persists to disk for seamless reload.",
      dataStructure: "Adjacency List in-place flag modification",
      badge: "Real-Time Updates"
    }
  ];

  const techStack = [
    {
      layer: "Core Engine",
      tech: "C++17 Native DSA Binary",
      desc: "Fast native execution of all graph structures, adjacency lists, and priority-queue algorithms compiled via GCC with -O2 optimization.",
      icon: Terminal,
      color: "border-blue-500 bg-blue-50 text-blue-700"
    },
    {
      layer: "Bridge Layer",
      tech: "Node.js & Express REST API",
      desc: "Lightweight, zero-overhead asynchronous bridge invoking C++ CLI subcommands with JSON serialization and disk-backed state synchronization.",
      icon: Server,
      color: "border-emerald-500 bg-emerald-50 text-emerald-700"
    },
    {
      layer: "Client Interface",
      tech: "React 18 + Vite + Tailwind CSS",
      desc: "Modern responsive dashboard inspired by university portals with interactive SVG graph visualizer, live filtering, and turn-by-turn guidance.",
      icon: Layers,
      color: "border-indigo-500 bg-indigo-50 text-indigo-700"
    }
  ];

  return (
    <div className="space-y-8 max-w-6xl mx-auto pb-12">
      {/* Header Banner */}
      <div className="bg-gradient-to-r from-[#1E3A8A] to-[#3B82F6] rounded-2xl p-8 text-white shadow-lg">
        <div className="flex items-center gap-3 mb-2">
          <span className="px-3 py-1 bg-white/20 text-white text-xs font-semibold rounded-full uppercase tracking-wider">
            DSA Course Project & System Architecture
          </span>
          <span className="px-3 py-1 bg-emerald-500/30 text-emerald-200 text-xs font-semibold rounded-full flex items-center gap-1">
            <CheckCircle2 className="w-3.5 h-3.5" /> Phase 1 & 2 Verified
          </span>
        </div>
        <h1 className="text-3xl font-extrabold tracking-tight">Campus Navigation System</h1>
        <p className="mt-2 text-blue-100 max-w-3xl leading-relaxed text-sm md:text-base">
          A hybrid native C++ Data Structures & Algorithms calculation core coupled with a high-performance 
          React web interface. All routing, reachability, alternative route calculations, and obstacle updates 
          are executed directly by compiled C++ algorithms—preserving algorithmic integrity as the single source of truth.
        </p>
      </div>

      {/* Architecture Cards */}
      <div>
        <h2 className="text-xl font-bold text-gray-900 mb-4 flex items-center gap-2">
          <Cpu className="w-5 h-5 text-[#1E3A8A]" />
          System Architecture & Data Flow
        </h2>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          {techStack.map((item, idx) => {
            const Icon = item.icon;
            return (
              <div key={idx} className="bg-white rounded-xl border border-gray-200 p-6 shadow-sm flex flex-col justify-between">
                <div>
                  <div className={`w-10 h-10 rounded-lg flex items-center justify-center mb-4 ${item.color}`}>
                    <Icon className="w-5 h-5" />
                  </div>
                  <span className="text-xs font-semibold text-gray-500 uppercase tracking-wider">{item.layer}</span>
                  <h3 className="text-lg font-bold text-gray-900 mt-1 mb-2">{item.tech}</h3>
                  <p className="text-sm text-gray-600 leading-relaxed">{item.desc}</p>
                </div>
                <div className="mt-4 pt-3 border-t border-gray-100 flex items-center gap-1.5 text-xs text-gray-500 font-medium">
                  <ShieldCheck className="w-4 h-4 text-emerald-600" />
                  <span>Production Ready</span>
                </div>
              </div>
            );
          })}
        </div>
      </div>

      {/* Algorithmic Complexity Table */}
      <div>
        <h2 className="text-xl font-bold text-gray-900 mb-4 flex items-center gap-2">
          <GitBranch className="w-5 h-5 text-[#1E3A8A]" />
          Core Graph Algorithms & Time Complexity
        </h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          {algorithms.map((algo, idx) => (
            <div key={idx} className="bg-white rounded-xl border border-gray-200 p-6 shadow-sm hover:border-blue-300 transition-all">
              <div className="flex items-start justify-between mb-3">
                <h3 className="text-base font-bold text-gray-900">{algo.name}</h3>
                <span className="px-2.5 py-0.5 text-xs font-semibold bg-blue-50 text-blue-700 rounded-full border border-blue-200">
                  {algo.badge}
                </span>
              </div>
              <p className="text-sm text-gray-600 mb-4">{algo.purpose}</p>

              <div className="grid grid-cols-2 gap-3 pt-3 border-t border-gray-100 text-xs">
                <div>
                  <span className="text-gray-400 block font-medium">Time Complexity</span>
                  <span className="font-mono font-semibold text-gray-800">{algo.complexity}</span>
                </div>
                <div>
                  <span className="text-gray-400 block font-medium">Auxiliary Space</span>
                  <span className="font-mono font-semibold text-gray-800">{algo.space}</span>
                </div>
                <div className="col-span-2">
                  <span className="text-gray-400 block font-medium">Data Structure</span>
                  <span className="font-medium text-gray-700">{algo.dataStructure}</span>
                </div>
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Key Architectural Principles */}
      <div className="bg-white rounded-xl border border-gray-200 p-6 shadow-sm">
        <h2 className="text-lg font-bold text-gray-900 mb-4 flex items-center gap-2">
          <ShieldCheck className="w-5 h-5 text-emerald-600" />
          Key DSA Engineering Decisions
        </h2>
        <div className="space-y-3 text-sm text-gray-700">
          <div className="flex items-start gap-2.5">
            <CheckCircle2 className="w-4 h-4 text-emerald-600 mt-0.5 flex-shrink-0" />
            <div>
              <strong>Single Source of Truth:</strong> Graph computations are never simulated or duplicated in JavaScript. Every route calculation, reachability tree, and detour candidate comes directly from native compiled C++ logic.
            </div>
          </div>
          <div className="flex items-start gap-2.5">
            <CheckCircle2 className="w-4 h-4 text-emerald-600 mt-0.5 flex-shrink-0" />
            <div>
              <strong>Non-Destructive Edge Removal:</strong> Alternative route suggestions temporarily toggle walkway flags and immediately restore them in reverse order, ensuring graph structural integrity even under concurrent searches.
            </div>
          </div>
          <div className="flex items-start gap-2.5">
            <CheckCircle2 className="w-4 h-4 text-emerald-600 mt-0.5 flex-shrink-0" />
            <div>
              <strong>Two-Way Synchronization:</strong> Undirected walkways maintain synchronized references in both vertex adjacency lists, preventing directional discrepancies when blocking obstacles.
            </div>
          </div>
          <div className="flex items-start gap-2.5">
            <CheckCircle2 className="w-4 h-4 text-emerald-600 mt-0.5 flex-shrink-0" />
            <div>
              <strong>Graceful Disconnection Handling:</strong> Validates reachability across both connected and partitioned graphs, providing informative feedback rather than infinite loops or unhandled exceptions.
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
