#include "engine/engine.h"

int main()
{

    Config config;
    config.engine_config_.mc_addr_str_ = "239.255.10.10";
    config.engine_config_.mc_port_ = 4321;
    config.engine_config_.thread_num_ = 2;

    Engine *engine = new Engine(config);

    engine->Run();
    return 0;
}