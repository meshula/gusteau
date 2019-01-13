

#define LABTEXT_ODR
#include "csp.h"
#include <iostream>

char* sample = R"csp(

CLOCK = (tick -> CLOCK "ticked")
CLOCK2 = (tick -> (tock -> CLOCK2 "clock2_tocked") "ticked")

)csp";

int main() try
{
    std::vector<std::unique_ptr<CSP_Process>> processes = parse_csp(sample, strlen(sample));
    std::cout << "Parsed " << processes.size() << " processes\n";
    for (auto& i : processes)
    {
        std::cout << i->name << " = (" << i->event << " -> " << i->behavior;
        if (i->out.length())
            std::cout << " \"" << i->out << "\"";
        std::cout << ")\n";
    }
    return 0;
}
catch(std::exception& exc)
{
    std::cerr << "Exception caught: " << exc.what();
}
