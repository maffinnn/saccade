#pragma once

#include <vector>
#include <unordered_map>

#include "../order/order.h"
#include "../engine/brokerage.h"

class OrderManager
{
public:
    OrderManager(Brokerage *broker)
    {
        broker_ = broker;
    }
    ~OrderManager()
    {
        for (auto it = open_orders_.begin(); it != open_orders_.end(); ++it)
            delete it->second;

        for (int i = 0; i < order_history_.size(); ++i)
            delete order_history_[i];
    }

    void NewOrder(const std::string &symbol, const bool is_buy, const double price, const int quantity)
    {
        std::string order_id = broker_->SubmitOrder(symbol, is_buy, price, quantity);
        open_orders_[order_id] = new Order(broker_->ID(), order_id, symbol, is_buy, price, quantity);
    }

    void CancelOrder(const std::string &symbol, const std::string &order_id, const bool is_buy)
    {
        if (broker_->CancelOrder(symbol, order_id, is_buy))
        {
            auto it = open_orders_.find(order_id);
            it->second->OnCancel();
            order_history_.push_back(it->second);
            open_orders_.erase(order_id);
        }
    }

    void CloseAll()
    {
        auto it = open_orders_.begin();
        for (; it != open_orders_.end(); it++)
        {
            CancelOrder(it->second->Symbol(), it->second->ID(), it->second->IsBuy());
        }
    }

private:
    Brokerage *broker_;
    std::unordered_map<std::string, Order *> open_orders_;
    std::vector<Order *> order_history_;
    // std::vector<Trade *> trades_;
};