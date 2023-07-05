#include "injector/injector.h"

int main()
{

    Injector injector("../data/SCS.log", "239.255.10.10", 4321);
    injector.Init();
    injector.Feed();
    return 0;
}