#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <fstream>

#include "../common.h"

// multicast injector
class Injector
{
public:
    Injector(const std::string& market_source, const std::string& multicast_addr, int multicast_port)
    {
        source_ = market_source;
        mc_addr_str_ = multicast_addr;
        mc_port_ = multicast_port;
    }

    ~Injector()
    {
        Close();
    }

    void Init()
    {
        SetUpConnection();
    }

    void Run()
    {
        std::ifstream input(source_);
        std::cout << "Reading from source " << source_ << std::endl;
        if (input.is_open())
        {
            std::string line;
            while (getline(input, line))
            {
                Publish(line);
                std::cout << "sent" << std::endl;
                // slow down
                sleep(5);
            }
        }
        input.close();
    }

    void Publish(std::string market_data)
    {
        int send_len = market_data.length();
        if ((sendto(socket_fd_, market_data.c_str(), send_len, 0, (struct sockaddr *)&mc_addr_, sizeof(mc_addr_))) != send_len)
        {
            std::cerr << "[Injector::Publish] sendto() sent incorrect number of bytes";
        }
    }

    void Close()
    {
        close(socket_fd_);
    }

private:
    void SetUpConnection()
    {
        /* create a socket for sending to the multicast address */
        if ((socket_fd_ = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            std::cerr << "[Injector::SetUpConnection] socket() failed";
            exit(1);
        }

        /* set the TTL (time to live/hop count) for the send */
        if ((setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&mc_ttl_, sizeof(mc_ttl_))) < 0)
        {
            std::cerr << "[Injector::SetUpConnection] setsockopt() failed";
            exit(1);
        }

        /* construct a multicast address structure */
        memset(&mc_addr_, 0, sizeof(mc_addr_));
        mc_addr_.sin_family = AF_INET;
        mc_addr_.sin_addr.s_addr = inet_addr(mc_addr_str_.c_str());
        mc_addr_.sin_port = htons(mc_port_);

        /* clear send buffer */
        memset(buffer_, 0, sizeof(buffer_));
    }

    std::string mc_addr_str_;
    int socket_fd_, mc_port_;
    unsigned char mc_ttl_ = 1;
    char buffer_[BUFFER_SIZE];
    struct sockaddr_in mc_addr_;
    // input source
    std::string source_;
};