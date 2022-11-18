#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <vector>
#include <unordered_map>

namespace transport::query {

struct Data
{
    std::string name;
    std::vector<std::string> values;
};

std::vector<std::string> Split(std::string& str, const std::string& delimiter);

std::unordered_map<std::string, Data> ParseStop(const std::string& query);
std::unordered_map<std::string, Data> ParseRoute(const std::string& query);
std::unordered_map<std::string, Data> Parse(const std::string& query);

void AddStops(transport::Catalogue& catalogue, std::vector<transport::query::Data>& queries);
void AddRoutes(transport::Catalogue& catalogue, std::vector<transport::query::Data>& queries);

transport::Catalogue ReadCatalogue(std::istream& in);

} // namespace transport::query
