// ============================================================
// Location.cpp – VIT Pune Campus Navigator
// ============================================================

#include "Location.h"
#include <algorithm>
#include <cctype>

namespace VITNav {

Location::Location(int id, const std::string& name, const std::string& shortName,
                   LocationCategory cat, double lat, double lon,
                   const std::string& desc,
                   CoordinateAccuracy acc,
                   const std::string& src)
    : id(id), name(name), shortName(shortName), category(cat),
      lat(lat), lon(lon), description(desc), accuracy(acc), source(src)
{}

std::string Location::categoryString() const {
    switch (category) {
        case LocationCategory::Gate:           return "Gate";
        case LocationCategory::Administration: return "Administration";
        case LocationCategory::Academic:       return "Academic";
        case LocationCategory::Department:     return "Department";
        case LocationCategory::Library:        return "Library";
        case LocationCategory::Facility:       return "Facility";
        case LocationCategory::Laboratory:     return "Laboratory";
        case LocationCategory::Sports:         return "Sports";
        case LocationCategory::Parking:        return "Parking";
        case LocationCategory::Hostel:         return "Hostel";
        case LocationCategory::Health:         return "Health";
        case LocationCategory::Faculty:        return "Faculty";
        case LocationCategory::Research:       return "Research";
        default:                               return "Unknown";
    }
}

std::string Location::accuracyString() const {
    switch (accuracy) {
        case CoordinateAccuracy::VERIFIED:    return "VERIFIED";
        case CoordinateAccuracy::APPROXIMATE: return "APPROXIMATE";
        case CoordinateAccuracy::PLACEHOLDER: return "PLACEHOLDER";
        default:                              return "UNKNOWN";
    }
}

LocationCategory categoryFromString(const std::string& s) {
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "gate")           return LocationCategory::Gate;
    if (lower == "administration") return LocationCategory::Administration;
    if (lower == "academic")       return LocationCategory::Academic;
    if (lower == "department")     return LocationCategory::Department;
    if (lower == "library")        return LocationCategory::Library;
    if (lower == "facility")       return LocationCategory::Facility;
    if (lower == "laboratory")     return LocationCategory::Laboratory;
    if (lower == "sports")         return LocationCategory::Sports;
    if (lower == "parking")        return LocationCategory::Parking;
    if (lower == "hostel")         return LocationCategory::Hostel;
    if (lower == "health")         return LocationCategory::Health;
    if (lower == "faculty")        return LocationCategory::Faculty;
    if (lower == "research")       return LocationCategory::Research;
    return LocationCategory::Unknown;
}

} // namespace VITNav
