#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

struct Point
{
    Point() = default;
    Point(double x, double y) : x(x), y(y)
    {
    }

    double x = 0;
    double y = 0;
};

struct Rgb
{
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b)
    {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba
{
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : red(r), green(g), blue(b), opacity(o)
    {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};

struct FillColor
{
    std::ostream& out;

    void operator()(std::monostate) const
    {
        out << "none";
    }
    void operator()(std::string color) const
    {
        out << color;
    }
    void operator()(svg::Rgb color) const
    {
        out << "rgb(" << std::to_string(color.red) << ',' << std::to_string(color.green) << ',' << std::to_string(color.blue) << ')';
    }
    void operator()(svg::Rgba color) const
    {
        out << "rgba(" << std::to_string(color.red) << ',' << std::to_string(color.green) << ',' << std::to_string(color.blue) << ',' << color.opacity << ')';
    }
};
std::ostream& operator<<(std::ostream& out, const Color& color);

enum class StrokeLineCap {BUTT, ROUND, SQUARE,};
std::ostream& operator<<(std::ostream &out, StrokeLineCap cap);

enum class StrokeLineJoin {ARCS, BEVEL, MITER, MITER_CLIP, ROUND,};
std::ostream& operator<<(std::ostream &out, StrokeLineJoin join);

template <typename Owner>
class PathProps
{
public:
    Owner& SetFillColor(Color color)
    {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color)
    {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double stroke_width)
    {
        stroke_width_ = stroke_width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap stroke_line_cap)
    {
        stroke_line_cap_ = stroke_line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin stroke_line_join)
    {
        stroke_line_join_ = stroke_line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;
    void RenderAttrs(std::ostream& out) const
    {
        using namespace std::literals;

        if(fill_color_)
        {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if(stroke_color_)
        {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if(stroke_width_)
        {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if(stroke_line_cap_)
        {
            out << " stroke-linecap=\"";
            if (stroke_line_cap_ == StrokeLineCap::BUTT)
            {
                out << "butt";
            }
            if (stroke_line_cap_ == StrokeLineCap::ROUND)
            {
                out << "round";
            }
            if (stroke_line_cap_ == StrokeLineCap::SQUARE)
            {
                out << "square";
            }
            out << "\"";
        }
        if(stroke_line_join_)
        {
            out << " stroke-linejoin=\"";
            if(stroke_line_join_ == StrokeLineJoin::ARCS)
            {
                out << "arcs";
            }
            if(stroke_line_join_ == StrokeLineJoin::BEVEL)
            {
                out << "bevel";
            }
            if(stroke_line_join_ == StrokeLineJoin::MITER)
            {
                out << "miter";
            }
            if(stroke_line_join_ == StrokeLineJoin::MITER_CLIP)
            {
                out << "miter-clip";
            }
            if(stroke_line_join_ == StrokeLineJoin::ROUND)
            {
                out << "round";
            }
            out << "\"";
        }
    }

private:
    Owner& AsOwner()
    {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

/*
* Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
* Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
*/
struct RenderContext
{
    RenderContext(std::ostream& out) : out(out)
    {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0) : out(out), indent_step(indent_step), indent(indent)
    {
    }

    RenderContext Indented() const;
    void RenderIndent() const;

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
* Абстрактный базовый класс Object служит для унифицированного хранения
* конкретных тегов SVG-документа
* Реализует паттерн "Шаблонный метод" для вывода содержимого тега
*/
class Object
{
public:
    void Render(const RenderContext& context) const;
    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
* Класс Circle моделирует элемент <circle> для отображения круга
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
*/
class Circle final : public Object, public PathProps<Circle>
{
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;
};

/*
* Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
*/
class Polyline final : public Object, public PathProps<Polyline>
{
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text>
{
public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_ = {0,0};
    Point offset_ = {0,0};
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer
{
public:
    virtual ~ObjectContainer() = default;
    virtual void AddPtr(std::unique_ptr<Object>&& object) = 0;

    template <typename T>
    void Add(T obj)
    {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }
};

class Drawable
{
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

class Document : public ObjectContainer
{
public:
    Document() = default;

    void AddPtr(std::unique_ptr<Object>&& obj) override;
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

} // namespace svg


namespace shapes {

class Triangle : public svg::Drawable
{
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3) : p1_(p1), p2_(p2), p3_(p3)
    {
    }

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable
{
public:
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) : center_(center),
                                                                                outer_rad_(outer_rad),
                                                                                inner_rad_(inner_rad),
                                                                                num_rays_(num_rays)
    {
    }

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const;
    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point center_;
    double outer_rad_;
    double inner_rad_;
    int num_rays_;
};

class Snowman : public svg::Drawable
{
public:
    Snowman(svg::Point head_center, double head_radius) : head_center_(head_center),
                                                          head_radius_(head_radius)
    {
    }

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point head_center_;
    double head_radius_;
};

} // namespace shapes
