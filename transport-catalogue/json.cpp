#include "json.h"
#include <algorithm>

using namespace std;

namespace json {

Node LoadNode(istream& input);

Node LoadArray(istream& input)
{
    Array result;
    char c;
    while (input >> c)
    {
        if(c == ']')
        {
            return {move(result)};
        }
        if (c != ',')
        {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    throw json::ParsingError("Array parsing error"s);
}

Node LoadNumber(istream& input)
{
    std::string str;

    while(isdigit(input.peek())
       || input.peek() == '.'
       || input.peek() == 'e'
       || input.peek() == 'E'
       || input.peek() == '-'
       || input.peek() == '+')
    {
        str.push_back(input.get());
    }

    std::size_t found_point = str.find('.');
    std::size_t found_e = str.find('e');
    std::size_t found_E = str.find('E');
    if(found_point != std::string::npos || found_e != std::string::npos || found_E != std::string::npos)
    {
        return {stod(str)};
    }
    else
    {
        return {stoi(str)};
    }
}

Node LoadString(istream& input)
{
    string str;
    for (char c; input >> std::noskipws >> c;)
    {
        if (c == '"')
        {
            input >> std::skipws;
            return {move(str)};
        }
        if (c == '\\')
        {
            input >> c;
            switch (c)
            {
                case '"':
                    str.push_back('"');
                    break;
                case 'n':
                    str.push_back('\n');
                    break;
                case 'r':
                    str.push_back('\r');
                    break;
                case '\\':
                    str.push_back('\\');
                    break;
                case 't':
                    str.push_back('\t');
                    break;
                default:
                    input >> std::skipws;
                    throw json::ParsingError("Bad string"s);
            }
        }
        else
        {
            str.push_back(c);
        }
    }

    throw json::ParsingError("String parsing error"s);
}

Node LoadDict(istream& input)
{
    Dict result;
    char c;
    while (input >> c)
    {
        if(c == '}')
        {
            return {move(result)};
        }

        if(c == ',')
        {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({ move(key), LoadNode(input) });
    }

    throw json::ParsingError("Dictionaty parsing error"s);
}

Node LoadNull(std::istream& input)
{
    string str(4, '\0');
    for(int i = 0; i < 4; ++i)
    {
        input >> str[i];
    }

    if(str == "null")
    {
        return {nullptr};
    }

    throw json::ParsingError("Parsing: null error"s);
}

Node LoadBool(std::istream& input)
{
    string str(5, '\0');

    for(int i = 0; i < 4; ++i)
    {
        input >> str[i];
    }

    if (str.substr(0, 4) == "true")
    {
        return {true};
    }

    input >> str[4];
    if(str == "false")
    {
        return {false};
    }

    throw json::ParsingError("Parsing bool error"s);
}

Node LoadNode(std::istream& input)
{
    char c;
    if (!(input >> c))
    {
        throw ParsingError("Data not found"s);
    }

    if(isdigit(c) || c == '-')
    {
        input.putback(c);
        return LoadNumber(input);
    }
    else if(c == 't' || c == 'f')
    {
        input.putback(c);
        return LoadBool(input);
    }
    else if(c == '"')
    {
        return LoadString(input);
    }
    else if(c == 'n')
    {
        input.putback(c);
        return LoadNull(input);
    }
    else if(c == '[')
    {
        return LoadArray(input);
    }
    else if(c == '{')
    {
        return LoadDict(input);
    }
    else
    {
        throw json::ParsingError("Parsing error"s);
    }
}

Node::Node(int value) : value_(value)
{
}

Node::Node(double value) : value_(value)
{
}

Node::Node(bool value) : value_(value)
{
}

Node::Node(string value) : value_(move(value))
{
}

Node::Node(std::nullptr_t) : value_({})
{
}

Node::Node(Array array) : value_(move(array))
{
}

Node::Node(Dict map) : value_(move(map))
{
}

const Node::Value& Node::GetValue() const
{
    return value_;
}


int Node::AsInt() const
{
    if(!IsInt())
    {
        throw std::logic_error("The value is not int");
    }

    return  std::get<int>(value_);
}

double Node::AsDouble() const
{
    if(!IsDouble())
    {
        throw std::logic_error("The value is not double");
    }

    return IsPureDouble() ? std::get<double>(value_) : AsInt();
}

bool Node::AsBool() const
{
    if(!IsBool())
    {
        throw std::logic_error("The value is not bool");
    }

    return  std::get<bool>(value_);
}

const string& Node::AsString() const
{
    if(!IsString())
    {
        throw std::logic_error("The value is not string");
    }

    return  std::get<std::string>(value_);
}

const Array& Node::AsArray() const
{
    if(!IsArray())
    {
        throw std::logic_error("The value is not array");
    }

    return  std::get<Array>(value_);
}

const Dict& Node::AsMap() const
{
    if(!IsDict())
    {
        throw std::logic_error("The value is not dictionary");
    }

    return  std::get<Dict>(value_);
}

bool Node::IsInt() const
{
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const
{
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const
{
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const
{
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const
{
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsNull() const
{
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const
{
    return std::holds_alternative<Array>(value_);
}

bool Node::IsDict() const
{
    return std::holds_alternative<Dict>(value_);
}

bool Node::IsMap() const
{
    return std::holds_alternative<Dict>(value_);
}

bool Node::operator==(const Node& rhs) const
{
    return value_ == rhs.value_;
}

bool Node::operator!=(const Node& rhs) const
{
    return value_ != rhs.value_;
}


void PrintValue(std::nullptr_t, std::ostream& out)
{
    out << "null"s;
}

void PrintValue(bool boolean, std::ostream& out)
{
    out << (boolean ? "true"s : "false"s);
}

void PrintValue(const std::string& str, std::ostream& out)
{
    out << "\""sv;
    for(const char c : str)
    {
        switch (c)
        {
            case '\n':
                out << "\\n"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '"':
                out << "\\\""sv;
                break;
            case '\\':
                out << "\\"sv;
                break;
            default:
                out << c;
                break;
        }
    }
    out << "\""sv;
}

void PrintValue(const Array& nodes, std::ostream& out)
{
    out << "["s;
    bool first = true;
    for(const Node& node : nodes)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ","s;
        }

        PrintNode(node, out);
    }
    out << "]"s;
}

void PrintValue(const Dict& nodes, std::ostream& out)
{
    out << "{"s;
    bool first = true;

    for(const auto& [key, node] : nodes)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ","s;
        }
        out << ": "s;
        PrintNode(node, out);
    }

    out << "}"s;
}

void PrintNode(const Node& node, std::ostream& out)
{
    std::visit([&out](const auto& value){
                   PrintValue(value, out);
               },
               node.GetValue());
}

Document::Document(Node root) : root_(move(root))
{
}

const Node& Document::GetRoot() const
{
    return root_;
}

bool Document::operator==(const Document& rhs) const
{
    return root_ == rhs.root_;
}

bool Document::operator!=(const Document& rhs) const
{
    return root_ != rhs.root_;
}

Document Load(std::istream& input)
{
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output)
{
    const auto& root = doc.GetRoot();
    if(root.IsNull())
    {
        output << "null"s;
        return;
    }
    if(root.IsInt())
    {
        output << root.AsInt();
        return;
    }
    if(root.IsPureDouble())
    {
        output << root.AsDouble();
        return;
    }
    if (root.IsString())
    {
        string str = "\""s;
        for(const char c : root.AsString())
        {
            switch(c)
            {
                case '\\':
                    str += "\\\\"s;
                    break;
                case '\"':
                    str += "\\\""s;
                    break;
                case '\n':
                    str += "\\n"s;
                    break;
                case '\r':
                    str += "\\r"s;
                    break;
                case '\t':
                    str += '\t';
                    break;
                default:
                    str += c;
            }
        }
        str += "\""s;

        output << str;
        return;
    }
    if(root.IsBool())
    {
        if(root.AsBool())
        {
            output << "true"s;
        }
        else
        {
            output << "false"s;
        }

        return;
    }
    if (root.IsArray())
    {
        Array vector = root.AsArray();

        size_t values_count = vector.size();
        output << '[';
        for(size_t i = 0; i < values_count; ++i)
        {
            Print(Document(vector[i]), output);

            if(i != values_count - 1)
            {
                output << ", "s;
            }
        }
        output << ']';

        return;
    }
    if(root.IsMap())
    {
        Dict map = root.AsMap();

        output << '{';
        for(auto it = map.begin(); it != map.end(); ++it)
        {
            output << "\""s << it->first << "\""s << ": "s;
            Print(Document(it->second), output);

            if(it != std::prev(map.end()))
            {
                output << ", "s;
            }
        }
        output << '}';

        return;
    }
}

}  // namespace json
