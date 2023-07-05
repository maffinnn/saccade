#pragma once

#include <netinet/in.h>
#include "../common.h"
#include "../utils.h"

// TODO: there shld be faster way of doing the serialization and deserialization
struct MarketData
{
    enum OrderCategory
    {
        OPEN,
        CANCEL,
        CHANGE,
        TRADE,
        UNKNOWN,
    };

    MarketData(const std::string &s)
    {

        std::vector<std::string> splits;
        split(s, ' ', &splits);
        if (splits.size() < 7 || splits.size() > 7)
        {
            std::cerr << "[MarketData] invalid parsing\n";
            return;
        }

        timestamp_ = std::stoull(splits[0]);
        order_id_ = splits[1];
        symbol_ = splits[2];
        is_buy_ = splits[3] == "BUY";
        if (splits[4] == "NEW")
        {
            category_ = OrderCategory::OPEN;
        }
        else if (splits[4] == "CANCEL")
        {
            category_ = OrderCategory::CANCEL;
        }
        else if (splits[4] == "TRADE")
        {
            category_ = OrderCategory::TRADE;
        }
        else if (splits[4] == "CHANGE")
        {
            category_ = OrderCategory::CHANGE;
        }
        else
        {
            category_ = OrderCategory::UNKNOWN;
        }
        price_ = std::stod(splits[5]);
        quantity_ = std::stoi(splits[6]);
    }

    ~MarketData() {}

    unsigned long long timestamp_;
    std::string order_id_, symbol_;
    bool is_buy_;
    OrderCategory category_;
    double price_;
    int quantity_;
};

struct Event
{
    int fd_;
    struct sockaddr_in mc_addr_;
    struct ip_mreq mc_req_;
    char buffer_[128];
};