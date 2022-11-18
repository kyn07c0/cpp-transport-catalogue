#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <unordered_set>

namespace transport::request {

class RequestHandler
{
public:
    RequestHandler(transport::Catalogue& catalogue, transport::renderer::MapRenderer& map_renderer);

    // Transport catalogue
    void AddStop(const std::string& name, double lat, double lng);
    void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
    void AddDistance(const std::string& stop1, const std::string& stop2, uint32_t distance);

    domain::Route* FindRoute(const std::string_view& name) const;

    domain::StopInfo GetStopInfo(const std::string& name) const;
    domain::RouteInfo GetRouteInfo(const std::string_view& name) const;

    // Map renderer
    void SetRendererSettings(transport::renderer::RenderSettings&& renderer_settings);
    svg::Document RenderMap() const;

private:
    transport::Catalogue& catalogue_;
    transport::renderer::MapRenderer& map_renderer_;
};

} // namespace transport::request
