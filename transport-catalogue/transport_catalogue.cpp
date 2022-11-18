#include "transport_catalogue.h"
#include <unordered_set>
#include <iostream>
#include <memory>
#include <algorithm>

namespace transport {

void Catalogue::AddStop(const std::string &name, double lat, double lng)
{
    geo::Coordinates coordinates = {lat, lng};

    auto *stop = new domain::Stop();
    stop->name_ = name;
    stop->coordinates_ = coordinates;

    base_stops_.push_back(*stop);
    stops_.insert({stop->name_, stop});
    stop_for_buses_[stop->name_];
}

void Catalogue::AddRoute(const std::string &name, const std::vector<std::string> &stops, bool is_roundtrip)
{
    std::deque<domain::Stop *> deque_stops;
    auto *route = new domain::Route();
    route->name_ = name;
    route->is_roundtrip_ = is_roundtrip;

    for(const auto &stop: stops)
    {
        domain::Stop *s = FindStop(stop);
        deque_stops.push_back(s);
        s->buses_.insert(route->name_);
        route->unique_stops_.insert(s->name_);
    }

    route->stops_ = deque_stops;

    for(auto stop: route->stops_)
    {
        stop_for_buses_[stop->name_].insert(route->name_);
    }

    for(size_t i = 0; i < route->stops_.size() - 1; ++i)
    {
        route->geo_distance_ += ComputeDistance(route->stops_[i]->coordinates_, route->stops_[i + 1]->coordinates_);
        route->distance_ += GetDistance(route->stops_[i]->name_, route->stops_[i + 1]->name_);
    }

    base_routes_.emplace_back(*route);
    routes_.insert({route->name_, route});
}

domain::Stop *Catalogue::FindStop(const std::string_view &name) const {
    if(stops_.count(name) != 0)
    {
        return stops_.at(name);
    }

    return nullptr;
}

domain::Route *Catalogue::FindRoute(const std::string_view &name) const {
    if(routes_.count(name) != 0)
    {
        return routes_.at(name);
    }

    return nullptr;
}

void Catalogue::AddDistance(const std::string &stop1, const std::string &stop2, double distance)
{
    domain::Stop *s1 = FindStop(stop1);
    domain::Stop *s2 = FindStop(stop2);
    distance_between_stops_[s1->name_][s2->name_] = distance;
}

double Catalogue::GetDistance(const std::string &stop1, const std::string &stop2) const
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

domain::StopInfo Catalogue::GetStopInfo(const std::string_view &name) const
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

domain::RouteInfo Catalogue::GetRouteInfo(const std::string_view &name) const
{
    domain::RouteInfo route_info;
    domain::Route *route = FindRoute(name);

    if(route == nullptr)
    {
        return domain::RouteInfo({name, 0, 0, 0.0, 0.0});
    }

    route_info.unique_stops_count_ = route->unique_stops_.size();

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
        route_info.stops_count_ = route->stops_.size();
        distance += GetDistance((*(route->stops_.end() - 1))->name_, (*route->stops_.begin())->name_);
        geo_distance += geo::ComputeDistance((*(route->stops_.end() - 1))->coordinates_, (*route->stops_.begin())->coordinates_);
    }
    else
    {
        route_info.stops_count_ = route->stops_.size() * 2 - 1;
    }

    route_info.distance_ = distance;
    route_info.curvature_ = distance / geo_distance;

    return route_info;
}

const std::vector<const domain::Route *> Catalogue::GetRoutes() const
{
    std::vector<const domain::Route *> routes;
    for(auto &route: routes_)
    {
        routes.push_back(route.second);
    }

    return routes;
}

const std::vector<const domain::Stop *> Catalogue::GetStops() const
{
    std::vector<const domain::Stop *> stops;
    for(auto &stop: stops_)
    {
        stops.push_back(stop.second);
    }

    return stops;
}

bool Catalogue::HasRoutes(const domain::Stop *stop) const
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

std::unordered_set<const domain::Route *> Catalogue::GetRoutesByStop(std::string_view stop_name) const
{
    std::unordered_set<const domain::Route *> routes;
    for(auto &route: stop_for_buses_.at(stop_name))
    {
        routes.insert(routes_.at(route));
    }

    return routes;
}

} // namespace transport
