#!/usr/bin/env python3
"""
VIT Pune Campus Navigator – PyQt6 Desktop Application
======================================================
Vishwakarma Institute of Technology, Bibwewadi, Pune

This is a fully functional PyQt6 implementation that mirrors the C++ Qt6
application. It runs immediately with:
    python main.py

Requirements:
    pip install PyQt6

Features:
- Interactive campus map (custom-rendered)
- BFS, DFS, Dijkstra, A*, Floyd-Warshall algorithms
- Route visualization with animation
- Algorithm comparison dashboard
- Path blocking and rerouting
- Location search
- Algorithm statistics panel

Data accuracy note:
    Building positions are APPROXIMATE – derived from OpenStreetMap data
    for the VIT Bibwewadi campus area. Not verified by official survey.
"""

import sys
import os
import json
import math
import time
import heapq
from collections import deque
from typing import Optional, List, Dict, Tuple, Set
from dataclasses import dataclass, field

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QHBoxLayout, QVBoxLayout,
    QGridLayout, QGroupBox, QLabel, QPushButton, QComboBox, QLineEdit,
    QListWidget, QListWidgetItem, QTextEdit, QSplitter, QStatusBar,
    QMessageBox, QMenuBar, QScrollArea, QFrame, QSizePolicy
)
from PyQt6.QtCore import (
    Qt, QTimer, QPointF, QRectF, pyqtSignal, QSize
)
from PyQt6.QtGui import (
    QPainter, QPen, QBrush, QColor, QFont, QFontMetrics,
    QRadialGradient, QLinearGradient, QPainterPath, QPolygonF,
    QAction, QKeySequence
)

# =============================================================================
#  DATA LAYER
# =============================================================================

LOCATION_CATEGORIES = {
    "Gate": "#f77306",
    "Administration": "#7b2d8b",
    "Academic": "#0077b6",
    "Department": "#0296c7",
    "Library": "#48cb49",
    "Facility": "#ff9900",
    "Laboratory": "#00b4d8",
    "Sports": "#06d6a0",
    "Parking": "#6c757d",
    "Hostel": "#e06c75",
    "Health": "#ef476f",
    "Faculty": "#ab83a1",
    "Research": "#f4d03f",
    "Unknown": "#909090",
}

CATEGORY_ICONS = {
    "Gate": "G", "Administration": "A", "Academic": "B",
    "Department": "D", "Library": "L", "Facility": "F",
    "Laboratory": "⚗", "Sports": "⚽", "Parking": "P",
    "Hostel": "H", "Health": "+", "Faculty": "F", "Research": "R",
}


@dataclass
class Location:
    id: int
    name: str
    short_name: str
    category: str
    lat: float
    lon: float
    description: str = ""
    accuracy: str = "APPROXIMATE"
    source: str = ""


@dataclass
class Edge:
    from_id: int
    to_id: int
    distance_m: float
    path_type: str = "paved_path"
    blocked: bool = False
    verified_distance: bool = False
    description: str = ""


@dataclass
class PathResult:
    found: bool = False
    source_id: int = -1
    dest_id: int = -1
    path: List[int] = field(default_factory=list)
    total_distance: float = 0.0
    total_time_min: float = 0.0
    algorithm_name: str = ""
    nodes_visited: int = 0
    edges_explored: int = 0
    execution_time_ms: float = 0.0
    visit_order: List[int] = field(default_factory=list)
    explored_edges: List[Tuple[int, int]] = field(default_factory=list)


# =============================================================================
#  GRAPH
# =============================================================================

class Graph:
    """Weighted undirected graph using adjacency list."""

    def __init__(self):
        self.locations: Dict[int, Location] = {}
        self.adj: Dict[int, List[Tuple[int, float]]] = {}  # id -> [(nbId, dist)]
        self.edges: List[Edge] = []

    def add_location(self, loc: Location):
        self.locations[loc.id] = loc
        if loc.id not in self.adj:
            self.adj[loc.id] = []

    def add_edge(self, edge: Edge):
        self.edges.append(edge)
        if not edge.blocked:
            self._add_to_adj(edge)

    def _add_to_adj(self, edge: Edge):
        if edge.from_id not in self.adj:
            self.adj[edge.from_id] = []
        if edge.to_id not in self.adj:
            self.adj[edge.to_id] = []
        # Avoid duplicates
        if not any(nb == edge.to_id for nb, _ in self.adj[edge.from_id]):
            self.adj[edge.from_id].append((edge.to_id, edge.distance_m))
        if not any(nb == edge.from_id for nb, _ in self.adj[edge.to_id]):
            self.adj[edge.to_id].append((edge.from_id, edge.distance_m))

    def neighbours(self, node_id: int) -> List[Tuple[int, float]]:
        return self.adj.get(node_id, [])

    def all_neighbours(self, node_id: int) -> List[Tuple[int, float]]:
        """Including blocked edges."""
        result = []
        for e in self.edges:
            if e.from_id == node_id:
                result.append((e.to_id, e.distance_m))
            elif e.to_id == node_id:
                result.append((e.from_id, e.distance_m))
        return result

    def edge_exists(self, a: int, b: int) -> bool:
        return any((e.from_id == a and e.to_id == b) or
                   (e.from_id == b and e.to_id == a) for e in self.edges)

    def edge_blocked(self, a: int, b: int) -> bool:
        for e in self.edges:
            if (e.from_id == a and e.to_id == b) or \
               (e.from_id == b and e.to_id == a):
                return e.blocked
        return False

    def get_edge(self, a: int, b: int) -> Optional[Edge]:
        for e in self.edges:
            if (e.from_id == a and e.to_id == b) or \
               (e.from_id == b and e.to_id == a):
                return e
        return None

    def set_edge_blocked(self, a: int, b: int, blocked: bool):
        for e in self.edges:
            if (e.from_id == a and e.to_id == b) or \
               (e.from_id == b and e.to_id == a):
                e.blocked = blocked
        # Rebuild adjacency for affected nodes
        self._rebuild_adj(a)
        self._rebuild_adj(b)

    def _rebuild_adj(self, node_id: int):
        self.adj[node_id] = []
        for e in self.edges:
            if not e.blocked:
                if e.from_id == node_id:
                    if not any(nb == e.to_id for nb, _ in self.adj[node_id]):
                        self.adj[node_id].append((e.to_id, e.distance_m))
                elif e.to_id == node_id:
                    if not any(nb == e.from_id for nb, _ in self.adj[node_id]):
                        self.adj[node_id].append((e.from_id, e.distance_m))

    @staticmethod
    def haversine(lat1, lon1, lat2, lon2) -> float:
        """Great-circle distance in metres."""
        R = 6371000
        φ1, φ2 = math.radians(lat1), math.radians(lat2)
        Δφ = math.radians(lat2 - lat1)
        Δλ = math.radians(lon2 - lon1)
        a = math.sin(Δφ/2)**2 + math.cos(φ1)*math.cos(φ2)*math.sin(Δλ/2)**2
        return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

    def haversine_ids(self, a: int, b: int) -> float:
        la, lb = self.locations[a], self.locations[b]
        return self.haversine(la.lat, la.lon, lb.lat, lb.lon)


# =============================================================================
#  ALGORITHMS
# =============================================================================

INF = float('inf')


def _reconstruct_path(source_id: int, dest_id: int,
                      parent: Dict[int, int], dist: Dict[int, float]) -> List[int]:
    if dist.get(dest_id, INF) == INF:
        return []
    path = []
    cur = dest_id
    visited_reconstruction = set()
    while cur is not None and cur not in visited_reconstruction:
        path.append(cur)
        visited_reconstruction.add(cur)
        if cur == source_id:
            break
        cur = parent.get(cur)
    if not path or path[-1] != source_id:
        return []
    return list(reversed(path))


