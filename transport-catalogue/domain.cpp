#include "domain.h"

#include <utility>

namespace domain {

Stop::Stop(std::string name, geo::Coordinates coordinates, std::set<std::string_view> buses)
            : name_(std::move(name)),
              coordinates_(coordinates),
              buses_(std::move(buses))
{
}

Route::Route(std::string name, std::deque<Stop*> stops, std::set<std::string_view> unique_stops, double geo_distance, double distance, bool is_roundtrip)
            : name_(std::move(name)),
              stops_(std::move(stops)),
              unique_stops_(std::move(unique_stops)),
              geo_distance_(geo_distance),
              distance_(distance),
              is_roundtrip_(is_roundtrip)
{
}

StopInfo::StopInfo(std::string name, std::set<std::string_view> buses, bool is_exist)
            : name_(std::move(name)),
              buses_(std::move(buses)),
              is_exist_(is_exist)
{
}

RouteInfo::RouteInfo(std::string_view name, size_t stops_count, size_t unique_stops_count, double distance, double curvature)
            : name_(name),
              stops_count_(stops_count),
              unique_stops_count_(unique_stops_count),
              distance_(distance),
              curvature_(curvature)
{
}

} // namespace domain
