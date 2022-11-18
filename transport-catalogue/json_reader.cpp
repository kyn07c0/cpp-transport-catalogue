#include "json_reader.h"
#include <algorithm>
#include <sstream>

JsonReader::JsonReader(transport::request::RequestHandler& request_handler) : request_handler_(request_handler)
{
}

void JsonReader::Exec(std::istream& in, std::ostream& out)
{
    const json::Document json_document = json::Load(in);
    const json::Node& json_root_node = json_document.GetRoot();
    const json::Dict& dictionary = json_root_node.AsMap();

    if(dictionary.count("base_requests"s) != 0)
    {
        std::map<std::string, json::Dict> distances;

        const auto& json_base_requests = dictionary.at("base_requests"s).AsArray();
        for(const auto& json_base_request : json_base_requests)
        {
            std::string type = json_base_request.AsMap().at("type"s).AsString();
            if(type == "Stop"s)
            {
                double lat = json_base_request.AsMap().at("latitude"s).AsDouble();
                double lng = json_base_request.AsMap().at("longitude"s).AsDouble();
                std::string name = json_base_request.AsMap().at("name"s).AsString();

                request_handler_.AddStop(name, lat, lng);

                distances[name] = json_base_request.AsMap().at("road_distances"s).AsMap();
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
            std::string type = json_base_request.AsMap().at("type"s).AsString();
            if(type == "Bus"s)
            {
                auto& json_route = json_base_request.AsMap();

                std::vector<std::string> stops;
                for(auto& stop : json_route.at("stops"s).AsArray())
                {
                    stops.push_back(stop.AsString());
                }

                bool is_roundtrip = json_route.at("is_roundtrip"s).AsBool();
                std::string name = json_route.at("name"s).AsString();

                request_handler_.AddRoute(name, stops, is_roundtrip);
            }
        }
    }

    if(dictionary.count("render_settings"s) != 0)
    {
        const json::Dict& json_render_settings = dictionary.at("render_settings"s).AsMap();
        request_handler_.SetRendererSettings(GetRenderSettings(json_render_settings));
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
    json::Array json_response;

    for(const auto& stat_request : json_stat_requests)
    {
        auto stat_request_map = stat_request.AsMap();

        int id = stat_request_map.at("id"s).AsInt();
        const std::string& type = stat_request_map.at("type"s).AsString();

        json::Dict response;
        response.insert({"request_id"s, json::Node(id)});

        if(type == "Stop"s)
        {
            const std::string& stop_name = stat_request_map.at("name"s).AsString();

            json::Array json_buses;
            auto buses = request_handler_.GetStopInfo(stop_name);
            if(!buses.buses_.empty() || buses.is_exist_)
            {
                for(const auto bus: buses.buses_)
                {
                    json_buses.push_back(json::Node{static_cast<std::string>(bus)});
                }

                response.insert({"buses"s, json_buses});
            }
            else
            {
                response.insert({"error_message"s, json::Node("not found"s)});
            }
        }
        else if(type == "Bus"s)
        {
            const std::string& bus_name = stat_request_map.at("name"s).AsString();

            if(request_handler_.FindRoute(bus_name) == nullptr)
            {
                response.insert({"error_message"s, json::Node("not found"s)});
            }
            else
            {
                auto route = request_handler_.GetRouteInfo(bus_name);
                response.insert({"curvature", json::Node(route.curvature_)});
                response.insert({"stop_count", json::Node(static_cast<int>(route.stops_count_))});
                response.insert({"unique_stop_count", json::Node(static_cast<int>(route.unique_stops_count_))});
                response.insert({"route_length", json::Node(static_cast<double>(route.distance_))});
            }
        }
        else if(type == "Map"s)
        {
            std::ostringstream o;
            svg::Document svg = request_handler_.RenderMap();
            svg.Render(o);

            response.insert({"map"s, json::Node(o.str())});
        }

        json_response.push_back(response);
    }

    json::Print(json::Document(json::Node(std::move(json_response))), out);
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

transport::renderer::RenderSettings JsonReader::GetRenderSettings(const json::Dict& json_render_settings)
{
    transport::renderer::RenderSettings render_settings;

    render_settings.width = json_render_settings.at("width").AsDouble();
    render_settings.height = json_render_settings.at("height").AsDouble();
    render_settings.padding = json_render_settings.at("padding").AsDouble();
    render_settings.line_width = json_render_settings.at("line_width").AsDouble();
    render_settings.stop_radius = json_render_settings.at("stop_radius").AsDouble();
    render_settings.bus_label_font_size = json_render_settings.at("bus_label_font_size").AsInt();
    render_settings.bus_label_offset.x = json_render_settings.at("bus_label_offset").AsArray()[0].AsDouble();
    render_settings.bus_label_offset.y = json_render_settings.at("bus_label_offset").AsArray()[1].AsDouble();
    render_settings.stop_label_offset.x = json_render_settings.at("stop_label_offset").AsArray()[0].AsDouble();
    render_settings.stop_label_offset.y = json_render_settings.at("stop_label_offset").AsArray()[1].AsDouble();
    render_settings.stop_label_font_size = json_render_settings.at("stop_label_font_size").AsInt();
    render_settings.underlayer_color = ReadColor(json_render_settings.at("underlayer_color"));
    render_settings.underlayer_width = json_render_settings.at("underlayer_width").AsDouble();

    for(const json::Node& color : json_render_settings.at("color_palette").AsArray())
    {
        render_settings.color_palette.push_back(ReadColor(color));
    }

    return render_settings;
}