def bfs(graph: Graph, source_id: int, dest_id: int) -> PathResult:
    """BFS: minimum-hop path (ignores weights). O(V+E)"""
    t0 = time.perf_counter()
    parent: Dict[int, Optional[int]] = {source_id: None}
    dist_map: Dict[int, float] = {source_id: 0.0}
    visit_order = [source_id]
    explored_edges = []
    queue = deque([source_id])
    nodes_visited = 1
    edges_explored = 0

    while queue:
        cur = queue.popleft()
        if cur == dest_id:
            break
        for nb, w in graph.neighbours(cur):
            edges_explored += 1
            explored_edges.append((cur, nb))
            if nb not in parent:
                parent[nb] = cur
                dist_map[nb] = dist_map[cur] + w
                visit_order.append(nb)
                nodes_visited += 1
                queue.append(nb)

    path = _reconstruct_path(source_id, dest_id, parent, dist_map)
    total = dist_map.get(dest_id, INF)
    ms = (time.perf_counter() - t0) * 1000
    return PathResult(
        found=bool(path), source_id=source_id, dest_id=dest_id,
        path=path, total_distance=total if path else 0,
        total_time_min=(total/1000/5*60) if path else 0,
        algorithm_name="BFS (Breadth-First Search)",
        nodes_visited=nodes_visited, edges_explored=edges_explored,
        execution_time_ms=ms, visit_order=visit_order,
        explored_edges=explored_edges
    )


def dfs(graph: Graph, source_id: int, dest_id: int) -> PathResult:
    """DFS: graph traversal (NOT guaranteed shortest). O(V+E)"""
    t0 = time.perf_counter()
    parent: Dict[int, Optional[int]] = {source_id: None}
    dist_map: Dict[int, float] = {source_id: 0.0}
    visited: Set[int] = set()
    visit_order = []
    explored_edges = []
    stack = [source_id]
    nodes_visited = 0
    edges_explored = 0
    found = False

    while stack and not found:
        cur = stack.pop()
        if cur in visited:
            continue
        visited.add(cur)
        visit_order.append(cur)
        nodes_visited += 1
        if cur == dest_id:
            found = True
            break
        for nb, w in graph.neighbours(cur):
            edges_explored += 1
            explored_edges.append((cur, nb))
            if nb not in visited:
                if nb not in parent:
                    parent[nb] = cur
                    dist_map[nb] = dist_map.get(cur, 0) + w
                stack.append(nb)

    path = _reconstruct_path(source_id, dest_id, parent, dist_map) if found else []
    total = dist_map.get(dest_id, INF) if found else 0
    ms = (time.perf_counter() - t0) * 1000
    return PathResult(
        found=bool(path), source_id=source_id, dest_id=dest_id,
        path=path, total_distance=total if path else 0,
        total_time_min=(total/1000/5*60) if path else 0,
        algorithm_name="DFS (Depth-First Search) [⚠ Not Guaranteed Shortest]",
        nodes_visited=nodes_visited, edges_explored=edges_explored,
        execution_time_ms=ms, visit_order=visit_order,
        explored_edges=explored_edges
    )


def dijkstra(graph: Graph, source_id: int, dest_id: int) -> PathResult:
    """Dijkstra: minimum weighted path. O((V+E)logV)"""
    t0 = time.perf_counter()
    dist_map: Dict[int, float] = {source_id: 0.0}
    parent: Dict[int, Optional[int]] = {source_id: None}
    settled: Set[int] = set()
    visit_order = []
    explored_edges = []
    pq = [(0.0, source_id)]
    nodes_visited = 0
    edges_explored = 0

    while pq:
        d, cur = heapq.heappop(pq)
        if cur in settled:
            continue
        settled.add(cur)
        visit_order.append(cur)
        nodes_visited += 1
        if cur == dest_id:
            break
        for nb, w in graph.neighbours(cur):
            edges_explored += 1
            explored_edges.append((cur, nb))
            if nb not in settled:
                nd = d + w
                if nd < dist_map.get(nb, INF):
                    dist_map[nb] = nd
                    parent[nb] = cur
                    heapq.heappush(pq, (nd, nb))

    path = _reconstruct_path(source_id, dest_id, parent, dist_map)
    total = dist_map.get(dest_id, INF)
    ms = (time.perf_counter() - t0) * 1000
    return PathResult(
        found=bool(path), source_id=source_id, dest_id=dest_id,
        path=path, total_distance=total if path else 0,
        total_time_min=(total/1000/5*60) if path else 0,
        algorithm_name="Dijkstra's Algorithm",
        nodes_visited=nodes_visited, edges_explored=edges_explored,
        execution_time_ms=ms, visit_order=visit_order,
        explored_edges=explored_edges
    )


def a_star(graph: Graph, source_id: int, dest_id: int) -> PathResult:
    """A*: informed shortest path with Haversine heuristic. O((V+E)logV)"""
    t0 = time.perf_counter()

    def h(node_id: int) -> float:
        return graph.haversine_ids(node_id, dest_id)

    g_cost: Dict[int, float] = {source_id: 0.0}
    parent: Dict[int, Optional[int]] = {source_id: None}
    closed: Set[int] = set()
    visit_order = []
    explored_edges = []
    pq = [(h(source_id), 0.0, source_id)]
    nodes_visited = 0
    edges_explored = 0

    while pq:
        f, g, cur = heapq.heappop(pq)
        if cur in closed:
            continue
        closed.add(cur)
        visit_order.append(cur)
        nodes_visited += 1
        if cur == dest_id:
            break
        for nb, w in graph.neighbours(cur):
            edges_explored += 1
            explored_edges.append((cur, nb))
            if nb not in closed:
                ng = g_cost[cur] + w
                if ng < g_cost.get(nb, INF):
                    g_cost[nb] = ng
                    parent[nb] = cur
                    heapq.heappush(pq, (ng + h(nb), ng, nb))

    path = _reconstruct_path(source_id, dest_id, parent, g_cost)
    total = g_cost.get(dest_id, INF)
    ms = (time.perf_counter() - t0) * 1000
    return PathResult(
        found=bool(path), source_id=source_id, dest_id=dest_id,
        path=path, total_distance=total if path else 0,
        total_time_min=(total/1000/5*60) if path else 0,
        algorithm_name="A* Search (Haversine Heuristic)",
        nodes_visited=nodes_visited, edges_explored=edges_explored,
        execution_time_ms=ms, visit_order=visit_order,
        explored_edges=explored_edges
    )


def floyd_warshall(graph: Graph) -> Tuple[Dict, Dict, float]:
    """All-pairs shortest paths. O(V³)"""
    t0 = time.perf_counter()
    ids = list(graph.locations.keys())
    n = len(ids)
    idx = {id_: i for i, id_ in enumerate(ids)}

    dist = [[INF]*n for _ in range(n)]
    nxt  = [[-1]*n  for _ in range(n)]

    for i in range(n):
        dist[i][i] = 0.0

    for id_ in ids:
        i = idx[id_]
        for nb, w in graph.neighbours(id_):
            j = idx[nb]
            if w < dist[i][j]:
                dist[i][j] = w
                dist[j][i] = w
                nxt[i][j] = j
                nxt[j][i] = i

    for k in range(n):
        for i in range(n):
            if dist[i][k] == INF:
                continue
            for j in range(n):
                via = dist[i][k] + dist[k][j]
                if via < dist[i][j]:
                    dist[i][j] = via
                    nxt[i][j] = nxt[i][k]

    ms = (time.perf_counter() - t0) * 1000
    return ids, idx, dist, nxt, ms


