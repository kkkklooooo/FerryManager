#include "Word.h"
#include "Organism.h"
#include "Environment.h"
#include <algorithm>
#include <cstdio>

World::World(Config &conf) : conf(conf)
{
    Reproducas.push_back(new Plant(Plant_id++, 5, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 4, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    for (int i = 0; i < conf.length; i++)
    {
        for (int j = 0; j < conf.width; j++)
        {
            Environments.push_back(new GressLand(std::make_pair(i, j), 2, 2));
        }
    }
}

void World::AddLeftEnergyRequest(const LeftEnergyRequest &request)
{
    if (request.energy > 0)
        Environments[request.pos.second * GetWidth() + request.pos.first]->deadOrganismEnergy += request.energy;
}

void World::Update()
{
    // 杀死死了的东西
    RemoveDeadOrganisms();

    // 先移动
    for (auto &i : Reproducas)
    {
        i->Step();
    }

    // 环境受天气的影响
    for (auto &i : Environments)
    {
        if (i->energy < 0)
        {
            printf("环境%d %d %f\n", i->Pos.first, i->Pos.second, i->energy);
        }
        i->Update(CurrentWeather);
        if (i->energy < 0)
        {
            printf("FUCK");
        }
    }
    // 排序
    std::sort(Reproducas.begin(), Reproducas.end(), [](Reproducable *a, Reproducable *b)
              {
        if (a->Pos.first != b->Pos.first)
            return a->Pos.first < b->Pos.first;
        return a->Pos.second < b->Pos.second; });

    // 动植物和环境交互
    
    for (auto i : Reproducas)
    {
        if(i->energy < -10){
            printf("FUCK");
        }
        Environments[i->Pos.second * GetWidth() + i->Pos.first]->EnergyExchange(i);
    }
    
    // 捕食和生孩子
    for (auto i = Reproducas.begin(); i != Reproducas.end(); i++)
    {
        for (auto j = i + 0; j != Reproducas.end(); j++)
        {
            if (isNaber(*i, *j))
            {
                PredationOrFuck(*i, *j);
            }
        }
    }
    // 繁衍
    World::Reproduce();
}
void World::Reproduce()
{
    for (auto &request : reproduce_requests)
    {
        Reproducas.push_back(ReprodueNewOrganism(request)); // 调用工厂函数
    }
    reproduce_requests.clear(); // 清空已处理的请求
}

/**
 * @brief 向世界添加一个繁殖请求
 * @param request 繁殖请求结构体
 */
bool World::AddReproduceRequest(const ReproduceRequest &request)
{
    // 能生
    if (Environments[request.pos.first * GetWidth() + request.pos.second]->canPlant(request))
    {
        reproduce_requests.push_back(request);
        return true;
    }
    return false;
}

/**
 * @brief 移除所有能量为0的生物
 */
void World::RemoveDeadOrganisms()
{
    // 移除能量为0的生物
    Reproducas.erase(std::remove_if(Reproducas.begin(), Reproducas.end(), [&](Reproducable *organism)
                                    {
                                        if (!(organism->energy > 0))
                                        {
                                            Environments[organism->Pos.second * GetWidth() + organism->Pos.first]->havePlant--;
                                            return true;
                                        }
                                        else
                                        {
                                            return false;
                                        }
                                        // return !(organism->energy>0);
                                    }),
                     Reproducas.end());
}

float World::calculate_overlay(std::pair<int, int> pos)
{
    std::vector<std::pair<int, int>> pos_list = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, 1}, {1, -1}, {-1, 1}};
    int cnt = 0;
    int len = GetHeight();
    int width = GetWidth();
    for (auto i : pos_list)
    {
        int x = pos.first + i.first;
        int y = pos.second + i.second;
        if (x >= 0 && x < len && y >= 0 && y < width)
        {
            cnt += Environments[y * width + x]->havePlant;
        }
        else
        {
            cnt += Environments[pos.second * width + pos.first]->maxPlant;
        }
    }
    return (float)cnt / (8.0 * Environments[pos.second * width + pos.first]->maxPlant);
}

/**
 * @brief 获取世界单例实例
 * @return World& 世界对象的引用
 */
World &World::GetWorld()
{
    static Config conf(50, 50);
    static World Instance(conf);
    return Instance;
}
