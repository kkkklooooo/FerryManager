#include "Organism.h"
#include <cassert>
#include "MyOperator.h"
#include <cmath>
#include "World.h"
#include "Environment.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include"Animals.h"
#include"Plants.h"
int Plant_id = 0;
int Animal_id = 0;
// 全局唯一标识，用于给新生植物和动物分配ID
std::random_device rd;
double rand01() { return rand() / (RAND_MAX + 1.0); }
std::mt19937 gen(rd()); // 随机旋转种子
const double pi = std::acos(-1.0);
std::uniform_real_distribution<double> dist(0.0, 2.0 * pi); // 范围

// 工厂函数 根据request返回对应指针 
Reproducable *ReprodueNewOrganism(ReproduceRequest request)
{
    if (request.type == PLANT)
    {
        return new UserPlant(Plant_id++, request.pos.first, request.pos.second, request.radius, World::GetWorld().conf.Organism_reproduce_energy_threshold, World::GetWorld().conf.Organism_reproduce_energy_cost, UserPlant::FindPlantConfig(request.name));
    }
    return new UserAnimal(Animal_id++, request.pos.first, request.pos.second, request.radius, World::GetWorld().conf.Organism_reproduce_energy_threshold, World::GetWorld().conf.Organism_reproduce_energy_cost, UserAnimal::FindAnimalConfig(request.name));
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
 * @brief 检查active
 */
void Organism::check_active() // 单独死亡
{
    if (energy <= 0)
    {
        active = false;
    }
    if (!active && energy > 0)
    {
        // printf("Organism %d die with %f energy left\n",Plant_id,energy);
        World::GetWorld().AddLeftEnergyRequest({Pos, energy});
        energy = 0;
    }
}

/**
 * @brief 执行一步：增加能量（例如光合作用），消耗能量（新陈代谢），然后检查生死
 */
void Organism::Step()
{
    energy -= step_energy_cost;
    // assert(energy >= -100);
    check_active();
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
    reproduce_able = false; // 默认不可繁殖，由派生类自行设置
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

Plant::Plant(int id, int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, float step_energy_cost)
    : Reproducable(reproduce_energy_threshold, reproduce_energy_cost, radius, step_energy_cost, PLANT)
{
    name = "Plant";
    this->id = id;
    Pos = std::make_pair(x, y);
    reproduce_able = true; // 植物始终可以繁殖（只要能量足够）
}

Plant::Plant(int iD, int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, PlantConfig& org)
    : id(iD), Reproducable(reproduce_energy_threshold, reproduce_energy_cost, radius, org.step_energy_cost, PLANT)
{
    energy = org.reproduce_original_energy;
    name = org.name;
    Pos = std::make_pair(x, y);
    reproduce_able = true;
}

/**
 * @brief 植物的繁殖行为：在半径范围内随机生成一个新位置，并向世界请求添加新植物
 */

void Plant::Reproduce()
{
    if (!active || !reproduce_able)
    { // 死了就不能活着
        return;
    }
    if (energy < reproduce_energy_threshold)
    {

        return;
    }
    int x = Pos.first;
    int y = Pos.second;
    // 在 [-reproduce_radius, +reproduce_radius] 范围内随机偏移
    int x_new = x + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    int y_new = y + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    // printf("\033[31mPlant request at (%d, %d) with radius %d\033[0m\n", x_new, y_new);
    // 确保新位置在有效世界边界内
    if (x_new >= 0 && x_new < World::GetWorld().GetHeight() && y_new >= 0 && y_new < World::GetWorld().GetWidth())
    {
        // 子代植物的半径在父半径的[0.25,2.0]倍之间随机，并取整
        float r = reproduce_radius * std::min(2.0, std::max(0.25, (double)std::rand() / RAND_MAX));
        int r_int = std::max(1, (int)r);
        if (!(World::GetWorld().AddReproduceRequest({PLANT, name, std::make_pair(x_new, y_new), r_int})))
        {
            energy -= reproduce_energy_cost;
            // assert(energy >= -100);
            return;
        }
        // std::printf("Plant request at (%d, %d) %id\n", x_new, y_new,id);
    }
}

void Plant::Step()
{
    float ori = step_energy_cost;
    step_energy_cost *= calculate_overlay_cost();
    Organism::Step();
    step_energy_cost = ori;
}

float Plant::calculate_overlay_cost()
{
    float overlay = World::GetWorld().calculate_overlay(Pos);
    float overlay_p = World::GetWorld().conf.Organism_overlay_param;
    float factor = (float)1 / (abs(overlay - overlay_p)) + (overlay_p - 1) / overlay_p;
    return factor;
}



Animal::Animal(int id, int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, float step_energy_cost)
    : id(id), Reproducable(reproduce_energy_threshold, reproduce_energy_cost, radius, step_energy_cost, ANIMAL)
{
    rate = SetRate();
    name = "Animal";
    energy = 20; // 测试用 每个动物初始能量应该不同
    Pos = std::make_pair(x, y);
    reproduce_able = true;
}

Animal::Animal(int iD, int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, AnimalConfig& org)
    : id(iD), Reproducable(reproduce_energy_threshold, reproduce_energy_cost, radius, org.step_energy_cost, ANIMAL)
{
    energy = org.reproduce_original_energy;
    rate = org.reproduce_original_rate;
    name = org.name;
    diet = org.diet;
    _energy_rate = org.energy_rate;
    max_rate = org.max_rate;
    Pos = std::make_pair(x, y);
    reproduce_able = true;
}

void Animal::Reproduce()
{
    
    if (!active || !reproduce_able)
    { // 死了就不能活着
        return;
    }
    if (energy < reproduce_energy_threshold)
    {

        return;
    }
    int x = Pos.first;
    int y = Pos.second;
    // 在 [-reproduce_radius, +reproduce_radius] 范围内随机偏移
    int x_new = x + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    int y_new = y + std::rand() % (2 * reproduce_radius + 1) - reproduce_radius;
    
    // printf("\033[31mAnimal request at (%d, %d) with radius %d\033[0m\n", x_new, y_new);
    // 确保新位置在有效世界边界内
    if (x_new >= 0 && x_new < TestConfig::GetTestConfig().The_Word.length && y_new >= 0 && y_new < TestConfig::GetTestConfig().The_Word.width)
    {
        // 子代植动物物的半径在父半径的[0.25,2.0]倍之间随机，并取整
        float r = reproduce_radius * std::min(2.0, std::max(0.25, (double)std::rand() / RAND_MAX));
        int r_int = std::max(1, (int)r);
        printf("%d\n",r_int);
        if (!(World::GetWorld().AddReproduceRequest({ANIMAL, name, std::make_pair(x_new, y_new), r_int})))
        {
            energy -= reproduce_energy_cost;
            // printf("Animal Generating");
            // assert(energy >= -100);
            return;
        }
        // std::printf("Plant request at (%d, %d) %id\n", x_new, y_new,id);
    }
}

float Animal::SetRate()
{
    return 3;
}

void Animal::SetRate(Animal *a)
{
    a->rate = a->energy * _energy_rate;
}

void Animal::Step()
{
    float ori = step_energy_cost;
    step_energy_cost *= calculate_overlay_cost();
    Organism::Step();
    step_energy_cost = ori;
    // 移动
    if(eat_intrval-- <= 0){
        double angle = dist(gen);
        int x_move = rate * sin(angle);
        int y_move = rate * cos(angle);
        if (x_move == 0 && y_move == 0)
        {
            // 至少移动一格
            x_move = (angle < pi) ? 1 : -1;
            y_move = (angle < pi / 2 || angle > 3 * pi / 2) ? 1 : -1;
        }

        x_move += Pos.first;
        y_move += Pos.second;
        Pos = std::make_pair(std::max(std::min(x_move, TestConfig::GetTestConfig().The_Word.length - 1), 0), std::max(std::min(y_move, TestConfig::GetTestConfig().The_Word.width - 1), 0));
    }
}

float Animal::calculate_overlay_cost() // 同植物
{
    float overlay = World::GetWorld().calculate_overlay(Pos);
    float overlay_p = World::GetWorld().conf.Organism_overlay_param;
    float factor = (float)1 / (abs(overlay - overlay_p)) + (overlay_p - 1) / overlay_p;
    return factor;
}

bool isNaber(Organism *a, Organism *b)
{ // 指针方便多态
    if (b->Pos.first >= a->Pos.first - 1 && b->Pos.first <= a->Pos.first + 1 && b->Pos.second >= a->Pos.second - 1 && b->Pos.second <= a->Pos.second + 1)
    {
        return true;
    }
    return false;
}

void PredationOrFuck(Reproducable *a, Reproducable *b)
{
    // 怀孕判断
    if (a->type == PLANT && a->reproduce_able)
    {
        a->Reproduce();
    }
    else if (a->name == b->name)
    {
        
        if (a->reproduce_able && !b->reproduce_able)
        {
            a->Reproduce();
        }
        if (b->reproduce_able && !a->reproduce_able)
        {
            b->Reproduce();
        }else{
            rand01()<0.5?b->Reproduce():a->Reproduce();
        }
        return;
    }
    // 捕食判断
    MyOperator::GetOp()(a, b);
    return;
}
