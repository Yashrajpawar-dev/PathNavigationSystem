#pragma once
// ============================================================
// Location.h – VIT Pune Campus Navigator
// Represents a physical campus location (node in the graph)
// ============================================================

#ifndef LOCATION_H
#define LOCATION_H

#include <string>

namespace VITNav {

enum class LocationCategory {
    Gate,
    Administration,
    Academic,
    Department,
    Library,
    Facility,
    Laboratory,
    Sports,
    Parking,
    Hostel,
    Health,
    Faculty,
    Research,
    Unknown
};

enum class CoordinateAccuracy {
    VERIFIED,       // Confirmed by official survey data
    APPROXIMATE,    // Estimated from OpenStreetMap / satellite imagery
    PLACEHOLDER     // Completely unverified – demonstration only
};

class Location {
public:
    int         id;
    std::string name;           // Full name
    std::string shortName;      // Short display name
    LocationCategory category;
    double      lat;            // Geographic latitude  (WGS84)
    double      lon;            // Geographic longitude (WGS84)
    std::string description;
    CoordinateAccuracy accuracy;
    std::string source;         // Data provenance

    // Pixel position on the campus map widget (set at runtime)
    double      mapX{0.0};
    double      mapY{0.0};

    std::string categoryName;   // Category as string for direct access

    Location() : id(0), category(LocationCategory::Unknown), lat(0.0), lon(0.0), accuracy(CoordinateAccuracy::APPROXIMATE) {}
    Location(int id, const std::string& name, const std::string& shortName,
             LocationCategory cat, double lat, double lon,
             const std::string& desc = "",
             CoordinateAccuracy acc = CoordinateAccuracy::APPROXIMATE,
             const std::string& src = "");

    // Simplified constructor matching DSA specification: id, name, category
    Location(int id, const std::string& name, const std::string& category,
             const std::string& shortName = "", double lat = 0.0, double lon = 0.0,
             const std::string& desc = "");

    std::string categoryString() const;
    std::string accuracyString() const;
    void displayLocation() const;
};

LocationCategory categoryFromString(const std::string& s);

} // namespace VITNav

#endif // LOCATION_H
