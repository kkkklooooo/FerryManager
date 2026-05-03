#include "main.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

// 全局唯一标识，用于给新生植物分配ID
int id = 0;

// 以下为被注释掉的 ReproduceManager 单例实现，暂未使用
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

/**
 * @brief 更新可繁殖对象的状态，若满足繁殖条件则执行繁殖
 * @param reproduceable 可繁殖对象引用
 */
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

/**
 * @brief 生物基类构造函数
 * @param step_energy_cost 每步消耗的能量
 * @param step_energy_gain  每步获得的能量
 * @param type              生物类型（植物、被捕食者、捕食者等）
 */
Organism::Organism(float step_energy_cost, float step_energy_gain, OrganismType type)
    : step_energy_cost(step_energy_cost), step_energy_gain(step_energy_gain), type(type)
{
}

/**
 * @brief 检查能量值，若能量<=0则标记为不活跃（死亡）
 */
void Organism::check_energy()
{
    if (energy <= 0)
    {
        active = false;
    }
}

/**
 * @brief 执行一步：增加能量（例如光合作用），消耗能量（新陈代谢），然后检查生死
 */
void Organism::Step()
{
    energy += step_energy_gain;
    energy -= step_energy_cost;
    check_energy();
}

/**
 * @brief 可繁殖生物构造函数
 * @param energy_threshold  繁殖所需的能量阈值
 * @param energy_cost       繁殖消耗的能量
 * @param radius            繁殖时子代可散布的半径范围
 * @param sec               每步能量消耗
 * @param seg               每步能量增益
 * @param type              生物类型
 */
Reproducable::Reproducable(float energy_threshold, float energy_cost, int radius, float sec, float seg, OrganismType type)
    : Organism(sec, seg, type)
{
    reproduce_energy_threshold = energy_threshold;
    reproduce_energy_cost = energy_cost;
    reproduce_radius = radius;
    reproduce_able = false;          // 默认不可繁殖，由派生类自行设置
}

/**
 * @brief 繁殖行为的默认实现（派生类应覆盖）
 */
void Reproducable::Reproduce()
{
    std::printf("Reproduce is not implemented");
}

/**
 * @brief 植物类构造函数
 * @param id     植物唯一ID
 * @param x      X坐标
 * @param y      Y坐标
 * @param radius 繁殖影响半径
 */
Plant::Plant(int id, int x, int y, int radius)
    : Reproducable(0.5, 0.1, radius, 0.01, 0.02, PLANT)
{
    this->id = id;
    Pos = std::make_pair(x, y);
    reproduce_able = true;            // 植物始终可以繁殖（只要能量足够）
}

/**
 * @brief 植物的繁殖行为：在半径范围内随机生成一个新位置，并向世界请求添加新植物
 */
void Plant::Reproduce()
{
    int x = Pos.first;
    int y = Pos.second;
    // 在 [-reproduce_radius, +reproduce_radius] 范围内随机偏移
    int x_new = x + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    int y_new = y + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    // 确保新位置在有效世界边界内（假设世界大小为100x100）
    if (x_new >= 0 && x_new < 100 && y_new >= 0 && y_new < 100)
    {
        // 子代植物的半径在父半径的[0.25,2.0]倍之间随机，并取整
        int r = reproduce_radius * std::min(2.0, std::max(0.25, (double)std::rand() / RAND_MAX));
        World::GetWorld().AddReproduceRequest({PLANT, std::make_pair(x_new, y_new), r});
        std::printf("Plant request at (%d, %d)\n", x_new, y_new);
    }
}

// 原有的被注释掉的植物能量检查函数，与基类行为重复
// void Plant::check_energy()
// {
//     if (energy <= 0)
//     {
//         active = false;
//     }
// }

/**
 * @brief 植物的一步更新（此处仅占位，具体逻辑可后续添加）
 */
void Plant::Step()
{
    // std::printf("Plant %d is at (%d, %d) with energy %f\n", id, Pos.first, Pos.second, energy);
}

/**
 * @brief 世界类的更新函数（当前为空，可添加环境变化等逻辑）
 */
void World::Update()
{
}

/**
 * @brief 处理所有繁殖请求，根据请求类型生成对应的生物对象，并加入到相应容器
 */
void World::Reproduce()
{
    for (auto &request : reproduce_requests)
    {
        switch (request.type)
        {
        case PLANT:
            plants.push_back(new Plant(id++, request.pos.first, request.pos.second, request.radius));
            break;
        // TODO: 后续可添加被捕食者和捕食者的繁殖支持
        default:
            break;
        }
    }
    reproduce_requests.clear();   // 清空已处理的请求
}

/**
 * @brief 向世界添加一个繁殖请求
 * @param request 繁殖请求结构体
 */
void World::AddReproduceRequest(const ReproduceRequest &request)
{
    reproduce_requests.push_back(request);
}

/**
 * @brief 移除所有不活跃（死亡）的生物
 */
void World::RemoveDeadOrganisms()
{
    // 移除死亡植物
    plants.erase(std::remove_if(plants.begin(), plants.end(), [](Plant *plant) { return !plant->active; }), plants.end());
    // TODO: 同样需要处理被捕食者和捕食者
    // predators.erase(std::remove_if(predators.begin(), predators.end(), [](Predator *predator) { return !predator->active; }), predators.end());
    // preys.erase(std::remove_if(preys.begin(), preys.end(), [](Prey *prey) { return !prey->active; }), preys.end());
}

/**
 * @brief 获取世界单例实例
 * @return World& 世界对象的引用
 */
World &World::GetWorld()
{
    static World Instance;
    return Instance;
}

/**
 * @brief 生物管理器的更新函数：调用生物的Step方法
 * @param organism 要更新的生物对象
 */
void OrganismManager::Update(Organism &organism)
{
    organism.Step();
}