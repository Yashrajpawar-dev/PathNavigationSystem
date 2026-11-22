import React, { useState } from 'react';
import { MapPin, Search, Plus, Filter, Navigation } from 'lucide-react';
import { addLocation } from '../services/api';

export default function CampusLocations({ locations = [], onRefresh, onFindRouteTo }) {
  const [search, setSearch] = useState('');
  const [selectedCategory, setSelectedCategory] = useState('ALL');
  const [showAddModal, setShowAddModal] = useState(false);
  const [loading, setLoading] = useState(false);

  // Form state
  const [newId, setNewId] = useState(locations.length > 0 ? Math.max(...locations.map(l => l.id)) + 1 : 20);
  const [newName, setNewName] = useState('');
  const [newCategory, setNewCategory] = useState('Department');
  const [newShortName, setNewShortName] = useState('');
  const [newDesc, setNewDesc] = useState('');

  const categories = ['ALL', ...new Set(locations.map(l => l.category).filter(Boolean))];

  const filteredLocations = locations.filter(loc => {
    const q = search.toLowerCase();
    const matchesSearch = loc.name?.toLowerCase().includes(q) ||
                          loc.short_name?.toLowerCase().includes(q) ||
                          loc.id.toString().includes(q) ||
                          loc.category?.toLowerCase().includes(q);

    if (selectedCategory === 'ALL') return matchesSearch;
    return matchesSearch && loc.category === selectedCategory;
  });

  const handleAddSubmit = async (e) => {
    e.preventDefault();
    if (!newName.trim()) return;

    setLoading(true);
    try {
      await addLocation({
        id: parseInt(newId),
        name: newName.trim(),
        category: newCategory,
        short_name: newShortName.trim() || newName.trim(),
        description: newDesc.trim()
      });
      setShowAddModal(false);
      setNewName('');
      setNewShortName('');
      setNewDesc('');
      if (onRefresh) await onRefresh();
    } catch (err) {
      alert(err.message || 'Failed to add location');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div>
          <h2 className="text-2xl font-bold text-slate-900">Campus Facilities Directory</h2>
          <p className="text-slate-500 text-sm">
            Complete registry of all {locations.length} vertices in the campus graph.
          </p>
        </div>

        <button
          onClick={() => {
            setNewId(locations.length > 0 ? Math.max(...locations.map(l => l.id)) + 1 : 20);
            setShowAddModal(true);
          }}
          className="px-4 py-2.5 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-xl text-sm transition flex items-center gap-2 shadow-sm shrink-0"
        >
          <Plus className="w-4 h-4" />
          <span>Add Location</span>
        </button>
      </div>

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

        <div className="flex gap-1 overflow-x-auto w-full sm:w-auto pb-1 text-xs">
          {categories.map(cat => (
            <button
              key={cat}
              onClick={() => setSelectedCategory(cat)}
              className={`px-3 py-1.5 rounded-xl font-semibold transition whitespace-nowrap ${
                selectedCategory === cat
                  ? 'bg-blue-600 text-white shadow-sm'
                  : 'bg-slate-100 text-slate-600 hover:bg-slate-200'
              }`}
            >
              {cat}
            </button>
          ))}
        </div>
      </div>

      {/* Locations Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {filteredLocations.map(loc => (
          <div key={loc.id} className="bg-white rounded-2xl p-5 border border-slate-200 shadow-sm hover:shadow-md transition flex flex-col justify-between">
            <div>
              <div className="flex items-center justify-between mb-2">
                <span className="w-7 h-7 rounded-lg bg-blue-50 text-blue-700 font-bold text-xs flex items-center justify-center border border-blue-200">
                  {loc.id}
                </span>
                <span className="px-2.5 py-0.5 rounded-full text-[10px] font-bold uppercase tracking-wider bg-slate-100 text-slate-600">
                  {loc.category}
                </span>
              </div>

              <h4 className="font-bold text-slate-900 text-sm">{loc.name}</h4>
              <div className="text-xs text-blue-600 font-medium mt-0.5">{loc.short_name}</div>

              {loc.description && (
                <p className="text-xs text-slate-500 mt-2 line-clamp-2">
                  {loc.description}
                </p>
              )}
            </div>

            <div className="pt-4 border-t border-slate-100 mt-4 flex items-center justify-between">
              <span className="text-[10px] text-slate-400 font-mono">
                {loc.lat?.toFixed(4)}, {loc.lon?.toFixed(4)}
              </span>
              <button
                onClick={() => onFindRouteTo(loc.id)}
                className="text-xs font-semibold text-blue-600 hover:text-blue-800 flex items-center gap-1"
              >
                <span>Navigate</span>
                <Navigation className="w-3 h-3" />
              </button>
            </div>
          </div>
        ))}
      </div>

      {/* Add Location Modal */}
      {showAddModal && (
        <div className="fixed inset-0 z-50 bg-slate-900/50 backdrop-blur-sm flex items-center justify-center p-4">
          <div className="bg-white rounded-2xl max-w-md w-full p-6 shadow-2xl border border-slate-200 space-y-4">
            <h3 className="text-lg font-bold text-slate-900">Add Campus Location (Vertex)</h3>

            <form onSubmit={handleAddSubmit} className="space-y-3">
              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Unique Location ID</label>
                <input
                  type="number"
                  value={newId}
                  onChange={(e) => setNewId(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                  required
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Full Facility Name</label>
                <input
                  type="text"
                  placeholder="e.g. Chemical Engineering Lab"
                  value={newName}
                  onChange={(e) => setNewName(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                  required
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Category</label>
                <select
                  value={newCategory}
                  onChange={(e) => setNewCategory(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                >
                  <option value="Department">Department</option>
                  <option value="Academic">Academic</option>
                  <option value="Laboratory">Laboratory</option>
                  <option value="Library">Library</option>
                  <option value="Facility">Facility</option>
                  <option value="Sports">Sports</option>
                  <option value="Hostel">Hostel</option>
                  <option value="Health">Health</option>
                  <option value="Parking">Parking</option>
                </select>
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Short Display Name</label>
                <input
                  type="text"
                  placeholder="e.g. Chem Lab"
                  value={newShortName}
                  onChange={(e) => setNewShortName(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-600 mb-1">Description</label>
                <textarea
                  rows="2"
                  placeholder="Brief description of the facility"
                  value={newDesc}
                  onChange={(e) => setNewDesc(e.target.value)}
                  className="w-full bg-slate-50 border border-slate-200 rounded-xl px-3 py-2 text-sm"
                ></textarea>
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
                  Save Location
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}
