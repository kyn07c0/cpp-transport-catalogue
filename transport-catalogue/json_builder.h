#pragma once

#include "json.h"

namespace json {

class KeyItemContext;
class ValueContext;
class DictItemContext;
class ArrayItemContext;
class ArrayValueContext;

class Builder
{
public:
    Builder();

    KeyItemContext Key(const std::string& key);
    Builder& Value(const Node& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    bool is_json_completed = false;
    int dict_complete_ = 0;
    int array_complete_ = 0;
};

class Context
{
public:
    Context(Builder& builder);
    KeyItemContext Key(const std::string& key);
    Builder& Value(const Node& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

private:
    Builder& builder_;
};

class KeyItemContext : public Context
{
public:
    KeyItemContext(Builder& builder);
    KeyItemContext Key(const std::string& key) = delete;
    ValueContext Value(const Node& value);
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

class ValueContext : public Context
{
public:
    ValueContext(Builder& builder);
    Builder& Value(Node value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class DictItemContext : public Context
{
public:
    DictItemContext(Builder& builder);

    Builder& Value(Node value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class ArrayItemContext : public Context
{
public:
    ArrayItemContext(Builder& builder);

    KeyItemContext Key(const std::string& key) = delete;
    ArrayValueContext Value(const Node& value);
    Builder& EndDict() = delete;
};

class ArrayValueContext : public Context
{
public:
    ArrayValueContext(Builder& builder);

    KeyItemContext Key(const std::string& key) = delete;
    ArrayValueContext Value(const Node& value);
    Builder& EndDict() = delete;
};

} // namespace json
