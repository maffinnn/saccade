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
    Brokerage(const std::string &id, const std::string &server_addr, int server_port)
    {
        id_ = id;
        serv_addr_str_ = server_addr;
        serv_port_ = server_port;
    }

    ~Brokerage() {}

    inline std::string ID() const { return id_; }

    // TODO: generate order id
    // return orderid
    std::string SubmitOrder(const std::string &symbol, const bool is_buy, const double price, const int quantity)
    {
        std::string order_id = "00000000000";
        Order *order = new Order(ID(), order_id, symbol, is_buy, price, quantity);
        if (Connect() < 0)
            return "error connecting";
        std::ostringstream ss;
        ss << "NEW " << *order;
        auto response = Send(ss);
        // simply log the response
        std::cout << response << std::endl;
        close(host_socket_fd_);
        return order_id;
    }

    bool CancelOrder(const std::string &symbol, const std::string &order_id, const bool is_buy)
    {
        if (Connect() < 0)
            return false;
        std::ostringstream ss;
        ss << "CANCEL " << order_id << (is_buy ? "BUY" : "SELL");
        auto response = Send(ss);
        // simply log the response
        std::cout << response << std::endl;
        close(host_socket_fd_);
        return true;
    }

private:
    int Connect()
    {
        // estabilish connection between engine and exchange
        if ((host_socket_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("[Brokerage::Connect] ERROR opening socket");
            return -1;
        }
        struct hostent *server;
        server = gethostbyname(serv_addr_str_.c_str());
        if (server == nullptr)
        {
            perror("[Brokerage::Connect] ERROR, no such host");
            return -1;
        }
        bzero((char *)&serv_addr_, sizeof(serv_addr_));
        serv_addr_.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr_.sin_addr.s_addr, server->h_length);
        serv_addr_.sin_port = htons(serv_port_);
        if (connect(host_socket_fd_, (struct sockaddr *)&serv_addr_, sizeof(serv_addr_)) < 0)
        {
            perror("[Brokerage::Connect] ERROR connecting");
            return -1;
        }
        return 0;
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
    std::string serv_addr_str_;
    int host_socket_fd_, serv_port_;
    struct sockaddr_in serv_addr_;

    char buffer_[BUFFER_SIZE];
};