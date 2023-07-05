#pragma once

#include <netinet/in.h>
#include "../common.h"

enum OrderCategory
{
    NEW,
    CANCEL,
    TRADE,
};

struct MarketData
{
    MarketData(const std::string &s)
    {
        std::string delimiter = " ";
        size_t start = 0, end = s.find(delimiter, start), delim_len = delimiter.length();
        timestamp_ = std::stoull(s.substr(start, end - start));
        start = end + delim_len;
        end = s.find(delimiter, start);
        order_id_ = s.substr(start, end - start);
        start = end + delim_len;
        end = s.find(delimiter, start);
        symbol_ = s.substr(start, end - start);
        start = end + delim_len;
        end = s.find(delimiter, start);
        is_buy_ = s.substr(start, end - start) == "BUY";
        start = end + delim_len;
        end = s.find(delimiter, start);
        std::string substr = s.substr(start, end - start);
        if (substr == "NEW")
        {
            category_ = OrderCategory::NEW;
        }
        else if ("CANCEL")
        {
            category_ = OrderCategory::CANCEL;
        }
        else
        {
            category_ = OrderCategory::TRADE;
        }
        start = end + delim_len;
        end = s.find(delimiter, start);
        price_ = std::stod(s.substr(start, end - start));
        start = end + delim_len;
        quantity_ = std::stoi(s.substr(start));
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