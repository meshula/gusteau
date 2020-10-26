

#define LABTEXT_ODR
#include "csp.h"
#include <iostream>

char* csp_src = R"csp(
    CLOCK = (tick -> CLOCK "ticked")
    CLOCK2 = (tick -> (tock -> CLOCK2 "clock2_tocked") "ticked")
)csp";

int main() try
{
    CSP* csp = csp_parse(nullptr, csp_src, strlen(csp_src));
    std::cout << "Parsed " << csp->processes.size() << " processes\n";
    for (auto& i : csp->processes)
    {
        std::cout << i->name << " = (" << i->event << " -> " << i->behavior;
        if (i->out.length())
            std::cout << " \"" << i->out << "\"";
        std::cout << ")\n";
    }
    csp_bind_lambda(csp, "ticked", [](){printf("tick\n");});
    csp_bind_lambda(csp, "clock2_tocked", [](){printf("tock\n");});
    csp_emit(csp, "tick");
    csp_emit(csp, "foo");
    csp_emit(csp, "tock");
    csp_emit(csp, "tick");
    csp_emit(csp, "tock");
    csp_update(csp);
    return 0;
}
catch(std::exception& exc)
{
    std::cerr << "Exception caught: " << exc.what();
}
