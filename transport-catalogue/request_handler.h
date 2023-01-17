#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "serialization.h"
#include <unordered_set>

namespace transport::request {

class RequestHandler
{
public:
    RequestHandler(transport::Catalogue& transport_catalogue,
                   route::TransportRouter& transport_router,
                   transport::renderer::MapRenderer& map_renderer);

    void AddStop(const std::string& name, double lat, double lng);
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
    void AddDistance(const std::string& stop1, const std::string& stop2, uint32_t distance);
    std::vector<const domain::Stop*> GetStops() const;
    std::vector<const domain::Bus*> GetBuses() const;
    std::vector<domain::Stop*> GetBusStops(const domain::Bus* bus);
    double GetDistance(const std::string& stop1, const std::string& stop2) const;
    domain::Bus* FindBus(const std::string_view& name) const;
    domain::StopInfo GetStopInfo(std::string_view name) const;
    domain::BusInfo GetBusInfo(std::string_view name) const;

    void SetRoutingSettings(route::Settings&& routing_settings);
    void AddStopToRouter(std::string_view name);
    void AddWaitEdgeToRouter(std::string_view stop_name);
    void AddBusEdgeToRouter(std::string_view stop_from, std::string_view stop_to, std::string_view bus_name, int span_count, int dist);
    void BuildRouter();
    std::optional<route::RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;

    void SetRendererSettings(transport::renderer::Settings&& renderer_settings);

    svg::Document RenderMap() const;

private:
    transport::Catalogue& transport_catalogue_;
    route::TransportRouter& transport_router_;
    transport::renderer::MapRenderer& map_renderer_;
};

} // namespace transport::request
