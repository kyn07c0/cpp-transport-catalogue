#include "input_reader.h"
#include <iostream>
#include <algorithm>

std::vector<std::string> transport::query::Split(std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;
    size_t pos = str.find(delimiter);
    size_t substr_start_pos = 0;

    while(pos != std::string::npos)
    {
        std::string substr = str.substr(substr_start_pos, pos - substr_start_pos);
        result.push_back(substr);
        substr_start_pos = pos + delimiter.length();
        pos = str.find(delimiter, substr_start_pos);
    }

    std::string final_substr = str.substr(substr_start_pos, pos);
    result.push_back(final_substr);

    return result;
}

std::unordered_map<std::string, transport::query::Data> transport::query::ParseStop(const std::string& query)
{
    std::unordered_map<std::string, transport::query::Data> result;

    transport::query::Data parsed_query;

    size_t pos_name_start = query.find_first_of(' ');
    size_t pos_name_stop = query.find_first_of(':') - 1;
    std::string name = query.substr(pos_name_start + 1, pos_name_stop - pos_name_start);

    size_t pos_values_start = query.find_first_of(' ', pos_name_stop) + 1;
    std::string str_values = query.substr(pos_values_start, query.size());
    std::vector<std::string> values = Split(str_values, ", ");

    parsed_query.name = name;
    parsed_query.values = values;

    result.insert({"Stop", std::move(parsed_query)});

    return result;
}

std::unordered_map<std::string, transport::query::Data> transport::query::ParseRoute(const std::string& query)
{
    std::unordered_map<std::string, transport::query::Data> result;
    transport::query::Data parse_query;

    size_t pos_name_start = query.find_first_of(' ');
    size_t pos_name_stop = query.find_first_of(':') - 1;
    parse_query.name = query.substr(pos_name_start + 1, pos_name_stop - pos_name_start);

    size_t pos_start_list_stops = query.find_first_of(':') + 2;
    std::string stops_list = query.substr(pos_start_list_stops, query.length() - pos_start_list_stops);

    if(query.find('-') != std::string::npos)
    {
        std::vector<std::string> stops = Split(stops_list, " - ");
        parse_query.values = stops;

        stops.resize(stops.size() - 1);
        std::reverse(stops.begin(), stops.end());

        parse_query.values.insert(parse_query.values.end(), stops.begin(), stops.end());

        result.emplace("Bus", std::move(parse_query));
    }
    else if(query.find('>') != std::string::npos)
    {
        std::vector<std::string> stops = Split(stops_list, " > ");
        parse_query.values = stops;

        result.emplace("Bus", parse_query);
    }

    return result;
}

std::unordered_map<std::string, transport::query::Data> transport::query::Parse(const std::string& query)
{
    std::unordered_map<std::string, transport::query::Data> result;

    if(query.find("Stop") == 0)
    {
        result = ParseStop(query);
    }
    else if(query.find("Bus") == 0)
    {
        result = ParseRoute(query);
    }

    return result;
}

void transport::query::AddStops(transport::Catalogue& catalogue, std::vector<transport::query::Data>& queries)
{
    for(auto& query : queries)
    {
        std::size_t offset = 0;
        catalogue.AddStop(query.name, std::stod(query.values[0], &offset), std::stod(query.values[1], &offset));
    }

    for(auto& query : queries)
    {
        for(size_t i = 2; i < query.values.size(); ++i)
        {
            std::vector<std::string> distance = Split(query.values[i], "m to ");
            catalogue.AddDistance(query.name, distance[1], stoi(distance[0]));
        }
    }
}

void transport::query::AddRoutes(transport::Catalogue& catalogue, std::vector<transport::query::Data>& queries)
{
    for(auto& query : queries)
    {
        catalogue.AddRoute(query.name, query.values);
    }
}

transport::Catalogue transport::query::ReadCatalogue(std::istream& in)
{
    int query_count;
    in >> query_count;
    in.get();

    std::unordered_map<std::string, transport::query::Data> parse_queries;
    std::vector<transport::query::Data> stop_queries;
    std::vector<transport::query::Data> route_queries;

    for(int i = 0; i < query_count; ++i)
    {
        std::string query;
        std::getline(in, query);

        parse_queries = Parse(query);
        if(parse_queries.count("Stop") != 0)
        {
            stop_queries.push_back(std::move(parse_queries.at("Stop")));
        }
        else if(parse_queries.count("Bus") != 0)
        {
            route_queries.push_back(std::move(parse_queries.at("Bus")));
        }
    }

    transport::Catalogue transport_catalogue;
    AddStops(transport_catalogue, stop_queries);
    AddRoutes(transport_catalogue, route_queries);

    return transport_catalogue;
}
