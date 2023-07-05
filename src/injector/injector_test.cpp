#include "injector.h"

int main(int argc, char *argv[])
{
    /* validate number of arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <Input File> <Multicast IP> <Multicast Port>\n", argv[0]);
        exit(1);
    }
    // Injector injector("../../data/SCH.log", "239.255.10.10", 4321);
    Injector injector(argv[1], argv[2], atoi(argv[3]));
    injector.Init();
    injector.Run();
    return 0;
}