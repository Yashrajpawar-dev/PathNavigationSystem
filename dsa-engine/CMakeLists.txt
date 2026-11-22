cmake_minimum_required(VERSION 3.16)
project(VITCampusNavigator VERSION 1.0.0 LANGUAGES CXX)

# ---- C++ Standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Qt6 ----
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# ---- Source files ----
set(SOURCES
    src/main.cpp
    src/Location.cpp
    src/Edge.cpp
    src/Graph.cpp
    src/Algorithms.cpp
    src/BFS.cpp
    src/DFS.cpp
    src/Dijkstra.cpp
    src/AStar.cpp
    src/NavigationController.cpp
    ui/MainWindow.cpp
    ui/MapWidget.cpp
)

set(HEADERS
    include/Location.h
    include/Edge.h
    include/Graph.h
    include/Algorithms.h
    include/NavigationController.h
    ui/MainWindow.h
    ui/MapWidget.h
)

# ---- Resources ----
set(RESOURCES resources/resources.qrc)

# ---- Executable ----
qt_add_executable(VITCampusNavigator
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# ---- Include paths ----
target_include_directories(VITCampusNavigator PRIVATE
    include
    ui
)

# ---- Link Qt ----
target_link_libraries(VITCampusNavigator PRIVATE
    Qt6::Core
    Qt6::Widgets
)

# ---- Copy data directory to build ----
add_custom_command(TARGET VITCampusNavigator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:VITCampusNavigator>/data"
    COMMENT "Copying campus data files..."
)

# ---- Windows: deploy Qt DLLs ----
if(WIN32)
    # windeployqt copies required Qt DLLs next to the executable
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        add_custom_command(TARGET VITCampusNavigator POST_BUILD
            COMMAND "${WINDEPLOYQT}" --no-compiler-runtime
                "$<TARGET_FILE:VITCampusNavigator>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()

# ---- Install ----
install(TARGETS VITCampusNavigator DESTINATION bin)
install(DIRECTORY data DESTINATION bin)
