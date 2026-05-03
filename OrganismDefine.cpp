#include "Organism.h"
#include "Word.h"
#include"Environment.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

// 全局唯一标识，用于给新生植物分配ID
int id = 0;

//工厂函数 根据request返回对应指针
Reproducable* ReprodueNewOrganism(ReproduceRequest request) {
    if (request.type == PLANT) {
        return new Plant(id++, request.pos.first, request.pos.second, request.radius);
    }
    else//TODO 动物和资源的实现
    {
        return nullptr;
    }
}



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
 * @brief 生物基类构造函数
 * @param step_energy_cost 每步消耗的能量
 * @param step_energy_gain  每步获得的能量
 * @param type              生物类型（植物、被捕食者、捕食者等）
 */
Organism::Organism(float step_energy_cost, OrganismType type)
    : step_energy_cost(step_energy_cost), type(type)
{
}

/**
 * @brief 检查能量值，若能量<=0则标记为不活跃（死亡）
 */
void Organism::check_energy() //单独死亡
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
Reproducable::Reproducable(float energy_threshold, float energy_cost, int radius, float sec, OrganismType type)
    : Organism(sec, type)
{
    reproduce_energy_threshold = energy_threshold;
    reproduce_energy_cost = energy_cost;
    reproduce_radius = radius;
    reproduce_able = false;          // 默认不可繁殖，由派生类自行设置
}

/**
 * @brief 繁殖行为的默认实现 纯虚函数
 */

/**
 * @brief 植物类构造函数
 * @param id     植物唯一ID
 * @param x      X坐标
 * @param y      Y坐标
 * @param radius 繁殖影响半径
 */

Plant::Plant(int id, int x, int y, int radius)
    : Reproducable(0.5, 0.1, radius, 0.01, PLANT)
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
    if (!active) {//死了就不能活着
        return;
    }
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
        World::GetWorld().AddReproduceRequest({PLANT,Plant_Name, std::make_pair(x_new, y_new), r});
        std::printf("Plant request at (%d, %d)\n", x_new, y_new);
    }
}

void Plant::Step() 
{
    energy -= step_energy_cost;
}

bool isNaber(Organism* a, Organism* b) {//指针方便多态
    if (b->Pos.first >= a->Pos.first - 1
        && b->Pos.first <= a->Pos.first + 1
        && b->Pos.second >= a->Pos.second - 1
        && b->Pos.second <= a->Pos.second + 1) {
        return true;
    }
    return false;
}

void PredationOrFuck(Reproducable* a, Reproducable* b) {
    //怀孕判断
    if (a->name == b->name) {
        if (a->reproduce_able && !b->reproduce_able) {
            a->Reproduce();
        }
        if (b->reproduce_able && !a->reproduce_able) {
            b->Reproduce();
        }
        return;
    }
    //捕食判断 
    bool aEb=false;
    bool bEa=false;
    if (std::find(a->diet.begin(), a->diet.end(), b->name)!=a->diet.end()) {
        aEb=true;
    }
    if (std::find(b->diet.begin(), b->diet.end(), a->name)!=b->diet.end()) {
        bEa=true;
    }
    if (aEb && !bEa) {
        a->energy += b->energy * AnimalAbsorbRate*lossRate;
        b->energy-= b->energy * AnimalAbsorbRate;
        b->active=false;
        return;
    }
    if (bEa && !aEb) {
        b->energy += a->energy * AnimalAbsorbRate*lossRate;
        a->energy -= a->energy * AnimalAbsorbRate;
        a->active = false;
        return;
    }
    if (aEb && bEa) {
        if (a->energy >= b->energy) {
            b->active = false;
            return;
        }
        else {
            a->active = false;
            return;
        }
    }
    return;
}
