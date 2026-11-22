import React from 'react';
import {
  LayoutDashboard,
  Navigation,
  Compass,
  Building2,
  GitFork,
  ShieldAlert,
  MapPin,
  GraduationCap
} from 'lucide-react';

const NAV_ITEMS = [
  { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard },
  { id: 'find-route', label: 'Find Route', icon: Navigation },
  { id: 'explore', label: 'Explore Campus', icon: Compass },
  { id: 'nearest', label: 'Nearest Facilities', icon: Building2 },
  { id: 'alternatives', label: 'Alternative Routes', icon: GitFork },
  { id: 'manage-walkways', label: 'Manage Walkways', icon: ShieldAlert },
  { id: 'locations', label: 'Campus Locations', icon: MapPin },
];

export default function Sidebar({ activeTab, setActiveTab }) {
  return (
    <aside className="w-64 bg-slate-900 border-r border-slate-800 flex flex-col justify-between shrink-0 select-none">
      <div>
        {/* Brand Header */}
        <div className="p-6 border-b border-slate-800 flex items-center gap-3">
          <div className="w-10 h-10 rounded-xl bg-blue-600 flex items-center justify-center text-white shadow-lg shadow-blue-500/20">
            <GraduationCap className="w-6 h-6" />
          </div>
          <div>
            <h1 className="font-bold text-white text-base leading-tight">VIT Navigator</h1>
            <p className="text-xs text-blue-400 font-medium">DSA Campus Core</p>
          </div>
        </div>

        {/* Navigation Menu */}
        <nav className="p-3 space-y-1">
          {NAV_ITEMS.map((item) => {
            const Icon = item.icon;
            const isActive = activeTab === item.id;
            return (
              <button
                key={item.id}
                onClick={() => setActiveTab(item.id)}
                className={`w-full flex items-center gap-3 px-3.5 py-2.5 rounded-xl font-medium text-sm transition-all duration-150 ${
                  isActive
                    ? 'bg-blue-600 text-white shadow-md shadow-blue-600/30'
                    : 'text-slate-400 hover:text-slate-100 hover:bg-slate-800/60'
                }`}
              >
                <Icon className={`w-5 h-5 ${isActive ? 'text-white' : 'text-slate-400'}`} />
                <span>{item.label}</span>
              </button>
            );
          })}
        </nav>
      </div>

      {/* Footer Info */}
      <div className="p-4 border-t border-slate-800 text-xs text-slate-500">
        <div className="flex items-center gap-2 mb-1">
          <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse"></span>
          <span className="text-slate-300 font-medium">C++ Engine Connected</span>
        </div>
        <p className="text-[11px]">VIT Pune • Bibwewadi Campus</p>
      </div>
    </aside>
  );
}
