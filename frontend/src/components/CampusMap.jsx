import React, { useState, useRef, useMemo, useEffect } from 'react';
import { ZoomIn, ZoomOut, RotateCcw, Navigation, MapPin, X, ArrowRight } from 'lucide-react';

const CATEGORY_COLORS = {
  Gate: '#10B981',           // Emerald
  Administration: '#6366F1', // Indigo
  Academic: '#3B82F6',       // Blue
  Department: '#0284C7',     // Sky
  Library: '#8B5CF6',        // Violet
  Facility: '#F59E0B',       // Amber
  Laboratory: '#06B6D4',     // Cyan
  Sports: '#14B8A6',         // Teal
  Parking: '#64748B',        // Slate
  Hostel: '#EC4899',         // Pink
  Health: '#EF4444',         // Red
  Faculty: '#84CC16',        // Lime
  Research: '#A855F7',       // Purple
};

export default function CampusMap({
  locations = [],
  allLocations = null,
  connections = [],
  activePath = [],
  sourceId = null,
  destId = null,
  onSelectNode = null,
  onSelectStart = null,
  onSelectDest = null,
  highlightCategory = null,
  height = '520px'
}) {
  const [zoom, setZoom] = useState(1);
  const [pan, setPan] = useState({ x: 0, y: 0 });
  const [isDragging, setIsDragging] = useState(false);
  const [dragStart, setDragStart] = useState({ x: 0, y: 0 });
  const [hoveredNode, setHoveredNode] = useState(null);
  const [selectedPopupNode, setSelectedPopupNode] = useState(null);
  const [showLabels, setShowLabels] = useState(true);

  const containerRef = useRef(null);

  // Use full location set for coordinate normalization to prevent span collapse
  const masterLocations = (allLocations && allLocations.length > 0) ? allLocations : locations;

  // Compute bounding box and normalized 2D coordinates from GPS lat/lon
  const { coords } = useMemo(() => {
    if (!masterLocations.length) return { coords: {} };

    let minLat = Infinity, maxLat = -Infinity;
    let minLon = Infinity, maxLon = -Infinity;

    masterLocations.forEach(loc => {
      const lat = parseFloat(loc.lat);
      const lon = parseFloat(loc.lon);
      if (!isNaN(lat) && lat < minLat) minLat = lat;
      if (!isNaN(lat) && lat > maxLat) maxLat = lat;
      if (!isNaN(lon) && lon < minLon) minLon = lon;
      if (!isNaN(lon) && lon > maxLon) maxLon = lon;
    });

    const latSpan = (maxLat - minLat) > 0.0001 ? (maxLat - minLat) : 0.003;
    const lonSpan = (maxLon - minLon) > 0.0001 ? (maxLon - minLon) : 0.003;

    const width = 1000;
    const height = 700;
    const pad = 90;

    const mapCoords = {};
    masterLocations.forEach(loc => {
      const lat = parseFloat(loc.lat);
      const lon = parseFloat(loc.lon);
      const x = pad + ((lon - minLon) / lonSpan) * (width - 2 * pad);
      const y = height - (pad + ((lat - minLat) / latSpan) * (height - 2 * pad));
      mapCoords[loc.id] = { x, y };
    });

    return { coords: mapCoords };
  }, [masterLocations]);

  // Set of edges that are part of the active path
  const pathEdgeSet = useMemo(() => {
    const set = new Set();
    if (activePath && activePath.length > 1) {
      for (let i = 0; i < activePath.length - 1; i++) {
        const u = activePath[i];
        const v = activePath[i + 1];
        set.add(`${Math.min(u, v)}-${Math.max(u, v)}`);
      }
    }
    return set;
  }, [activePath]);

  // Construct active path coordinates polyline
  const activePathPoints = useMemo(() => {
    if (!activePath || activePath.length < 2) return null;
    return activePath
      .map(id => coords[id])
      .filter(Boolean)
      .map(pt => `${pt.x},${pt.y}`)
      .join(' ');
  }, [activePath, coords]);

  // Pan controls
  const handleMouseDown = (e) => {
    if (e.button !== 0) return;
    setIsDragging(true);
    setDragStart({ x: e.clientX - pan.x, y: e.clientY - pan.y });
  };

  const handleMouseMove = (e) => {
    if (!isDragging) return;
    setPan({
      x: e.clientX - dragStart.x,
      y: e.clientY - dragStart.y
    });
  };

  const handleMouseUp = () => setIsDragging(false);

  const handleZoom = (factor) => {
    setZoom(prev => Math.min(Math.max(prev * factor, 0.5), 3));
  };

  const resetView = () => {
    setZoom(1);
    setPan({ x: 0, y: 0 });
    setSelectedPopupNode(null);
  };

  const handleNodeClick = (loc) => {
    if (onSelectStart || onSelectDest) {
      // Toggle floating popup on node
      setSelectedPopupNode(loc);
    } else if (onSelectNode) {
      onSelectNode(loc.id);
    }
  };

  const handleSetStart = (id) => {
    if (onSelectStart) onSelectStart(id);
    else if (onSelectNode) onSelectNode(id, 'start');
    setSelectedPopupNode(null);
  };

  const handleSetDest = (id) => {
    if (onSelectDest) onSelectDest(id);
    else if (onSelectNode) onSelectNode(id, 'dest');
    setSelectedPopupNode(null);
  };

  return (
    <div
      ref={containerRef}
      className="relative w-full bg-slate-900 rounded-2xl overflow-hidden border border-slate-700 shadow-inner select-none"
      style={{ height }}
      onMouseDown={handleMouseDown}
      onMouseMove={handleMouseMove}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp}
    >
      {/* Zoom / Pan Control Buttons */}
      <div className="absolute top-4 right-4 z-20 flex flex-col gap-2 bg-slate-800/95 backdrop-blur-md p-1.5 rounded-xl border border-slate-700 shadow-lg">
        <button
          onClick={() => handleZoom(1.2)}
          title="Zoom In"
          className="p-2 text-slate-300 hover:text-white hover:bg-slate-700 rounded-lg transition"
        >
          <ZoomIn className="w-4 h-4" />
        </button>
        <button
          onClick={() => handleZoom(0.8)}
          title="Zoom Out"
          className="p-2 text-slate-300 hover:text-white hover:bg-slate-700 rounded-lg transition"
        >
          <ZoomOut className="w-4 h-4" />
        </button>
        <button
          onClick={resetView}
          title="Reset View"
          className="p-2 text-slate-300 hover:text-white hover:bg-slate-700 rounded-lg transition"
        >
          <RotateCcw className="w-4 h-4" />
        </button>
        <button
          onClick={() => setShowLabels(!showLabels)}
          title="Toggle Labels"
          className={`p-2 rounded-lg transition text-xs font-semibold ${showLabels ? 'bg-blue-600 text-white' : 'text-slate-400 hover:text-white'}`}
        >
          Aa
        </button>
      </div>

      {/* Legend Badge */}
      <div className="absolute bottom-4 left-4 z-20 bg-slate-800/95 backdrop-blur-md px-3.5 py-2.5 rounded-xl border border-slate-700 shadow-lg text-xs text-slate-300 flex flex-wrap items-center gap-4">
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded-full bg-emerald-500 ring-2 ring-emerald-300"></span>
          <span>Start [S]</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded-full bg-rose-500 ring-2 ring-rose-300"></span>
          <span>Goal [D]</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-4 h-1 bg-cyan-400 rounded"></span>
          <span>Active Route</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-4 h-1 border-t-2 border-dashed border-red-500"></span>
          <span>Blocked Walkway</span>
        </div>
      </div>

      {/* Interactive Node Action Popover */}
      {selectedPopupNode && coords[selectedPopupNode.id] && (
        <div
          className="absolute z-30 bg-slate-800/95 backdrop-blur-md p-3.5 rounded-xl border border-blue-500/50 shadow-2xl text-xs min-w-[210px]"
          style={{
            left: Math.min(Math.max(coords[selectedPopupNode.id].x * (zoom) + pan.x, 20), 750),
            top: Math.min(Math.max(coords[selectedPopupNode.id].y * (zoom) + pan.y - 100, 20), 450)
          }}
        >
          <div className="flex items-center justify-between gap-2 mb-2">
            <span className="font-bold text-white text-xs truncate">
              [{selectedPopupNode.id}] {selectedPopupNode.name}
            </span>
            <button
              onClick={() => setSelectedPopupNode(null)}
              className="text-slate-400 hover:text-white p-0.5 rounded"
            >
              <X className="w-3.5 h-3.5" />
            </button>
          </div>
          <div className="text-[11px] text-slate-400 mb-3">
            {selectedPopupNode.category} • {selectedPopupNode.short_name}
          </div>
          <div className="grid grid-cols-2 gap-2">
            <button
              onClick={() => handleSetStart(selectedPopupNode.id)}
              className="px-2.5 py-1.5 bg-emerald-600 hover:bg-emerald-500 text-white font-semibold rounded-lg text-xs transition flex items-center justify-center gap-1 shadow-sm"
            >
              <span>Set Start</span>
            </button>
            <button
              onClick={() => handleSetDest(selectedPopupNode.id)}
              className="px-2.5 py-1.5 bg-rose-600 hover:bg-rose-500 text-white font-semibold rounded-lg text-xs transition flex items-center justify-center gap-1 shadow-sm"
            >
              <span>Set Goal</span>
            </button>
          </div>
        </div>
      )}

      {/* SVG Canvas */}
      <svg
        viewBox="0 0 1000 700"
        className="w-full h-full cursor-grab active:cursor-grabbing"
      >
        <defs>
          {/* Subtle grid background */}
          <pattern id="grid" width="40" height="40" patternUnits="userSpaceOnUse">
            <path d="M 40 0 L 0 0 0 40" fill="none" stroke="#1E293B" strokeWidth="0.8" />
          </pattern>
          {/* Cyan route pulse glow filter */}
          <filter id="routeGlow" x="-30%" y="-30%" width="160%" height="160%">
            <feGaussianBlur stdDeviation="4" result="blur" />
            <feComposite in="SourceGraphic" in2="blur" operator="over" />
          </filter>
        </defs>

        <rect width="100%" height="100%" fill="url(#grid)" />

        <g transform={`translate(${pan.x}, ${pan.y}) scale(${zoom})`} transformOrigin="500 350">
          {/* 1. Walkway Background Edges */}
          {connections.map((conn, idx) => {
            const p1 = coords[conn.from_id];
            const p2 = coords[conn.to_id];
            if (!p1 || !p2) return null;

            const edgeKey = `${Math.min(conn.from_id, conn.to_id)}-${Math.max(conn.from_id, conn.to_id)}`;
            const isRouteEdge = pathEdgeSet.has(edgeKey);
            const isBlocked = conn.blocked;

            const midX = (p1.x + p2.x) / 2;
            const midY = (p1.y + p2.y) / 2;

            return (
              <g key={`edge-${idx}`}>
                {/* Base walkway line */}
                <line
                  x1={p1.x}
                  y1={p1.y}
                  x2={p2.x}
                  y2={p2.y}
                  stroke={isBlocked ? '#DC2626' : (isRouteEdge ? '#0284C7' : '#334155')}
                  strokeWidth={isRouteEdge ? 4 : (isBlocked ? 2.5 : 2)}
                  strokeDasharray={isBlocked ? '6 4' : 'none'}
                  strokeLinecap="round"
                />

                {/* Blocked indicator */}
                {isBlocked && (
                  <circle cx={midX} cy={midY} r="7" fill="#DC2626" stroke="#991B1B" strokeWidth="1" />
                )}

                {/* Walkway Distance label */}
                {showLabels && !isBlocked && (
                  <text
                    x={midX}
                    y={midY - 4}
                    fill={isRouteEdge ? '#38BDF8' : '#64748B'}
                    fontSize="9"
                    fontFamily="monospace"
                    textAnchor="middle"
                    className="select-none pointer-events-none font-semibold"
                  >
                    {Math.round(conn.distance_m)}m
                  </text>
                )}
              </g>
            );
          })}

          {/* 2. Highlighted Route Polyline (Glow overlay on active route) */}
          {activePathPoints && (
            <polyline
              points={activePathPoints}
              fill="none"
              stroke="#38BDF8"
              strokeWidth="5"
              strokeLinecap="round"
              strokeLinejoin="round"
              filter="url(#routeGlow)"
              className="transition-all duration-300"
            />
          )}

          {/* 3. Campus Facility Nodes */}
          {masterLocations.map((loc) => {
            const pt = coords[loc.id];
            if (!pt) return null;

            const isSource = sourceId === loc.id;
            const isDest = destId === loc.id;
            const isPathNode = activePath.includes(loc.id);
            const isHovered = hoveredNode === loc.id;
            const isDimmed = highlightCategory && highlightCategory !== 'ALL' && loc.category !== highlightCategory;

            const baseColor = CATEGORY_COLORS[loc.category] || '#3B82F6';

            return (
              <g
                key={`loc-${loc.id}`}
                transform={`translate(${pt.x}, ${pt.y})`}
                className="cursor-pointer transition-transform duration-150"
                style={{ opacity: isDimmed ? 0.35 : 1 }}
                onClick={() => handleNodeClick(loc)}
                onMouseEnter={() => setHoveredNode(loc.id)}
                onMouseLeave={() => setHoveredNode(null)}
              >
                {/* Node Halo for Source / Destination / Active Route */}
                {(isSource || isDest || isPathNode) && (
                  <circle
                    r={isSource || isDest ? 24 : 16}
                    fill={isSource ? '#10B981' : (isDest ? '#EF4444' : '#0284C7')}
                    fillOpacity="0.3"
                    className={isSource || isDest ? 'animate-pulse' : ''}
                  />
                )}

                {/* Node Circle */}
                <circle
                  r={isSource || isDest ? 14 : (isHovered ? 12 : 9.5)}
                  fill={isSource ? '#10B981' : (isDest ? '#EF4444' : baseColor)}
                  stroke="#FFFFFF"
                  strokeWidth={isSource || isDest || isHovered ? 3 : 2}
                  className="shadow-lg"
                />

                {/* Node Label: [S], [D], or numeric ID */}
                <text
                  textAnchor="middle"
                  dy="4"
                  fill="#FFFFFF"
                  fontSize={isSource || isDest ? "11" : "9"}
                  fontWeight="bold"
                  className="pointer-events-none select-none"
                >
                  {isSource ? 'S' : (isDest ? 'D' : loc.id)}
                </text>

                {/* Name Label */}
                {showLabels && (
                  <text
                    y={isSource || isDest ? 30 : 23}
                    textAnchor="middle"
                    fill={isSource ? '#34D399' : (isDest ? '#F87171' : (isPathNode ? '#7DD3FC' : '#E2E8F0'))}
                    fontSize="11"
                    fontWeight={isSource || isDest || isPathNode ? 'bold' : 'normal'}
                    className="select-none pointer-events-none drop-shadow"
                  >
                    {loc.short_name || loc.name}
                  </text>
                )}
              </g>
            );
          })}
        </g>
      </svg>

      {/* Hover Info Tooltip */}
      {hoveredNode !== null && !selectedPopupNode && (
        <div className="absolute top-4 left-4 z-20 bg-slate-800/95 backdrop-blur-md p-3 rounded-xl border border-slate-700 shadow-xl max-w-xs text-xs pointer-events-none">
          {(() => {
            const loc = masterLocations.find(l => l.id === hoveredNode);
            if (!loc) return null;
            return (
              <div>
                <div className="flex items-center gap-2 mb-1">
                  <span
                    className="w-2.5 h-2.5 rounded-full"
                    style={{ backgroundColor: CATEGORY_COLORS[loc.category] || '#3B82F6' }}
                  />
                  <span className="font-bold text-white text-sm">[{loc.id}] {loc.name}</span>
                </div>
                <div className="text-slate-400 mb-1">{loc.category} • {loc.short_name}</div>
                {loc.description && (
                  <div className="text-slate-300 border-t border-slate-700/80 pt-1 italic line-clamp-2">
                    {loc.description}
                  </div>
                )}
                <div className="mt-2 text-[10px] text-blue-400 font-medium">
                  Click to set as Start or Goal
                </div>
              </div>
            );
          })()}
        </div>
      )}
    </div>
  );
}
