#include "stat_reader.h"

std::ostream& operator<<(std::ostream& out, const transport::detail::RouteInfo& info)
{
    if(info.stops_count_ == 0)
    {
        out << "Bus " << info.name_ << ": not found";
        return out;
    }
    else
    {
        out << "Bus " << info.name_ << ": ";
        out << info.stops_count_ << " stops on route, ";
        out << info.unique_stops_count_ << " unique stops, ";
        out << std::setprecision(6) << info.distance_ << " route length, ";
        out << std::setprecision(6) << info.curvature_ << " curvature";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const transport::detail::StopInfo& info)
{
    if(!info.is_exist)
    {
        out << "Stop " << info.name_ << ": not found";
        return out;
    }

    if(info.buses_.empty())
    {
        out << "Stop " << info.name_ << ": no buses";
    }
    else
    {
        out << "Stop " << info.name_ << ": " << "buses";
        for(auto bus : info.buses_)
        {
            out <<" "<< bus;
        }
    }

    return out;
}

void transport::print::RouteInfo(std::ostream& out, Catalogue& catalogue, const std::string& query_info)
{
    out << catalogue.GetRouteInfo(query_info) << std::endl;
}

void transport::print::StopInfo(std::ostream& out, transport::Catalogue& catalogue, const std::string& query_info)
{
    out << catalogue.GetStopInfo(query_info) << std::endl;
}

void transport::print::ExecuteRequests(std::istream& in, transport::Catalogue& catalogue)
{
    int query_count;
    in >> query_count;
    in.get();

    std::vector<std::pair<std::string, std::string>> queries;

    for(int i = 0; i < query_count; ++i)
    {
        std::string query;
        std::getline(in, query);

        if(query.find("Bus") == 0)
        {
            size_t name_start = query.find_first_of(' ');
            std::string name = query.substr(name_start + 1);
            queries.emplace_back("Bus", name);
        }
        if(query.find("Stop") == 0)
        {
            size_t name_start = query.find_first_of(' ');
            std::string name = query.substr(name_start + 1);
            queries.emplace_back("Stop", name);
        }
    }

    for(auto& [name, str] : queries)
    {
        if(name == "Stop")
        {
            transport::print::StopInfo(std::cout, catalogue, str);
        }
        if(name == "Bus")
        {
            transport::print::RouteInfo(std::cout, catalogue, str);
        }
    }
}
