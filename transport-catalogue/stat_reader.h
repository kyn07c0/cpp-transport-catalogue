#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <iomanip>

namespace transport::print {

    void OutputReader(transport::Catalogue& transport_catalogue);
    void RouteInfo(transport::Catalogue& transport_catalogue, const std::string& query_info);
    void StopInfo(transport::Catalogue& transport_catalogue, const std::string& query_info);

} // namespace transport::print
