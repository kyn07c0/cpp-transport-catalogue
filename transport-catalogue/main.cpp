#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

int main()
{
    transport::Catalogue catalogue;
    transport::renderer::MapRenderer map_renderer;
    transport::request::RequestHandler request_handler(catalogue, map_renderer);
    JsonReader json_reader(request_handler);
    json_reader.Exec(std::cin, std::cout);

    return 0;
}
