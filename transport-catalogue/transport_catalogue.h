#pragma once

#include "domain.h"
#include "geo.h"

#include <string>
#include <deque>
#include <utility>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace transport {

class Catalogue
{
public:
    void AddStop(const std::string& name, double lat, double lng);
    void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);

    domain::Stop* FindStop(const std::string_view& name) const;
    domain::Route* FindRoute(const std::string_view& name) const;

    void AddDistance(const std::string& stop1, const std::string& stop2, double distance);
    double GetDistance(const std::string& stop1, const std::string& stop2) const;

    domain::StopInfo GetStopInfo(const std::string_view& name) const;
    domain::RouteInfo GetRouteInfo(const std::string_view& name) const;

    const std::vector<const domain::Route*> GetRoutes() const;
    const std::vector<const domain::Stop*> GetStops() const;

    bool HasRoutes(const domain::Stop* stop) const;
    std::unordered_set<const domain::Route*> GetRoutesByStop(std::string_view stop_name) const;

private:
    std::deque<domain::Stop> base_stops_;
    std::deque<domain::Route> base_routes_;

    std::unordered_map<std::string_view, domain::Stop*> stops_;
    std::unordered_map<std::string_view, domain::Route*> routes_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_for_buses_;
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> distance_between_stops_;
};

} // namespace transport
