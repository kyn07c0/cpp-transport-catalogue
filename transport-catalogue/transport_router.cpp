#include "transport_router.h"
#include <algorithm>
#include <utility>

namespace route {
    
const int METERS_IN_KILOMETER = 1000;
const int MINUTES_IN_HOUR = 60;    

Router::Router(const size_t graph_size) : graph_(graph_size)
{
}

void Router::SetSettings(Settings&& settings)
{
    settings_ = std::move(settings);
}

void Router::AddWaitEdge(const std::string_view stop_name)
{
    graph::Edge<double> edge = {vertex_id_[stop_name].start_wait, vertex_id_[stop_name].end_wait, settings_.bus_wait_time};
    EdgeInfo edge_info{edge, stop_name, -1, settings_.bus_wait_time};

    edges_.push_back(edge_info);
}

void Router::AddBusEdge(const std::string_view from, const std::string_view to, const std::string_view bus_name, const int span_count, const int distance)
{
    graph::Edge<double> edge = {vertex_id_[from].end_wait, vertex_id_[to].start_wait, distance / (settings_.bus_velocity * METERS_IN_KILOMETER / MINUTES_IN_HOUR)};
    double time = distance / (settings_.bus_velocity * METERS_IN_KILOMETER / MINUTES_IN_HOUR);

    EdgeInfo edge_info{edge, bus_name, span_count, time};
    edges_.push_back(edge_info);
}

void Router::AddStop(const std::string_view stop_name)
{
    if(vertex_id_.count(stop_name) == 0)
    {
        size_t start_wait = vertex_id_.size() * 2;
        size_t end_wait = vertex_id_.size() * 2 + 1;
        vertex_id_[stop_name] = {start_wait, end_wait};
    }
}

void Router::Build()
{
    if(!graph_)
    {
        graph_ = Graph(vertex_id_.size() * 2);
    }

    for(const auto& edge_info : edges_)
    {
        graph_->AddEdge(edge_info.edge);
    }

    if(graph_ && !router_)
    {
        router_.emplace(Route(*graph_));
    }
}

std::optional<RouteInfo> Router::GetRouteInfo(const std::string_view from, const std::string_view to) const
{
    const auto route = router_->BuildRoute(vertex_id_.at(from).start_wait, vertex_id_.at(to).start_wait);

    if(!route)
    {
        return std::nullopt;
    }

    std::vector<RouteItem> items;
    for(const auto id : route->edges)
    {
        const EdgeInfo& edge_info = edges_[id];
        RouteItem item;
        if(edge_info.span_count == -1)
        {
            item.wait_item = {edge_info.name, edge_info.time};
        }
        else
        {
            item.bus_item = {edge_info.name, edge_info.span_count, edge_info.time};
        }

        items.push_back(item);
    }

    return RouteInfo{route->weight, items};
}

} // namespace transport
