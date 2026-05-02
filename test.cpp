#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "main.h"

namespace
{
constexpr int kMapWidth = 100;
constexpr int kMapHeight = 100;
constexpr int kPlantCount = 30;
constexpr int kPlantRadius = 5;
constexpr int kMaxRegisteredPlants = 10000;
constexpr int kTicks = 800;
constexpr int kFrameDelayMs = 120;

struct WorldAccess
{
    std::vector<ReproduceRequest> reproduce_requests;
    std::vector<Plant *> plants;
    std::vector<Prey *> preys;
    std::vector<Predator *> predators;
};

struct UpdateResult
{
    int newborn_count = 0;
    int death_count = 0;
};

int randomCoord(int limit)
{
    return std::rand() % limit;
}

WorldAccess &worldAccess()
{
    return reinterpret_cast<WorldAccess &>(World::GetWorld());
}

const std::vector<Plant *> &collectPlants()
{
    return worldAccess().plants;
}

void addPlantReproduceRequest(Plant &plant)
{
    const int x = plant.Pos.first;
    const int y = plant.Pos.second;
    const int x_new = x + std::rand() % (2 * plant.reproduce_radius + 1) - plant.reproduce_radius;
    const int y_new = y + std::rand() % (2 * plant.reproduce_radius + 1) - plant.reproduce_radius;
    if (x_new >= 0 && x_new < kMapWidth && y_new >= 0 && y_new < kMapHeight)
    {
        const int r = plant.reproduce_radius * std::min(2.0, std::max(0.25, (double)std::rand() / RAND_MAX));
        World::GetWorld().AddReproduceRequest({PLANT, std::make_pair(x_new, y_new), r});
    }
}

UpdateResult updateWorld(bool stats_only)
{
    World &world = World::GetWorld();
    std::vector<Plant *> &plants = worldAccess().plants;
    const size_t before_count = plants.size();

    // Use a fixed-size snapshot so newly born plants join the next tick safely.
    for (size_t i = 0; i < before_count; ++i)
    {
        Plant *plant = plants[i];
        if (!plant->active)
        {
            continue;
        }

        static_cast<Organism &>(*plant).Step();
        if (!stats_only)
        {
            plant->Step();
        }

        if (plants.size() + worldAccess().reproduce_requests.size() >= kMaxRegisteredPlants)
        {
            continue;
        }

        if (plant->reproduce_able &&
            plant->energy >= plant->reproduce_energy_threshold)
        {
            plant->energy -= plant->reproduce_energy_cost;
            if (stats_only)
            {
                addPlantReproduceRequest(*plant);
            }
            else
            {
                plant->Reproduce();
            }
        }
    }

    const size_t newborn_count = worldAccess().reproduce_requests.size();
    world.Reproduce();
    world.RemoveDeadOrganisms();

    const int death_count = static_cast<int>(before_count + newborn_count - plants.size());
    return {static_cast<int>(newborn_count), death_count};
}
}

void scatterPlant()
{
    static bool generated = false;
    if (generated)
    {
        return;
    }

    generated = true;
    World &world = World::GetWorld();
    worldAccess().plants.reserve(kMaxRegisteredPlants);
    for (int i = 0; i < kPlantCount; ++i)
    {
        const int x = randomCoord(kMapWidth);
        const int y = randomCoord(kMapHeight);
        world.AddReproduceRequest({PLANT, std::make_pair(x, y), kPlantRadius});
    }

    world.Reproduce();
}

void drawMap(const std::vector<Plant *> &plants, int newborn_count, int tick)
{
    std::vector<std::string> map(kMapHeight, std::string(kMapWidth, '.'));

    const size_t newborn_start = newborn_count <= static_cast<int>(plants.size())
                                     ? plants.size() - newborn_count
                                     : plants.size();

    for (size_t i = 0; i < plants.size(); ++i)
    {
        const Plant *plant = plants[i];
        const int x = plant->Pos.first;
        const int y = plant->Pos.second;
        if (x < 0 || x >= kMapWidth || y < 0 || y >= kMapHeight)
        {
            continue;
        }

        const char mark = i >= newborn_start ? 'N' : 'P';
        map[y][x] = map[y][x] == '.' ? mark : '*';
    }

    std::cout << "\x1B[2J\x1B[H";
    std::cout << "Tick: " << tick
              << " | Plants: " << plants.size()
              << " | Newborns: " << newborn_count << '\n';
    std::cout << "Legend: . empty, P plant, N newborn, * overlap\n";

    for (const std::string &row : map)
    {
        std::cout << row << '\n';
    }
}

int main(int argc, char *argv[])
{
    bool stats_only = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--stats-only" || arg == "--counts-only")
        {
            stats_only = true;
        }
    }

    std::srand(1);

    scatterPlant();
    scatterPlant(); // No-op after the first call.

    int cumulative_births = static_cast<int>(collectPlants().size());
    int cumulative_deaths = 0;

    if (stats_only)
    {
        std::cout << "Tick: 0 | Total: " << collectPlants().size()
                  << " | Births: " << cumulative_births
                  << " | Deaths: " << cumulative_deaths << '\n';
    }
    else
    {
        drawMap(collectPlants(), 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMs));
    }

    for (int tick = 1; tick <= kTicks; ++tick)
    {
        const UpdateResult result = updateWorld(stats_only);
        cumulative_births += result.newborn_count;
        cumulative_deaths += result.death_count;

        if (stats_only)
        {
            std::cout << "Tick: " << tick
                      << " | Total: " << collectPlants().size()
                      << " | Births: " << cumulative_births
                      << " | Deaths: " << cumulative_deaths << '\n';
        }
        else
        {
            drawMap(collectPlants(), result.newborn_count, tick);
            std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMs));
        }
    }

    return 0;
}
