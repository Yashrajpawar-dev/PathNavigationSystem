@echo off
REM ============================================================
REM build.bat - Build script for Campus Navigation System
REM Run from D:\PathNavigationSystem\
REM ============================================================

set CXX=C:\msys64\ucrt64\bin\g++.exe
if not exist "%CXX%" (
    where g++ >nul 2>nul
    if %errorlevel% equ 0 (
        set CXX=g++
    ) else (
        echo [ERROR] g++ compiler not found in C:\msys64\ucrt64\bin or PATH.
        exit /b 1
    )
)

echo [INFO] Using C++ Compiler: %CXX%
echo [INFO] Compiling Campus Navigation System DSA Engine...

%CXX% -std=c++17 -O2 -I"dsa-engine/include" ^
    "dsa-engine/src/Location.cpp" ^
    "dsa-engine/src/Edge.cpp" ^
    "dsa-engine/src/Graph.cpp" ^
    "dsa-engine/src/Algorithms.cpp" ^
    "dsa-engine/src/BFS.cpp" ^
    "dsa-engine/src/DFS.cpp" ^
    "dsa-engine/src/Dijkstra.cpp" ^
    "dsa-engine/src/AStar.cpp" ^
    "dsa-engine/src/NavigationController.cpp" ^
    "dsa-engine/src/CampusGraph.cpp" ^
    "dsa-engine/src/Navigation.cpp" ^
    "dsa-engine/src/RouteManager.cpp" ^
    "dsa-engine/src/FacilityFinder.cpp" ^
    "dsa-engine/src/PathManager.cpp" ^
    "dsa-engine/src/CampusData.cpp" ^
    "dsa-engine/src/CampusNavigation.cpp" ^
    "dsa-engine/src/main.cpp" ^
    -o campus_nav.exe

if %errorlevel% equ 0 (
    echo [SUCCESS] campus_nav.exe compiled successfully!
    echo Run "campus_nav.exe" for the interactive CLI.
    echo Run "campus_nav.exe --test" to run all 13 DSA tests.
) else (
    echo [ERROR] Build failed! Check compiler errors above.
    exit /b 1
)
