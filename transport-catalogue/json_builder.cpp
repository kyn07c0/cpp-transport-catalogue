#include "json_builder.h"

#include <utility>

namespace json {

using namespace std::literals;

Builder::Builder()
{
    nodes_stack_.push_back(&root_);
}

KeyItemContext Builder::Key(const std::string& key)
{
    if(nodes_stack_.back() == nullptr || !nodes_stack_.back()->IsDict())
    {
        throw std::logic_error("Add key error");
    }

    nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]);

    return *this;
}

Builder& Builder::Value(const Node& value)
{
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Add value error");
    }

    if(nodes_stack_.back()->IsArray())
    {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value);
    }
    else
    {
        *nodes_stack_.back() = value;
        nodes_stack_.erase(nodes_stack_.end() - 1);
    }

    is_json_completed = true;

    return *this;
}

DictItemContext Builder::StartDict()
{
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Dictionary start error");
    }

    if(nodes_stack_.back()->IsArray())
    {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
        Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
        nodes_stack_.push_back(node);
    }
    else
    {
        *nodes_stack_.back() = Dict();
    }

    ++dict_complete_;

    return *this;
}

ArrayItemContext Builder::StartArray()
{
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Array start error");
    }

    if(nodes_stack_.back()->IsArray())
    {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
        Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
        nodes_stack_.push_back(node);
    }
    else
    {

        *nodes_stack_.back() = Array();
    }

    ++array_complete_;

    return *this;
}

Builder& Builder::EndDict()
{
    if(nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
    {
        throw std::logic_error("Dictionary end error");
    }

    nodes_stack_.erase(nodes_stack_.end() - 1);
    --dict_complete_;
    is_json_completed = true;

    return *this;
}

Builder& Builder::EndArray()
{
    if(nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
    {
        throw std::logic_error("Array end error");
    }

    nodes_stack_.erase(nodes_stack_.end() - 1);
    --array_complete_;
    is_json_completed = true;

    return *this;
}

Node Builder::Build()
{
    if(root_ == nullptr || !nodes_stack_.empty())
    {
        throw std::logic_error("JSON is empty");
    }
    else if(!is_json_completed)
    {
        throw std::logic_error("JSON is not completed");
    }
    else if(dict_complete_ != 0)
    {
        throw std::logic_error("One of the json dictionaries is not closed");
    }
    else if(array_complete_ != 0)
    {
        throw std::logic_error("One of the json arrays is not closed");
    }

    return root_;
}

// -------------------------------- Context -----------------------------------------

Context::Context(Builder& builder) : builder_(builder)
{
}

KeyItemContext Context::Key(const std::string& key)
{
    return builder_.Key(key);
}

Builder& Context::Value(const Node& value)
{
    return builder_.Value(value);
}

DictItemContext Context::StartDict()
{
    return builder_.StartDict();
}

ArrayItemContext Context::StartArray()
{
    return builder_.StartArray();
}

Builder& Context::EndDict()
{
    return builder_.EndDict();
}

Builder& Context::EndArray()
{
    return builder_.EndArray();
}

// -------------------------------- KeyItemContext -----------------------------------------
KeyItemContext::KeyItemContext(Builder& builder) : Context(builder)
{
}

ValueContext KeyItemContext::Value(const Node& value)
{
    return Context::Value(value);
}

// -------------------------------- ValueContext ---------------------------------------
ValueContext::ValueContext(Builder& builder) : Context(builder)
{
}

// -------------------------------- DictItemContext -----------------------------------
DictItemContext::DictItemContext(Builder& builder) : Context(builder)
{
}

// -------------------------------- ArrayItemContext ------------------------------------
ArrayItemContext::ArrayItemContext(Builder& builder) : Context(builder)
{
}

ArrayValueContext ArrayItemContext::Value(const Node& value)
{
    return Context::Value(value);
}

// -------------------------------- ArrayValueContext -----------------------------------
ArrayValueContext::ArrayValueContext(Builder& builder) : Context(builder)
{
}

ArrayValueContext ArrayValueContext::Value(const Node& value)
{
    return Context::Value(value);
}

} // namespace json
