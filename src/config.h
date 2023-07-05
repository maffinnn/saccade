#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class Config
{
public:
    struct InjectorConfig
    {
        std::string source_;
        std::string mc_addr_str_;
        int port_;
    };

    InjectorConfig injector_config_[2];

    struct BrokerConfig
    {
        BrokerConfig(const std::string &name, const std::string &host_addr, int host_port) : name_(name), host_addr_(host_addr), host_port_(host_port) {}
        std::string name_;
        std::string host_addr_;
        int host_port_;
    };

    struct EngineConfig
    {
        int thread_num_;
        std::string mc_addr_str_;
        int mc_port_;
        // instrument:broker
        std::unordered_map<std::string, BrokerConfig> broker_configs_;
    };

    EngineConfig engine_config_;
};