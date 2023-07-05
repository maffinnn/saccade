#pragma once

#include <string>
#include <iostream>

#include "order_manager.h"
#include "risk_manager.h"

class Strategy
{
public:
    Strategy(const std::string &symbol, Brokerage *broker)
    {
        symbol_ = symbol;
        order_manager_ = new OrderManager(broker);
    }

    ~Strategy() {}

    virtual void Init() = 0;
    // OnFill: Called when an order is filled
    virtual void OnFill() = 0;
    // OnOpen: Called when a new order occurs
    virtual void OnOpen() = 0;
    // OnChange: Called when an order is modified
    virtual void OnChange() = 0;
    // onHalt: Called when trading is halted
    virtual void onHalt() = 0;
    // OnTraded: called when a strategy's order is bought or sold
    virtual void OnTraded() = 0;
    // OnBought: called when a strategy's order is bought
    virtual void OnBought() = 0;
    // OnSold: called when a strategy's order is sold
    virtual void OnSold() = 0;
    // OnReceived: called when a strategy's order is received
    virtual void OnReceived() = 0;
    // OnRejected: called when a strategy's order is rejected
    virtual void OnRejected() = 0;
    // OnCanceled: called when a strategy's order is canceled
    virtual void OnCancelled() = 0;
    // OnExit: Called when the program shuts down
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

private:
    std::string symbol_;
    OrderManager *order_manager_;
    RiskManager *risk_manager_;
};