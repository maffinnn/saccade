#include "engine.h"

class BuyAndHoldStrategy : public Strategy
{
public:
    BuyAndHoldStrategy(const std::string &name, const std::string &symbol) : Strategy(name, symbol) {}
    void Init() override {}
    void OnOpen(const MarketData *data) override
    {
        std::cout << "[BuyAndHoldStrategy::OnOpen]" << std::endl;
        if (!(GetOrderManager()->LookUp(data->symbol_)))
        {
            (GetOrderManager()->NewOrder(data->symbol_, true, data->price_, 100));
        }
    }
    void OnChange(const MarketData *) override
    {
        std::cout << "[BuyAndHoldStrategy::OnChange]" << std::endl;
    }
    void OnHalt(const MarketData *) override
    {
        std::cout << "[BuyAndHoldStrategy::OnHalt]" << std::endl;
    }
    void OnTrade(const MarketData *) override
    {
        std::cout << "[BuyAndHoldStrategy::OnTrade]" << std::endl;
    }
    void OnCancel(const MarketData *) override
    {
        std::cout << "[BuyAndHoldStrategy::OnCancel]" << std::endl;
    }
    void OnExit() override
    {
        std::cout << "[BuyAndHoldStrategy::OnExit]" << std::endl;
    }
};

int main(int argc, char *argv[])
{

    Config config;
    config.engine_config_.mc_addr_str_ = "239.255.10.10";
    config.engine_config_.mc_port_ = 4321;
    config.engine_config_.thread_num_ = 2;

    config.engine_config_.broker_configs_ = std::unordered_map<std::string, Config::BrokerConfig>{
        {"SCH", Config::BrokerConfig{"test.exchange.SCH", "127.0.0.1", 8000}},
        {"SCS", Config::BrokerConfig{"test.exchange.SCS", "127.0.0.1", 8001}},
    };
    BuyAndHoldStrategy *sch = new BuyAndHoldStrategy("bnh-sch", "SCH");
    BuyAndHoldStrategy *scs = new BuyAndHoldStrategy("bnh-scs", "SCS");
    Engine *engine = new Engine(config);
    engine->RegisterStrategy(sch);
    engine->RegisterStrategy(scs);
    engine->Init();
    engine->Run();
    delete engine;
    delete sch;
    delete scs;
    return 0;
}