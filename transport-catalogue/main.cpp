#include "transport_catalogue.h"
#include "transport_router.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr)
{
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if(mode == "make_base"sv)
    {
        transport::Catalogue transport_catalogue;
        route::TransportRouter transport_router;
        transport::renderer::MapRenderer map_renderer;
        transport::request::RequestHandler request_handler(transport_catalogue, transport_router, map_renderer);
        transport::request::JsonReader json_reader(request_handler, transport_catalogue, map_renderer, transport_router);
        json_reader.ExecMakeBase(std::cin);
    }
    else if(mode == "process_requests"sv)
    {
        transport::Catalogue transport_catalogue;
        route::TransportRouter transport_router;
        transport::renderer::MapRenderer map_renderer;
        transport::request::RequestHandler request_handler(transport_catalogue, transport_router, map_renderer);
        transport::request::JsonReader json_reader(request_handler, transport_catalogue, map_renderer, transport_router);
        json_reader.ExecProcessRequest(std::cin, std::cout);
    }
    else
    {
        PrintUsage();
        return 1;
    }

    return 0;
}
