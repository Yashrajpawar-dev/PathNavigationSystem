"""
Test script â€“ runs all graph algorithms without the GUI.
Run with: python test_algorithms.py
"""
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python_app'))

from main import (
    NavigationController, bfs, dfs, dijkstra, a_star, floyd_warshall
)

def run_tests():
    print("=" * 60)
    print("VIT Campus Navigator â€“ Algorithm Test Suite")
    print("=" * 60)

    ctrl = NavigationController()
    data_dir = os.path.join(os.path.dirname(__file__), '..', 'data')
    if not ctrl.load_all(data_dir):
        print(f"ERROR: {ctrl.last_error}")
        sys.exit(1)

    g = ctrl.graph
    print(f"\nGraph loaded:")
    print(f"  Locations : {len(g.locations)}")
    print(f"  Edges     : {len(g.edges)}")

    # Test all pairs with Dijkstra
    ids = list(g.locations.keys())
    source_id = 0   # Main Gate
    dest_id   = 8   # Library

    print(f"\n--- Dijkstra: Main Gate -> Library ---")
    r = dijkstra(g, source_id, dest_id)
    print(f"  Found     : {r.found}")
    if r.found:
        path_names = [g.locations[i].short_name for i in r.path]
        print(f"  Path      : {' â†’ '.join(path_names)}")
        print(f"  Distance  : {r.total_distance:.0f} m")
        print(f"  Est. Time : {r.total_time_min:.1f} min")
        print(f"  Nodes     : {r.nodes_visited}")
        print(f"  Edges     : {r.edges_explored}")
        print(f"  Time      : {r.execution_time_ms:.3f} ms")

    print(f"\n--- A*: Main Gate â†’ Library ---")
    r = a_star(g, source_id, dest_id)
    print(f"  Found     : {r.found}")
    if r.found:
        path_names = [g.locations[i].short_name for i in r.path]
        print(f"  Path      : {' â†’ '.join(path_names)}")
        print(f"  Distance  : {r.total_distance:.0f} m")
        print(f"  Nodes     : {r.nodes_visited}")
        print(f"  Time      : {r.execution_time_ms:.3f} ms")

    print(f"\n--- BFS: Main Gate â†’ Library ---")
    r = bfs(g, source_id, dest_id)
    print(f"  Found     : {r.found}")
    if r.found:
        path_names = [g.locations[i].short_name for i in r.path]
        print(f"  Path      : {' â†’ '.join(path_names)}")
        print(f"  Distance  : {r.total_distance:.0f} m")
        print(f"  Hops      : {len(r.path)-1}")

    print(f"\n--- DFS: Main Gate â†’ Library ---")
    r = dfs(g, source_id, dest_id)
    print(f"  Found     : {r.found}")
    if r.found:
        path_names = [g.locations[i].short_name for i in r.path]
        print(f"  Path      : {' â†’ '.join(path_names)}")
        print(f"  Distance  : {r.total_distance:.0f} m")

    print(f"\n--- Floyd-Warshall (all pairs) ---")
    ids_list, idx, dist_mat, nxt_mat, fw_ms = floyd_warshall(g)
    print(f"  Computed {len(ids_list)}x{len(ids_list)} matrix in {fw_ms:.3f} ms")
    si = idx[source_id]
    di = idx[dest_id]
    print(f"  Main Gate â†’ Library: {dist_mat[si][di]:.0f} m")

    print(f"\n--- Block edge test ---")
    ctrl.block_edge(1, 8)  # Block Admin â†’ Library
    r2 = dijkstra(g, source_id, dest_id)
    if r2.found:
        path_names = [g.locations[i].short_name for i in r2.path]
        print(f"  Alt path  : {' â†’ '.join(path_names)}")
        print(f"  Distance  : {r2.total_distance:.0f} m")
    else:
        print("  No alternate path found!")

    ctrl.unblock_edge(1, 8)
    print(f"\n--- All pairs reachability ---")
    ids_list = list(g.locations.keys())
    reachable = 0
    for a in ids_list:
        for b in ids_list:
            if a != b:
                r = dijkstra(g, a, b)
                if r.found:
                    reachable += 1
    total_pairs = len(ids_list) * (len(ids_list) - 1)
    print(f"  Reachable pairs: {reachable}/{total_pairs} ({reachable/total_pairs*100:.1f}%)")

    print("\n" + "=" * 60)
    print("All tests passed!")
    print("=" * 60)

if __name__ == "__main__":
    run_tests()

