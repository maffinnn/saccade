#include "injector/injector.h"

int main()
{

    Injector injector("../data/SCH.log", "239.255.10.10", 4321);
    injector.Init();
    injector.Feed();
    return 0;
}