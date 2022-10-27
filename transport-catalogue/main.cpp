#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main()
{
    transport::Catalogue transport_catalogue;
    transport::query::InputReader(transport_catalogue);
    transport::print::OutputReader(transport_catalogue);

    return 0;
}
