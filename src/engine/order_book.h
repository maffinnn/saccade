#pragma once
#include <string>
#include <set>

#include "../order/order.h"
#include "event.h"

// orderbook building support based on market feed
class OrderBook
{
public:
    // not used
    struct OrderCompare
    {
        bool operator()(const Order *lhs, const Order *rhs) const
        {
            if (lhs->IsBuy() && rhs->IsBuy())
            {
                return lhs->Price() < rhs->Price();
            }
            else if (!lhs->IsBuy() && !rhs->IsBuy())
            {
                return lhs->Price() < rhs->Price();
            }
            throw std::invalid_argument("order not comparable");
        }
    };
    // best bid ask
    struct BidAsk
    {
        double bid_price_, ask_price_;
        int bid_quantity_, ask_quantity_;
    };

    typedef std::multiset<Order *, OrderCompare> OrderList;
    OrderBook(const std::string &symbol) { symbol_ = symbol; }
    ~OrderBook()
    {
        CleanUp(bids_);
        CleanUp(asks_);
    }

    inline double MarketPrice() const { return market_price_history_.back(); }

    void OnOpen(const MarketData *data)
    {
        if (data->is_buy_)
        {
            bids_.insert(new Order(data->order_id_, data->order_id_, data->symbol_, data->is_buy_, data->price_, data->quantity_));
        }
        else
        {
            asks_.insert(new Order(data->order_id_, data->order_id_, data->symbol_, data->is_buy_, data->price_, data->quantity_));
        }
    }

    void OnCancel(const MarketData *data)
    {
        Order order(data->order_id_, data->order_id_, data->symbol_, data->is_buy_, data->price_, data->quantity_);
        if (data->is_buy_)
        {
            Remove(bids_, &order);
        }
        else
        {
            Remove(asks_, &order);
        }
    }

    void OnTrade(const MarketData *data)
    {

        UpdateMarketPrice(data->price_);
        Order order(data->order_id_, data->order_id_, data->symbol_, data->is_buy_, data->price_, data->quantity_);
        if (data->is_buy_)
        {
            Remove(bids_, &order);
        }
        else
        {
            Remove(asks_, &order);
        }
    }

    BidAsk BestBidAsk()
    {
        BidAsk res;
        auto it = bids_.begin();
        if (it != bids_.end())
        {
            res.bid_price_ = (*it)->Price();
            while (it != bids_.end() && res.bid_price_ == (*it)->Price())
            {
                res.bid_quantity_ += (*it)->Quantity();
                it++;
            }
        }

        it = asks_.begin();
        if (it != asks_.end())
        {
            res.ask_price_ = (*it)->Price();
            while (it != bids_.end() && res.ask_price_ == (*it)->Price())
            {
                res.ask_quantity_ += (*it)->Quantity();
                it++;
            }
        }

        return res;
    }

    // TODO: function to get order book level/depth
private:
    void Remove(OrderList &order_list, Order *order)
    {
        auto it = order_list.lower_bound(order);
        if (it != order_list.end())
        {
            while (it != order_list.end() && (!(*it)->Equals(order)))
                it++;
            if ((*it)->Equals(order))
            {
                delete *it;
                order_list.erase(it);
            }
        }
    }
    
    void CleanUp(OrderList &orderlist)
    {
        auto it = orderlist.begin();
        for (; it != orderlist.end(); it++)
        {
            delete *it;
            orderlist.erase(it);
        }
    }
    void UpdateMarketPrice(double price) { market_price_history_.push_back(price); }

    std::string symbol_;
    std::vector<double> market_price_history_ = {0}; // initialized at 0;
    OrderList bids_, asks_;
};