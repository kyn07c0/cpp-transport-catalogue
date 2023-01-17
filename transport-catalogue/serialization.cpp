#include "serialization.h"
#include <fstream>
#include <string>
#include <utility>

namespace proto {

using namespace std::literals;

Serializer::Serializer(transport::Catalogue& transport_catalogue,
                       transport::renderer::MapRenderer& map_renderer,
                       route::TransportRouter& transport_router) : transport_catalogue_(transport_catalogue),
                                                                   map_renderer_(map_renderer),
                                                                   transport_router_(transport_router)
{
}

void Serializer::operator()(const std::string& file)
{
    SerializationStops();
    SerializationDistances();
    SerializationBuses();

    SerializationRenderSettings();

    SerializationRouterSettings();
    SerializationGraph();
    SerializationVertexIds();
    SerializationRouteInfo();

    std::ofstream out(file, std::ios::binary);
    proto_main_.SerializeToOstream(&out);
}

void Serializer::SerializationStops()
{
    for(const auto& stop : transport_catalogue_.GetStops())
    {
        Coordinates proto_coordinates;
        proto_coordinates.set_lat(stop->coordinates_.lat);
        proto_coordinates.set_lng(stop->coordinates_.lng);

        Stop proto_stop;
        proto_stop.set_name(stop->name_);
        *proto_stop.mutable_coordinates() = proto_coordinates;

        *proto_main_.mutable_transport_catalogue()->add_stops() = proto_stop;
    }
}

void Serializer::SerializationDistances()
{
    for(const auto& [stop1, stop2_distance] : transport_catalogue_.GetAllDistances())
    {
        for(const auto& [stop2, distance] : stop2_distance)
        {
            Distance proto_distance;
            proto_distance.set_stop_from(std::string(stop1));
            proto_distance.set_stop_to(std::string(stop2));
            proto_distance.set_distance(distance);

            *proto_main_.mutable_transport_catalogue()->add_distances() = proto_distance;
        }
    }
}

void Serializer::SerializationBuses()
{
    for(const auto& bus : transport_catalogue_.GetBuses())
    {
        Bus proto_bus;

        proto_bus.set_name(bus->name_);
        proto_bus.set_is_roundtrip(bus->is_roundtrip_);

        for(const auto& stop : bus->stops_)
        {
            Coordinates proto_coordinates;
            proto_coordinates.set_lat(stop->coordinates_.lat);
            proto_coordinates.set_lng(stop->coordinates_.lng);

            Stop proto_stop;
            proto_stop.set_name(stop->name_);
            *proto_stop.mutable_coordinates() = proto_coordinates;

            *proto_bus.add_stops() = proto_stop;
        }

        *proto_main_.mutable_transport_catalogue()->add_buses() = proto_bus;
    }
}

void Serializer::SerializationRenderSettings()
{
    transport::renderer::Settings renderer_settings = map_renderer_.GetSettings();

    Point proto_point_bus_label_offset;
    proto_point_bus_label_offset.set_x(renderer_settings.bus_label_offset.x);
    proto_point_bus_label_offset.set_y(renderer_settings.bus_label_offset.y);

    Point proto_point_stop_label_offset;
    proto_point_stop_label_offset.set_x(renderer_settings.stop_label_offset.x);
    proto_point_stop_label_offset.set_y(renderer_settings.stop_label_offset.y);

    Color proto_color_underlayer_color = SerializationColor(renderer_settings.underlayer_color);

    RenderSettings proto_render_settings;

    proto_render_settings.set_width(renderer_settings.width);
    proto_render_settings.set_height(renderer_settings.height);
    proto_render_settings.set_padding(renderer_settings.padding);
    proto_render_settings.set_line_width(renderer_settings.line_width);
    proto_render_settings.set_stop_radius(renderer_settings.stop_radius);
    proto_render_settings.set_bus_label_font_size(renderer_settings.bus_label_font_size);
    *proto_render_settings.mutable_bus_label_offset() = proto_point_bus_label_offset;
    proto_render_settings.set_stop_label_font_size(renderer_settings.stop_label_font_size);
    *proto_render_settings.mutable_stop_label_offset() = proto_point_stop_label_offset;
    *proto_render_settings.mutable_underlayer_color() = SerializationColor(renderer_settings.underlayer_color);
    proto_render_settings.set_underlayer_width(renderer_settings.underlayer_width);

    for(const auto& color : renderer_settings.color_palette)
    {
        *proto_render_settings.add_color_palette() = SerializationColor(color);
    }

    *proto_main_.mutable_render_settings() = proto_render_settings;
}

Color Serializer::SerializationColor(const svg::Color& color)
{
    Color proto_color;

    if(std::holds_alternative<std::string>(color))
    {
        proto_color.set_name(std::get<std::string>(color));

        proto_color.set_color_type("str"s);
    }
    else if(std::holds_alternative<svg::Rgb>(color))
    {
        Rgb proto_rgb;
        proto_rgb.set_red(std::get<svg::Rgb>(color).red);
        proto_rgb.set_green(std::get<svg::Rgb>(color).green);
        proto_rgb.set_blue(std::get<svg::Rgb>(color).blue);
        *proto_color.mutable_rgb() = proto_rgb;

        proto_color.set_color_type("rgb"s);
    }
    else if(std::holds_alternative<svg::Rgba>(color))
    {
        Rgba proto_rgba;
        proto_rgba.set_red(std::get<svg::Rgba>(color).red);
        proto_rgba.set_green(std::get<svg::Rgba>(color).green);
        proto_rgba.set_blue(std::get<svg::Rgba>(color).blue);
        proto_rgba.set_opacity(std::get<svg::Rgba>(color).opacity);
        *proto_color.mutable_rgba() = proto_rgba;

        proto_color.set_color_type("rgba"s);
    }

    return proto_color;
}

void Serializer::SerializationRouterSettings()
{
    route::Settings router = transport_router_.GetSettings();

    RoutingSettings proto_routing_settings;
    proto_routing_settings.set_bus_wait_time(router.bus_wait_time);
    proto_routing_settings.set_bus_velocity(router.bus_velocity);

    *proto_main_.mutable_transport_router()->mutable_routing_settings() = proto_routing_settings;
}

void Serializer::SerializationGraph()
{
    const auto& edges = transport_router_.GetGraph()->GetEdges();
    for(const auto& edge : edges)
    {
        auto& proto_edge = *proto_main_.mutable_transport_router()->mutable_graph()->add_edges();
        proto_edge.set_from(edge.from);
        proto_edge.set_to(edge.to);
        proto_edge.set_weight(edge.weight);
    }

    const auto& incidence_lists = transport_router_.GetGraph()->GetIncidenceLists();
    for(const auto& incidence_list : incidence_lists)
    {
        auto& proto_incidence_list = *proto_main_.mutable_transport_router()->mutable_graph()->add_incidence_lists();
        for(const auto& edge_id : incidence_list)
        {
            proto_incidence_list.add_list(edge_id);
        }
    }
}

void Serializer::SerializationVertexIds()
{
    for(const auto& [stop, vertex_ids] : transport_router_.GetStopVertexIds())
    {
        auto& proto_stop_vertex_ids = *proto_main_.mutable_transport_router()->add_stop_vertex_ids();
        proto_stop_vertex_ids.set_name(std::string(stop));
        proto_stop_vertex_ids.set_start_wait(vertex_ids.start_wait);
        proto_stop_vertex_ids.set_end_wait(vertex_ids.end_wait);
    }
}

void Serializer::SerializationRouteInfo()
{
    for(const auto& info : transport_router_.GetEdgesInfo())
    {
        auto& proto_edges_info = *proto_main_.mutable_transport_router()->add_edges_info();
        proto_edges_info.set_name(std::string(info.name));
        proto_edges_info.set_span_count(info.span_count);
        proto_edges_info.set_time(info.time);

        Edge edge;
        edge.set_from(info.edge.from);
        edge.set_to(info.edge.to);
        edge.set_weight(info.edge.weight);
        *proto_edges_info.mutable_edge() = edge;
    }
}


Deserializer::Deserializer(transport::Catalogue& transport_catalogue,
                           transport::renderer::MapRenderer& map_renderer,
                           route::TransportRouter& transport_router) : transport_catalogue_(transport_catalogue),
                                                                       map_renderer_(map_renderer),
                                                                       transport_router_(transport_router)
{
}

void Deserializer::operator()(const std::string& file)
{
    std::ifstream in(file, std::ios::binary);
    proto_main_.ParseFromIstream(&in);

    DeserializationStops();
    DeserializationDistances();
    DeserializationBuses();

    DeserializationRenderSettings();

    DeserializationRouterSettings();
    DeserializationGraph();
    DeserializationVertexIds();
    DeserializationRouteInfo();

    transport_router_.Build();
}

void Deserializer::DeserializationStops()
{
    for(int i = 0; i < proto_main_.mutable_transport_catalogue()->stops_size(); ++i)
    {
        Stop proto_stop = proto_main_.transport_catalogue().stops(i);
        transport_catalogue_.AddStop(proto_stop.name(), proto_stop.coordinates().lat(), proto_stop.coordinates().lng());
    }
}

void Deserializer::DeserializationDistances()
{
    for(int i = 0; i < proto_main_.mutable_transport_catalogue()->distances_size(); ++i)
    {
        std::string stop1 = proto_main_.mutable_transport_catalogue()->distances(i).stop_from();
        std::string stop2 = proto_main_.mutable_transport_catalogue()->distances(i).stop_to();
        double distance = proto_main_.mutable_transport_catalogue()->distances(i).distance();
        transport_catalogue_.AddDistance(stop1, stop2, distance);
    }
}

void Deserializer::DeserializationBuses()
{
    for(int i = 0; i < proto_main_.transport_catalogue().buses_size(); ++i)
    {
        Bus proto_bus = proto_main_.mutable_transport_catalogue()->buses(i);

        const std::string& name = proto_bus.name();
        bool is_roundtrip = proto_bus.is_roundtrip();

        std::vector<std::string> stops;
        for(const auto& proto_stop : proto_bus.stops())
        {
            stops.push_back(proto_stop.name());
        }

        transport_catalogue_.AddBus(name, stops, is_roundtrip);
    }
}

void Deserializer::DeserializationRenderSettings()
{
    RenderSettings proto_render_settings = proto_main_.render_settings();

    transport::renderer::Settings renderer_settings;

    renderer_settings.width = proto_render_settings.width();
    renderer_settings.height = proto_render_settings.height();
    renderer_settings.padding = proto_render_settings.padding();
    renderer_settings.line_width = proto_render_settings.line_width();
    renderer_settings.stop_radius = proto_render_settings.stop_radius();
    renderer_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();

    auto proto_bus_label_offset_x = proto_render_settings.bus_label_offset().x();
    auto proto_bus_label_offset_y = proto_render_settings.bus_label_offset().y();
    renderer_settings.bus_label_offset = {proto_bus_label_offset_x, proto_bus_label_offset_y};

    renderer_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();
    renderer_settings.underlayer_width = proto_render_settings.underlayer_width();

    auto proto_stop_label_offset_x = proto_render_settings.stop_label_offset().x();
    auto proto_stop_label_offset_y = proto_render_settings.stop_label_offset().y();
    renderer_settings.stop_label_offset = {proto_stop_label_offset_x, proto_stop_label_offset_y};

    renderer_settings.underlayer_color = DeserializationColor(proto_render_settings.underlayer_color());

    for(const auto& proto_color : proto_render_settings.color_palette())
    {
        svg::Color color = DeserializationColor(proto_color);
        renderer_settings.color_palette.push_back(color);
    }

    map_renderer_.SetSettings(std::move(renderer_settings));
}


svg::Color Deserializer::DeserializationColor(const Color& proto_color)
{
    svg::Color color;

    if(proto_color.color_type() == "str"s)
    {
        color = svg::Color{ proto_color.name() };
    }
    else if(proto_color.color_type() == "rgb"s)
    {
        color = svg::Rgb(proto_color.rgb().red(), proto_color.rgb().green(), proto_color.rgb().blue());
    }
    else if(proto_color.color_type() == "rgba"s)
    {
        color = svg::Rgba(proto_color.rgba().red(), proto_color.rgba().green(), proto_color.rgba().blue(), proto_color.rgba().opacity());
    }

    return color;
}

void Deserializer::DeserializationRouterSettings()
{
    RoutingSettings proto_routing_settings = proto_main_.transport_router().routing_settings();
    route::Settings route_settings;
    route_settings.bus_wait_time = proto_routing_settings.bus_wait_time();
    route_settings.bus_velocity = proto_routing_settings.bus_velocity();
    transport_router_.SetSettings(route_settings);
}

void Deserializer::DeserializationGraph()
{
    Graph proto_graph = proto_main_.transport_router().graph();

    std::vector<graph::Edge<double>> edges;
    for(int i = 0; i < proto_graph.edges_size(); ++i)
    {
        graph::Edge<double> edge{proto_graph.edges(i).from(), proto_graph.edges(i).to(), proto_graph.edges(i).weight()};
        edges.push_back(edge);
    }

    std::vector<std::vector<size_t>> incidence_lists;
    for(int i = 0; i < proto_graph.incidence_lists_size(); ++i)
    {
        const auto& incidence_list = proto_graph.incidence_lists(i);
        incidence_lists.emplace_back();
        for(int j = 0; j < incidence_list.list_size(); ++j)
        {
            incidence_lists[i].push_back(incidence_list.list(j));
        }
    }

    route::TransportRouter::Graph graph;
    graph.SetGraph(edges, incidence_lists);
    transport_router_.SetGraph(std::move(graph));
}

void Deserializer::DeserializationVertexIds()
{
    std::unordered_map<std::string_view, route::Vertexe> stop_vertex_ids;
    for(const auto& proto_stop_vertex_id : proto_main_.transport_router().stop_vertex_ids())
    {
        const domain::Stop* stop = transport_catalogue_.FindStop(proto_stop_vertex_id.name());
        stop_vertex_ids[stop->name_] = {proto_stop_vertex_id.start_wait(), proto_stop_vertex_id.end_wait()};
    }

    transport_router_.SetStopVertexIds(stop_vertex_ids);
}

void Deserializer::DeserializationRouteInfo()
{
    std::vector<route::EdgeInfo> edges_info;

    for(int i = 0; i < proto_main_.transport_router().edges_info_size(); ++i)
    {
        Edge proto_edge = proto_main_.mutable_transport_router()->edges_info(i).edge();
        graph::Edge<double> edge = {proto_edge.from(), proto_edge.to(), proto_edge.weight()};

        int span_count = proto_main_.mutable_transport_router()->edges_info(i).span_count();

        double time = proto_main_.mutable_transport_router()->edges_info(i).time();

        const domain::Stop* stop = transport_catalogue_.FindStop(proto_main_.mutable_transport_router()->edges_info(i).name());
        if(stop != nullptr)
        {
            edges_info.push_back({edge, stop->name_, span_count, time});
        }
        else
        {
            const domain::Bus* bus = transport_catalogue_.FindBus(proto_main_.mutable_transport_router()->edges_info(i).name());
            edges_info.push_back({edge, bus->name_, span_count, time});
        }
    }

    transport_router_.SetEdgesInfo(edges_info);
}

} // namespace serialization
