#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <iostream>
#include <memory>

namespace transport::request {

class JsonReader 
{
public:
    JsonReader(RequestHandler& request_handler);

    svg::Color ReadColor(const json::Node& json_color);

    transport::renderer::RenderSettings GetRenderSettings(const json::Dict& json_render_settings);

    void Exec(std::istream& in, std::ostream& out);

    void RequestsProcessing(const json::Array& json_stat_requests, std::ostream& out);

private:
    RequestHandler& request_handler_;
};

} // namespace transport::request
