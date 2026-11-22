import React, { useState, useEffect, useCallback } from 'react';
import Sidebar from './components/Sidebar';
import Dashboard from './pages/Dashboard';
import FindRoute from './pages/FindRoute';
import ExploreCampus from './pages/ExploreCampus';
import NearestFacilities from './pages/NearestFacilities';
import AlternativeRoutes from './pages/AlternativeRoutes';
import ManageWalkways from './pages/ManageWalkways';
import CampusLocations from './pages/CampusLocations';
import { fetchLocations, fetchConnections, fetchDashboardStats } from './services/api';
import { RefreshCw, AlertCircle } from 'lucide-react';

export default function App() {
  const [activeTab, setActiveTab] = useState('dashboard');
  const [locations, setLocations] = useState([]);
  const [connections, setConnections] = useState([]);
  const [dashboardData, setDashboardData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [routeParams, setRouteParams] = useState({ src: 0, dst: 8 });

  const loadData = useCallback(async () => {
    try {
      setLoading(true);
      setError(null);
      const [locsRes, connsRes, dashRes] = await Promise.all([
        fetchLocations(),
        fetchConnections(),
        fetchDashboardStats()
      ]);

      const locs = Array.isArray(locsRes) ? locsRes : (locsRes?.locations || []);
      const conns = Array.isArray(connsRes) ? connsRes : (connsRes?.connections || []);
      console.log(`[App] Loaded ${locs.length} locations and ${conns.length} connections from C++ backend.`);
      setLocations(locs);
      setConnections(conns);
      setDashboardData(dashRes);
    } catch (err) {
      console.error('Failed to load campus graph data:', err);
      setError(err.message || 'Unable to connect to the backend server. Ensure node backend/server.js is running.');
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    loadData();
  }, [loadData]);

  const handleQuickRoute = (src, dst) => {
    setRouteParams({ src, dst });
    setActiveTab('find-route');
  };

  const handleFindRouteTo = (dstId) => {
    setRouteParams((prev) => ({ ...prev, dst: dstId }));
    setActiveTab('find-route');
  };

  return (
    <div className="flex h-screen bg-[#F8FAFC] text-slate-800 font-sans overflow-hidden">
      {/* Left Sidebar */}
      <Sidebar activeTab={activeTab} setActiveTab={setActiveTab} />

      {/* Main Content Area */}
      <div className="flex-1 flex flex-col min-w-0 overflow-hidden">
        {/* Top Navbar */}
        <header className="h-16 bg-white border-b border-gray-200 px-8 flex items-center justify-between shrink-0 shadow-sm">
          <div className="flex items-center gap-3">
            <span className="text-xs font-semibold uppercase tracking-wider text-blue-600 bg-blue-50 px-2.5 py-1 rounded-md border border-blue-200">
              Vishwakarma Institute of Technology
            </span>
            <span className="text-gray-300">/</span>
            <span className="text-sm font-semibold text-gray-700 capitalize">
              {activeTab.replace('-', ' ')}
            </span>
          </div>

          <div className="flex items-center gap-4">
            <button
              onClick={loadData}
              disabled={loading}
              title="Refresh Graph State from C++ Engine"
              className="inline-flex items-center gap-1.5 px-3 py-1.5 rounded-lg border border-gray-200 bg-gray-50 text-gray-700 text-xs font-medium hover:bg-gray-100 transition-colors disabled:opacity-50"
            >
              <RefreshCw className={`w-3.5 h-3.5 ${loading ? 'animate-spin text-blue-600' : ''}`} />
              <span>Sync Graph</span>
            </button>
            <div className="hidden sm:flex items-center gap-2 pl-3 border-l border-gray-200 text-xs text-gray-500">
              <span className="w-2 h-2 rounded-full bg-emerald-500"></span>
              <span className="font-medium text-gray-700">{locations.length} Locations</span>
              <span className="text-gray-300">•</span>
              <span className="font-medium text-gray-700">{connections.length} Walkways</span>
            </div>
          </div>
        </header>

        {/* Global Error Banner */}
        {error && (
          <div className="bg-red-50 border-b border-red-200 px-8 py-3 flex items-center justify-between text-red-800 text-sm">
            <div className="flex items-center gap-2">
              <AlertCircle className="w-4 h-4 text-red-600 shrink-0" />
              <span>{error}</span>
            </div>
            <button
              onClick={loadData}
              className="text-xs font-semibold underline hover:text-red-900 ml-4 shrink-0"
            >
              Retry
            </button>
          </div>
        )}

        {/* Page Content Scroll Container */}
        <main className="flex-1 overflow-y-auto p-8">
          {loading && locations.length === 0 ? (
            <div className="h-full flex flex-col items-center justify-center text-gray-400 space-y-3">
              <RefreshCw className="w-8 h-8 animate-spin text-blue-600" />
              <p className="text-sm font-medium text-gray-600">Initializing C++ Campus Graph Engine...</p>
            </div>
          ) : (
            <>
              {activeTab === 'dashboard' && (
                <Dashboard
                  dashboardData={dashboardData}
                  locations={locations}
                  connections={connections}
                  onNavigate={setActiveTab}
                  onQuickRoute={handleQuickRoute}
                />
              )}

              {activeTab === 'find-route' && (
                <FindRoute
                  locations={locations}
                  connections={connections}
                  initialSrc={routeParams.src}
                  initialDst={routeParams.dst}
                />
              )}

              {activeTab === 'explore' && (
                <ExploreCampus
                  locations={locations}
                  connections={connections}
                />
              )}

              {activeTab === 'nearest' && (
                <NearestFacilities
                  locations={locations}
                  connections={connections}
                />
              )}

              {activeTab === 'alternatives' && (
                <AlternativeRoutes
                  locations={locations}
                  connections={connections}
                />
              )}

              {activeTab === 'manage-walkways' && (
                <ManageWalkways
                  locations={locations}
                  connections={connections}
                  onRefresh={loadData}
                />
              )}

              {activeTab === 'locations' && (
                <CampusLocations
                  locations={locations}
                  onRefresh={loadData}
                  onFindRouteTo={handleFindRouteTo}
                />
              )}
            </>
          )}
        </main>
      </div>
    </div>
  );
}
