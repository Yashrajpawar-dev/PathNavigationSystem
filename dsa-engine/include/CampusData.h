#pragma once
// ============================================================
// CampusData.h – Campus Navigation System
// Centralized campus data loader: JSON parser + embedded data fallback
// ============================================================

#ifndef CAMPUS_DATA_H
#define CAMPUS_DATA_H

#include "CampusGraph.h"
#include <string>

namespace VITNav {

class CampusData {
public:
    // Attempts to load from JSON files in dataDir
    static bool loadFromJsonFiles(CampusGraph& graph, const std::string& dataDir);

    // Centralized realistic sample campus data (VIT Pune - 20 facilities & walkways)
    // Ensures the system can run standalone without requiring external files
    static void loadCentralizedSampleData(CampusGraph& graph);

    // Dynamic Walkway Blocking Persistence
    static std::string resolveDataDir();
    static bool loadBlockedWalkways(CampusGraph& graph, const std::string& dataDir = "");
    static bool saveBlockedWalkways(const CampusGraph& graph, const std::string& dataDir = "");

private:
    static std::string readFile(const std::string& filepath);
    static std::string jsonValue(const std::string& obj, const std::string& key);
    static std::vector<std::string> splitJsonArray(const std::string& json, const std::string& arrayKey);
};

} // namespace VITNav

#endif // CAMPUS_DATA_H
