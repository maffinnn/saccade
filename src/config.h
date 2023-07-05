#pragma once

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

    struct EngineConfig
    {
        int thread_num_;
        std::string mc_addr_str_;
        int mc_port_;
    };

    EngineConfig engine_config_;
};