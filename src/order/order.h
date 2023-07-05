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
        MODIFIED,
        FILLED,
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
        state_.store(State::SUBMITTED);
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
    inline bool Equals(Order *order) const { return id_ == order->ID() && is_buy_ == order->IsBuy() && price_ == order->Price(); }
    // NOT USED
    void OnFill() { state_.store(State::FILLED); }
    void OnOpen() { state_.store(State::ACCEPTED); }
    void OnChange() { state_.store(State::MODIFIED); }
    void OnTrade() { state_.store(State::FILLED); }
    // NOT USED
    void OnReject() { state_.store(State::REJECTED); }
    void OnCancel() { state_.store(State::CANCELLED); }

    friend std::ostream &operator<<(std::ostream &os, const Order &order)
    {
        if (order.IsBuy())
            os << "Bid:";
        else
            os << "Ask:";
        os << order.Symbol() << "|" << order.Quantity() << "@" << std::setprecision(2) << order.Price() << "#" << order.ID() << std::endl;
        return os;
    }

    friend std::ostream &operator<<(std::ostream &os, const std::atomic_int &state)
    {
        switch (state.load())
        {
        case Order::SUBMITTED:
            os << "SUBMITTED";
            break;
        case Order::REJECTED:
            os << "REJECTED";
            break;
        case Order::ACCEPTED:
            os << "ACCEPTED";
            break;
        case Order::MODIFIED:
            os << "MODIFIED";
            break;
        case Order::FILLED:
            os << "FILLED";
            break;
        case Order::CANCELLED:
            os << "CANCELLED";
            break;
        default:
            os << "UNKNOWN";
            break;
        }
        return os;
    }

private:
    std::string id_, exchange_;
    bool is_buy_;
    double price_;
    int quantity_;
    std::string symbol_;

    std::atomic_int state_;
};