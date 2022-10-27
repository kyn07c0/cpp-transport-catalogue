#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main()
{
    transport::Catalogue transport_catalogue = transport::query::ReadCatalogue(std::cin);
    transport::print::ExecuteRequests(std::cin, transport_catalogue);

    return 0;
}
