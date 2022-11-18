#pragma once

#include "geo.h"
#include <string>
#include <string_view>
#include <deque>
#include <set>

namespace domain {

struct Stop
{
    Stop() = default;
    Stop(std::string name,
         geo::Coordinates coordinates,
         std::set<std::string_view> buses);

    std::string name_;
    geo::Coordinates coordinates_;
    std::set<std::string_view> buses_;
};

struct Route
{
    Route() = default;
    Route(std::string name,
          std::deque<Stop*> stops,
          std::set<std::string_view> unique_stops,
          double geo_distance,
          double distance,
          bool is_roundtrip);

    std::string name_;
    std::deque<Stop*> stops_;
    std::set<std::string_view> unique_stops_;
    double geo_distance_;
    double distance_;
    bool is_roundtrip_;
};

struct StopInfo
{
    StopInfo() = default;
    StopInfo(std::string name,
             std::set<std::string_view> buses,
             bool is_exist = true);

    std::string name_;
    std::set<std::string_view> buses_;
    bool is_exist_ = true;
};

struct RouteInfo
{
    RouteInfo() = default;
    RouteInfo(std::string_view name,
              size_t stops_count = 0,
              size_t unique_stops_count = 0,
              double distance = 0,
              double curvature = 0.0);

    std::string_view name_;
    size_t stops_count_ = 0;
    size_t unique_stops_count_ = 0;
    double distance_ = 0.0;
    double curvature_ = 0.0;
};


} // namespace domain
