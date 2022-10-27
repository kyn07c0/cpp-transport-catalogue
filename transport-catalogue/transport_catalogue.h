#pragma once

#include "geo.h"
#include <string>
#include <deque>
#include <utility>
#include <vector>
#include <unordered_map>
#include <set>

namespace transport {
namespace detail {

    struct Stop
    {
        std::string name_;
        geo::Coordinates coordinates_;
        std::set<std::string_view> buses_;
    };

    struct Route
    {
        std::string name_;
        std::deque<Stop*> stops_;
        std::set<std::string_view> unique_stops_;
        double geo_distance_;
        uint32_t distance_;
    };

    struct StopInfo
    {
        std::string name_;
        std::set<std::string_view> buses_;
        bool is_exist = true;
    };

    struct RouteInfo
    {
        std::string_view name_;
        size_t stops_count_ = 0;
        size_t unique_stops_count_ = 0;
        uint32_t distance_ = 0;
        double curvature_ = 0.0;
    };

} // namespace detail

    class Catalogue
    {
        public:
            void AddStop(const std::string& name, double lat, double lng);
            void AddRoute(const std::string& name, const std::vector<std::string>& stops);

            detail::Stop* FindStop(const std::string& name) const;
            detail::Route* FindRoute(const std::string& name) const;

            void AddDistance(const std::string& stop1, const std::string& stop2, uint32_t distance);
            uint32_t GetDistance(const std::string& stop1, const std::string& stop2) const;

            detail::StopInfo GetStopInfo(const std::string& name) const;
            detail::RouteInfo GetRouteInfo(const std::string& name) const;

        private:
            std::deque<detail::Stop> base_stops_;
            std::deque<detail::Route> base_routes_;

            std::unordered_map<std::string_view, detail::Stop*> stops_;
            std::unordered_map<std::string_view, detail::Route*> routes_;
            std::unordered_map<std::string_view, std::set<std::string_view>> stop_for_buses_;
            std::unordered_map<std::string_view, std::unordered_map<std::string_view, uint32_t>> distance_between_stops_;
    };

} // namespace transport
