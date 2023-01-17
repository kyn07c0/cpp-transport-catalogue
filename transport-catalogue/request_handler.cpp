#include "request_handler.h"

namespace transport::request {

RequestHandler::RequestHandler(transport::Catalogue& transport_catalogue,
                               route::TransportRouter& transport_router,
                               transport::renderer::MapRenderer& map_renderer)
    : transport_catalogue_(transport_catalogue), transport_router_(transport_router), map_renderer_(map_renderer)
{
}


void RequestHandler::AddStop(const std::string& name, double lat, double lng)
{
    transport_catalogue_.AddStop(name, lat, lng);
}

void RequestHandler::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip)
{
    transport_catalogue_.AddBus(name, stops, is_roundtrip);
}

void RequestHandler::AddDistance(const std::string& stop1, const std::string& stop2, uint32_t distance)
{
    transport_catalogue_.AddDistance(stop1, stop2, distance);
}

std::vector<const domain::Stop*> RequestHandler::GetStops() const
{
    return transport_catalogue_.GetStops();
}

std::vector<const domain::Bus*> RequestHandler::GetBuses() const
{
    return transport_catalogue_.GetBuses();
}

std::vector<domain::Stop*> RequestHandler::GetBusStops(const domain::Bus* bus)
{
    return transport_catalogue_.GetBusStops(bus);
}

double RequestHandler::GetDistance(const std::string& stop1, const std::string& stop2) const
{
    return transport_catalogue_.GetDistance(stop1, stop2);
}

domain::Bus* RequestHandler::FindBus(const std::string_view& name) const
{
    return transport_catalogue_.FindBus(name);
}

domain::StopInfo RequestHandler::GetStopInfo(std::string_view name) const
{
    return transport_catalogue_.GetStopInfo(name);
}

domain::BusInfo RequestHandler::GetBusInfo(std::string_view name) const
{
    return transport_catalogue_.GetBusInfo(name);
}


void RequestHandler::SetRoutingSettings(route::Settings&& routing_settings)
{
    transport_router_.SetSettings(routing_settings);
}

void RequestHandler::AddStopToRouter(std::string_view name)
{
    transport_router_.AddStop(name);
}

void RequestHandler::AddWaitEdgeToRouter(std::string_view stop_name)
{
    transport_router_.AddWaitEdge(stop_name);
}

void RequestHandler::AddBusEdgeToRouter(std::string_view stop_from, std::string_view stop_to, std::string_view bus_name, int span_count, int dist)
{
    transport_router_.AddBusEdge(stop_from, stop_to, bus_name, span_count, dist);
}

void RequestHandler::BuildRouter()
{
    transport_router_.Build();
}

std::optional<route::RouteInfo> RequestHandler::GetRouteInfo(std::string_view from, std::string_view to) const
{
    return transport_router_.GetRouteInfo(from, to);
}

void RequestHandler::SetRendererSettings(transport::renderer::Settings&& renderer_settings)
{
    map_renderer_.SetSettings(std::move(renderer_settings));
}

svg::Document RequestHandler::RenderMap() const
{
    svg::Document result;
    std::vector<const domain::Bus *> buses_for_render;
    std::vector<const domain::Stop *> stops_for_render;

    std::unordered_set<const domain::Stop *> usered_stops;
    std::vector<geo::Coordinates> points;

    for(const auto *route : transport_catalogue_.GetBuses())
    {
        if(!route->stops_.empty())
        {
            buses_for_render.push_back(route);
            for(const auto *stop: transport_catalogue_.GetStops())
            {
                if(usered_stops.count(stop) == 0 && transport_catalogue_.HasBuses(stop))
                {
                    usered_stops.insert(stop);
                    stops_for_render.push_back(stop);
                    points.push_back(stop->coordinates_);
                }
            }
        }
    }

    map_renderer_.CalculateSphereProjector(points);

    std::sort(buses_for_render.begin(), buses_for_render.end(), [](const domain::Bus *lhs, const domain::Bus *rhs) {
        return lhs->name_ < rhs->name_;
    });

    std::sort(stops_for_render.begin(), stops_for_render.end(), [](const domain::Stop *lhs, const domain::Stop *rhs) {
        return lhs->name_ < rhs->name_;
    });

    size_t color_counter = 0;
    for(const domain::Bus *bus : buses_for_render)
    {
        map_renderer_.RenderBus(result, bus, color_counter);
        ++color_counter;
    }

    color_counter = 0;
    for(const domain::Bus *bus : buses_for_render)
    {
        map_renderer_.RenderBusName(result, bus, color_counter);
        ++color_counter;
    }

    usered_stops = {};
    for(const domain::Stop *stop: stops_for_render)
    {
        if(usered_stops.count(stop) > 0 || transport_catalogue_.GetBusesByStop(stop->name_).empty())
        {
            continue;
        }

        map_renderer_.RenderStop(result, stop);
        usered_stops.insert(stop);
    }

    usered_stops = {};
    for(const domain::Stop *stop: stops_for_render)
    {
        if(usered_stops.count(stop) > 0 || transport_catalogue_.GetBusesByStop(stop->name_).empty())
        {
            continue;
        }

        map_renderer_.RenderStopName(result, stop);
        usered_stops.insert(stop);
    }

    return result;
}

} // namespace transport::request
