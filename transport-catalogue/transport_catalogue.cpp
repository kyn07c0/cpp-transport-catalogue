#include "transport_catalogue.h"
#include <unordered_set>
#include <iostream>

void transport::Catalogue::AddStop(const std::string& name, double lat, double lng)
{
    geo::Coordinates coordinates = {lat, lng};

    auto *stop = new detail::Stop(name, coordinates);

    base_stops_.push_back(*stop);
    stops_.insert({stop->name_, stop});
    stop_for_buses_[stop->name_];
}

void transport::Catalogue::AddRoute(const std::string& name, const std::vector<std::string>& stops)
{
    std::deque<detail::Stop*> deque_stops;

    std::set<std::string_view> route_stops;

    for(const auto& stop : stops)
    {
        detail::Stop* s = FindStop(stop);
        route_stops.insert(s->name_);
        deque_stops.push_back(s);
    }

    auto *route = new detail::Route(name, deque_stops);

    base_routes_.emplace_back(*route);
    routes_.insert({route->name_, route});

    for(auto stop : route_stops)
    {
        stop_for_buses_[stop].insert(route->name_);
    }
}

transport::detail::Stop* transport::Catalogue::FindStop(const std::string& name) const
{
    if(stops_.count(name) != 0)
    {
        return stops_.at(name);
    }

    return nullptr;
}

transport::detail::Route* transport::Catalogue::FindRoute(const std::string& name) const
{
    if(routes_.count(name) != 0)
    {
        return routes_.at(name);
    }

    return nullptr;
}

void transport::Catalogue::AddDistance(const std::string& stop1, const std::string& stop2, uint32_t distance)
{
    detail::Stop* s1 = FindStop(stop1);
    detail::Stop* s2 = FindStop(stop2);
    distance_between_stops_[s1->name_][s2->name_] = distance;

    if(distance_between_stops_.count(s2->name_) == 0 || distance_between_stops_.at(s2->name_).count(s1->name_) == 0)
    {
        distance_between_stops_[s2->name_][s1->name_] = distance;
    }
}

uint32_t transport::Catalogue::GetDistance(const std::string& stop1, const std::string& stop2) const
{
    if(distance_between_stops_.at(stop1).count(stop2) != 0)
    {
        uint32_t d = distance_between_stops_.at(stop1).at(stop2);
        return d;
    }

    return 0;
}

transport::detail::StopInfo transport::Catalogue::GetStopInfo(const std::string& name) const
{
    detail::StopInfo stop_info;
    stop_info.name_ = name;

    if(stop_for_buses_.count(name) == 0)
    {
        stop_info.is_exist = false;
        return stop_info;
    }

    for(auto bus : stop_for_buses_.at(name))
    {
        stop_info.buses_.insert(bus);
    }

    return stop_info;
}

transport::detail::RouteInfo transport::Catalogue::GetRouteInfo(const std::string& name) const
{
    detail::RouteInfo route_info;
    detail::Route* route = FindRoute(name);

    if(route == nullptr)
    {
        return detail::RouteInfo({name, 0, 0, 0, 0.0});
    }

    route_info.name_ = name;
    route_info.stops_count_ = route->stops_.size();

    std::unordered_set<std::string_view> unique_stops;
    for(const auto& stop : route->stops_)
    {
        unique_stops.insert(stop->name_);
    }
    route_info.unique_stops_count_ = unique_stops.size();

    double geo_distance = 0.0;
    uint32_t distance = 0;
    for(size_t i = 0; i < route->stops_.size() - 1; ++i)
    {
        geo_distance += ComputeDistance(route->stops_[i]->coordinates_, route->stops_[i + 1]->coordinates_);
        distance += GetDistance(route->stops_[i]->name_, route->stops_[i + 1]->name_);
    }
    route_info.distance_ = distance;
    route_info.curvature_ = distance / geo_distance;

    return route_info;
}
