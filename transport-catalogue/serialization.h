#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"

namespace proto {

class Serializer
{
public:
    Serializer(transport::Catalogue& transport_catalogue, transport::renderer::MapRenderer& map_renderer, route::TransportRouter& transport_router);
    void operator()(const std::string& file);

private:
    transport::Catalogue& transport_catalogue_;
    transport::renderer::MapRenderer map_renderer_;
    route::TransportRouter& transport_router_;
    Main proto_main_;

    void SerializationStops();
    void SerializationDistances();
    void SerializationBuses();

    void SerializationRenderSettings();
    Color SerializationColor(const svg::Color& color);

    void SerializationRouterSettings();
    void SerializationGraph();
    void SerializationVertexIds();
    void SerializationRouteInfo();
};

class Deserializer
{
public:
    Deserializer(transport::Catalogue& transport_catalogue, transport::renderer::MapRenderer& map_renderer, route::TransportRouter& transport_router);
    void operator()(const std::string& file);

private:
    transport::Catalogue& transport_catalogue_;
    transport::renderer::MapRenderer& map_renderer_;
    route::TransportRouter& transport_router_;
    Main proto_main_;

    void DeserializationStops();
    void DeserializationDistances();
    void DeserializationBuses();

    void DeserializationRenderSettings();
    svg::Color DeserializationColor(const Color& proto_color);

    void DeserializationRouterSettings();
    void DeserializationGraph();
    void DeserializationVertexIds();
    void DeserializationRouteInfo();
};

} // namespace serialization
