#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <utility>
#include <netdb.h>

#include "../common.h"
#include "../order/order.h"

class Brokerage
{
public:
    Brokerage(const std::string &id, const std::string &host_addr, int host_port)
    {
        id_ = id;
        host_addr_str_ = host_addr;
        host_port_ = host_port;
    }

    ~Brokerage() {}

    inline std::string ID() const { return id_; }

    // TODO: generate order id
    // return orderid
    std::string SubmitOrder(const std::string &symbol, const bool is_buy, const double price, const int quantity)
    {
        std::string order_id = "";
        Order *order = new Order(ID(), order_id, symbol, is_buy, price, quantity);
        Connect();
        std::ostringstream ss;
        ss << "NEW " << *order;
        auto response = Send(ss);
        // TODO: check response
        close(host_socket_fd_);
        return order_id;
    }

    bool CancelOrder(const std::string &symbol, const std::string &order_id, const bool is_buy)
    {
        Connect();
        std::ostringstream ss;
        ss << "CANCEL " << order_id << (is_buy ? "BUY" : "SELL");
        auto response = Send(ss);
        // TODO: check response
        close(host_socket_fd_);
        return true;
    }

private:
    void Connect()
    {
        // estabilish connection between engine and exchange
        if ((host_socket_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("ERROR opening socket");
            exit(1);
        }
        struct hostent *server;
        server = gethostbyname(host_addr_str_.c_str());
        if (server == nullptr)
        {
            perror("ERROR, no such host");
            exit(1);
        }
        bzero((char *)&server_addr_, sizeof(server_addr_));
        server_addr_.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&server_addr_.sin_addr.s_addr, server->h_length);
        server_addr_.sin_port = htons(host_port_);
        if (connect(host_socket_fd_, (struct sockaddr *)&server_addr_, sizeof(server_addr_)) < 0)
        {
            perror("ERROR connecting");
            exit(1);
        }
    }

    std::string Send(std::ostringstream &ss)
    {
        /* send the message line to the server */
        int n = send(host_socket_fd_, ss.str().c_str(), sizeof(ss.str()), 0);
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }

        /* print the server's reply */
        bzero(buffer_, BUFFER_SIZE);
        n = recv(host_socket_fd_, buffer_, BUFFER_SIZE, 0);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }
        printf("Reply from exchange: %s\n", buffer_);
        return std::string(buffer_, BUFFER_SIZE);
    }

    std::string id_;
    std::string host_addr_str_;
    int host_socket_fd_, host_port_;
    struct sockaddr_in server_addr_;

    char buffer_[BUFFER_SIZE];
};