#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <iomanip>

namespace transport::print {

    void RouteInfo(std::ostream& out, transport::Catalogue& catalogue, const std::string& query_info);
    void StopInfo(std::ostream& out, transport::Catalogue& catalogue, const std::string& query_info);
    void ExecuteRequests(std::istream& in, transport::Catalogue& catalogue);

} // namespace transport::print
