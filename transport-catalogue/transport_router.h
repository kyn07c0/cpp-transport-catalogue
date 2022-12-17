#pragma once

#include "domain.h"
#include "router.h"
#include <unordered_map>

namespace route {

struct Settings
{
    double bus_wait_time = 6;
    double bus_velocity = 40.0;
};

struct RouteWait
{
    std::string_view stop_name;
    double time;
};

struct RouteMove
{
    std::string_view bus_name;
    int span_count;
    double time;
};

struct RouteItem
{
    std::optional<RouteWait> wait_item;
    std::optional<RouteMove> bus_item;
};

struct EdgeInfo
{
    graph::Edge<double> edge;
    std::string_view name;
    int span_count = -1;
    double time = 0.0;
};

struct RouteInfo
{
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

struct Vertexe
{
    size_t start_wait;
    size_t end_wait;
};

class Router
{
public:
    using Graph = graph::DirectedWeightedGraph<double>;
    using Route = graph::Router<double>;

    Router() = default;
    explicit Router(const size_t graph_size);

    void SetSettings(Settings&& settings);
    void AddWaitEdge(const std::string_view stop_name);
    void AddBusEdge(const std::string_view from, const std::string_view to, const std::string_view bus_name, const int span_count, const int distance);
    void AddStop(const std::string_view stop_name);

    void Build();

    std::optional<RouteInfo> GetRouteInfo(const std::string_view from, const std::string_view to) const;

private:
    Settings settings_;

    std::optional<Graph> graph_ = std::nullopt;
    std::optional<Route> router_ = std::nullopt;

    std::unordered_map<std::string_view, Vertexe> vertex_id_;
    std::vector<EdgeInfo> edges_;
};

} //namespace transport
