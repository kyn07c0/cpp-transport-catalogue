#include "json_reader.h"
#include "json_builder.h"
#include <algorithm>
#include <sstream>

namespace transport::request {

JsonReader::JsonReader(RequestHandler& request_handler,
                       transport::Catalogue& transport_catalogue,
                       transport::renderer::MapRenderer& map_renderer,
                       route::TransportRouter& transport_router)
    : request_handler_(request_handler), transport_catalogue_(transport_catalogue), map_renderer_(map_renderer), transport_router_(transport_router)
{
}

void JsonReader::ExecMakeBase(std::istream& in)
{
    const json::Document json_document = json::Load(in);
    const json::Node& json_root = json_document.GetRoot();
    const json::Dict& dictionary = json_root.AsDict();

    if(dictionary.count("routing_settings"s) != 0)
    {
        const json::Dict& json_routing_settings = dictionary.at("routing_settings"s).AsDict();
        request_handler_.SetRoutingSettings(GetRoutingSettings(json_routing_settings));
    }

    if(dictionary.count("render_settings"s) != 0)
    {
        const json::Dict& json_render_settings = dictionary.at("render_settings"s).AsDict();
        request_handler_.SetRendererSettings(GetRenderSettings(json_render_settings));
    }

    if(dictionary.count("base_requests"s) != 0)
    {
        std::map<std::string, json::Dict> distances;

        const auto& json_base_requests = dictionary.at("base_requests"s).AsArray();

        for(const auto& json_base_request : json_base_requests)
        {
            std::string type = json_base_request.AsDict().at("type"s).AsString();
            if(type == "Stop"s)
            {
                double lat = json_base_request.AsDict().at("latitude"s).AsDouble();
                double lng = json_base_request.AsDict().at("longitude"s).AsDouble();
                std::string name = json_base_request.AsDict().at("name"s).AsString();

                request_handler_.AddStop(name, lat, lng);

                distances[name] = json_base_request.AsDict().at("road_distances"s).AsDict();
            }
        }

        for(auto& distance : distances)
        {
            for(auto [stop, dist] : distance.second)
            {
                request_handler_.AddDistance(distance.first, stop, dist.AsInt());
            }
        }

        for(const auto& json_base_request : json_base_requests)
        {
            std::string type = json_base_request.AsDict().at("type"s).AsString();
            if(type == "Bus"s)
            {
                auto& json_route = json_base_request.AsDict();

                std::vector<std::string> stops;
                for(auto& stop : json_route.at("stops"s).AsArray())
                {
                    stops.push_back(stop.AsString());
                }

                bool is_roundtrip = json_route.at("is_roundtrip"s).AsBool();
                std::string name = json_route.at("name"s).AsString();

                request_handler_.AddBus(name, stops, is_roundtrip);
            }
        }

        for(const auto& stop : request_handler_.GetStops())
        {
            request_handler_.AddStopToRouter(stop->name_);
            request_handler_.AddWaitEdgeToRouter(stop->name_);
        }

        for(const auto& bus : request_handler_.GetBuses())
        {
            std::vector<domain::Stop*> stops = request_handler_.GetBusStops(bus);

            for(size_t i = 0; i < stops.size() - 1; ++i)
            {
                int full_distance = 0;
                std::string prev_stop_name = stops[i]->name_;

                for(size_t j = i + 1; j < stops.size(); ++j)
                {
                    double distance = request_handler_.GetDistance(prev_stop_name, stops[j]->name_);

                    request_handler_.AddBusEdgeToRouter(stops[i]->name_,
                                                        stops[j]->name_,
                                                        bus->name_,
                                                        j - i,
                                                        full_distance + distance);

                    prev_stop_name = stops[j]->name_;
                    full_distance += distance;
                }
            }
        }
    }

    request_handler_.BuildRouter();

    if(dictionary.count("serialization_settings"s) != 0)
    {
        const json::Dict& json_serialization_settings = dictionary.at("serialization_settings"s).AsDict();
        std::string file = json_serialization_settings.at("file").AsString();

        proto::Serializer serializer(transport_catalogue_, map_renderer_, transport_router_);
        serializer(file);
    }
}

void JsonReader::ExecProcessRequest(std::istream& in, std::ostream& out)
{
    const json::Document json_document = json::Load(in);
    const json::Node& json_root = json_document.GetRoot();
    const json::Dict& dictionary = json_root.AsDict();

    if(dictionary.count("serialization_settings"s) != 0)
    {
        const json::Dict& json_serialization_settings = dictionary.at("serialization_settings"s).AsDict();
        std::string file = json_serialization_settings.at("file").AsString();

        //serialization::Deserialization(file, catalogue_, map_renderer_, transport_router_);

        proto::Deserializer deserializer(transport_catalogue_, map_renderer_, transport_router_);
        deserializer(file);
    }

    if(dictionary.count("stat_requests"s) != 0)
    {
        const json::Array& json_stat_requests = dictionary.at("stat_requests"s).AsArray();

        if(!json_stat_requests.empty())
        {
            RequestsProcessing(json_stat_requests, out);
        }
    }
}

void JsonReader::RequestsProcessing(const json::Array& json_stat_requests, std::ostream& out)
{
    json::Builder json_responses;
    json_responses.StartArray();

    for(const auto& stat_request : json_stat_requests)
    {
        auto stat_request_map = stat_request.AsDict();
        int id = stat_request_map.at("id"s).AsInt();
        const std::string& type = stat_request_map.at("type"s).AsString();

        json::Builder json_response;
        json_response.StartDict().Key("request_id"s).Value(json::Node(id));

        if(type == "Stop"s)
        {
            const std::string& stop_name = stat_request_map.at("name"s).AsString();

            auto buses = request_handler_.GetStopInfo(stop_name);
            if(!buses.buses_.empty() || buses.is_exist_)
            {
                json_response.Key("buses"s).StartArray();

                for(const auto bus: buses.buses_)
                {
                    json_response.Value(json::Node{static_cast<std::string>(bus)});
                }

                json_response.EndArray();
            }
            else
            {
                json_response.Key("error_message"s).Value("not found"s);
            }
        }
        else if(type == "Bus"s)
        {
            const std::string& bus_name = stat_request_map.at("name"s).AsString();

            if(request_handler_.FindBus(bus_name) == nullptr)
            {
                json_response.Key("error_message"s).Value("not found"s);
            }
            else
            {
                auto route = request_handler_.GetBusInfo(bus_name);

                json_response.Key("curvature"s).Value(route.curvature_)
                             .Key("stop_count"s).Value(static_cast<int>(route.stops_count_))
                             .Key("unique_stop_count"s).Value(static_cast<int>(route.unique_stops_count_))
                             .Key("route_length"s).Value(static_cast<double>(route.distance_));
            }
        }
        else if(type == "Map"s)
        {
            std::ostringstream o;
            svg::Document svg = request_handler_.RenderMap();
            svg.Render(o);

            json_response.Key("map"s).Value(o.str());
        }
        else if(type == "Route"s)
        {
            const std::string& from = stat_request_map.at("from"s).AsString();
            const std::string& to = stat_request_map.at("to"s).AsString();

            auto route_info = request_handler_.GetRouteInfo(from, to);

            if(route_info)
            {
                json_response.Key("items"s).StartArray();
                for(const auto& item : route_info->items)
                {
                    json_response.StartDict();
                    if(item.wait_item)
                    {
                        json_response.Key("type"s).Value(json::Node{static_cast<std::string>("Wait"s)});
                        json_response.Key("stop_name"s).Value(json::Node{static_cast<std::string>(item.wait_item->stop_name)});
                        json_response.Key("time"s).Value(json::Node{static_cast<double>(item.wait_item->time)});
                    }
                    else
                    {
                        json_response.Key("type"s).Value(json::Node{static_cast<std::string>("Bus"s)});
                        json_response.Key("bus"s).Value(json::Node{static_cast<std::string>(item.bus_item->bus_name)});
                        json_response.Key("span_count"s).Value(json::Node{static_cast<int>(item.bus_item->span_count)});
                        json_response.Key("time"s).Value(json::Node{static_cast<double>(item.bus_item->time)});
                    }
                    json_response.EndDict();
                }
                json_response.EndArray();

                json_response.Key("total_time"s).Value(route_info->total_time);
            }
            else
            {
                json_response.Key("error_message"s).Value("not found"s);
            }
        }

        json_response.EndDict();
        json_responses.Value(json_response.Build());
    }

    json_responses.EndArray();

    json::Print(json::Document{json_responses.Build()}, out);
}

svg::Color JsonReader::ReadColor(const json::Node& json_color)
{
    svg::Color color;

    if(json_color.IsString())
    {
        color = svg::Color{json_color.AsString()};
    }
    else if(json_color.IsArray())
    {
        const json::Array& color_array = json_color.AsArray();
        uint8_t r = color_array[0].AsInt();
        uint8_t g = color_array[1].AsInt();
        uint8_t b = color_array[2].AsInt();

        if (color_array.size() == 3)
        {
            color = svg::Color{svg::Rgb{r,g,b}};
        }
        else
        {
            double a = color_array[3].AsDouble();
            color = svg::Color{svg::Rgba{r, g, b, a}};
        }
    }

    return color;
}

transport::renderer::Settings JsonReader::GetRenderSettings(const json::Dict& json_render_settings)
{
    transport::renderer::Settings renderer_settings;

    renderer_settings.width = json_render_settings.at("width").AsDouble();
    renderer_settings.height = json_render_settings.at("height").AsDouble();
    renderer_settings.padding = json_render_settings.at("padding").AsDouble();
    renderer_settings.line_width = json_render_settings.at("line_width").AsDouble();
    renderer_settings.stop_radius = json_render_settings.at("stop_radius").AsDouble();
    renderer_settings.bus_label_font_size = json_render_settings.at("bus_label_font_size").AsInt();
    renderer_settings.bus_label_offset.x = json_render_settings.at("bus_label_offset").AsArray()[0].AsDouble();
    renderer_settings.bus_label_offset.y = json_render_settings.at("bus_label_offset").AsArray()[1].AsDouble();
    renderer_settings.stop_label_offset.x = json_render_settings.at("stop_label_offset").AsArray()[0].AsDouble();
    renderer_settings.stop_label_offset.y = json_render_settings.at("stop_label_offset").AsArray()[1].AsDouble();
    renderer_settings.stop_label_font_size = json_render_settings.at("stop_label_font_size").AsInt();
    renderer_settings.underlayer_color = ReadColor(json_render_settings.at("underlayer_color"));
    renderer_settings.underlayer_width = json_render_settings.at("underlayer_width").AsDouble();

    for(const json::Node& color : json_render_settings.at("color_palette").AsArray())
    {
        renderer_settings.color_palette.push_back(ReadColor(color));
    }

    return renderer_settings;
}

route::Settings JsonReader::GetRoutingSettings(const json::Dict& json_routing_settings)
{
    route::Settings routing_settings;

    routing_settings.bus_wait_time = json_routing_settings.at("bus_wait_time").AsInt();
    routing_settings.bus_velocity = json_routing_settings.at("bus_velocity").AsDouble();

    return routing_settings;
}

} // namespace transport::request
