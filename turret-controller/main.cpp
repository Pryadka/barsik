#include <iostream>
#include "orbital-controller.h"
#include <chrono>

using namespace std::chrono_literals;

int main()
{
    turret::OrbitalController orbi;
    std::this_thread::sleep_for(1s);
    
    orbi.SetAngelsMissmatch(1., 1.);

    std::this_thread::sleep_for(10s);

    return 0;
}