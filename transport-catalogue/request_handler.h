#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include <unordered_set>

namespace transport::request {

class RequestHandler
{
public:
    RequestHandler(transport::Catalogue& catalogue, route::Router& router, transport::renderer::MapRenderer& map_renderer);

    void AddStop(const std::string& name, double lat, double lng);
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
    void AddDistance(const std::string& stop1, const std::string& stop2, uint32_t distance);

    const std::vector<const domain::Stop*> GetStops() const;
    const std::vector<const domain::Bus*> GetBuses() const;
    const std::vector<domain::Stop*> GetBusStops(const domain::Bus* bus);

    double GetDistance(const std::string& stop1, const std::string& stop2) const;

    domain::Bus* FindBus(const std::string_view& name) const;

    domain::StopInfo GetStopInfo(const std::string_view name) const;
    domain::BusInfo GetBusInfo(const std::string_view name) const;
    std::optional<route::RouteInfo> GetRouteInfo(const std::string_view from, const std::string_view to) const;

    void SetRendererSettings(transport::renderer::RenderSettings&& renderer_settings);

    void SetRoutingSettings(route::Settings&& routing_settings);
    void AddStopToRouter(const std::string_view name);
    void AddWaitEdgeToRouter(const std::string_view stop_name);
    void AddBusEdgeToRouter(const std::string_view stop_from, const std::string_view stop_to, const std::string_view bus_name, const int span_count, const int dist);
    void BuildRouter();

    svg::Document RenderMap() const;

private:
    transport::Catalogue& catalogue_;
    route::Router& router_;
    transport::renderer::MapRenderer& map_renderer_;
};

} // namespace transport::request
