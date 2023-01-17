#include "transport_catalogue.h"
#include <unordered_set>
#include <iostream>
#include <memory>
#include <algorithm>

namespace transport {

void Catalogue::AddStop(const std::string& name, double lat, double lng)
{
    geo::Coordinates coordinates = {lat, lng};

    auto *stop = new domain::Stop();
    stop->name_ = name;
    stop->coordinates_ = coordinates;

    base_stops_.push_back(*stop);
    stops_.insert({stop->name_, stop});
    stop_for_buses_[stop->name_];
}

void Catalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip)
{
    std::deque<domain::Stop *> deque_stops;
    auto *bus = new domain::Bus();
    bus->name_ = name;
    bus->is_roundtrip_ = is_roundtrip;

    for(const auto& stop: stops)
    {
        domain::Stop *s = FindStop(stop);
        deque_stops.push_back(s);
        s->buses_.insert(bus->name_);
        bus->unique_stops_.insert(s->name_);
    }

    bus->stops_ = deque_stops;

    for(auto stop: bus->stops_)
    {
        stop_for_buses_[stop->name_].insert(bus->name_);
    }

    for(size_t i = 0; i < bus->stops_.size() - 1; ++i)
    {
        bus->geo_distance_ += ComputeDistance(bus->stops_[i]->coordinates_, bus->stops_[i + 1]->coordinates_);
        bus->distance_ += GetDistance(bus->stops_[i]->name_, bus->stops_[i + 1]->name_);
    }

    base_buses_.emplace_back(*bus);
    buses_.insert({bus->name_, bus});
}

domain::Stop *Catalogue::FindStop(const std::string_view& name) const
{
    if(stops_.count(name) != 0)
    {
        return stops_.at(name);
    }

    return nullptr;
}

domain::Bus *Catalogue::FindBus(const std::string_view& name) const
{
    if(buses_.count(name) != 0)
    {
        return buses_.at(name);
    }

    return nullptr;
}

void Catalogue::AddDistance(const std::string& stop1, const std::string& stop2, double distance)
{
    domain::Stop* s1 = FindStop(stop1);
    domain::Stop* s2 = FindStop(stop2);
    distance_between_stops_[s1->name_][s2->name_] = distance;
}

double Catalogue::GetDistance(const std::string& stop1, const std::string& stop2) const
{
    if (distance_between_stops_.count(stop1) != 0)
    {
        if (distance_between_stops_.at(stop1).count(stop2) != 0)
        {
            return distance_between_stops_.at(stop1).at(stop2);
        }
    }

    if (distance_between_stops_.count(stop2) != 0)
    {
        if (distance_between_stops_.at(stop2).count(stop1) != 0)
        {
            return distance_between_stops_.at(stop2).at(stop1);
        }
    }

    return geo::ComputeDistance(FindStop(stop1)->coordinates_, FindStop(stop2)->coordinates_);
}

std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> Catalogue::GetAllDistances() const
{
    return distance_between_stops_;
}

domain::StopInfo Catalogue::GetStopInfo(const std::string_view& name) const
{
    domain::StopInfo stop_info;
    stop_info.name_ = name;

    if (stop_for_buses_.count(name) == 0)
    {
        stop_info.is_exist_ = false;
        return stop_info;
    }

    domain::Stop *s = FindStop(name);
    stop_info.buses_ = s->buses_;

    return stop_info;
}

domain::BusInfo Catalogue::GetBusInfo(const std::string_view& name) const
{
    domain::BusInfo bus_info;
    domain::Bus* route = FindBus(name);

    if(route == nullptr)
    {
        return domain::BusInfo({name, 0, 0, 0.0, 0.0});
    }

    bus_info.unique_stops_count_ = route->unique_stops_.size();

    double distance = 0.0;
    double geo_distance = 0.0;

    for(auto stop_from = route->stops_.begin(); stop_from + 1 != route->stops_.end(); ++stop_from)
    {
        auto stop_to = next(stop_from, 1);
        if (route->is_roundtrip_)
        {
            distance += GetDistance((*stop_from)->name_, (*stop_to)->name_);
            geo_distance += geo::ComputeDistance((*stop_from)->coordinates_, (*stop_to)->coordinates_);
        }
        else
        {
            distance += GetDistance((*stop_from)->name_, (*stop_to)->name_) + GetDistance((*stop_to)->name_, (*stop_from)->name_);
            geo_distance += geo::ComputeDistance((*stop_from)->coordinates_, (*stop_to)->coordinates_) * 2;
        }
    }

    if(route->is_roundtrip_)
    {
        bus_info.stops_count_ = route->stops_.size();
        distance += GetDistance((*(route->stops_.end() - 1))->name_, (*route->stops_.begin())->name_);
        geo_distance += geo::ComputeDistance((*(route->stops_.end() - 1))->coordinates_, (*route->stops_.begin())->coordinates_);
    }
    else
    {
        bus_info.stops_count_ = route->stops_.size() * 2 - 1;
    }

    bus_info.distance_ = distance;
    bus_info.curvature_ = distance / geo_distance;

    return bus_info;
}

std::vector<const domain::Bus*> Catalogue::GetBuses() const
{
    std::vector<const domain::Bus*> buses;
    for(auto& bus : base_buses_)
    {
        buses.push_back(buses_.at(bus.name_));
    }

    return buses;
}

std::vector<const domain::Stop*> Catalogue::GetStops() const
{
    std::vector<const domain::Stop*> stops;

    for(auto& stop: base_stops_)
    {
        stops.push_back(stops_.at(stop.name_));
    }

    return stops;
}

std::vector<domain::Stop*> Catalogue::GetBusStops(const domain::Bus* bus) const
{
    if(bus->is_roundtrip_)
    {
        return std::vector(bus->stops_.begin(), bus->stops_.end());
    }
    else
    {
        std::vector<domain::Stop*> result(bus->stops_.begin(), bus->stops_.end());
        for(auto it = bus->stops_.end() - 2; it >= bus->stops_.begin(); --it)
        {
            result.push_back(*it);
        }

        return result;
    }
}

bool Catalogue::HasBuses(const domain::Stop *stop) const
{
    if(stop_for_buses_.count(stop->name_))
    {
        if(!stop_for_buses_.at(stop->name_).empty())
        {
            return true;
        }
    }

    return false;
}

std::unordered_set<const domain::Bus*> Catalogue::GetBusesByStop(std::string_view stop_name) const
{
    std::unordered_set<const domain::Bus*> buses;
    for(auto& route: stop_for_buses_.at(stop_name))
    {
        buses.insert(buses_.at(route));
    }

    return buses;
}

} // namespace transport
