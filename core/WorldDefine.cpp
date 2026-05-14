#include "World.h"
#include<exception>
#include "MyOperator.h"
#include "Organism.h"
#include "Environment.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "Animals.h"
#include"Plants.h"
static World* TheOnlyWord;//世界指针单例

World::World(Config& Conf, TestConfig& Game_conf)
    :conf(Conf),game_conf(Game_conf)
{
    TheOnlyWord = this;
    int w = game_conf.The_Word.width;
    int h = game_conf.The_Word.length;
    int cx = w/2, cy = h/2;
    int lx = std::max(2, w/10), rx = std::min(w-3, w - w/10);
    int ty = std::max(2, h/10), by = std::min(h-3, h - h/10);

    // plants: 5 clusters spread across the map
    auto addPlant = [&](int x, int y) {
        Reproducas.push_back(MyOperator::GetOp()(x, y,conf.Plant_init_radius, "Gress", Plant_id++));
    };
    // center
    addPlant(cx, cy); addPlant(cx+2, cy-1); addPlant(cx-1, cy+2);
    addPlant(cx+3, cy+1); addPlant(cx-2, cy-2); addPlant(cx+1, cy-3);
    // bottom-left
    addPlant(lx, ty); addPlant(lx+2, ty); addPlant(lx, ty+2);
    addPlant(lx+3, ty+1); addPlant(lx+1, ty+3);
    // top-right
    addPlant(rx, by); addPlant(rx-2, by); addPlant(rx, by-2);
    addPlant(rx-3, by-1); addPlant(rx-1, by-3);
    // top-left
    addPlant(lx, by); addPlant(lx+2, by-1); addPlant(lx-1, by+1);
    addPlant(lx+3, by); addPlant(lx, by-3);
    // bottom-right
    addPlant(rx, ty); addPlant(rx-2, ty+1); addPlant(rx+1, ty-1);
    addPlant(rx, ty+3); addPlant(rx-1, ty-2);
    // scattered
    addPlant(cx+w/5, cy+h/5); addPlant(cx-w/5, cy-h/5); addPlant(lx+w/5, cy);

    // sheep: 4 groups of 4
    auto addSheep = [&](int x, int y) {
        Reproducas.push_back(MyOperator::GetOp()(x, y, 5, "Sheep", Animal_id++));
    };
    addSheep(cx+2, cy+3); addSheep(cx+4, cy);   addSheep(cx+1, cy+4); addSheep(cx+3, cy+1);
    addSheep(rx-3, by-3); addSheep(rx-5, by-1); addSheep(rx-2, by-5); addSheep(rx-4, by-2);
    addSheep(lx+3, ty+3); addSheep(lx+5, ty+1); addSheep(lx+2, ty+5); addSheep(lx+4, ty+2);
    addSheep(lx+3, by-3); addSheep(lx+5, by-1); addSheep(lx+2, by-5); addSheep(lx+4, by-2);

    // wolves: scattered
    auto addWolf = [&](int x, int y) {
        Reproducas.push_back(MyOperator::GetOp()(x, y, 5, "Wolf", Animal_id++));
    };
    addWolf(cx, cy);
    addWolf(lx+5, ty+5);
    addWolf(rx-5, by-5);

    // environment: energy peaks at center, falls off toward edges
    float falloff = std::max(w, h) / 3.0f;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            float distX = (i - cy) * (i - cy);
            float distY = (j - cx) * (j - cx);
            float dist = std::sqrt(distX + distY);
            float initEnergy = 3.0f + 15.0f * std::exp(-dist / falloff);
            Environments.push_back(new GressLand(std::make_pair(i, j), initEnergy));
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
            printf_s("error %d %d %f\n", i->Pos.first, i->Pos.second, i->energy);
        }
        i->Update(CurrentWeather);
    }
    for (auto &i : Reproducas)
    {
        Environments[i->Pos.second * GetWidth() + i->Pos.first]->Organisms.push_back(i);
        Environments[i->Pos.second * GetWidth() + i->Pos.first]->EnergyExchange(i);
        i->Step();
    }

    int R = conf.Organism_interact_radius;
    for(int x=0;x<GetWidth();x++)
    for(int y=0;y<GetHeight();y++)
    {
        Environment* e=Environments[y*GetWidth()+x];
        for(auto i:e->Organisms)
        for(int dy=-R;dy<=R;dy++)
        for(int dx=-R;dx<=R;dx++){
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
    last_requests = reproduce_requests;
    for (auto &request : reproduce_requests)
    {
        //printf("128 %s\n", request.name.data());
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
        if (x >= 0 && x < width && y >= 0 && y < len)
            cnt += (int)Environments[y * width + x]->Organisms.size();
        else
            cnt += conf.max_organisms_per_cell;
    }
    return (float)cnt / (8.0f * conf.max_organisms_per_cell);
}

World& World::GetWorld()
{
    if(!TheOnlyWord) throw std::runtime_error("World not exist,can't get it without config");
    return *(TheOnlyWord);
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
    last_requests.clear();
    Plant_id = 0;
    Animal_id = 0;

    int w = game_conf.The_Word.width;
    int h = game_conf.The_Word.length;
    int cx = w/2, cy = h/2;
    int lx = std::max(2, w/10), rx = std::min(w-3, w - w/10);
    int ty = std::max(2, h/10), by = std::min(h-3, h - h/10);

    auto addPlant = [&](int x, int y) {
        Reproducas.push_back(MyOperator::GetOp()(x, y, conf.Plant_init_radius, "Gress", Plant_id++));
    };
    addPlant(cx, cy); addPlant(cx+2, cy-1); addPlant(cx-1, cy+2);
    addPlant(cx+3, cy+1); addPlant(cx-2, cy-2); addPlant(cx+1, cy-3);
    addPlant(lx, ty); addPlant(lx+2, ty); addPlant(lx, ty+2);
    addPlant(lx+3, ty+1); addPlant(lx+1, ty+3);
    addPlant(rx, by); addPlant(rx-2, by); addPlant(rx, by-2);
    addPlant(rx-3, by-1); addPlant(rx-1, by-3);
    addPlant(lx, by); addPlant(lx+2, by-1); addPlant(lx-1, by+1);
    addPlant(lx+3, by); addPlant(lx, by-3);
    addPlant(rx, ty); addPlant(rx-2, ty+1); addPlant(rx+1, ty-1);
    addPlant(rx, ty+3); addPlant(rx-1, ty-2);
    addPlant(cx+w/5, cy+h/5); addPlant(cx-w/5, cy-h/5); addPlant(lx+w/5, cy);

    auto addSheep = [&](int x, int y) {
        Reproducas.push_back(MyOperator()(x, y, 5, "Sheep", Animal_id++));
    };
    addSheep(cx+2, cy+3); addSheep(cx+4, cy);   addSheep(cx+1, cy+4); addSheep(cx+3, cy+1);
    addSheep(rx-3, by-3); addSheep(rx-5, by-1); addSheep(rx-2, by-5); addSheep(rx-4, by-2);
    addSheep(lx+3, ty+3); addSheep(lx+5, ty+1); addSheep(lx+2, ty+5); addSheep(lx+4, ty+2);
    addSheep(lx+3, by-3); addSheep(lx+5, by-1); addSheep(lx+2, by-5); addSheep(lx+4, by-2);

    auto addWolf = [&](int x, int y) {
        Reproducas.push_back(MyOperator()(x, y, 5, "Wolf", Animal_id++));
    };
    addWolf(cx, cy);
    addWolf(lx+5, ty+5);
    addWolf(rx-5, by-5);

    float falloff = std::max(w, h) / 3.0f;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            float distX = (i - cy) * (i - cy);
            float distY = (j - cx) * (j - cx);
            float dist = std::sqrt(distX + distY);
            float initEnergy = 3.0f + 15.0f * std::exp(-dist / falloff);
            Environments.push_back(new GressLand(std::make_pair(i, j), initEnergy));
        }
    }
}

World& World::GetWorld(TestConfig& Game_conf) {
    static World The_World(Config::GetConfig(), Game_conf);
    return The_World;
}