# =============================================================================
#  NAVIGATION CONTROLLER
# =============================================================================

class NavigationController:
    def __init__(self):
        self.graph = Graph()
        self.loaded = False
        self.last_error = ""
        self._floyd_cache = None
        self._blocked: Set[Tuple[int, int]] = set()

    def load_all(self, data_dir: str) -> bool:
        ok1 = self.load_locations(os.path.join(data_dir, "locations.json"))
        if not ok1:
            return False
        ok2 = self.load_connections(os.path.join(data_dir, "connections.json"))
        self.loaded = ok2
        return ok2

    def load_locations(self, path: str) -> bool:
        try:
            with open(path, encoding='utf-8') as f:
                data = json.load(f)
            for item in data["locations"]:
                loc = Location(
                    id=item["id"], name=item["name"],
                    short_name=item["short_name"],
                    category=item["category"],
                    lat=item["lat"], lon=item["lon"],
                    description=item.get("description", ""),
                    accuracy=item.get("accuracy", "APPROXIMATE"),
                    source=item.get("source", ""),
                )
                self.graph.add_location(loc)
            return True
        except Exception as e:
            self.last_error = str(e)
            return False

    def load_connections(self, path: str) -> bool:
        try:
            with open(path, encoding='utf-8') as f:
                data = json.load(f)
            for item in data["connections"]:
                edge = Edge(
                    from_id=item["from_id"], to_id=item["to_id"],
                    distance_m=item["distance_m"],
                    path_type=item.get("path_type", "paved_path"),
                    blocked=item.get("blocked", False),
                    verified_distance=item.get("verified_distance", False),
                    description=item.get("description", ""),
                )
                self.graph.add_edge(edge)
            return True
        except Exception as e:
            self.last_error = str(e)
            return False

    def find_route(self, source_id: int, dest_id: int,
                   algorithm: str) -> PathResult:
        alg_map = {
            "dijkstra": dijkstra,
            "a_star":   a_star,
            "bfs":      bfs,
            "dfs":      dfs,
        }
        if algorithm in alg_map:
            return alg_map[algorithm](self.graph, source_id, dest_id)
        elif algorithm == "floyd":
            return self._floyd_query(source_id, dest_id)
        return dijkstra(self.graph, source_id, dest_id)

    def _floyd_query(self, source_id: int, dest_id: int) -> PathResult:
        if self._floyd_cache is None:
            self._floyd_cache = floyd_warshall(self.graph)
        ids, idx, dist, nxt, ms = self._floyd_cache
        si = idx.get(source_id, -1)
        di = idx.get(dest_id, -1)
        if si < 0 or di < 0 or dist[si][di] == INF:
            return PathResult(found=False, source_id=source_id,
                              dest_id=dest_id, algorithm_name="Floyd-Warshall",
                              execution_time_ms=ms)
        # Reconstruct path
        path_ids = []
        cur = si
        for _ in range(len(ids) + 1):
            path_ids.append(ids[cur])
            if cur == di:
                break
            cur = nxt[cur][di]
            if cur == -1:
                break
        total = dist[si][di]
        return PathResult(
            found=True, source_id=source_id, dest_id=dest_id,
            path=path_ids, total_distance=total,
            total_time_min=total/1000/5*60,
            algorithm_name="Floyd-Warshall",
            execution_time_ms=ms
        )

    def compare_all(self, source_id: int, dest_id: int):
        return {
            "BFS":      bfs(self.graph, source_id, dest_id),
            "DFS":      dfs(self.graph, source_id, dest_id),
            "Dijkstra": dijkstra(self.graph, source_id, dest_id),
            "A*":       a_star(self.graph, source_id, dest_id),
        }

    def search(self, query: str) -> List[Location]:
        q = query.lower()
        return [loc for loc in self.graph.locations.values()
                if q in loc.name.lower() or q in loc.short_name.lower()
                or q in loc.category.lower()]

    def block_edge(self, a: int, b: int):
        self.graph.set_edge_blocked(a, b, True)
        key = (min(a, b), max(a, b))
        self._blocked.add(key)
        self._floyd_cache = None

    def unblock_edge(self, a: int, b: int):
        self.graph.set_edge_blocked(a, b, False)
        key = (min(a, b), max(a, b))
        self._blocked.discard(key)
        self._floyd_cache = None

    def clear_all_blocks(self):
        for a, b in list(self._blocked):
            self.graph.set_edge_blocked(a, b, False)
        self._blocked.clear()
        self._floyd_cache = None


# =============================================================================
#  MAP WIDGET
# =============================================================================

# Geographic bounding box of VIT Bibwewadi campus
LAT_MIN, LAT_MAX = 18.4625, 18.4670
LON_MIN, LON_MAX = 73.8668, 73.8700


