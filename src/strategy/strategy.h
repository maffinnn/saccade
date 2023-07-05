#pragma once

#include <string>
#include <iostream>

#include "../engine/order_manager.h"
#include "risk_manager.h"

class Strategy
{
public:
    Strategy(const std::string &name, const std::string &symbol)
    {
        name_ = name;
        symbol_ = symbol;
    }

    ~Strategy() {}
    void SetOrderBook(OrderBook *order_book)
    {
        order_book_ = order_book;
    }

    void SetOrderManager(OrderManager *order_manager)
    {
        order_manager_ = order_manager;
    }

    std::string Name() const { return name_; }
    std::string Symbol() const { return symbol_; }
    virtual void Init() = 0;
    // OnOpen: Called when a new order occurs
    virtual void OnOpen(const MarketData *) = 0;
    // OnChange: Called when an order is modified
    virtual void OnChange(const MarketData *) = 0;
    // [NOT USED] OnHalt: Called when trading is halted
    virtual void OnHalt(const MarketData *) = 0;
    // OnTraded: called when a strategy's order is bought or sold
    virtual void OnTrade(const MarketData *) = 0;
    // OnCancelled: called when a strategy's order is canceled
    virtual void OnCancel(const MarketData *) = 0;
    // [NOT USED] OnExit: Called when the program shuts down
    virtual void OnExit() = 0;

    void NewOrder(bool buy, double price, int quantity)
    {
        order_manager_->NewOrder(symbol_, buy, price, quantity);
    }

    void CancelOrder(std::string order_id, bool buy)
    {
        order_manager_->CancelOrder(symbol_, order_id, buy);
    }

    void CloseAll()
    {
        order_manager_->CloseAll();
    }

    OrderManager *GetOrderManager() { return order_manager_; }
    OrderBook *GetOrderBook() { return order_book_; }
    RiskManager *GetRiskManager() { return risk_manager_; }

private:
    std::string symbol_, name_;
    OrderBook *order_book_;
    OrderManager *order_manager_;
    RiskManager *risk_manager_;
};