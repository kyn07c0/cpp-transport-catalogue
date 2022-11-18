#include "svg.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace svg
{
using namespace std::literals;

std::ostream& operator<<(std::ostream &out, StrokeLineCap cap)
{
    switch(cap)
    {
        case StrokeLineCap::BUTT:
            return out << "butt";
        case StrokeLineCap::ROUND:
            return out << "round";
        case StrokeLineCap::SQUARE:
            return out << "square";
    }

    return out;
}

std::ostream& operator<<(std::ostream &out, StrokeLineJoin join)
{
    switch(join)
    {
        case StrokeLineJoin::ARCS:
            return out << "arcs";
        case StrokeLineJoin::BEVEL:
            return out << "bevel";
        case StrokeLineJoin::MITER:
            return out << "miter";
        case StrokeLineJoin::MITER_CLIP:
            return out << "miter-clip";
        case StrokeLineJoin::ROUND:
            return out << "round";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const Color& color)
{
    std::visit(FillColor{out}, color);

    return out;
}

RenderContext RenderContext::Indented() const
{
    return { out, indent_step, indent + indent_step };
}

void RenderContext::RenderIndent() const
{
    for(int i = 0; i < indent; ++i)
    {
        out.put(' ');
    }
}

void Object::Render(const RenderContext& context) const
{
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)
{
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)
{
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point)
{
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<polyline points=\""sv;
    if(points_.empty())
    {
        out << "\"";
    }
    else
    {
        for(size_t i = 0; i < points_.size(); ++i)
        {
            out << points_[i].x << "," << points_[i].y;
            if (i + 1 == points_.size())
            {
                out << "\"";
                break;
            }
            out << " ";
        }
    }
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text --------------------

Text& Text::SetPosition(Point pos)
{
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size)
{
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data)
{
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;

    out << "<text";
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\"" << size_ << "\" ";
    if(!font_family_.empty())
    {
        out << "font-family=\""sv << font_family_ << "\""sv;
    }
    if(!font_family_.empty() && !font_weight_.empty())
    {
        out << " "sv;
    }
    if(!font_weight_.empty())
    {
        out << "font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;

    std::string convert_data{};
    for (auto c : data_)
    {
        if(c == '"')
        {
            convert_data += "&quot;";
        }
        else if(c == '\'')
        {
            convert_data += "&apos;";
        }
        else if(c == '<')
        {
            convert_data += "&lt;";
        }
        else if(c == '>')
        {
            convert_data += "&gt;";
        }
        else if(c == '&')
        {
            convert_data += "&amp;";
        }
        else
        {
            convert_data += c;
        }
    }

    out << convert_data << "</text>"sv;
}

// ---------- Document --------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj)
{
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for(const auto& object : objects_)
    {
        out << "  "sv;
        object->Render(out);
    }
    out << "</svg>"sv;
}

}  // namespace svg


namespace shapes {

// ---------- Triangle ------------------
void Triangle::Draw(svg::ObjectContainer& container) const
{
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

// ---------- Star --------------------
svg::Polyline Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const
{
    svg::Polyline polyline;
    for(int i = 0; i <= num_rays; ++i)
    {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if(i == num_rays)
        {
            break;
        }

        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }

    return polyline;
}

void Star::Draw(svg::ObjectContainer& container) const
{
    container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_).SetFillColor("red").SetStrokeColor("black"));
}

// ---------- Snowman --------------------
void Snowman::Draw(svg::ObjectContainer& container) const
{
    container.Add(svg::Circle().SetCenter({head_center_.x, head_center_.y + head_radius_ * 5})
                               .SetRadius(head_radius_ * 2)
                               .SetFillColor("rgb(240,240,240)")
                               .SetStrokeColor("black"));

    container.Add(svg::Circle().SetCenter({head_center_.x, head_center_.y + head_radius_ * 2})
                               .SetRadius(head_radius_ * 1.5)
                               .SetFillColor("rgb(240,240,240)")
                               .SetStrokeColor("black"));

    container.Add(svg::Circle().SetCenter({head_center_.x, head_center_.y})
                                .SetRadius(head_radius_)
                                .SetFillColor("rgb(240,240,240)")
                                .SetStrokeColor("black"));
}

} // namespace shapes
