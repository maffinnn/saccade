#pragma once
#include <vector>
#include <thread>

#include "../common.h"
#include "../config.h"
#include "order_manager.h"
#include "brokerage.h"
#include "event.h"
#include "order_book.h"
#include "../strategy/strategy.h"

class Engine
{
public:
    Engine(Config &config)
    {
        thread_num_ = std::min(config.engine_config_.thread_num_, MAX_THREAD);
        mc_addr_str_ = config.engine_config_.mc_addr_str_;
        mc_port_ = config.engine_config_.mc_port_;

        std::unordered_map<std::string, Brokerage *> brokers;
        for (auto &[instrument, broker_config] : config.engine_config_.broker_configs_)
        {
            brokers[instrument] = new Brokerage(broker_config.name_, broker_config.host_addr_, broker_config.host_port_);
        }
        order_manager_ = new OrderManager(brokers);
    }

    ~Engine()
    {
        for (auto it = books_.begin(); it != books_.end(); it++)
            delete it->second;
        for (auto it = strategies_.begin(); it != strategies_.end(); it++)
            delete it->second;
        delete order_manager_;
    }

    void RegisterStrategy(Strategy *strategy)
    {
        strategy->SetOrderManager(order_manager_);
        strategies_[strategy->Symbol()] = strategy;
        if (strategies_.find(strategy->Symbol()) == strategies_.end())
            std::cout << "ERROR: Unknown strategy\n";
        if (books_.find(strategy->Symbol()) == books_.end())
            books_[strategy->Symbol()] = new OrderBook(strategy->Symbol());
        strategy->SetOrderBook(books_[strategy->Symbol()]);
    }

    void RegisterCallBack(const std::string &instrument, MarketData::OrderCategory type, std::function<void(const MarketData *)> fn)
    {
        event_subscribers_[instrument][type].push_back(fn);
    }

    void Init()
    {
        std::cout << "initializing..." << std::endl;
        for (auto subscriber = strategies_.begin(); subscriber != strategies_.end(); subscriber++)
        {
            subscriber->second->Init();
            std::string instrument = subscriber->first;
            // 1:1 mapping of instrument to strategy, so it's okay
            event_subscribers_[instrument][MarketData::OrderCategory::OPEN].push_back(std::bind(&OrderManager::OnOpen, order_manager_, std::placeholders::_1));
            event_subscribers_[instrument][MarketData::OrderCategory::TRADE].push_back(std::bind(&OrderManager::OnTrade, order_manager_, std::placeholders::_1));
            event_subscribers_[instrument][MarketData::OrderCategory::CANCEL].push_back(std::bind(&OrderManager::OnCancel, order_manager_, std::placeholders::_1));
            event_subscribers_[instrument][MarketData::OrderCategory::OPEN].push_back(std::bind(&OrderBook::OnOpen, books_[instrument], std::placeholders::_1));
            event_subscribers_[instrument][MarketData::OrderCategory::TRADE].push_back(std::bind(&OrderBook::OnTrade, books_[instrument], std::placeholders::_1));
            event_subscribers_[instrument][MarketData::OrderCategory::CANCEL].push_back(std::bind(&OrderBook::OnCancel, books_[instrument], std::placeholders::_1));
        }
    }

    void Run()
    {
        std::cout << "running threads  " << thread_num_ << std::endl;
        for (int t = 0; t < thread_num_; t++)
        {
            Event *event = &events_[t];
            SetUpUPD(event);
            threads_.push_back(std::thread(&Engine::EventLoopDispatcher, this, event));
        }

        for (int t = 0; t < thread_num_; t++)
        {
            threads_[t].join();
        }
    }

    void Shutdown()
    {

        std::cout << "shutting down..." << std::endl;
        for (int t = 0; t < thread_num_; t++)
        {
            /* send a DROP MEMBERSHIP message via setsockopt */
            if ((setsockopt(events_[t].fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *)&events_[t].mc_req_, sizeof(events_[t].mc_req_))) < 0)
            {
                std::cerr << "[Engine::Shutdown] setsockopt() failed\n";
                return;
            }
            close(events_[t].fd_);
        }
    }

private:
    void EventLoopDispatcher(Event *event)
    {
        int recv_len;
        for (;;)
        {
            if ((recv_len = Recv(event)) > 0)
            {
                MarketData data(std::string(event->buffer_));
                auto subscribers = event_subscribers_.find(data.symbol_);
                if (subscribers == event_subscribers_.end())
                {
                    std::cout << "subscriber not found" << std::endl;
                    continue;
                }
                auto hanlders = (subscribers->second).find(data.category_);
                if (hanlders == subscribers->second.end())
                {
                    std::cout << "hanlders not found" << std::endl;
                    continue;
                }
                for (auto fn : hanlders->second)
                {
                    std::cout << "callbacks" << std::endl;
                    fn(&data);
                }
            }
        }
    }
    void SetUpUPD(Event *event)
    {
        int sockfd;
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            perror("[Engine::SetUpUDP] socket() failed");
            exit(1);
        }
        int option = 1;
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))) < 0)
        {
            perror("[Engine::SetUpUDP] setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }
        // default reuse port
        int reuseport = 1;
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport))) < 0)
        {
            perror("[Engine::SetUpUDP] setsockopt(SO_REUSEPORT) failed");
            exit(1);
        }

        event->mc_req_.imr_multiaddr.s_addr = inet_addr(mc_addr_str_.c_str());
        event->mc_req_.imr_interface.s_addr = htonl(INADDR_ANY);
        if ((setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&event->mc_req_, sizeof(event->mc_req_))) < 0)
        {
            perror("[Engine::SetUpUDP] setsockopt() failed");
            exit(1);
        }

        event->mc_addr_.sin_family = AF_INET;
        event->mc_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        event->mc_addr_.sin_port = htons(mc_port_);
        if ((bind(sockfd, (struct sockaddr *)&event->mc_addr_, sizeof(event->mc_addr_))) < 0)
        {
            perror("[Engine::SetUpUDP] bind() failed");
            exit(1);
        }
        event->fd_ = sockfd;
    }

    int Recv(Event *event)
    {
        /* Blocking recv. */
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        int recv_len = recvfrom(event->fd_, event->buffer_, sizeof(event->buffer_), 0, (struct sockaddr *)&from_addr, &from_len);
        if (recv_len < 0)
        {
            perror("[Engine::Recv] ecvfrom() failed");
            exit(1);
        }
        printf("Received %d bytes from %s\n", recv_len, inet_ntoa(from_addr.sin_addr));
        return recv_len;
    }

    int thread_num_;
    std::vector<std::thread> threads_;
    std::string mc_addr_str_;
    int mc_port_;
    Event events_[MAX_THREAD];
    // instrument : strategy
    std::unordered_map<std::string, Strategy *> strategies_;
    typedef std::unordered_map<MarketData::OrderCategory, std::vector<std::function<void(const MarketData *)>>> SubscriptionList;
    std::unordered_map<std::string, SubscriptionList> event_subscribers_;
    std::unordered_map<std::string, OrderBook *> books_;
    OrderManager *order_manager_;
};