class MapWidget(QWidget):
    location_clicked = pyqtSignal(int)
    edge_block_requested = pyqtSignal(int, int)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.graph: Optional[Graph] = None
        self.route: Optional[PathResult] = None
        self.anim_step = -1
        self.highlighted_id = -1
        self.blocking_mode = False
        self.block_from = -1

        self.scale = 1.0
        self.pan_offset = QPointF(0, 0)
        self._last_mouse = QPointF(0, 0)
        self._panning = False

        self.setMouseTracking(True)
        self.setMinimumSize(600, 400)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

    def set_graph(self, graph: Graph):
        self.graph = graph
        self.update()

    def set_route(self, result: PathResult):
        self.route = result
        self.anim_step = -1
        self.update()

    def set_anim_step(self, step: int):
        self.anim_step = step
        self.update()

    def clear_route(self):
        self.route = None
        self.anim_step = -1
        self.update()

    def set_highlighted(self, loc_id: int):
        self.highlighted_id = loc_id
        self.update()

    # ---- Coordinate mapping ----
    def lat_lon_to_pixel(self, lat: float, lon: float) -> QPointF:
        nx = (lon - LON_MIN) / (LON_MAX - LON_MIN)
        ny = 1.0 - (lat - LAT_MIN) / (LAT_MAX - LAT_MIN)
        bx = nx * self.width()
        by = ny * self.height()
        return QPointF(bx * self.scale + self.pan_offset.x(),
                       by * self.scale + self.pan_offset.y())

    def _loc_at_pixel(self, pt: QPointF) -> int:
        if not self.graph:
            return -1
        for loc in self.graph.locations.values():
            p = self.lat_lon_to_pixel(loc.lat, loc.lon)
            dx, dy = pt.x()-p.x(), pt.y()-p.y()
            if math.sqrt(dx*dx + dy*dy) <= 16:
                return loc.id
        return -1

    # ---- Paint ----
    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)
        p.setRenderHint(QPainter.RenderHint.TextAntialiasing)
        self._draw_background(p)
        if not self.graph:
            p.setPen(QColor("#ffffff"))
            p.setFont(QFont("Segoe UI", 16))
            p.drawText(self.rect(), Qt.AlignmentFlag.AlignCenter, "Loading campus data...")
            return
        self._draw_campus_area(p)
        self._draw_edges(p)
        if self.route and self.route.found:
            self._draw_route(p)
        self._draw_nodes(p)
        self._draw_legend(p)
        self._draw_compass(p)
        self._draw_scale(p)
        self._draw_header(p)

    def _draw_background(self, p: QPainter):
        grad = QLinearGradient(0, 0, 0, self.height())
        grad.setColorAt(0.0, QColor(0x1a, 0x1a, 0x2e))
        grad.setColorAt(1.0, QColor(0x16, 0x21, 0x3e))
        p.fillRect(self.rect(), grad)
        # grid
        p.setPen(QPen(QColor(255, 255, 255, 12), 0.5))
        for x in range(0, self.width(), 60):
            p.drawLine(x, 0, x, self.height())
        for y in range(0, self.height(), 60):
            p.drawLine(0, y, self.width(), y)

    def _draw_campus_area(self, p: QPainter):
        tl = self.lat_lon_to_pixel(LAT_MAX, LON_MIN)
        br = self.lat_lon_to_pixel(LAT_MIN, LON_MAX)
        rect = QRectF(tl, br)
        grad = QLinearGradient(tl, br)
        grad.setColorAt(0.0, QColor(0x0f, 0x3c, 0x60, 80))
        grad.setColorAt(1.0, QColor(0x0a, 0x29, 0x4a, 60))
        p.fillRect(rect, grad)
        p.setPen(QPen(QColor(0x00, 0xb4, 0xd8, 100), 1.5, Qt.PenStyle.DashLine))
        p.setBrush(Qt.BrushStyle.NoBrush)
        path = QPainterPath()
        path.addRoundedRect(rect, 8, 8)
        p.drawPath(path)

    def _draw_edges(self, p: QPainter):
        if not self.graph:
            return
        drawn = set()
        for id_ in self.graph.locations:
            for nb, dist in self.graph.all_neighbours(id_):
                key = (min(id_, nb), max(id_, nb))
                if key in drawn:
                    continue
                drawn.add(key)
                blocked = self.graph.edge_blocked(id_, nb)
                loc_a = self.graph.locations[id_]
                loc_b = self.graph.locations.get(nb)
                if not loc_b:
                    continue
                pa = self.lat_lon_to_pixel(loc_a.lat, loc_a.lon)
                pb = self.lat_lon_to_pixel(loc_b.lat, loc_b.lon)
                if blocked:
                    p.setPen(QPen(QColor(0xe6, 0x3b, 0x2e, 180), 2.5, Qt.PenStyle.DashLine))
                else:
                    p.setPen(QPen(QColor(0x48, 0xca, 0xe4, 160), 2.0))
                p.drawLine(pa, pb)
                if not blocked:
                    mid = QPointF((pa.x()+pb.x())/2, (pa.y()+pb.y())/2)
                    p.setPen(QColor(0xad, 0xe8, 0xf4, 180))
                    p.setFont(QFont("Segoe UI", 7))
                    p.drawText(mid, f"{int(dist)}m")

    def _draw_route(self, p: QPainter):
        if not self.route or not self.route.path:
            return
        n = len(self.route.path)
        count = n if self.anim_step < 0 else min(self.anim_step, n)

        for i in range(count - 1):
            a = self.graph.locations.get(self.route.path[i])
            b = self.graph.locations.get(self.route.path[i+1])
            if not a or not b:
                continue
            pa = self.lat_lon_to_pixel(a.lat, a.lon)
            pb = self.lat_lon_to_pixel(b.lat, b.lon)
            # glow
            p.setPen(QPen(QColor(0xff, 0xd6, 0x00, 60), 10))
            p.drawLine(pa, pb)
            # main line
            p.setPen(QPen(QColor(0xff, 0xd6, 0x00), 4))
            p.drawLine(pa, pb)
            # arrow
            dx, dy = pb.x()-pa.x(), pb.y()-pa.y()
            length = math.sqrt(dx*dx + dy*dy)
            if length > 20:
                dx, dy = dx/length, dy/length
                mx, my = (pa.x()+pb.x())/2, (pa.y()+pb.y())/2
                perp_x, perp_y = -dy, dx
                poly = QPolygonF([
                    QPointF(mx + dx*8, my + dy*8),
                    QPointF(mx - dx*4 + perp_x*4, my - dy*4 + perp_y*4),
                    QPointF(mx - dx*4 - perp_x*4, my - dy*4 - perp_y*4),
                ])
                p.setBrush(QColor(0xff, 0xd6, 0x00))
                p.setPen(Qt.PenStyle.NoPen)
                p.drawPolygon(poly)
                p.setBrush(Qt.BrushStyle.NoBrush)

        # Source node
        src_loc = self.graph.locations.get(self.route.path[0])
        if src_loc:
            sp = self.lat_lon_to_pixel(src_loc.lat, src_loc.lon)
            p.setBrush(QColor(0x06, 0xd6, 0xa0))
            p.setPen(QPen(QColor("white"), 2))
            p.drawEllipse(sp, 14, 14)
            p.setPen(QColor("white"))
            p.setFont(QFont("Segoe UI", 8, QFont.Weight.Bold))
            p.drawText(QRectF(sp.x()-14, sp.y()-14, 28, 28),
                       Qt.AlignmentFlag.AlignCenter, "S")

        # Destination node
        if count >= n and len(self.route.path) > 1:
            dst_loc = self.graph.locations.get(self.route.path[-1])
            if dst_loc:
                dp = self.lat_lon_to_pixel(dst_loc.lat, dst_loc.lon)
                p.setBrush(QColor(0xef, 0x47, 0x6f))
                p.setPen(QPen(QColor("white"), 2))
                p.drawEllipse(dp, 14, 14)
                p.setPen(QColor("white"))
                p.drawText(QRectF(dp.x()-14, dp.y()-14, 28, 28),
                           Qt.AlignmentFlag.AlignCenter, "D")

    def _draw_nodes(self, p: QPainter):
        if not self.graph:
            return
        for loc in self.graph.locations.values():
            pt = self.lat_lon_to_pixel(loc.lat, loc.lon)
            in_route = (self.route and self.route.found and
                        loc.id in self.route.path)
            highlighted = loc.id == self.highlighted_id
            radius = 14 if highlighted else (12 if in_route else 10)

            col = QColor(LOCATION_CATEGORIES.get(loc.category, "#909090"))
            if in_route or highlighted:
                glow = QRadialGradient(pt, radius*2)
                gc = QColor(col.red(), col.green(), col.blue(), 120)
                glow.setColorAt(0, gc)
                glow.setColorAt(1, QColor(col.red(), col.green(), col.blue(), 0))
                p.setBrush(glow)
                p.setPen(Qt.PenStyle.NoPen)
                p.drawEllipse(pt, radius*2, radius*2)

            p.setBrush(col)
            p.setPen(QPen(QColor("white"), 2.5 if highlighted else 1.5))
            p.drawEllipse(pt, radius, radius)

            icon = CATEGORY_ICONS.get(loc.category, "?")
            p.setPen(QColor("white"))
            p.setFont(QFont("Segoe UI", 7, QFont.Weight.Bold))
            p.drawText(QRectF(pt.x()-radius, pt.y()-radius, radius*2, radius*2),
                       Qt.AlignmentFlag.AlignCenter, icon)

            # Label
            p.setFont(QFont("Segoe UI", 7))
            fm = QFontMetrics(p.font())
            tw = fm.horizontalAdvance(loc.short_name)
            lbl_rect = QRectF(pt.x()-tw/2-3, pt.y()+radius+2, tw+6, 14)
            p.fillRect(lbl_rect, QColor(0, 0, 0, 140))
            p.setPen(QColor(0xe0, 0xe0, 0xff))
            p.drawText(lbl_rect, Qt.AlignmentFlag.AlignCenter, loc.short_name)

    def _draw_legend(self, p: QPainter):
        entries = [
            ("Gate", "#f77306"), ("Academic", "#0077b6"),
            ("Department", "#0296c7"), ("Library", "#48cb49"),
            ("Facility", "#ff9900"), ("Lab", "#00b4d8"),
            ("Sports", "#06d6a0"), ("Hostel", "#e06c75"),
            ("Health", "#ef476f"), ("Research", "#f4d03f"),
        ]
        x, y = 10, self.height() - 10
        p.setFont(QFont("Segoe UI", 7))
        for label, col in reversed(entries):
            y -= 18
            p.setBrush(QColor(col))
            p.setPen(QPen(QColor("white"), 1))
            p.drawEllipse(x, y+3, 10, 10)
            p.setPen(QColor(0xcc, 0xcc, 0xff))
            p.drawText(x+14, y+13, label)
        y -= 30
        p.fillRect(x, y+5, 24, 4, QColor(0xff, 0xd6, 0x00))
        p.setPen(QColor(0xcc, 0xcc, 0xff))
        p.drawText(x+28, y+13, "Shortest Path")
        y -= 16
        p.setPen(QPen(QColor(0xe6, 0x3b, 0x2e), 2, Qt.PenStyle.DashLine))
        p.drawLine(x, y+7, x+24, y+7)
        p.setPen(QColor(0xcc, 0xcc, 0xff))
        p.drawText(x+28, y+13, "Blocked Path")

    def _draw_compass(self, p: QPainter):
        cx, cy = self.width()-40, 40
        p.setBrush(QColor(0, 0, 0, 140))
        p.setPen(Qt.PenStyle.NoPen)
        p.drawEllipse(cx-20, cy-20, 40, 40)
        p.setPen(QPen(QColor("white"), 1))
        p.setFont(QFont("Segoe UI", 8, QFont.Weight.Bold))
        p.drawText(cx-4, cy-8, "N")
        p.setPen(QColor(0xef, 0x47, 0x6f))
        p.drawLine(cx, cy-2, cx, cy-14)
        p.setPen(QColor("white"))
        p.drawLine(cx, cy+2, cx, cy+14)

    def _draw_scale(self, p: QPainter):
        m_per_px = 338.0 / max(self.width(), 1) / max(self.scale, 0.01)
        bar_px = int(100 / m_per_px)
        bx = self.width() - bar_px - 15
        by = self.height() - 20
        p.setPen(QColor("white"))
        p.fillRect(bx, by, bar_px, 3, QColor("white"))
        p.drawLine(bx, by-3, bx, by+3)
        p.drawLine(bx+bar_px, by-3, bx+bar_px, by+3)
        p.setFont(QFont("Segoe UI", 7))
        p.setPen(QColor(0xcc, 0xcc, 0xff))
        p.drawText(bx + bar_px//2 - 10, by-5, "≈ 100 m")

    def _draw_header(self, p: QPainter):
        p.setFont(QFont("Segoe UI", 10, QFont.Weight.Bold))
        p.setPen(QColor(0xad, 0xe8, 0xf4, 180))
        p.drawText(10, 20, "VIT Pune — Bibwewadi Campus")
        p.setFont(QFont("Segoe UI", 7))
        p.setPen(QColor(0xff, 0x99, 0x00, 160))
        p.drawText(10, 36, "⚠ Building positions are APPROXIMATE – demonstration only")

    # ---- Mouse events ----
    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            loc_id = self._loc_at_pixel(QPointF(event.position()))
            if loc_id >= 0:
                if self.blocking_mode:
                    if self.block_from < 0:
                        self.block_from = loc_id
                        self.set_highlighted(loc_id)
                    else:
                        self.edge_block_requested.emit(self.block_from, loc_id)
                        self.block_from = -1
                        self.set_highlighted(-1)
                        self.blocking_mode = False
                else:
                    self.location_clicked.emit(loc_id)
            else:
                self._panning = True
                self._last_mouse = QPointF(event.position())
        elif event.button() == Qt.MouseButton.RightButton:
            self._panning = False
            self.blocking_mode = False
            self.block_from = -1

    def mouseMoveEvent(self, event):
        if self._panning:
            delta = QPointF(event.position()) - self._last_mouse
            self.pan_offset = QPointF(self.pan_offset.x()+delta.x(),
                                       self.pan_offset.y()+delta.y())
            self._last_mouse = QPointF(event.position())
            self.update()
        else:
            loc_id = self._loc_at_pixel(QPointF(event.position()))
            if loc_id != self.highlighted_id:
                self.highlighted_id = loc_id
                self.update()

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self._panning = False

    def wheelEvent(self, event):
        factor = 1.15 if event.angleDelta().y() > 0 else 0.87
        c = QPointF(event.position())
        self.pan_offset = QPointF(
            c.x() + (self.pan_offset.x()-c.x())*factor,
            c.y() + (self.pan_offset.y()-c.y())*factor
        )
        self.scale = max(0.4, min(self.scale * factor, 8.0))
        self.update()


# =============================================================================
#  MAIN WINDOW
# =============================================================================

STYLESHEET = """
QMainWindow, QWidget {
    background-color: #0d1b2a;
    color: #e0e0ff;
    font-family: "Segoe UI", Arial, sans-serif;
    font-size: 11px;
}
QGroupBox {
    border: 1px solid #1a3a5c;
    border-radius: 8px;
    margin-top: 8px;
    padding-top: 8px;
    font-weight: bold;
    color: #90e0ef;
    font-size: 11px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
}
QComboBox {
    background-color: #0d2137;
    border: 1px solid #1a3a5c;
    border-radius: 6px;
    padding: 5px 10px;
    color: #e0e0ff;
}
QComboBox::drop-down { border: none; }
QComboBox QAbstractItemView {
    background-color: #0d2137;
    border: 1px solid #1a3a5c;
    selection-background-color: #023e8a;
    color: #e0e0ff;
}
QLineEdit {
    background-color: #0d2137;
    border: 1px solid #1a3a5c;
    border-radius: 6px;
    padding: 6px 10px;
    color: #e0e0ff;
}
QLineEdit:focus { border-color: #90e0ef; }
QListWidget {
    background-color: #0a1628;
    border: 1px solid #1a3a5c;
    border-radius: 6px;
    color: #e0e0ff;
}
QListWidget::item:hover { background-color: #023e8a; }
QListWidget::item:selected { background-color: #0077b6; }
QTextEdit {
    background-color: #0a1628;
    border: 1px solid #1a3a5c;
    border-radius: 6px;
    color: #c0c8d8;
    font-size: 10px;
}
QPushButton {
    background-color: #023e8a;
    border: 1px solid #0077b6;
    border-radius: 8px;
    color: white;
    font-weight: bold;
    padding: 6px 12px;
}
QPushButton:hover { background-color: #0077b6; }
QPushButton:pressed { background-color: #0096c7; }
#findRouteBtn {
    background-color: #0096c7;
    border-color: #00b4d8;
    font-size: 13px;
}
#findRouteBtn:hover { background-color: #00b4d8; }
#compareBtn {
    background-color: #7b2d8b;
    border-color: #9b51a0;
}
#compareBtn:hover { background-color: #9b51a0; }
#resetBtn { background-color: #333d4a; border-color: #4a5568; }
QSplitter::handle { background-color: #1a3a5c; }
QMenuBar {
    background-color: #0a1628;
    color: #e0e0ff;
    border-bottom: 1px solid #1a3a5c;
}
QMenuBar::item:selected { background-color: #0077b6; border-radius: 4px; }
QMenu {
    background-color: #0d2137;
    border: 1px solid #1a3a5c;
    color: #e0e0ff;
}
QMenu::item:selected { background-color: #0077b6; }
QScrollBar:vertical { background: #0a1628; width: 8px; }
QScrollBar::handle:vertical { background: #1a3a5c; border-radius: 4px; }
QStatusBar { background: #0d1b2a; color: #6c757d; font-size: 10px; }
#headerWidget {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #0a1628, stop:1 #0d1f38);
    border-bottom: 1px solid #1a3a5c;
}
"""

ALG_DESCRIPTIONS = {
    "dijkstra": (
        "Dijkstra's Algorithm\n\n"
        "Guarantees the minimum-distance (shortest weighted) path.\n"
        "Uses a min-heap priority queue.\n\n"
        "Time:  O((V+E) log V)\n"
        "Space: O(V)\n\n"
        "Best for: Minimum walking distance routes."
    ),
    "a_star": (
        "A* Search\n\n"
        "Informed shortest path using Haversine geographic heuristic.\n"
        "Explores fewer nodes than Dijkstra using estimated remaining dist.\n\n"
        "Time:  O((V+E) log V)\n"
        "Space: O(V)\n\n"
        "Best for: Faster route finding using geography."
    ),
    "bfs": (
        "Breadth-First Search\n\n"
        "Minimum-hop path (ignores edge weights).\n"
        "Visits all nodes level by level.\n\n"
        "Time:  O(V+E)\n"
        "Space: O(V)\n\n"
        "Best for: Fewest intermediate stops.\n"
        "⚠ Does NOT guarantee minimum distance!"
    ),
    "dfs": (
        "Depth-First Search\n\n"
        "Graph traversal – explores deeply before backtracking.\n\n"
        "Time:  O(V+E)\n"
        "Space: O(V)\n\n"
        "⚠ Does NOT guarantee shortest path!\n"
        "Use for reachability analysis only."
    ),
    "floyd": (
        "Floyd-Warshall\n\n"
        "All-pairs shortest paths.\n"
        "Computes shortest routes between ALL pairs at once.\n\n"
        "Time:  O(V³)\n"
        "Space: O(V²)\n\n"
        "Best for: Pre-computing full routing tables.\n"
        "Slow for large graphs."
    ),
}


class MainWindow(QMainWindow):
    def __init__(self, data_dir: str):
        super().__init__()
        self.data_dir = data_dir
        self.controller = NavigationController()
        self.current_result: Optional[PathResult] = None
        self._anim_step = 0

        self.setWindowTitle("VIT Pune Campus Navigator")
        self.setMinimumSize(1280, 720)
        self.resize(1400, 820)

        self._setup_ui()
        self._apply_stylesheet()

        loaded = self.controller.load_all(data_dir)
        if loaded:
            self._populate_combos()
            self.map_widget.set_graph(self.controller.graph)
            n = len(self.controller.graph.locations)
            e = len(self.controller.graph.edges)
            self.statusBar().showMessage(
                f"✓ Campus loaded – {n} locations, {e} paths")
        else:
            self.statusBar().showMessage(
                f"⚠ Could not load campus data: {self.controller.last_error}")

        self._anim_timer = QTimer(self)
        self._anim_timer.timeout.connect(self._on_anim_tick)

    def _setup_ui(self):
        # Menu bar
        file_menu = self.menuBar().addMenu("&File")
        file_menu.addAction("&Reset Route", self._on_reset)
        file_menu.addSeparator()
        file_menu.addAction("E&xit", self.close)

        tools_menu = self.menuBar().addMenu("&Tools")
        tools_menu.addAction("&Compare All Algorithms", self._on_compare)
        tools_menu.addAction("Clear &Blocked Paths", self._on_clear_blocks)

        help_menu = self.menuBar().addMenu("&Help")
        help_menu.addAction("About", self._on_about)

        # Header
        header = QWidget()
        header.setObjectName("headerWidget")
        header.setFixedHeight(60)
        hl = QHBoxLayout(header)
        hl.setContentsMargins(16, 8, 16, 8)

        logo = QLabel("🗺")
        logo.setFont(QFont("Segoe UI", 22))
        title = QLabel("VIT Pune Campus Navigator")
        title.setFont(QFont("Segoe UI", 16, QFont.Weight.Bold))
        title.setStyleSheet("color: #90e0ef;")
        subtitle = QLabel("Vishwakarma Institute of Technology · Bibwewadi · Pune")
        subtitle.setFont(QFont("Segoe UI", 9))
        subtitle.setStyleSheet("color: #adb5bd;")
        tc = QVBoxLayout()
        tc.setSpacing(2)
        tc.addWidget(title)
        tc.addWidget(subtitle)
        hl.addWidget(logo)
        hl.addLayout(tc)
        hl.addStretch()
        ver = QLabel("DSA Project  |  v1.0  |  Graph Algorithms")
        ver.setStyleSheet("color:#6c757d;font-size:10px;")
        hl.addWidget(ver)

        # Splitter
        splitter = QSplitter(Qt.Orientation.Horizontal)
        splitter.setHandleWidth(2)
        self._setup_left_panel(splitter)
        self._setup_map(splitter)
        self._setup_right_panel(splitter)
        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)
        splitter.setStretchFactor(2, 0)
        splitter.setSizes([290, 820, 310])

        central = QWidget()
        ml = QVBoxLayout(central)
        ml.setSpacing(0)
        ml.setContentsMargins(0, 0, 0, 0)
        ml.addWidget(header)
        ml.addWidget(splitter, 1)
        self.setCentralWidget(central)

        self.setStatusBar(QStatusBar())

    def _setup_left_panel(self, parent):
        panel = QWidget()
        panel.setObjectName("leftPanel")
        panel.setFixedWidth(300)
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(10)

        # Search
        sg = QGroupBox("🔍 Search Locations")
        sl = QVBoxLayout(sg)
        self.search_box = QLineEdit()
        self.search_box.setPlaceholderText("Search buildings, labs, facilities...")
        self.search_box.setClearButtonEnabled(True)
        self.search_results = QListWidget()
        self.search_results.setMaximumHeight(100)
        self.search_results.setVisible(False)
        sl.addWidget(self.search_box)
        sl.addWidget(self.search_results)
        self.search_box.textChanged.connect(self._on_search)
        self.search_results.itemClicked.connect(self._on_search_item)
        layout.addWidget(sg)

        # Route
        rg = QGroupBox("🧭 Route Selection")
        rl = QGridLayout(rg)
        rl.setSpacing(8)
        src_lbl = QLabel("From:")
        src_lbl.setStyleSheet("color:#90e0ef;font-weight:bold;")
        self.source_combo = QComboBox()
        dst_lbl = QLabel("To:")
        dst_lbl.setStyleSheet("color:#ef476f;font-weight:bold;")
        self.dest_combo = QComboBox()
        rl.addWidget(src_lbl, 0, 0)
        rl.addWidget(self.source_combo, 0, 1)
        rl.addWidget(dst_lbl, 1, 0)
        rl.addWidget(self.dest_combo, 1, 1)
        layout.addWidget(rg)

        # Algorithm
        ag = QGroupBox("⚙ Algorithm")
        al = QVBoxLayout(ag)
        self.alg_combo = QComboBox()
        self.alg_combo.addItem("Dijkstra's Algorithm",       "dijkstra")
        self.alg_combo.addItem("A* Search (Haversine)",       "a_star")
        self.alg_combo.addItem("BFS (Min Hops)",              "bfs")
        self.alg_combo.addItem("DFS (Traversal Demo)",        "dfs")
        self.alg_combo.addItem("Floyd-Warshall (All Pairs)",  "floyd")
        al.addWidget(self.alg_combo)
        layout.addWidget(ag)
        self.alg_combo.currentIndexChanged.connect(self._on_alg_changed)

        # Buttons
        self.find_btn = QPushButton("▶  Find Route")
        self.find_btn.setObjectName("findRouteBtn")
        self.find_btn.setFixedHeight(42)
        self.compare_btn = QPushButton("📊  Compare All Algorithms")
        self.compare_btn.setObjectName("compareBtn")
        self.compare_btn.setFixedHeight(36)
        self.reset_btn = QPushButton("↺  Reset")
        self.reset_btn.setObjectName("resetBtn")
        self.reset_btn.setFixedHeight(36)
        self.clear_btn = QPushButton("🔓  Clear Blocked Paths")
        self.clear_btn.setFixedHeight(36)

        layout.addWidget(self.find_btn)
        layout.addWidget(self.compare_btn)
        br = QHBoxLayout()
        br.addWidget(self.reset_btn)
        br.addWidget(self.clear_btn)
        layout.addLayout(br)

        self.find_btn.clicked.connect(self._on_find_route)
        self.compare_btn.clicked.connect(self._on_compare)
        self.reset_btn.clicked.connect(self._on_reset)
        self.clear_btn.clicked.connect(self._on_clear_blocks)

        hint = QLabel(
            "<small style='color:#6c757d;'>"
            "💡 Click a node on the map to select it.<br>"
            "Pan: drag on empty area | Zoom: scroll wheel</small>")
        hint.setWordWrap(True)
        layout.addWidget(hint)
        layout.addStretch()
        parent.addWidget(panel)

    def _setup_map(self, parent):
        self.map_widget = MapWidget()
        self.map_widget.location_clicked.connect(self._on_map_click)
        self.map_widget.edge_block_requested.connect(self._on_block_edge)
        parent.addWidget(self.map_widget)

    def _setup_right_panel(self, parent):
        panel = QWidget()
        panel.setObjectName("rightPanel")
        panel.setFixedWidth(320)
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(10)

        # Route Details
        rg = QGroupBox("🗺 Route Details")
        rl = QVBoxLayout(rg)
        self.route_summary = QLabel("Select source and destination\nthen click Find Route.")
        self.route_summary.setWordWrap(True)
        self.route_summary.setStyleSheet("color:#90e0ef;font-weight:bold;")
        self.dist_label  = QLabel("Distance: —")
        self.dist_label.setStyleSheet("color:#06d6a0;")
        self.time_label  = QLabel("Est. Time: — (walking at 5 km/h)")
        self.time_label.setStyleSheet("color:#ffd166;")
        steps_lbl = QLabel("Route Steps:")
        steps_lbl.setStyleSheet("color:#adb5bd;font-size:10px;")
        self.steps_edit = QTextEdit()
        self.steps_edit.setReadOnly(True)
        self.steps_edit.setMinimumHeight(140)
        rl.addWidget(self.route_summary)
        rl.addWidget(self.dist_label)
        rl.addWidget(self.time_label)
        rl.addWidget(steps_lbl)
        rl.addWidget(self.steps_edit)
        layout.addWidget(rg)

        # Stats
        sg = QGroupBox("📊 Algorithm Statistics")
        sl = QGridLayout(sg)
        sl.setSpacing(6)
        def stat_row(label, colour):
            lbl = QLabel(label + ":")
            lbl.setStyleSheet("color:#adb5bd;font-size:10px;")
            val = QLabel("—")
            val.setStyleSheet(f"color:{colour};font-weight:bold;font-size:11px;")
            return lbl, val
        lbl, self.alg_name_lbl   = stat_row("Algorithm",      "#90e0ef")
        sl.addWidget(lbl, 0, 0); sl.addWidget(self.alg_name_lbl, 0, 1)
        lbl, self.nodes_lbl      = stat_row("Nodes Visited",  "#ffd166")
        sl.addWidget(lbl, 1, 0); sl.addWidget(self.nodes_lbl, 1, 1)
        lbl, self.edges_lbl      = stat_row("Edges Explored", "#ffd166")
        sl.addWidget(lbl, 2, 0); sl.addWidget(self.edges_lbl, 2, 1)
        lbl, self.exec_lbl       = stat_row("Exec Time",      "#06d6a0")
        sl.addWidget(lbl, 3, 0); sl.addWidget(self.exec_lbl, 3, 1)
        layout.addWidget(sg)

        # Description
        dg = QGroupBox("ℹ Algorithm Info")
        dl = QVBoxLayout(dg)
        self.desc_edit = QTextEdit()
        self.desc_edit.setReadOnly(True)
        self.desc_edit.setMinimumHeight(100)
        self.desc_edit.setPlainText(ALG_DESCRIPTIONS["dijkstra"])
        dl.addWidget(self.desc_edit)
        layout.addWidget(dg)

        layout.addStretch()
        parent.addWidget(panel)

    def _apply_stylesheet(self):
        self.setStyleSheet(STYLESHEET)

    def _populate_combos(self):
        self.source_combo.clear()
        self.dest_combo.clear()
        self.source_combo.addItem("— Select Starting Point —", 0)
        self.dest_combo.addItem("— Select Destination —", 0)
        for loc in sorted(self.controller.graph.locations.values(), key=lambda l: l.name):
            self.source_combo.addItem(loc.short_name, loc.id)
            self.dest_combo.addItem(loc.short_name, loc.id)
        # Defaults: Main Gate → Library
        for i in range(self.source_combo.count()):
            if self.source_combo.itemData(i) == 0:
                data = self.source_combo.itemData(i)
                for loc in self.controller.graph.locations.values():
                    if "Gate" in loc.category:
                        for j in range(self.source_combo.count()):
                            if self.source_combo.itemData(j) == loc.id:
                                self.source_combo.setCurrentIndex(j)
                                break
                        break
                break
        for i in range(self.dest_combo.count()):
            if self.dest_combo.itemData(i):
                loc_id = self.dest_combo.itemData(i)
                if loc_id and loc_id in self.controller.graph.locations:
                    if "Library" in self.controller.graph.locations[loc_id].name:
                        self.dest_combo.setCurrentIndex(i)
                        break

    # ---- Slots ----
    def _on_find_route(self):
        src_id = self.source_combo.currentData()
        dst_id = self.dest_combo.currentData()
        alg_key = self.alg_combo.currentData()

        if not src_id or not dst_id or src_id == dst_id:
            QMessageBox.warning(self, "Invalid Route",
                "Please select different source and destination locations.")
            return

        result = self.controller.find_route(src_id, dst_id, alg_key)
        self.current_result = result

        if result.found:
            self._display_result(result)
            self.map_widget.set_route(result)
            self._start_anim(result)
            src = self.controller.graph.locations[src_id].short_name
            dst = self.controller.graph.locations[dst_id].short_name
            self.statusBar().showMessage(
                f"✓ Route found: {src} → {dst} ({self._fmt_dist(result.total_distance)})")
        else:
            self.map_widget.clear_route()
            self._show_no_path()
            self.statusBar().showMessage("⚠ No path found. Check for blocked paths.")

    def _on_reset(self):
        self.map_widget.clear_route()
        self.route_summary.setText("Select source and destination\nthen click Find Route.")
        self.dist_label.setText("Distance: —")
        self.time_label.setText("Est. Time: — (walking at 5 km/h)")
        self.steps_edit.clear()
        self.alg_name_lbl.setText("—")
        self.nodes_lbl.setText("—")
        self.edges_lbl.setText("—")
        self.exec_lbl.setText("—")
        self._stop_anim()
        self.current_result = None
        self.statusBar().showMessage("Route cleared.")

    def _on_alg_changed(self):
        key = self.alg_combo.currentData()
        self.desc_edit.setPlainText(ALG_DESCRIPTIONS.get(key, ""))

    def _on_search(self, text):
        self.search_results.clear()
        if len(text) < 2:
            self.search_results.setVisible(False)
            return
        results = self.controller.search(text)
        if not results:
            self.search_results.setVisible(False)
            return
        for loc in results:
            item = QListWidgetItem(loc.name)
            item.setData(Qt.ItemDataRole.UserRole, loc.id)
            self.search_results.addItem(item)
        self.search_results.setVisible(True)

    def _on_search_item(self, item):
        loc_id = item.data(Qt.ItemDataRole.UserRole)
        # Set as dest if source already chosen
        src_id = self.source_combo.currentData()
        combo = self.dest_combo if src_id else self.source_combo
        for i in range(combo.count()):
            if combo.itemData(i) == loc_id:
                combo.setCurrentIndex(i)
                break
        self.search_results.setVisible(False)
        self.search_box.clear()
        self.map_widget.set_highlighted(loc_id)

    def _on_map_click(self, loc_id: int):
        src_id = self.source_combo.currentData()
        if not src_id:
            for i in range(self.source_combo.count()):
                if self.source_combo.itemData(i) == loc_id:
                    self.source_combo.setCurrentIndex(i)
                    break
        else:
            for i in range(self.dest_combo.count()):
                if self.dest_combo.itemData(i) == loc_id:
                    self.dest_combo.setCurrentIndex(i)
                    break
        self.map_widget.set_highlighted(loc_id)
        loc = self.controller.graph.locations.get(loc_id)
        if loc:
            self.statusBar().showMessage(
                f"📍 {loc.name}  |  {loc.category}  |  {loc.accuracy}")

    def _on_block_edge(self, a: int, b: int):
        if not self.controller.graph.edge_exists(a, b):
            self.statusBar().showMessage("⚠ No direct path between those locations.")
            return
        self.controller.block_edge(a, b)
        self.map_widget.set_graph(self.controller.graph)
        self.map_widget.clear_route()
        la = self.controller.graph.locations.get(a)
        lb = self.controller.graph.locations.get(b)
        if la and lb:
            self.statusBar().showMessage(
                f"🚧 Blocked: {la.short_name} ↔ {lb.short_name}")

    def _on_compare(self):
        src_id = self.source_combo.currentData()
        dst_id = self.dest_combo.currentData()
        if not src_id or not dst_id or src_id == dst_id:
            QMessageBox.information(self, "Compare",
                "Select a source and destination first.")
            return
        results = self.controller.compare_all(src_id, dst_id)
        lines = ["=== ALGORITHM COMPARISON ===\n"]
        for name, r in results.items():
            lines.append(f"• {r.algorithm_name}")
            if r.found:
                lines.append(f"  Distance:  {r.total_distance:.0f} m")
                lines.append(f"  Nodes:     {r.nodes_visited}")
                lines.append(f"  Edges:     {r.edges_explored}")
                lines.append(f"  Time:      {r.execution_time_ms:.3f} ms")
                lines.append(f"  Hops:      {len(r.path)-1}")
            else:
                lines.append("  → No path found")
            lines.append("")
        self.steps_edit.setPlainText("\n".join(lines))
        self.route_summary.setText("📊 Algorithm Comparison")
        # Show Dijkstra route on map
        d = results.get("Dijkstra")
        if d and d.found:
            self.map_widget.set_route(d)
            self.current_result = d
            self._update_stats(d)
            self.dist_label.setText(f"Distance: {self._fmt_dist(d.total_distance)}")
            self.time_label.setText(f"Est. Time: {self._fmt_time(d.total_time_min)}")

    def _on_clear_blocks(self):
        self.controller.clear_all_blocks()
        self.map_widget.set_graph(self.controller.graph)
        self.statusBar().showMessage("✓ All blocked paths cleared.")

    def _on_about(self):
        QMessageBox.about(self, "About VIT Campus Navigator",
            "<b>VIT Pune Campus Navigator</b><br>"
            "Version 1.0 — Python/PyQt6<br><br>"
            "DSA Project — Graph Algorithms Demo<br>"
            "Campus: Vishwakarma Institute of Technology,<br>"
            "Bibwewadi, Pune 411037<br><br>"
            "<small>Building positions are APPROXIMATE.<br>"
            "Based on OpenStreetMap data.</small>")

    def _on_anim_tick(self):
        self._anim_step += 1
        total = len(self.current_result.path) if self.current_result else 0
        if self._anim_step > total:
            self.map_widget.set_anim_step(-1)
            self._stop_anim()
            return
        self.map_widget.set_anim_step(self._anim_step)

    def _start_anim(self, result: PathResult):
        self._stop_anim()
        self._anim_step = 0
        self._anim_timer.start(200)

    def _stop_anim(self):
        self._anim_timer.stop()
        self._anim_step = 0

    def _display_result(self, result: PathResult):
        self._update_stats(result)
        g = self.controller.graph
        if result.path:
            src = g.locations.get(result.path[0])
            dst = g.locations.get(result.path[-1])
            if src and dst:
                self.route_summary.setText(
                    f"🟢 {src.name}\n🔴 {dst.name}")
        self.dist_label.setText(f"Distance: {self._fmt_dist(result.total_distance)}")
        self.time_label.setText(
            f"Est. Time: {self._fmt_time(result.total_time_min)} (walking 5 km/h)")
        lines = []
        for i, loc_id in enumerate(result.path):
            loc = g.locations.get(loc_id)
            if not loc:
                continue
            prefix = "🚶 START  " if i == 0 else (
                "📍 END    " if i == len(result.path)-1 else f"  {i}.  ")
            lines.append(f"{prefix}{loc.name}")
            if i + 1 < len(result.path):
                edge = g.get_edge(loc_id, result.path[i+1])
                if edge:
                    lines.append(f"         ↓ {int(edge.distance_m)} m")
        self.steps_edit.setPlainText("\n".join(lines))

    def _update_stats(self, result: PathResult):
        self.alg_name_lbl.setText(result.algorithm_name)
        self.nodes_lbl.setText(str(result.nodes_visited))
        self.edges_lbl.setText(str(result.edges_explored))
        self.exec_lbl.setText(f"{result.execution_time_ms:.3f} ms")

    def _show_no_path(self):
        self.route_summary.setText("⚠ No path found.\nCheck for blocked paths.")
        self.dist_label.setText("Distance: N/A")
        self.time_label.setText("Est. Time: N/A")
        self.steps_edit.setPlainText(
            "No route available between selected locations.\n"
            "Some paths may be blocked. Use 'Clear Blocked Paths' and try again.")

    @staticmethod
    def _fmt_dist(metres: float) -> str:
        if metres >= 1000:
            return f"{metres/1000:.2f} km"
        return f"{int(metres)} m"

    @staticmethod
    def _fmt_time(minutes: float) -> str:
        if minutes < 1:
            return f"{int(minutes*60)} sec"
        mins = int(minutes)
        secs = int((minutes - mins) * 60)
        if secs:
            return f"{mins} min {secs} sec"
        return f"{mins} min"


# =============================================================================
#  ENTRY POINT
# =============================================================================

def main():
    app = QApplication(sys.argv)
    app.setApplicationName("VIT Campus Navigator")
    app.setOrganizationName("VIT Pune")

    # Find data directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, "..", "data")
    if not os.path.isdir(data_dir):
        data_dir = os.path.join(script_dir, "data")
    if not os.path.isdir(data_dir):
        data_dir = os.path.join(os.getcwd(), "data")

    window = MainWindow(data_dir)
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
