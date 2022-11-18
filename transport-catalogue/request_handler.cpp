#include "request_handler.h"

namespace transport::request {

RequestHandler::RequestHandler(transport::Catalogue &catalogue, transport::renderer::MapRenderer &map_renderer)
        : catalogue_(catalogue), map_renderer_(map_renderer)
{
}

// ------------------------------------ Transport catalogue ------------------------------------------

void RequestHandler::AddStop(const std::string &name, double lat, double lng)
{
    catalogue_.AddStop(name, lat, lng);
}

void RequestHandler::AddRoute(const std::string &name, const std::vector<std::string> &stops, bool is_roundtrip)
{
    catalogue_.AddRoute(name, stops, is_roundtrip);
}

void RequestHandler::AddDistance(const std::string &stop1, const std::string &stop2, uint32_t distance)
{
    catalogue_.AddDistance(stop1, stop2, distance);
}

domain::Route *RequestHandler::FindRoute(const std::string_view &name) const
{
    return catalogue_.FindRoute(name);
}

domain::StopInfo RequestHandler::GetStopInfo(const std::string &name) const
{
    return catalogue_.GetStopInfo(name);
}

domain::RouteInfo RequestHandler::GetRouteInfo(const std::string_view &name) const
{
    return catalogue_.GetRouteInfo(name);
}

// ------------------------------------ Map renderer -------------------------------------------------

void RequestHandler::SetRendererSettings(transport::renderer::RenderSettings &&renderer_settings)
{
    map_renderer_.SetRendererSettings(std::move(renderer_settings));
}

svg::Document RequestHandler::RenderMap() const
{
    svg::Document result;
    std::vector<const domain::Route *> routes_for_render;
    std::vector<const domain::Stop *> stops_for_render;

    std::unordered_set<const domain::Stop *> usered_stops;
    std::vector<geo::Coordinates> points;

    for(const auto *route: catalogue_.GetRoutes())
    {
        if(!route->stops_.empty())
        {
            routes_for_render.push_back(route);
            for(const auto *stop: catalogue_.GetStops())
            {
                if(usered_stops.count(stop) == 0 && catalogue_.HasRoutes(stop))
                {
                    usered_stops.insert(stop);
                    stops_for_render.push_back(stop);
                    points.push_back(stop->coordinates_);
                }
            }
        }
    }

    map_renderer_.CalculateSphereProjector(points);

    std::sort(routes_for_render.begin(), routes_for_render.end(), [](const domain::Route *lhs, const domain::Route *rhs) {
        return lhs->name_ < rhs->name_;
    });

    std::sort(stops_for_render.begin(), stops_for_render.end(), [](const domain::Stop *lhs, const domain::Stop *rhs) {
        return lhs->name_ < rhs->name_;
    });

    size_t color_counter = 0;
    for(const domain::Route *route: routes_for_render)
    {
        map_renderer_.RenderRoute(result, route, color_counter);
        ++color_counter;
    }

    color_counter = 0;
    for(const domain::Route *route: routes_for_render)
    {
        map_renderer_.RenderRouteName(result, route, color_counter);
        ++color_counter;
    }

    usered_stops = {};
    for(const domain::Stop *stop: stops_for_render)
    {
        if(usered_stops.count(stop) > 0 || catalogue_.GetRoutesByStop(stop->name_).empty())
        {
            continue;
        }

        map_renderer_.RenderStop(result, stop);
        usered_stops.insert(stop);
    }

    usered_stops = {};
    for(const domain::Stop *stop: stops_for_render)
    {
        if(usered_stops.count(stop) > 0 || catalogue_.GetRoutesByStop(stop->name_).empty())
        {
            continue;
        }

        map_renderer_.RenderStopName(result, stop);
        usered_stops.insert(stop);
    }

    return result;
}

} // namespace transport::request
