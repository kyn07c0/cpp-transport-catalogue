#pragma once

#include "domain.h"
#include "router.h"
#include <unordered_map>

namespace route {

struct Settings
{
    double bus_wait_time;
    double bus_velocity;
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

class TransportRouter
{
public:
    using Graph = graph::DirectedWeightedGraph<double>;
    using Route = graph::Router<double>;

    TransportRouter() = default;
    explicit TransportRouter(size_t graph_size);

    void SetSettings(Settings& settings);
    Settings GetSettings();

    void SetGraph(const Graph& graph);
    std::optional<graph::DirectedWeightedGraph<double>> GetGraph();

    void SetStopVertexIds(std::unordered_map<std::string_view, Vertexe> stop_vertex_ids);
    const std::unordered_map<std::string_view, Vertexe>& GetStopVertexIds() const;

    void SetEdgesInfo(std::vector<EdgeInfo> edges_info);
    std::vector<EdgeInfo> GetEdgesInfo();

    void AddWaitEdge(std::string_view stop_name);
    void AddBusEdge(std::string_view from, std::string_view to, std::string_view bus_name, int span_count, int distance);
    void AddStop(std::string_view stop_name);

    void Build();

    std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;

private:
    Settings settings_ = {};

    std::optional<Graph> graph_ = std::nullopt;
    std::optional<Route> router_ = std::nullopt;

    std::unordered_map<std::string_view, Vertexe> vertex_id_;
    std::vector<EdgeInfo> edges_info_;
};

} //namespace transport
