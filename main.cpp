#include "main.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
//
int id = 0;
// ReproduceManager *ReproduceManager::Instance = nullptr;

// ReproduceManager &ReproduceManager::GetInstance()
// {
//     if (!Instance)
//     {
//         Instance = new ReproduceManager();
//     }

//     return *Instance;
// }

// void ReproduceManager::RegisterReproducable(Reproducable *reproduceable)
// {
//     reproduceables.push_back(reproduceable);
// }

void ReproduceManager::Update(Reproducable &reproduceable)
{

    float energy = reproduceable.energy;
    float threshold = reproduceable.reproduce_energy_threshold;
    if (reproduceable.reproduce_able && energy >= threshold)
    {
        reproduceable.energy -= reproduceable.reproduce_energy_cost;
        reproduceable.Reproduce();
    }
}

Organism::Organism(float step_energy_cost, float step_energy_gain, OrganismType type)
    : step_energy_cost(step_energy_cost), step_energy_gain(step_energy_gain), type(type)
{
}

void Organism::check_energy()
{
    if (energy <= 0)
    {
        active = false;
    }
}

void Organism::Step()
{
    energy += step_energy_gain;
    energy -= step_energy_cost;
    check_energy();
}

Reproducable::Reproducable(float energy_threshold, float energy_cost, int radius, float sec, float seg, OrganismType type)
    : Organism(sec, seg, type)
{
    reproduce_energy_threshold = energy_threshold;
    reproduce_energy_cost = energy_cost;
    reproduce_radius = radius;
    reproduce_able = false;
}

void Reproducable::Reproduce()
{
    std::printf("Reproduce is not implemented");
}

Plant::Plant(int id, int x, int y, int radius)
    : Reproducable(0.5, 0.1, radius, 0.01, 0.02, PLANT)
{
    this->id = id;
    Pos = std::make_pair(x, y);
    reproduce_able = true;
}

void Plant::Reproduce()
{
    int x = Pos.first;
    int y = Pos.second;
    int x_new = x + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    int y_new = y + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    if (x_new >= 0 && x_new < 100 && y_new >= 0 && y_new < 100)
    {
        int r = reproduce_radius * std::min(2.0, std::max(0.25, (double)std::rand() / RAND_MAX));
        World::GetWorld().AddReproduceRequest({PLANT, std::make_pair(x_new, y_new), r});
        std::printf("Plant request at (%d, %d)\n", x_new, y_new);
    }
}

// void Plant::check_energy()
// {
//     if (energy <= 0)
//     {
//         active = false;
//     }
// }

void Plant::Step()
{
    
    // std::printf("Plant %d is at (%d, %d) with energy %f\n", id, Pos.first, Pos.second, energy);
}

void World::Update()
{

}

void World::Reproduce()
{
    for (auto &request : reproduce_requests)
    {
        switch (request.type)
        {
        case PLANT:
            plants.push_back(new Plant(id++, request.pos.first, request.pos.second, request.radius));
            break;
        // TODO
        default:
            break;
        }
    }
    reproduce_requests.clear();
}

void World::AddReproduceRequest(const ReproduceRequest &request)
{
    reproduce_requests.push_back(request);
}

void World::RemoveDeadOrganisms()
{
    plants.erase(std::remove_if(plants.begin(), plants.end(), [](Plant *plant) { return !plant->active; }), plants.end());
    //TODO not implemented
    // predators.erase(std::remove_if(predators.begin(), predators.end(), [](Predator *predator) { return !predator->active; }), predators.end());
    // preys.erase(std::remove_if(preys.begin(), preys.end(), [](Prey *prey) { return !prey->active; }), preys.end());
}

World &World::GetWorld()
{
    static World Instance;
    return Instance;
}

void OrganismManager::Update(Organism &organism)
{
   organism.Step(); 
}
