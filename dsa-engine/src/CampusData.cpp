#include "CampusData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

namespace VITNav {

std::string CampusData::readFile(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return "";
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string CampusData::jsonValue(const std::string& obj, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = obj.find(searchKey);
    if (pos == std::string::npos) return "";
    pos = obj.find(':', pos + searchKey.size());
    if (pos == std::string::npos) return "";
    ++pos;
    while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t' ||
                                 obj[pos] == '\n' || obj[pos] == '\r')) ++pos;
    if (pos >= obj.size()) return "";
    if (obj[pos] == '"') {
        ++pos;
        std::string val;
        while (pos < obj.size() && obj[pos] != '"') {
            if (obj[pos] == '\\' && pos + 1 < obj.size()) { ++pos; }
            val += obj[pos++];
        }
        return val;
    } else {
        std::string val;
        while (pos < obj.size() && obj[pos] != ',' && obj[pos] != '}' &&
               obj[pos] != '\n' && obj[pos] != '\r') {
            val += obj[pos++];
        }
        while (!val.empty() && std::isspace((unsigned char)val.back())) val.pop_back();
        return val;
    }
}

std::vector<std::string> CampusData::splitJsonArray(const std::string& json,
                                                    const std::string& arrayKey) {
    std::vector<std::string> items;
    std::string searchKey = "\"" + arrayKey + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return items;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return items;
    ++pos;

    int depth = 0;
    std::string cur;
    for (; pos < json.size(); ++pos) {
        char c = json[pos];
        if (c == '{') { depth++; cur += c; }
        else if (c == '}') {
            cur += c; --depth;
            if (depth == 0) { items.push_back(cur); cur.clear(); }
        } else if (c == ']' && depth == 0) {
            break;
        } else {
            cur += c;
        }
    }
    return items;
}

bool CampusData::loadFromJsonFiles(CampusGraph& graph, const std::string& dataDir) {
    std::string sep = (!dataDir.empty() && (dataDir.back() == '/' || dataDir.back() == '\\')) ? "" : "/";
    std::string locFile = dataDir + sep + "locations.json";
    std::string connFile = dataDir + sep + "connections.json";

    std::string locContent = readFile(locFile);
    std::string connContent = readFile(connFile);

    if (locContent.empty() || connContent.empty()) {
        return false;
    }

    graph.clear();

    // Parse locations
    auto locItems = splitJsonArray(locContent, "locations");
    for (const auto& item : locItems) {
        try {
            int id = std::stoi(jsonValue(item, "id"));
            std::string name = jsonValue(item, "name");
            std::string shortName = jsonValue(item, "short_name");
            std::string category = jsonValue(item, "category");
            double lat = 0.0, lon = 0.0;
            std::string sLat = jsonValue(item, "lat");
            std::string sLon = jsonValue(item, "lon");
            if (!sLat.empty()) lat = std::stod(sLat);
            if (!sLon.empty()) lon = std::stod(sLon);
            std::string desc = jsonValue(item, "description");

            Location loc(id, name, category, shortName, lat, lon, desc);
            graph.addLocation(loc);
        } catch (...) {
            continue;
        }
    }

    // Parse connections
    auto connItems = splitJsonArray(connContent, "connections");
    for (const auto& item : connItems) {
        try {
            int fromId = std::stoi(jsonValue(item, "from_id"));
            int toId = std::stoi(jsonValue(item, "to_id"));
            double dist = std::stod(jsonValue(item, "distance_m"));
            std::string pType = jsonValue(item, "path_type");
            std::string desc = jsonValue(item, "description");
            std::string blk = jsonValue(item, "blocked");
            bool blocked = (blk == "true");

            if (!graph.edgeExists(fromId, toId)) {
                graph.addEdge(fromId, toId, dist, -1.0, pType.empty() ? "paved_path" : pType, desc);
            }
            if (blocked) {
                graph.blockWalkway(fromId, toId);
            }
        } catch (...) {
            continue;
        }
    }

    return (graph.numLocations() > 0 && graph.numEdges() > 0);
}

