#include "brokerage.h"

int main(int argc, char *argv[])
{
    Brokerage broker("test.exchange", "127.0.0.1", 8000);
    std::string symbol = "APPL";
    double price = 190.14;
    int quantity = 100;
    for (int i = 0; i < 5; i++)
    {
        broker.SubmitOrder(symbol, true, price, quantity);
    }

    return 0;
}