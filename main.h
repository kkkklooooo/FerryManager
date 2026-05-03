#ifndef FERRY_MANAGER_MAIN_H
#define FERRY_MANAGER_MAIN_H

#include <utility>
#include <vector>

// 全局变量：用于生成生物的唯一ID
extern int id;

// 生物类型枚举
enum OrganismType
{
    PLANT,
    ANIMAL，
    SOURCE;
};

enum EnvironmentType {

};

enum OrganismName {

};

class Organism;
class Plant;
class Reproducable;


/**
 * @brief 繁殖请求结构体，用于在世界中暂存待生成的子代信息
 */
struct ReproduceRequest
{
    OrganismType type;           // 请求生成的生物类型
    std::pair<int, int> pos;     // 生成位置 (x, y)
    int radius;                  // 子代生物的初始半径
};

/**
 * @brief 世界类，管理所有生物以及繁殖请求
 */
class World
{
    // 待处理的繁殖请求列表
    std::vector<ReproduceRequest> reproduce_requests;
    // 所有植物对象指针列表
    std::vector<Plant *> plants;
    // 所有被捕食动物对象指针列表（暂时未完全实现）
    std::vector<Prey *> preys;
    // 所有捕食者对象指针列表（暂时未完全实现）
    std::vector<Predator *> predators;

public:
    // 更新世界状态（例如环境变化等）
    void Update();
    // 处理所有繁殖请求，生成新生物
    void Reproduce();
    // 向世界添加一个繁殖请求
    void AddReproduceRequest(const ReproduceRequest &request);
    // 移除所有死亡（active == false）的生物
    void RemoveDeadOrganisms();
    // 获取世界单例实例
    static World &GetWorld();
};



/**
 * @brief 繁殖管理器，负责检查条件并触发繁殖
 */
class ReproduceManager
{
public:
    // 更新可繁殖对象的状态，若满足条件则调用其Reproduce方法
    void Update(Reproducable &reproducable);

private:
    // 构造函数私有化（但该类未实现单例模式，仅为占位）
    ReproduceManager() = default;
};

/**
 * @brief 生物管理器，负责调用生物的Step方法进行单步更新
 */
class OrganismManager
{
public:
    // 更新单个生物的一步行为
    void Update(Organism &organism);
};

/**
 * @brief 所有生物的基类
 */
class Organism
{
public:
    float energy = 0;               // 当前能量值
    float step_energy_cost;         // 每步消耗的能量
    float step_energy_gain;         // 每步获得的能量
    std::pair<int, int> Pos;        // 位置坐标 (x, y)
    bool active = true;             // 是否活跃（false表示死亡）
    OrganismType type;              // 生物类型

    // 构造函数：初始化每步能量消耗和获得以及类型
    Organism(float step_energy_cost, float step_energy_gain, OrganismType type);
    // 检查能量，若能量≤0则设置active为false
    void check_energy();
    // 执行一步：增减能量并检查生死
    void Step();
};

/**
 * @brief 可繁殖生物类，继承自Organism，增加了繁殖相关属性
 */
class Reproducable : public Organism
{
public:
    // 构造函数：传入繁殖阈值、消耗、半径，以及每步能量消耗和获得、类型
    Reproducable(float energy_threshold, float energy_cost, int radius, float sec, float seg, OrganismType type);
    virtual ~Reproducable() = default;
    std::vector<EnvironmentType> live_environment;
    std::vector<OrganismName>diet;
    float reproduce_energy_threshold;
    float reproduce_energy_cost;
    int reproduce_radius;
    bool reproduce_able;

    // 纯虚函数（实际有默认实现），派生类应重写具体的繁殖行为
    virtual void Reproduce();
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
    // void check_energy() override;  // 注释掉：直接使用基类实现即可
    // 植物的单步行为（当前为空实现）
    void Step();
};

#endif // FERRY_MANAGER_MAIN_H