#include "World.h"
#include"MyOperator.h"
#include "Organism.h"
#include "Environment.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "Animals.h"

World::World(Config &conf) : conf(conf)
{
    // ---- 植物: 30株分散在草地各处（生产者基础） ----
    auto addPlant = [&](int x, int y) {
        Reproducas.push_back(new Plant(Plant_id++, x, y, conf.Plant_init_radius,
            conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    };
    // 左下草地集群
    addPlant(5,5); addPlant(7,4); addPlant(4,7); addPlant(8,6); addPlant(6,8);
    // 中部集群
    addPlant(20,25); addPlant(22,23); addPlant(19,26); addPlant(23,27); addPlant(21,28);
    addPlant(24,24); addPlant(18,22);
    // 右上集群
    addPlant(40,40); addPlant(42,38); addPlant(39,42); addPlant(43,41); addPlant(41,43);
    addPlant(38,39);
    // 左上集群
    addPlant(8,40); addPlant(10,38); addPlant(7,42); addPlant(11,41); addPlant(9,43);
    // 右下集群
    addPlant(42,8); addPlant(44,6); addPlant(40,9); addPlant(43,10); addPlant(41,7);
    // 中央散落
    addPlant(25,10); addPlant(30,35); addPlant(15,30);

    // ---- 绵羊: 8只, 分2群（初级消费者） ----
    auto addSheep = [&](int x, int y) {
        Reproducas.push_back(MyOperator()(x, y, 3, "Sheep", Animal_id++));
    };
    addSheep(20, 23); addSheep(22, 24); addSheep(21, 26); addSheep(23, 25);  // 中部草场
    addSheep(40, 38); addSheep(42, 39); addSheep(41, 41); addSheep(39, 40);  // 右上草场

    // ---- 狼: 3只, 分散巡逻（顶级捕食者） ----
    auto addWolf = [&](int x, int y) {
        Reproducas.push_back(MyOperator()(x, y, 3, "Wolf", Animal_id++));
    };
    addWolf(25, 25);  // 中部, 靠近一群羊
    addWolf(10, 10);  // 左下, 远离羊群
    addWolf(44, 42);  // 右上, 靠近另一群羊

    // ---- 环境: 100x100 草地, 初始能量有自然波动 ----
    for (int i = 0; i < conf.length; i++)
    {
        for (int j = 0; j < conf.width; j++)
        {
            float distX = (i - 50) * (i - 50);
            float distY = (j - 50) * (j - 50);
            float dist = std::sqrt(distX + distY);
            float initEnergy = 3.0f + 10.0f * std::exp(-dist / 15.0f);
            Environments.push_back(new GressLand(std::make_pair(i, j), initEnergy, 2));
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
    for (auto &env : Environments) env->Organisms.clear();
    RemoveDeadOrganisms();
    for (auto &i : Environments)
    {
        if (i->energy < 0)
        {
            printf("error %d %d %f\n", i->Pos.first, i->Pos.second, i->energy);
        }
        i->Update(CurrentWeather);
    }
    for (auto &i : Reproducas)
    {
        Environments[i->Pos.second * GetWidth() + i->Pos.first]->Organisms.push_back(i);
        Environments[i->Pos.second * GetWidth() + i->Pos.first]->EnergyExchange(i);
        i->Step();
    }

    for(int x=0;x<GetWidth();x++)
    for(int y=0;y<GetHeight();y++)
    {
        Environment* e=Environments[y*GetWidth()+x];
        for(auto i:e->Organisms)
        for(int dy=-1;dy<=1;dy++)
        for(int dx=-1;dx<=1;dx++){
            int nx=x+dx;
            int ny=y+dy;
            if(ny<0||ny>=GetHeight()||nx<0||nx>=GetWidth()) continue;
            Environment* n=Environments[ny*GetWidth()+nx];
            for(auto j:n->Organisms){
                if(i<j){
                    PredationOrFuck(i,j);
                }
            }
        }
    }

    World::Reproduce();
}
void World::Reproduce()
{
    for (auto &request : reproduce_requests)
    {
        auto *org = ReprodueNewOrganism(request);
        if (org)
            Reproducas.push_back(org);
    }
    reproduce_requests.clear();
}

bool World::AddReproduceRequest(const ReproduceRequest &request)
{
    if (Environments[request.pos.second * GetWidth() + request.pos.first]->canPlant(request))
    {
        reproduce_requests.push_back(request);
        return true;
    }
    return false;
}

void World::RemoveDeadOrganisms()
{
    Reproducas.erase(std::remove_if(Reproducas.begin(), Reproducas.end(), [&](Reproducable *organism)
                                    {
                                        if (organism == nullptr) return true;
                                        if (!(organism->active))
                                        {
                                            Environments[organism->Pos.second * GetWidth() + organism->Pos.first]->getDeadOrgnismEnergy(organism->energy);
                                            if (organism->type == PLANT)
                                                Environments[organism->Pos.second * GetWidth() + organism->Pos.first]->havePlant--;
                                            delete organism;
                                            return true;
                                        }
                                        else
                                        {
                                            return false;
                                        } }),
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

World &World::GetWorld()
{
    static Config conf(100, 100);
    static World Instance(conf);
    return Instance;
}

void World::Reset()
{
    for (auto *org : Reproducas)
        delete org;
    for (auto *env : Environments)
        delete env;
    Reproducas.clear();
    Environments.clear();
    reproduce_requests.clear();
    Plant_id = 0;
    Animal_id = 0;

    auto addPlant = [&](int x, int y) {
        Reproducas.push_back(new Plant(Plant_id++, x, y, conf.Plant_init_radius,
            conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    };
    addPlant(5,5); addPlant(7,4); addPlant(4,7); addPlant(8,6); addPlant(6,8);
    addPlant(20,25); addPlant(22,23); addPlant(19,26); addPlant(23,27); addPlant(21,28);
    addPlant(24,24); addPlant(18,22);
    addPlant(40,40); addPlant(42,38); addPlant(39,42); addPlant(43,41); addPlant(41,43);
    addPlant(38,39);
    addPlant(8,40); addPlant(10,38); addPlant(7,42); addPlant(11,41); addPlant(9,43);
    addPlant(42,8); addPlant(44,6); addPlant(40,9); addPlant(43,10); addPlant(41,7);
    addPlant(25,10); addPlant(30,35); addPlant(15,30);

    auto addSheep = [&](int x, int y) {
        Reproducas.push_back(MyOperator()(x, y, 3, "Sheep", Animal_id++));
    };
    addSheep(20, 23); addSheep(22, 24); addSheep(21, 26); addSheep(23, 25);
    addSheep(40, 38); addSheep(42, 39); addSheep(41, 41); addSheep(39, 40);

    auto addWolf = [&](int x, int y) {
        Reproducas.push_back(MyOperator()(x, y, 3, "Wolf", Animal_id++));
    };
    addWolf(25, 25);
    addWolf(10, 10);
    addWolf(44, 42);

    for (int i = 0; i < conf.length; i++)
    {
        for (int j = 0; j < conf.width; j++)
        {
            float distX = (i - 50) * (i - 50);
            float distY = (j - 50) * (j - 50);
            float dist = std::sqrt(distX + distY);
            float initEnergy = 3.0f + 10.0f * std::exp(-dist / 15.0f);
            Environments.push_back(new GressLand(std::make_pair(i, j), initEnergy, 2));
        }
    }
}
