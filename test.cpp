#include <cstdlib>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#define private public
#include "main.h"
#undef private

namespace
{
constexpr int kMapWidth = 100;
constexpr int kMapHeight = 100;
constexpr int kPlantCount = 30;
constexpr int kPlantRadius = 5;
constexpr int kMaxRegisteredPlants = 10000;
constexpr int kTicks = 400;
constexpr int kFrameDelayMs = 0;

std::vector<std::unique_ptr<Plant>> g_plants;

struct UpdateResult
{
    int newborn_count = 0;
};

int randomCoord(int limit)
{
    return std::rand() % limit;
}

std::vector<Plant *> collectPlants()
{
    std::vector<Plant *> plants;
    for (Reproducable *reproduceable : ReproduceManager::GetInstance().reproduceables)
    {
        Plant *plant = dynamic_cast<Plant *>(reproduceable);
        if (plant)
        {
            plants.push_back(plant);
        }
    }

    return plants;
}

UpdateResult updateWorld()
{
    ReproduceManager &manager = ReproduceManager::GetInstance();
    const size_t before_count = manager.reproduceables.size();

    // Use a fixed-size snapshot so newly born plants join the next tick safely.
    for (size_t i = 0; i < before_count; ++i)
    {
        Reproducable *reproduceable = manager.reproduceables[i];
        reproduceable->energy += reproduceable->step_energy_gain;

        if (reproduceable->reproduce_able &&
            reproduceable->energy >= reproduceable->reproduce_energy_threshold)
        {
            reproduceable->energy -= reproduceable->reproduce_energy_cost;
            reproduceable->Reproduce();
        }
    }

    return {static_cast<int>(manager.reproduceables.size() - before_count)};
}
}

const std::vector<std::unique_ptr<Plant>> &scatterPlant()
{
    static bool generated = false;
    if (generated)
    {
        return g_plants;
    }

    generated = true;
    ReproduceManager::GetInstance().reproduceables.reserve(kMaxRegisteredPlants);
    g_plants.reserve(kPlantCount);
    for (int i = 0; i < kPlantCount; ++i)
    {
        const int x = randomCoord(kMapWidth);
        const int y = randomCoord(kMapHeight);
        g_plants.push_back(std::make_unique<Plant>(i, x, y, kPlantRadius));
    }

    return g_plants;
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

int main()
{
    std::srand(1);

    scatterPlant();
    scatterPlant(); // No-op after the first call.

    drawMap(collectPlants(), 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMs));

    for (int tick = 1; tick <= kTicks; ++tick)
    {
        const UpdateResult result = updateWorld();
        drawMap(collectPlants(), result.newborn_count, tick);
        std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMs));
    }

    return 0;
}

