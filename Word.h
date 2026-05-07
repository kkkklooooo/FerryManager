#pragma once
#include"Registry.h"
#include"Organism.h"
#include"Environment.h"
#include"Config.h"
class World
{
    // 待处理的繁殖请求列表
    std::vector<ReproduceRequest> reproduce_requests;
    // 所有可繁殖对象指针列表
    std::vector<Reproducable*> Reproducas;
    //所有环境对象
    std::vector<Environment*>Environments;

public:
    Config conf;
    Weather CurrentWeather;
    World(Config&conf);
    void AddLeftEnergyRequest(const LeftEnergyRequest& request);
    // 更新世界状态（例如环境变化等）
    void Update();
    // 处理所有繁殖请求，生成新生物
    void Reproduce();
    // 向世界添加一个繁殖请求
    bool AddReproduceRequest(const ReproduceRequest& request);
    // 移除所有能量为0（active == false）的生物
    void RemoveDeadOrganisms();
    float calculate_overlay(std::pair<int, int> pos);
    // 获取世界单例实例
    static World& GetWorld();
    //只读测试 不是真正的代码
    const std::vector<Reproducable*>& GetReproducas() const { return Reproducas; }
    const std::vector<Environment*>& GetEnvironments() const { return Environments; }
    int GetWidth() const { return conf.width; }
    int GetHeight() const { return conf.length; }
};
