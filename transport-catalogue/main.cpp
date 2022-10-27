#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main()
{
    transport::Catalogue catalogue = transport::query::ReadCatalogue(std::cin);
    transport::print::ExecuteRequests(std::cin, catalogue);

    return 0;
}
