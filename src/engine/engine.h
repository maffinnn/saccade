#pragma once
#include <vector>
#include <thread>
#include <map>

#include "../common.h"
#include "../config.h"
#include "brokerage.h"
#include "event.h"
#include "../strategy/order_manager.h"
#include "../strategy/strategy.h"

class Engine
{
public:
    Engine(Config &config)
    {
        thread_num_ = std::min(config.engine_config_.thread_num_, MAX_THREAD);
        mc_addr_str_ = config.engine_config_.mc_addr_str_;
        mc_port_ = config.engine_config_.mc_port_;
    }

    ~Engine() {}

    void Init() {}
    void RegisterStrategy(const std::string &instrument, Strategy *strategy)
    {
        strategies_.insert({instrument, strategy});
    }

    void Run()
    {
        std::cout << "running threads  " << thread_num_ << std::endl;
        for (int t = 0; t < thread_num_; t++)
        {
            Event *event = &events_[t];
            SetupUPD(event);
            threads_.push_back(std::thread(&Engine::EventLoopDispatcher, this, event));
        }

        for (int t = 0; t < thread_num_; t++)
        {
            threads_[t].join();
        }
    }

    void Shutdown()
    {

        std::cout << "Shutting down..." << std::endl;
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
                switch (data.category_)
                {
                case NEW:
                    break;
                case CANCEL:
                    break;
                case TRADE:
                    break;
                }
            }
        }
    }

    void SetupUPD(Event *event)
    {
        int sockfd;
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            perror("socket() failed");
            exit(1);
        }
        int option = 1;
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }
        // default reuse port
        int reuseport = 1;
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport))) < 0)
        {
            perror("setsockopt(SO_REUSEPORT) failed");
            exit(1);
        }

        event->mc_req_.imr_multiaddr.s_addr = inet_addr(mc_addr_str_.c_str());
        event->mc_req_.imr_interface.s_addr = htonl(INADDR_ANY);
        if ((setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&event->mc_req_, sizeof(event->mc_req_))) < 0)
        {
            perror("setsockopt() failed");
            exit(1);
        }

        event->mc_addr_.sin_family = AF_INET;
        event->mc_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        event->mc_addr_.sin_port = htons(mc_port_);
        if ((bind(sockfd, (struct sockaddr *)&event->mc_addr_, sizeof(event->mc_addr_))) < 0)
        {
            perror("bind() failed");
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
            perror("recvfrom() failed");
            exit(1);
        }
        printf("Received %d bytes from %s: ", recv_len, inet_ntoa(from_addr.sin_addr));
        // not sure about the conversion, using codec or protobuf would be faster
        return recv_len;
    }

    int thread_num_;
    std::vector<std::thread> threads_;
    std::string mc_addr_str_;
    int mc_port_;
    Event events_[MAX_THREAD];
    // instrument : strategy
    // allow multiple {instrument, strategy} pair
    std::multimap<std::string, Strategy *> strategies_;
};