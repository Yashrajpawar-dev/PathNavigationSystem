import React, { useState } from 'react';
import { ShieldAlert, AlertTriangle, CheckCircle, RefreshCw, Plus, Search, Filter } from 'lucide-react';
import { blockWalkway, restoreWalkway, restoreAllWalkways, addWalkway } from '../services/api';

export default function ManageWalkways({ locations = [], connections = [], onRefresh }) {
  const [search, setSearch] = useState('');
  const [filterStatus, setFilterStatus] = useState('ALL'); // ALL | ACTIVE | BLOCKED
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState(null);
  const [showAddModal, setShowAddModal] = useState(false);

  // Add walkway form state
  const [newFrom, setNewFrom] = useState(0);
  const [newTo, setNewTo] = useState(1);
  const [newDist, setNewDist] = useState(100);
  const [newType, setNewType] = useState('paved_path');

  const handleToggleBlock = async (conn) => {
    const isCurrentlyBlocked = conn.blocked;
    const actionText = isCurrentlyBlocked ? 'RESTORE (re-open)' : 'BLOCK (close)';
    const confirmed = window.confirm(
      `Are you sure you want to ${actionText} the walkway between [${conn.from_id}] and [${conn.to_id}]?`
    );
    if (!confirmed) return;

    setLoading(true);
    setMessage(null);
    try {
      if (isCurrentlyBlocked) {
        await restoreWalkway(conn.from_id, conn.to_id);
        setMessage({ type: 'success', text: `Walkway between [${conn.from_id}] and [${conn.to_id}] successfully restored!` });
      } else {
        await blockWalkway(conn.from_id, conn.to_id);
        setMessage({ type: 'warning', text: `Walkway between [${conn.from_id}] and [${conn.to_id}] is now BLOCKED! Routing will bypass it.` });
      }
      if (onRefresh) await onRefresh();
    } catch (err) {
      setMessage({ type: 'error', text: err.message || 'Operation failed.' });
    } finally {
      setLoading(false);
    }
  };

  const handleRestoreAll = async () => {
    const confirmed = window.confirm('Are you sure you want to restore ALL currently blocked walkways?');
    if (!confirmed) return;

    setLoading(true);
    setMessage(null);
    try {
      await restoreAllWalkways();
      setMessage({ type: 'success', text: 'All walkways restored to active status!' });
      if (onRefresh) await onRefresh();
    } catch (err) {
      setMessage({ type: 'error', text: err.message || 'Failed to restore walkways.' });
    } finally {
      setLoading(false);
    }
  };

  const handleAddWalkway = async (e) => {
    e.preventDefault();
    if (newFrom === newTo) {
      alert('Source and destination cannot be identical (no self-loops).');
      return;
    }
    setLoading(true);
    try {
      await addWalkway({
        from_id: newFrom,
        to_id: newTo,
        distance_m: parseFloat(newDist),
        path_type: newType,
        description: `Custom walkway ${newFrom} <-> ${newTo}`
      });
      setShowAddModal(false);
      setMessage({ type: 'success', text: `Walkway [${newFrom}] <-> [${newTo}] added successfully to C++ graph!` });
      if (onRefresh) await onRefresh();
    } catch (err) {
      alert(err.message || 'Failed to add walkway');
    } finally {
      setLoading(false);
    }
  };

  // Filter connections (showing each undirected edge once: from_id < to_id)
  const uniqueConnections = connections.filter(c => c.from_id < c.to_id);

  const filteredConnections = uniqueConnections.filter(c => {
    const fromLoc = locations.find(l => l.id === c.from_id)?.name?.toLowerCase() || '';
    const toLoc = locations.find(l => l.id === c.to_id)?.name?.toLowerCase() || '';
    const q = search.toLowerCase();

    const matchesSearch = fromLoc.includes(q) || toLoc.includes(q) || c.from_id.toString().includes(q) || c.to_id.toString().includes(q);

    if (filterStatus === 'ACTIVE') return matchesSearch && !c.blocked;
    if (filterStatus === 'BLOCKED') return matchesSearch && c.blocked;
    return matchesSearch;
  });

  const blockedCount = uniqueConnections.filter(c => c.blocked).length;

  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div>
          <h2 className="text-2xl font-bold text-slate-900">Manage Walkways & Obstacles</h2>
          <p className="text-slate-500 text-sm">
            Dynamically block or restore walkways to simulate construction, events, or maintenance.
          </p>
        </div>

        <div className="flex gap-2">
          {blockedCount > 0 && (
            <button
              onClick={handleRestoreAll}
              disabled={loading}
              className="px-4 py-2.5 bg-emerald-600 hover:bg-emerald-700 text-white font-medium rounded-xl text-sm transition flex items-center gap-2 shadow-sm"
            >
              <RefreshCw className="w-4 h-4" />
              <span>Restore All ({blockedCount})</span>
            </button>
          )}

          <button
            onClick={() => setShowAddModal(true)}
            className="px-4 py-2.5 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-xl text-sm transition flex items-center gap-2 shadow-sm"
          >
            <Plus className="w-4 h-4" />
            <span>Add Walkway</span>
          </button>
        </div>
      </div>

      {/* Notifications */}
      {message && (
        <div className={`p-4 rounded-xl border text-sm flex items-center justify-between ${
          message.type === 'success'
            ? 'bg-emerald-50 border-emerald-200 text-emerald-800'
            : message.type === 'warning'
            ? 'bg-amber-50 border-amber-200 text-amber-800'
            : 'bg-rose-50 border-rose-200 text-rose-800'
        }`}>
          <div className="flex items-center gap-2">
            {message.type === 'success' ? <CheckCircle className="w-5 h-5 text-emerald-600" /> : <AlertTriangle className="w-5 h-5" />}
            <span>{message.text}</span>
          </div>
          <button onClick={() => setMessage(null)} className="text-slate-400 hover:text-slate-600 text-xs">✕</button>
        </div>
      )}

      {/* Filter and Search Bar */}
      <div className="bg-white rounded-2xl p-4 border border-slate-200 shadow-sm flex flex-col sm:flex-row justify-between items-center gap-3">
        <div className="relative w-full sm:w-80">
          <Search className="w-4 h-4 text-slate-400 absolute left-3 top-3" />
          <input
            type="text"
            placeholder="Search by facility name or ID..."
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            className="w-full bg-slate-50 border border-slate-200 rounded-xl pl-9 pr-3 py-2 text-sm text-slate-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
          />
        </div>

        <div className="flex items-center gap-2 w-full sm:w-auto">
          <span className="text-xs text-slate-500 font-medium">Filter:</span>
          {['ALL', 'ACTIVE', 'BLOCKED'].map(status => (
            <button
              key={status}
              onClick={() => setFilterStatus(status)}
              className={`px-3 py-1.5 rounded-xl text-xs font-semibold transition ${
                filterStatus === status
                  ? 'bg-blue-600 text-white shadow-sm'
                  : 'bg-slate-100 text-slate-600 hover:bg-slate-200'
              }`}
            >
              {status}
            </button>
          ))}
        </div>
      </div>

      {/* Walkways Table */}
      <div className="bg-white rounded-2xl border border-slate-200 shadow-sm overflow-hidden">
        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs text-slate-700">
            <thead className="bg-slate-50 border-b border-slate-200 text-[11px] uppercase tracking-wider text-slate-500 font-bold">
              <tr>
                <th className="px-5 py-3.5">Source Location</th>
                <th className="px-5 py-3.5">Destination Location</th>
                <th className="px-5 py-3.5">Distance</th>
                <th className="px-5 py-3.5">Est. Time</th>
                <th className="px-5 py-3.5">Path Type</th>
                <th className="px-5 py-3.5">Status</th>
                <th className="px-5 py-3.5 text-right">Action</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-100">
              {filteredConnections.length > 0 ? (
                filteredConnections.map((conn, idx) => {
                  const fromLoc = locations.find(l => l.id === conn.from_id);
                  const toLoc = locations.find(l => l.id === conn.to_id);

                  return (
                    <tr key={idx} className="hover:bg-slate-50/80 transition">
                      <td className="px-5 py-3.5 font-semibold text-slate-900">
                        [{conn.from_id}] {fromLoc?.name || 'Hub'}
                      </td>
                      <td className="px-5 py-3.5 font-semibold text-slate-900">
                        [{conn.to_id}] {toLoc?.name || 'Hub'}
                      </td>
                      <td className="px-5 py-3.5 font-mono">{Math.round(conn.distance_m)} m</td>
                      <td className="px-5 py-3.5 font-mono">{conn.walking_time_min?.toFixed(1)} min</td>
                      <td className="px-5 py-3.5 text-slate-500 capitalize">{conn.path_type?.replace('_', ' ')}</td>
                      <td className="px-5 py-3.5">
                        <span className={`px-2.5 py-1 rounded-full text-[10px] font-bold tracking-wider uppercase inline-flex items-center gap-1 ${
                          conn.blocked
                            ? 'bg-rose-100 text-rose-700 border border-rose-200'
                            : 'bg-emerald-100 text-emerald-700 border border-emerald-200'
                        }`}>
                          <span className={`w-1.5 h-1.5 rounded-full ${conn.blocked ? 'bg-rose-600' : 'bg-emerald-600'}`}></span>
                          {conn.blocked ? 'BLOCKED' : 'AVAILABLE'}
                        </span>
                      </td>
                      <td className="px-5 py-3.5 text-right">
                        <button
                          onClick={() => handleToggleBlock(conn)}
                          disabled={loading}
                          className={`px-3 py-1.5 rounded-xl font-semibold text-xs transition ${
                            conn.blocked
                              ? 'bg-emerald-50 hover:bg-emerald-100 text-emerald-700 border border-emerald-300'
                              : 'bg-rose-50 hover:bg-rose-100 text-rose-700 border border-rose-300'
                          }`}
                        >
                          {conn.blocked ? 'Restore Walkway' : 'Block Walkway'}
                        </button>
                      </td>
                    </tr>
                  );
                })
              ) : (
                <tr>
                  <td colSpan="7" className="px-5 py-10 text-center text-slate-400">
                    No walkways match your search or filter.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Add Walkway Modal */}
      {showAddModal && (
        <div className="fixed inset-0 z-50 bg-slate-900/50 backdrop-blur-sm flex items-center justify-center p-4">
          <div className="bg-white rounded-2xl max-w-md w-full p-6 shadow-2xl border border-slate-200 space-y-4">
            <h3 className="text-lg font-bold text-slate-900">Add New Walkway</h3>

            <form onSubmit={handleAddWalkway} className="space-y-3">
              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Source Location</label>
                <select
                  value={newFrom}
                  onChange={(e) => setNewFrom(parseInt(e.target.value))}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                >
                  {locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))}
                </select>
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Destination Location</label>
                <select
                  value={newTo}
                  onChange={(e) => setNewTo(parseInt(e.target.value))}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                >
                  {locations.map(loc => (
                    <option key={loc.id} value={loc.id}>[{loc.id}] {loc.name}</option>
                  ))}
                </select>
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Walking Distance (metres)</label>
                <input
                  type="number"
                  min="1"
                  step="1"
                  value={newDist}
                  onChange={(e) => setNewDist(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                  required
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Path Type</label>
                <select
                  value={newType}
                  onChange={(e) => setNewType(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                >
                  <option value="paved_path">Paved Path</option>
                  <option value="paved_road">Paved Road</option>
                  <option value="corridor">Indoor Corridor</option>
                  <option value="staircase">Staircase</option>
                </select>
              </div>

              <div className="flex justify-end gap-2 pt-3">
                <button
                  type="button"
                  onClick={() => setShowAddModal(false)}
                  className="px-4 py-2 bg-slate-100 hover:bg-slate-200 text-slate-700 text-xs font-semibold rounded-xl"
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  disabled={loading}
                  className="px-5 py-2 bg-blue-600 hover:bg-blue-700 text-white text-xs font-semibold rounded-xl"
                >
                  Add to Graph
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}