void CampusData::loadCentralizedSampleData(CampusGraph& graph) {
    graph.clear();

    // 20 Realistic Campus Locations (VIT Pune Bibwewadi Campus)
    graph.addLocation(Location(0,  "Main Gate (Entry/Exit)", "Gate", "Main Gate", 18.4631, 73.8676, "Primary entrance/exit on main road"));
    graph.addLocation(Location(1,  "Administrative Block", "Administration", "Admin Block", 18.4638, 73.8680, "Principal office, registrar & admin"));
    graph.addLocation(Location(2,  "Main Building (A-Block)", "Academic", "A-Block", 18.4645, 73.8682, "Central lecture halls and classrooms"));
    graph.addLocation(Location(3,  "Computer Engineering Dept", "Department", "CE Dept", 18.4648, 73.8686, "Computer Engg labs & faculty cabins"));
    graph.addLocation(Location(4,  "Information Technology Dept", "Department", "IT Dept", 18.4650, 73.8689, "IT department classrooms and labs"));
    graph.addLocation(Location(5,  "Electronics & TC Dept", "Department", "E&TC Dept", 18.4652, 73.8683, "E&TC department and research labs"));
    graph.addLocation(Location(6,  "Mechanical Engineering Dept", "Department", "Mech Dept", 18.4647, 73.8678, "Mechanical Engg workshops & CAD lab"));
    graph.addLocation(Location(7,  "Civil Engineering Dept", "Department", "Civil Dept", 18.4644, 73.8690, "Civil department and structural lab"));
    graph.addLocation(Location(8,  "Central Campus Library", "Library", "Library", 18.4642, 73.8685, "Multi-floor library & reading rooms"));
    graph.addLocation(Location(9,  "Canteen / Food Court", "Facility", "Canteen", 18.4639, 73.8688, "Student dining hall and cafeteria"));
    graph.addLocation(Location(10, "Auditorium & Convocation Hall", "Facility", "Auditorium", 18.4636, 73.8685, "Major events and cultural auditorium"));
    graph.addLocation(Location(11, "Computer Lab Complex", "Laboratory", "Comp Labs", 18.4649, 73.8687, "High-performance computing cluster"));
    graph.addLocation(Location(12, "Sports Ground & Gymnasium", "Sports", "Sports Ground", 18.4633, 73.8692, "Athletics track, football field & gym"));
    graph.addLocation(Location(13, "Parking Area (Two/Four Wheeler)", "Parking", "Parking", 18.4630, 73.8681, "Student and staff parking facility"));
    graph.addLocation(Location(14, "Workshop & Manufacturing Lab", "Laboratory", "Workshop", 18.4646, 73.8675, "CNC machines and fabrication workshop"));
    graph.addLocation(Location(15, "Research & Innovation Centre", "Research", "R&I Centre", 18.4653, 73.8687, "Industry projects & innovation incubation"));
    graph.addLocation(Location(16, "Boys Hostel Complex", "Hostel", "Boys Hostel", 18.4655, 73.8695, "On-campus boys residence halls"));
    graph.addLocation(Location(17, "Girls Hostel Complex", "Hostel", "Girls Hostel", 18.4654, 73.8672, "On-campus girls residence halls"));
    graph.addLocation(Location(18, "Campus Medical Centre", "Health", "Medical Centre", 18.4635, 73.8677, "First aid, emergency care & dispensary"));
    graph.addLocation(Location(19, "Faculty Quarters & Staff Block", "Faculty", "Faculty Block", 18.4658, 73.8680, "Staff residence and guest house"));

    // Realistic Walkway Network
    graph.addEdge(0, 13, 50.0,  0.6, "paved_road", "Main Gate to Parking Area");
    graph.addEdge(0, 1,  120.0, 1.4, "paved_path", "Main Gate to Admin Block");
    graph.addEdge(0, 18, 90.0,  1.1, "paved_path", "Main Gate to Medical Centre");
    graph.addEdge(13, 1, 80.0,  1.0, "paved_path", "Parking to Admin Block");
    graph.addEdge(1, 2,  100.0, 1.2, "paved_path", "Admin Block to A-Block");
    graph.addEdge(1, 9,  150.0, 1.8, "paved_path", "Admin Block to Canteen");
    graph.addEdge(1, 8,  130.0, 1.6, "paved_path", "Admin Block to Library");
    graph.addEdge(18, 9, 100.0, 1.2, "paved_path", "Medical Centre to Canteen");
    graph.addEdge(2, 3,  80.0,  1.0, "corridor",   "A-Block to CE Department");
    graph.addEdge(2, 6,  70.0,  0.8, "corridor",   "A-Block to Mech Department");
    graph.addEdge(2, 8,  60.0,  0.7, "paved_path", "A-Block to Library");
    graph.addEdge(2, 10, 180.0, 2.2, "paved_path", "A-Block to Auditorium");
    graph.addEdge(3, 4,  50.0,  0.6, "corridor",   "CE Department to IT Department");
    graph.addEdge(3, 11, 90.0,  1.1, "corridor",   "CE Department to Computer Labs");
    graph.addEdge(3, 5,  100.0, 1.2, "corridor",   "CE Department to E&TC Department");
    graph.addEdge(4, 11, 60.0,  0.7, "corridor",   "IT Department to Computer Labs");
    graph.addEdge(4, 7,  120.0, 1.4, "paved_path", "IT Department to Civil Dept");
    graph.addEdge(5, 15, 80.0,  1.0, "paved_path", "E&TC to R&I Centre");
    graph.addEdge(5, 10, 120.0, 1.4, "paved_path", "E&TC to Auditorium");
    graph.addEdge(6, 14, 110.0, 1.3, "paved_path", "Mech Dept to Workshop");
    graph.addEdge(6, 12, 200.0, 2.4, "paved_path", "Mech Dept to Sports Ground");
    graph.addEdge(7, 14, 100.0, 1.2, "paved_path", "Civil Dept to Workshop");
    graph.addEdge(8, 9,  170.0, 2.0, "paved_path", "Library to Canteen");
    graph.addEdge(10, 15, 90.0, 1.1, "paved_path", "Auditorium to R&I Centre");
    graph.addEdge(10, 12, 150.0, 1.8, "paved_path", "Auditorium to Sports Ground");
    graph.addEdge(11, 15, 120.0, 1.4, "paved_path", "Computer Labs to R&I Centre");
    graph.addEdge(12, 16, 80.0, 1.0, "paved_path", "Sports Ground to Boys Hostel");
    graph.addEdge(12, 17, 120.0, 1.4, "paved_path", "Sports Ground to Girls Hostel");
    graph.addEdge(15, 16, 100.0, 1.2, "paved_path", "R&I Centre to Boys Hostel");
    graph.addEdge(15, 19, 80.0,  1.0, "paved_path", "R&I Centre to Faculty Block");
    graph.addEdge(16, 19, 90.0,  1.1, "paved_path", "Boys Hostel to Faculty Block");
    graph.addEdge(14, 11, 130.0, 1.6, "paved_path", "Workshop to Computer Labs");
}

