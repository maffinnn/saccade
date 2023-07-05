#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

#include "../common.h"

// tcp server
class Exchange
{
public:
    Exchange(const std::string &name, int port)
    {
        name_ = name;
        port_ = port;
    }

    ~Exchange() {}

    void Init()
    {
        SetUpConnection();
    }

    void Run()
    {
        std::cout << name_ << " is running..." << std::endl;

        for (int i = 0; i < MAX_THREAD; i++)
        {
            threads_.push_back(std::thread(&Exchange::Worker, this));
        }
        for (int i = 0; i < MAX_THREAD; i++)
        {
            threads_[i].join();
        }
    }

    void Shutdown()
    {
        Close();
    }

    void Recv() {}

    void Send(std::string message) {}

private:
    void SetUpConnection()
    {
        if ((parent_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Exchange::SetUpConnection] socket() error");
            exit(1);
        }

        // set socket for reuse (otherwise might have to wait 4 minutes every time socket is closed)
        int option = 1;
        setsockopt(parent_fd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&option, sizeof(int));

        bzero((char *)&serv_addr_, sizeof(serv_addr_));
        serv_addr_.sin_family = AF_INET;
        serv_addr_.sin_addr.s_addr = INADDR_ANY;
        serv_addr_.sin_port = htons(port_);

        if (bind(parent_fd_, (struct sockaddr *)&serv_addr_, sizeof(serv_addr_)) < 0)
        {
            perror("[Exchange::SetUpConnection] bind() address failed");
            exit(1);
        }

        if (listen(parent_fd_, 1) < 0)
        {
            perror("[Exchange::SetUpConnection] listen address failed");
            exit(1);
        }
    }

    void Worker()
    {
        // created a temporary buffer for convenience
        char buffer[BUFFER_SIZE];
        for (;;)
        {
            socklen_t cli_len = sizeof(cli_addr_);
            int child_fd_ = accept(parent_fd_, (struct sockaddr *)&cli_addr_, &cli_len);
            if (child_fd_ < 0)
            {
                perror("[Exchange::Run] ERROR on accept");
                continue;
            }

            hostp_ = gethostbyaddr((const char *)&cli_addr_.sin_addr.s_addr, sizeof(cli_addr_.sin_addr.s_addr), AF_INET);
            if (hostp_ == nullptr)
            {
                perror("[Exchange::Run] ERROR on gethostbyaddr");
                continue;
            }

            hostaddrp_ = inet_ntoa(cli_addr_.sin_addr);
            if (hostaddrp_ == nullptr)
            {
                perror("[Exchange::Run] ERROR on inet_ntoa\n");
                continue;
            }

            printf("server established connection with %s (%s)\n", hostp_->h_name, hostaddrp_);

            bzero(buffer, sizeof(buffer));
            int n = read(child_fd_, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("[Exchange::Run] ERROR reading from socket");
                continue;
            }

            printf("server received %d bytes: %s", n, buffer);
            std::string response = "OK";
            n = write(child_fd_, response.c_str(), strlen(response.c_str()));
            if (n < 0)
            {
                perror("[Exchange::Run] ERROR writing to socket");
                continue;
            }

            close(child_fd_);
        }
    }

    void Close()
    {
        close(parent_fd_);
    }

    std::string name_;
    int parent_fd_, port_;
    struct sockaddr_in serv_addr_, cli_addr_;
    struct hostent *hostp_; /* client host info */
    char *hostaddrp_;

    std::vector<std::thread> threads_;
};
