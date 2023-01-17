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
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);

    domain::Stop* FindStop(const std::string_view& name) const;
    domain::Bus* FindBus(const std::string_view& name) const;

    void AddDistance(const std::string& stop1, const std::string& stop2, double distance);
    double GetDistance(const std::string& stop1, const std::string& stop2) const;
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> GetAllDistances() const;

    domain::StopInfo GetStopInfo(const std::string_view& name) const;
    domain::BusInfo GetBusInfo(const std::string_view& name) const;

    std::vector<const domain::Bus*> GetBuses() const;
    std::vector<const domain::Stop*> GetStops() const;
    std::vector<domain::Stop*> GetBusStops(const domain::Bus* bus) const;

    bool HasBuses(const domain::Stop* stop) const;
    std::unordered_set<const domain::Bus*> GetBusesByStop(std::string_view stop_name) const;

private:
    std::deque<domain::Stop> base_stops_;
    std::deque<domain::Bus> base_buses_;

    std::unordered_map<std::string_view, domain::Stop*> stops_;
    std::unordered_map<std::string_view, domain::Bus*> buses_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_for_buses_;
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> distance_between_stops_;
};

} // namespace transport
