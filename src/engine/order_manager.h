#pragma once

#include <vector>
#include <unordered_map>

#include "../order/order.h"
#include "brokerage.h"
#include "event.h"

class OrderManager
{
public:
    OrderManager(std::unordered_map<std::string, Brokerage *> &brokers)
    {
        brokers_ = std::move(brokers);
    }

    ~OrderManager() {}

    void NewOrder(const std::string &symbol, const bool is_buy, const double price, const int quantity)
    {

        auto it = brokers_.find(symbol);

        if (it != brokers_.end())
        {
            std::string order_id = it->second->SubmitOrder(symbol, is_buy, price, quantity);
            open_orders_[is_buy][symbol][order_id] = std::make_unique<Order>(it->second->ID(), order_id, symbol, is_buy, price, quantity);
        }
    }

    void CancelOrder(const std::string &symbol, const std::string &order_id, const bool is_buy)
    {

        auto it = brokers_.find(symbol);
        if (it != brokers_.end())
        {
            it->second->CancelOrder(symbol, order_id, is_buy);
        }
    }

    void CloseAll(const bool is_buy, const std::string &symbol)
    {
        auto instrument_iter = open_orders_[is_buy].find(symbol);
        if (instrument_iter != open_orders_[is_buy].end())
        {
            auto order_iter = instrument_iter->second.begin();
            if (order_iter != instrument_iter->second.end())
            {
                CancelOrder(order_iter->second->Symbol(), order_iter->second->ID(), order_iter->second->IsBuy());
            }
        }
    }

    void CloseAll()
    {
        for (int i = 0; i < 2; i++)
        {
            auto it = open_orders_[i].begin();
            for (; it != open_orders_[i].end(); it++)
            {
                CloseAll(i, it->first);
            }
        }
    }

    void OnOpen(const MarketData *data)
    {
        std::cout << "[OrderManager::OnOpen]" << std::endl;
        auto instrument_iter = pending_orders_[data->is_buy_].find(data->symbol_);
        if (instrument_iter == pending_orders_[data->is_buy_].end())
            return;
        auto order_iter = instrument_iter->second.find(data->order_id_);
        if (order_iter == instrument_iter->second.end())
            return;

        order_iter->second->OnOpen();
        open_orders_[data->is_buy_][data->symbol_][data->order_id_] = std::move(order_iter->second);
        instrument_iter->second.erase(order_iter);
    }

    void OnTrade(const MarketData *data)
    {
        std::cout << "[OrderManager::OnTrade]" << std::endl;
        auto instrument_iter = open_orders_[data->is_buy_].find(data->symbol_);
        if (instrument_iter == open_orders_[data->is_buy_].end())
            return;
        auto order_iter = instrument_iter->second.find(data->order_id_);
        if (order_iter == instrument_iter->second.end())
            return;
        order_iter->second->OnTrade();
        order_history_[data->is_buy_][data->symbol_][data->order_id_] = std::move(order_iter->second);
        instrument_iter->second.erase(order_iter);
    }

    void OnCancel(const MarketData *data)
    {
        std::cout << "[OrderManager::OnCancel]" << std::endl;
        // look up in open orders
        auto instrument_iter = open_orders_[data->is_buy_].find(data->symbol_);
        if (instrument_iter != open_orders_[data->is_buy_].end())
        {
            auto order_iter = instrument_iter->second.find(data->order_id_);
            if (order_iter != instrument_iter->second.end())
            {
                order_iter->second->OnCancel();
                order_history_[data->is_buy_][data->symbol_][data->order_id_] = std::move(order_iter->second);
                instrument_iter->second.erase(order_iter);
            }
        }

        // look up in pending orders
        instrument_iter = pending_orders_[data->is_buy_].find(data->symbol_);
        if (instrument_iter != pending_orders_[data->is_buy_].end())
        {
            auto order_iter = instrument_iter->second.find(data->order_id_);
            if (order_iter != instrument_iter->second.end())
            {
                order_iter->second->OnCancel();
                order_history_[data->is_buy_][data->symbol_][data->order_id_] = std::move(order_iter->second);
                instrument_iter->second.erase(order_iter);
            }
        }
    }

    /**
     *@brief check if there exists an order for a given instrument
     *       return true if exists, false otherwise
     */
    bool LookUp(const std::string &symbol)
    {
        for (int i = 0; i < 2; i++)
        {
            auto in_pending = pending_orders_[i].find(symbol);
            auto in_open = open_orders_[i].find(symbol);
            auto in_history = order_history_[i].find(symbol);
            if (in_pending != pending_orders_[i].end() || in_open != open_orders_[i].end() || in_history != order_history_[i].end())
                return true;
        }
        return false;
    }

private:
    // instrument:broker
    std::unordered_map<std::string, Brokerage *> brokers_;
    // instrument:order_id:order
    std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<Order>>> pending_orders_[2]; // two side
    std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<Order>>> open_orders_[2];    // two side
    std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<Order>>> order_history_[2];  // two side
};