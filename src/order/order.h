#pragma once

#include <string>
#include <iomanip>

class Order
{
public:
    enum State
    {
        SUBMITTED,
        REJECTED,
        ACCEPTED,
        MODIFYREQUESTED,
        MODIFYREJECTED,
        MODIFIED,
        PARTIALFILLED,
        FILLED,
        CANCELREQESTED,
        CANCELREJECTED,
        CANCELLED,
        UNKNOWN
    };

    Order(const std::string &exchange_id, const std::string &id, const std::string &symbol, bool buy, double price, int quantity)
    {
        exchange_ = exchange_id;
        id_ = id;
        symbol_ = symbol;
        is_buy_ = buy;
        price_ = price;
        quantity_ = quantity;
        state_ = State::UNKNOWN;
    }

    Order(const Order &) = delete;
    Order &operator=(const Order &) = delete;
    ~Order() {}

    inline std::string ID() const { return id_; }
    inline std::string Exchange() const { return exchange_; }
    inline std::string Symbol() const { return symbol_; }
    inline bool IsBuy() const { return is_buy_; }
    inline int Price() const { return price_; }
    inline int Quantity() const { return quantity_; }
    inline bool Equals(Order *order) const
    {
        return id_ == order->ID() && is_buy_ == order->IsBuy() && price_ == order->Price();
    }

    inline bool Filled() const { return quantity_ == 0; }

    void OnFill() { state_ = State::FILLED; }
    void OnOpen() { state_ = State::ACCEPTED; }
    void OnChange() { state_ = State::MODIFYREQUESTED; }
    void OnTraded() { state_ = State::FILLED; }
    void OnReject() { state_ = State::REJECTED; }
    void OnCancel() { state_ = State::CANCELLED; }

    friend std::ostream &operator<<(std::ostream &os, const Order &order)
    {
        if (order.IsBuy())
            os << "Bid:";
        else
            os << "Ask:";
        os << order.Symbol() << "|" << order.Quantity() << "@" << std::setprecision(2) << order.Price() << "#" << order.ID() << std::endl;
        return os;
    }

private:
    std::string id_, exchange_;
    bool is_buy_;
    double price_;
    int quantity_;
    std::string symbol_;

    State state_;
};