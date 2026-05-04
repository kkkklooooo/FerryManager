#ifndef FERRY_MANAGER_MAIN_H
#define FERRY_MANAGER_MAIN_H

#include <utility>
#include <vector>
#include"Registry.h"
// 全局变量：用于生成生物的唯一ID
const float AnimalAbsorbRate=0.5;
const float lossRate = 0.9;
extern int id;



class Organism;
class Plant;
class Reproducable;


struct LeftEnergyRequest{
    std::pair<int, int> pos;
    float energy;
};
/**
 * @brief 生成请求结构体，用于在世界中暂存待生成的子代信息
 */
struct ReproduceRequest
{
    OrganismType type;           // 请求生成的生物名称
    OrganismName name;           //生成生物的具体名称
    std::pair<int, int> pos;     // 生成位置 (x, y)
    int radius;                  // 子代生物的初始半径
};

/**
 * @brief 所有生物的基类
 */

class Organism
{
public:
    float energy = 5;               // 当前能量值
    float step_energy_cost;         // 每帧消耗的能量
    std::pair<int, int> Pos;         // 位置坐标 (x, y)
    bool active = true;             // 是否活跃（false表示死亡）
    OrganismType type;              // 生物类型
    OrganismName name;              //生物名称
    // 构造函数：初始化每步能量消耗和获得以及类型
    Organism(float step_energy_cost, OrganismType type);
    // 检查能量，若能量≤0则设置active为false
    void check_active();
    // 移动一步：增减能量并检查生死
    virtual void Step()=0;
};

/**
 * @brief 可繁殖生物类，继承自Organism，增加了繁殖相关属性
 */
class Reproducable : public Organism
{
public:
    // 构造函数：传入繁殖阈值、消耗、半径，以及每步能量消耗和获得、类型
    Reproducable(float energy_threshold, float energy_cost, int radius, float sec, OrganismType type);
    virtual ~Reproducable() = default;
    std::vector<EnvironmentType> live_environment;
    std::vector<OrganismName>diet;
    float reproduce_energy_threshold;
    float reproduce_energy_cost;
    int reproduce_radius;
    bool reproduce_able;

    // 纯虚函数（实际有默认实现），派生类应重写具体的繁殖行为
    virtual void Reproduce()=0;
};

/**
 * @brief 植物类，继承自Reproducable，实现了具体的繁殖行为
 */
class Plant : public Reproducable
{
public:
    // 构造函数：传入ID、坐标和半径
    Plant(int id, int x, int y, int radius);

    int id;   // 植物唯一标识符

    // 重写繁殖方法：随机生成子代位置并向世界提交繁殖请求
    void Reproduce() override;
    void Step();
    float calculate_overlay_cost();
};


Reproducable* ReprodueNewOrganism(ReproduceRequest request);
bool isNaber(Organism* a, Organism* b);//判断是不是相邻方便捕食

void PredationOrFuck(Reproducable* a, Reproducable* b);
#endif // FERRY_MANAGER_MAIN_H