std::string CampusData::resolveDataDir() {
    std::ifstream f0("dsa-engine/data/locations.json");
    if (f0.is_open()) return "dsa-engine/data";
    std::ifstream f0b("../dsa-engine/data/locations.json");
    if (f0b.is_open()) return "../dsa-engine/data";
    std::ifstream f1("data/locations.json");
    if (f1.is_open()) return "data";
    std::ifstream f2("../data/locations.json");
    if (f2.is_open()) return "../data";
    std::ifstream f3("DSA project/DSA project/VIT-Campus-Navigator/data/locations.json");
    if (f3.is_open()) return "DSA project/DSA project/VIT-Campus-Navigator/data";
    return "dsa-engine/data";
}

bool CampusData::loadBlockedWalkways(CampusGraph& graph, const std::string& dataDir) {
    std::string dir = dataDir.empty() ? resolveDataDir() : dataDir;
    std::string sep = (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) ? "" : "/";
    std::string filepath = dir + sep + "blocked_walkways.json";

    std::string content = readFile(filepath);
    if (content.empty()) return false;

    auto items = splitJsonArray(content, "blocked_walkways");
    if (items.empty()) {
        // Try parsing directly as an array of objects
        size_t pos = content.find('[');
        if (pos != std::string::npos) {
            int depth = 0;
            std::string cur;
            for (pos++; pos < content.size(); ++pos) {
                char c = content[pos];
                if (c == '{') { depth++; cur += c; }
                else if (c == '}') {
                    cur += c; --depth;
                    if (depth == 0) { items.push_back(cur); cur.clear(); }
                } else if (c == ']' && depth == 0) break;
                else cur += c;
            }
        }
    }

    for (const auto& item : items) {
        try {
            int u = std::stoi(jsonValue(item, "from_id"));
            int v = std::stoi(jsonValue(item, "to_id"));
            graph.blockWalkway(u, v);
        } catch (...) {
            continue;
        }
    }
    return true;
}

bool CampusData::saveBlockedWalkways(const CampusGraph& graph, const std::string& dataDir) {
    std::string dir = dataDir.empty() ? resolveDataDir() : dataDir;
    std::string sep = (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) ? "" : "/";
    std::string filepath = dir + sep + "blocked_walkways.json";

    auto blocked = graph.getBlockedEdges();

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "{\n  \"blocked_walkways\": [\n";
    for (size_t i = 0; i < blocked.size(); ++i) {
        out << "    {\"from_id\": " << blocked[i].sourceId
            << ", \"to_id\": " << blocked[i].destinationId << "}";
        if (i + 1 < blocked.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

} // namespace VITNav
