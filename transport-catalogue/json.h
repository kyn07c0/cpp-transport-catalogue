#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

using namespace std::literals;

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class Node
{
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;

    Node(int value);
    Node(double value);
    Node(bool value);
    Node(std::string value);
    Node(std::nullptr_t);
    Node(Array array);
    Node(Dict map);

    const Value& GetValue() const;

    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsDict() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsDict() const;

    bool operator==(const Node& rhs) const;
    bool operator!=(const Node& rhs) const;

private:
    Value value_;
};

void PrintNode(const Node& node, std::ostream& out);

template <typename Value>
void PrintValue(const Value& value, std::ostream& out)
{
    out << value;
}

void PrintValue(std::nullptr_t, std::ostream& out);
void PrintValue(bool boolean, std::ostream& out);
void PrintValue(const std::string& str, std::ostream& out);
void PrintValue(const Array& nodes, std::ostream& out);
void PrintValue(const Dict& nodes, std::ostream& out);

class Document
{
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const;
    bool operator!=(const Document& rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
