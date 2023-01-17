#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <iostream>
#include <memory>

namespace transport::request {

class JsonReader
{
public:
    JsonReader(RequestHandler& request_handler,
               transport::Catalogue& transport_catalogue,
               transport::renderer::MapRenderer& map_renderer,
               route::TransportRouter& transport_router);

    svg::Color ReadColor(const json::Node& json_color);

    transport::renderer::Settings GetRenderSettings(const json::Dict& json_render_settings);
    route::Settings GetRoutingSettings(const json::Dict& json_routing_settings);

    void ExecMakeBase(std::istream& in);
    void ExecProcessRequest(std::istream& in, std::ostream& out);

    void RequestsProcessing(const json::Array& json_stat_requests, std::ostream& out);

private:
    RequestHandler& request_handler_;
    transport::Catalogue& transport_catalogue_;
    transport::renderer::MapRenderer& map_renderer_;
    route::TransportRouter& transport_router_;
};

} // namespace transport::request
