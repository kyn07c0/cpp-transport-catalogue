#include "map_renderer.h"
#include <unordered_set>

namespace transport::renderer {

bool IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

void MapRenderer::SetRendererSettings(RenderSettings &&renderer_settings)
{
    render_setting_ = std::move(renderer_settings);
}

void MapRenderer::CalculateSphereProjector(const std::vector<geo::Coordinates> &points)
{
    sphere_projector_ = SphereProjector{points.begin(),
                                        points.end(),
                                        render_setting_.width,
                                        render_setting_.height,
                                        render_setting_.padding};
}

void MapRenderer::RenderRoute(svg::Document &doc, const domain::Route *route, size_t color_number)
{
    svg::Polyline polyline;
    for (const domain::Stop *stop: route->stops_)
    {
        polyline.AddPoint(sphere_projector_(stop->coordinates_));
    }

    if (!route->is_roundtrip_)
    {
        for (size_t i = route->stops_.size() - 2; i > 0; --i)
        {
            polyline.AddPoint(sphere_projector_(route->stops_[i]->coordinates_));
        }

        polyline.AddPoint(sphere_projector_(route->stops_[0]->coordinates_));
    }

    size_t color = color_number % render_setting_.color_palette.size();
    polyline.SetFillColor(svg::NoneColor)
            .SetStrokeColor(render_setting_.color_palette[color])
            .SetStrokeWidth(render_setting_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    doc.Add(polyline);
}

void MapRenderer::RenderRouteName(svg::Document &doc, const domain::Route *route, size_t color_number)
{
    svg::Text text;
    svg::Text text_background;
    size_t color = color_number % render_setting_.color_palette.size();

    text_background.SetPosition(sphere_projector_({route->stops_[0]->coordinates_}))
            .SetOffset({render_setting_.bus_label_offset})
            .SetFontSize(render_setting_.bus_label_font_size)
            .SetFontFamily("Verdana").SetFontWeight("bold")
            .SetData(route->name_)
            .SetFillColor(render_setting_.underlayer_color)
            .SetStrokeColor(render_setting_.underlayer_color)
            .SetStrokeWidth(render_setting_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    text.SetPosition(sphere_projector_({route->stops_[0]->coordinates_}))
            .SetOffset({render_setting_.bus_label_offset})
            .SetFontSize(render_setting_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight(
                    "bold").SetData(route->name_)
            .SetFillColor(render_setting_.color_palette[color]);

    doc.Add(text_background);
    doc.Add(text);

    if (!route->is_roundtrip_ && route->stops_[0] != route->stops_.back()) {
        text_background.SetPosition(sphere_projector_({route->stops_.back()->coordinates_}))
                .SetOffset({render_setting_.bus_label_offset})
                .SetFontSize(render_setting_.bus_label_font_size)
                .SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(route->name_)
                .SetFillColor(render_setting_.underlayer_color)
                .SetStrokeColor(render_setting_.underlayer_color)
                .SetStrokeWidth(render_setting_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetPosition(sphere_projector_({route->stops_.back()->coordinates_}))
                .SetOffset({render_setting_.bus_label_offset})
                .SetFontSize(render_setting_.bus_label_font_size)
                .SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(route->name_)
                .SetFillColor(render_setting_.color_palette[color]);

        doc.Add(text_background);
        doc.Add(text);
    }
}

void MapRenderer::RenderStop(svg::Document &doc, const domain::Stop *stop)
{
    svg::Circle stop_point;
    stop_point.SetCenter(sphere_projector_({stop->coordinates_}))
            .SetRadius(render_setting_.stop_radius)
            .SetFillColor("white");

    doc.Add(stop_point);
}

void MapRenderer::RenderStopName(svg::Document &doc, const domain::Stop *stop)
{
    svg::Text text;
    svg::Text text_background;

    text_background.SetPosition(sphere_projector_({stop->coordinates_}))
                   .SetOffset({render_setting_.stop_label_offset})
                   .SetFontSize(render_setting_.stop_label_font_size)
                   .SetFontFamily("Verdana").SetData(stop->name_)
                   .SetFillColor(render_setting_.underlayer_color)
                   .SetStrokeColor(render_setting_.underlayer_color)
                   .SetStrokeWidth(render_setting_.underlayer_width)
                   .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                   .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    text.SetPosition(sphere_projector_({stop->coordinates_}))
        .SetOffset({render_setting_.stop_label_offset})
        .SetFontSize(render_setting_.stop_label_font_size)
        .SetFontFamily("Verdana").SetData(stop->name_)
        .SetFillColor("black");

    doc.Add(text_background);
    doc.Add(text);
}

} // namespace transport::renderer
