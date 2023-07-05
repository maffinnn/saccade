#include "exchange.h"

int main(int argc, char *argv[])
{
    /* validate number of arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Exchange Name> <Host Port>\n", argv[0]);
        exit(1);
    }
    Exchange exchange(argv[1], atoi(argv[2]));
    exchange.Init();
    exchange.Run();

    return 0;
}