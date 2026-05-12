#include "Word.h"
#include"MyOperator.h"
#include "Organism.h"
#include "Environment.h"
#include <algorithm>
#include <cstdio>
#include"Animals.h"

World::World(Config &conf) : conf(conf)
{
    Reproducas.push_back(new Plant(Plant_id++, 5, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 10, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 7, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 6, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 4, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(MyOperator()(10, 13, 3, "Wolf", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep",Animal_id++));
    Reproducas.push_back(MyOperator()(10, 22, 3, "Sheep",Animal_id++));
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
    RemoveDeadOrganisms();

    for (auto &i : Reproducas)
    {
        i->Step();
    }

    for (auto &i : Environments)
    {
        if (i->energy < 0)
        {
            printf("error %d %d %f\n", i->Pos.first, i->Pos.second, i->energy);
        }
        i->Update(CurrentWeather);
        if (i->energy < 0)
        {
            printf("FUCK");
        }
    }

    std::sort(Reproducas.begin(), Reproducas.end(), [](Reproducable *a, Reproducable *b)
              {
        if (a->Pos.first != b->Pos.first)
            return a->Pos.first < b->Pos.first;
        return a->Pos.second < b->Pos.second; });

    for (auto i : Reproducas)
    {
        if(i->energy < -10){
            printf("FUCK");
        }
        Environments[i->Pos.second * GetWidth() + i->Pos.first]->EnergyExchange(i);
    }

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

    World::Reproduce();
}
void World::Reproduce()
{
    for (auto &request : reproduce_requests)
    {
        auto* org = ReprodueNewOrganism(request);
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
                                        if (!(organism->energy > 0))
                                        {
                                            if (organism->type == PLANT)
                                                Environments[organism->Pos.second * GetWidth() + organism->Pos.first]->havePlant--;
                                            delete organism;
                                            return true;
                                        }
                                        else
                                        {
                                            return false;
                                        }
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

World &World::GetWorld()
{
    static Config conf(50, 50);
    static World Instance(conf);
    return Instance;
}

void World::Reset()
{
    for (auto* org : Reproducas) delete org;
    for (auto* env : Environments) delete env;
    Reproducas.clear();
    Environments.clear();
    reproduce_requests.clear();
    Plant_id = 0;
    Animal_id = 0;

    Reproducas.push_back(new Plant(Plant_id++, 5, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 10, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 7, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 6, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(new Plant(Plant_id++, 4, 5, conf.Plant_init_radius, conf.Organism_reproduce_energy_threshold, conf.Organism_reproduce_energy_cost, conf.Organism_step_energy_cost));
    Reproducas.push_back(MyOperator()(10, 13, 3, "Wolf", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 26, 3, "Sheep", Animal_id++));
    Reproducas.push_back(MyOperator()(10, 22, 3, "Sheep", Animal_id++));
    for (int i = 0; i < conf.length; i++)
    {
        for (int j = 0; j < conf.width; j++)
        {
            Environments.push_back(new GressLand(std::make_pair(i, j), 2, 2));
        }
    }
}
