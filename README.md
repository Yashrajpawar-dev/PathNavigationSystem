Campus Navigation System — DSA Course Project

**1. Project Overview**
- Language: C++
- Purpose: Help students find locations and shortest routes within a college campus.
- Core concept: Represent the campus as a graph where buildings are vertices and walkways are edges.
- Main goal: Demonstrate important Data Structures and Algorithms through a real-world application.
  
**2. Important Features**
1. Display all campus locations.
2. Search for a building by name.
3. Find the shortest-distance route between two locations.
4. Find a route with the fewest walkway segments.
5. Explore connected locations.
6. Display reachable locations.
7. Sort locations by name or distance.
8. Add or block walkways and recalculate routes.

**3. Architecture**
   
                                                          1. User Interface
                                              Select source, destination, or an operation
                                                                  |
                                                                  \/
                                                        2. Location Management
                                                    Store and search building details
                                                                  |
                                                                  \/

                                                            3. Campus Graph
                                              Vertices + weighted edges + adjacency list
                                                                  |
                                                                  \/

                                                                BFS / DFS
                                                              Graph traversal
                                                                    
                                                                    OR
                                                                    
                                                                 Dijkstra
                                                            Shortest distance
                                                                    |
                                                                    \/


                                                          4. Route Reconstruction
                                              Determine the sequence of locations and total distance
                                                                    |
                                                                    \/

                                                            5. Display Results
                                              Show route, distance, and intermediate locations
                                              

**4. Project Structure**

```text
PathNavigationSystem/
├── .vscode/                  # VS Code build tasks
├── build/                    # Generated executable and build output (ignored by Git)
├── Docs/                     # Project documentation
├── include/
│   ├── algorithms/           # Public interfaces for BFS, DFS, and Dijkstra
│   ├── graph/                # Campus graph and walkway interfaces
│   ├── models/               # Campus locations, edges, and route result types
│   └── services/             # Location and route management interfaces
├── Resources/                # Campus map or other project data
├── src/
│   ├── algorithms/           # Graph algorithm implementations
│   ├── graph/                # Campus graph implementations
│   ├── services/             # Location and route management implementations
│   └── main.cpp              # Application entry point and terminal menu
└── tests/                    # Automated tests
```

Keep declarations in `include/` and their implementations in the corresponding `src/` subdirectories. The terminal menu and input/output can live in `src/main.cpp` for now; a separate UI module is unnecessary. `include/models/CampusMap.h` currently defines the location, edge, and route-result data types; feature-specific modules can be added as the project grows.

**5. Basic Working**
   
1. The user selects a starting location and destination.
2. The system identifies their location IDs.
3. The graph provides connected walkways and their distances.
4. The appropriate algorithm calculates the route.
5. The system displays the route and total distance.

   
**6. Expected Outcome**

A working C++ application that demonstrates how graphs and other DSA concepts can solve a practical campus navigation problem.
