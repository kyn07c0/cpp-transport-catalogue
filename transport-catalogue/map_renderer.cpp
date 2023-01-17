#include "map_renderer.h"
#include <unordered_set>

namespace transport::renderer {

bool IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

void MapRenderer::SetSettings(Settings&& settings)
{
    setting_ = std::move(settings);
}

Settings MapRenderer::GetSettings()
{
    return setting_;
}

void MapRenderer::CalculateSphereProjector(const std::vector<geo::Coordinates>& points)
{
    sphere_projector_ = SphereProjector{points.begin(),
                                        points.end(),
                                        setting_.width,
                                        setting_.height,
                                        setting_.padding};
}

void MapRenderer::RenderBus(svg::Document& doc, const domain::Bus* bus, size_t color_number)
{
    svg::Polyline polyline;
    for (const domain::Stop *stop: bus->stops_)
    {
        polyline.AddPoint(sphere_projector_(stop->coordinates_));
    }

    if (!bus->is_roundtrip_)
    {
        for (size_t i = bus->stops_.size() - 2; i > 0; --i)
        {
            polyline.AddPoint(sphere_projector_(bus->stops_[i]->coordinates_));
        }

        polyline.AddPoint(sphere_projector_(bus->stops_[0]->coordinates_));
    }

    size_t color = color_number % setting_.color_palette.size();
    polyline.SetFillColor(svg::NoneColor)
            .SetStrokeColor(setting_.color_palette[color])
            .SetStrokeWidth(setting_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    doc.Add(polyline);
}

void MapRenderer::RenderBusName(svg::Document& doc, const domain::Bus* bus, size_t color_number)
{
    svg::Text text;
    svg::Text text_background;
    size_t color = color_number % setting_.color_palette.size();

    text_background.SetPosition(sphere_projector_({bus->stops_[0]->coordinates_}))
            .SetOffset({setting_.bus_label_offset})
            .SetFontSize(setting_.bus_label_font_size)
            .SetFontFamily("Verdana").SetFontWeight("bold")
            .SetData(bus->name_)
            .SetFillColor(setting_.underlayer_color)
            .SetStrokeColor(setting_.underlayer_color)
            .SetStrokeWidth(setting_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    text.SetPosition(sphere_projector_({bus->stops_[0]->coordinates_}))
            .SetOffset({setting_.bus_label_offset})
            .SetFontSize(setting_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name_)
            .SetFillColor(setting_.color_palette[color]);

    doc.Add(text_background);
    doc.Add(text);

    if(!bus->is_roundtrip_ && bus->stops_[0] != bus->stops_.back())
    {
        text_background.SetPosition(sphere_projector_({bus->stops_.back()->coordinates_}))
                .SetOffset({setting_.bus_label_offset})
                .SetFontSize(setting_.bus_label_font_size)
                .SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(bus->name_)
                .SetFillColor(setting_.underlayer_color)
                .SetStrokeColor(setting_.underlayer_color)
                .SetStrokeWidth(setting_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetPosition(sphere_projector_({bus->stops_.back()->coordinates_}))
                .SetOffset({setting_.bus_label_offset})
                .SetFontSize(setting_.bus_label_font_size)
                .SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(bus->name_)
                .SetFillColor(setting_.color_palette[color]);

        doc.Add(text_background);
        doc.Add(text);
    }
}

void MapRenderer::RenderStop(svg::Document& doc, const domain::Stop* stop)
{
    svg::Circle stop_point;
    stop_point.SetCenter(sphere_projector_({stop->coordinates_}))
              .SetRadius(setting_.stop_radius)
              .SetFillColor("white");

    doc.Add(stop_point);
}

void MapRenderer::RenderStopName(svg::Document& doc, const domain::Stop* stop)
{
    svg::Text text;
    svg::Text text_background;

    text_background.SetPosition(sphere_projector_({stop->coordinates_}))
                   .SetOffset({setting_.stop_label_offset})
                   .SetFontSize(setting_.stop_label_font_size)
                   .SetFontFamily("Verdana").SetData(stop->name_)
                   .SetFillColor(setting_.underlayer_color)
                   .SetStrokeColor(setting_.underlayer_color)
                   .SetStrokeWidth(setting_.underlayer_width)
                   .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                   .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    text.SetPosition(sphere_projector_({stop->coordinates_}))
        .SetOffset({setting_.stop_label_offset})
        .SetFontSize(setting_.stop_label_font_size)
        .SetFontFamily("Verdana").SetData(stop->name_)
        .SetFillColor("black");

    doc.Add(text_background);
    doc.Add(text);
}

} // namespace transport::renderer
