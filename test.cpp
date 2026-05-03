#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "main.h"

// 匿名命名空间：内部使用的常量、辅助函数和数据结构，避免外部链接
namespace
{
// 地图宽度（X轴范围 0~99）
constexpr int kMapWidth = 100;
// 地图高度（Y轴范围 0~99）
constexpr int kMapHeight = 100;
// 初始生成的植物数量
constexpr int kPlantCount = 30;
// 初始植物的繁殖半径
constexpr int kPlantRadius = 5;
// 允许注册到世界中的最大植物数量（防止无限增长）
constexpr int kMaxRegisteredPlants = 10000;
// 模拟的总tick数（时间步数）
constexpr int kTicks = 800;
// 非统计模式下，每帧之间的延迟（毫秒）
constexpr int kFrameDelayMs = 120;

/**
 * @brief 内部辅助结构，用于访问 World 对象中的私有容器。
 *        通过 reinterpret_cast 将 World 对象转换为该结构，
 *        仅用于本文件内部的全局访问，属于临时破解手段。
 */
struct WorldAccess
{
    std::vector<ReproduceRequest> reproduce_requests; // 繁殖请求列表
    std::vector<Plant *> plants;                      // 植物指针列表
    std::vector<Prey *> preys;                        // 被捕食动物指针列表（暂未使用）
    std::vector<Predator *> predators;                // 捕食者指针列表（暂未使用）
};

/**
 * @brief 单个时间步更新后返回的结果，记录新生和死亡数量
 */
struct UpdateResult
{
    int newborn_count = 0;  // 本步新增的生物数量
    int death_count = 0;    // 本步死亡的生物数量
};

/**
 * @brief 生成 [0, limit) 范围内的随机整数坐标
 * @param limit 上限（不包含）
 * @return 随机坐标值
 */
int randomCoord(int limit)
{
    return std::rand() % limit;
}

/**
 * @brief 将世界单例对象强制转换为 WorldAccess 结构，以便直接操作其内部容器。
 * @return WorldAccess 的引用
 */
WorldAccess &worldAccess()
{
    return reinterpret_cast<WorldAccess &>(World::GetWorld());
}

/**
 * @brief 获取当前所有植物的只读引用（通过 worldAccess）
 * @return 植物指针常向量引用
 */
const std::vector<Plant *> &collectPlants()
{
    return worldAccess().plants;
}

/**
 * @brief 为特定植物添加一个繁殖请求（统计模式下使用，不调用植物真正的 Reproduce）
 * @param plant 要繁殖的植物
 */
void addPlantReproduceRequest(Plant &plant)
{
    const int x = plant.Pos.first;
    const int y = plant.Pos.second;
    // 在繁殖半径范围内随机偏移生成子代坐标
    const int x_new = x + std::rand() % (2 * plant.reproduce_radius + 1) - plant.reproduce_radius;
    const int y_new = y + std::rand() % (2 * plant.reproduce_radius + 1) - plant.reproduce_radius;
    // 边界检查（世界为 100x100）
    if (x_new >= 0 && x_new < kMapWidth && y_new >= 0 && y_new < kMapHeight)
    {
        // 子代半径随机变化：父半径 * [0.25, 2.0] 之间的随机系数
        const int r = plant.reproduce_radius * std::min(2.0, std::max(0.25, (double)std::rand() / RAND_MAX));
        World::GetWorld().AddReproduceRequest({PLANT, std::make_pair(x_new, y_new), r});
    }
}

/**
 * @brief 执行一个时间步的更新：遍历所有植物，调用生命周期逻辑，处理繁殖和死亡。
 * @param stats_only 若为 true，则只统计数量变化，不实际调用植物的 Reproduce（仅记录请求）
 * @return UpdateResult 包含本步的新生和死亡数量
 */
UpdateResult updateWorld(bool stats_only)
{
    World &world = World::GetWorld();
    std::vector<Plant *> &plants = worldAccess().plants;
    const size_t before_count = plants.size();  // 更新前的植物总数

    // 使用快照方式遍历：只处理更新前的植物，本步新繁殖的植物留到下一 tick
    for (size_t i = 0; i < before_count; ++i)
    {
        Plant *plant = plants[i];
        if (!plant->active)   // 已死亡的跳过
        {
            continue;
        }

        // 调用基类的 Step 方法，更新能量并检查生死
        static_cast<Organism &>(*plant).Step();
        if (!stats_only)
        {
            plant->Step();    // 非统计模式下调用派生类的 Step（当前为空实现）
        }

        // 如果植物总数加上待处理的繁殖请求已经达到上限，则不进行新的繁殖
        if (plants.size() + worldAccess().reproduce_requests.size() >= kMaxRegisteredPlants)
        {
            continue;
        }

        // 如果允许繁殖且能量足够
        if (plant->reproduce_able &&
            plant->energy >= plant->reproduce_energy_threshold)
        {
            plant->energy -= plant->reproduce_energy_cost;   // 扣除繁殖消耗的能量
            if (stats_only)
            {
                // 统计模式：直接向世界添加繁殖请求，而不执行植物自身的 Reproduce
                addPlantReproduceRequest(*plant);
            }
            else
            {
                // 正常模式：调用植物的繁殖函数，它内部会向世界添加请求
                plant->Reproduce();
            }
        }
    }

    // 统计本步新增的请求数量
    const size_t newborn_count = worldAccess().reproduce_requests.size();
    world.Reproduce();                   // 处理所有繁殖请求，向 plants 容器添加子代
    world.RemoveDeadOrganisms();         // 移除所有 active == false 的植物

    // 计算死亡数 = (原数量 + 新增请求数) - 最终植物数
    const int death_count = static_cast<int>(before_count + newborn_count - plants.size());
    return {static_cast<int>(newborn_count), death_count};
}
} // namespace anonymous

/**
 * @brief 初始化植物分布：生成第一个繁殖请求并调用 Reproduce 生成初始植物种群。
 *        该函数只会生效一次（通过 static 标志保证）。
 */
void scatterPlant()
{
    static bool generated = false;
    if (generated)
    {
        return;   // 已经生成过，直接返回
    }

    generated = true;
    World &world = World::GetWorld();
    worldAccess().plants.reserve(kMaxRegisteredPlants);  // 预分配内存
    for (int i = 0; i < kPlantCount; ++i)
    {
        const int x = randomCoord(kMapWidth);
        const int y = randomCoord(kMapHeight);
        world.AddReproduceRequest({PLANT, std::make_pair(x, y), kPlantRadius});
    }

    world.Reproduce();   // 批量处理繁殖请求，生成实际植物对象
}

/**
 * @brief 绘制当前地图的字符界面
 * @param plants       所有植物指针列表
 * @param newborn_count 本 tick 新增的植物数量（用于标记新生个体）
 * @param tick         当前 tick 序号
 */
void drawMap(const std::vector<Plant *> &plants, int newborn_count, int tick)
{
    // 创建二维字符地图，初始都填充 '.'
    std::vector<std::string> map(kMapHeight, std::string(kMapWidth, '.'));

    // 新生植物在 plants 列表中的起始索引（如果新生数量超过总数量，则取总数量）
    const size_t newborn_start = newborn_count <= static_cast<int>(plants.size())
                                     ? plants.size() - newborn_count
                                     : plants.size();

    // 遍历所有植物，将对应位置填上字符
    for (size_t i = 0; i < plants.size(); ++i)
    {
        const Plant *plant = plants[i];
        const int x = plant->Pos.first;
        const int y = plant->Pos.second;
        if (x < 0 || x >= kMapWidth || y < 0 || y >= kMapHeight)
        {
            continue;   // 位置无效（理论上不应该出现）
        }

        // 根据是否为新生个体使用不同标记，如果该格子已有其他植物则显示为 '*'
        const char mark = i >= newborn_start ? 'N' : 'P';
        map[y][x] = map[y][x] == '.' ? mark : '*';
    }

    // 清屏并移动光标到左上角（ANSI 转义序列）
    std::cout << "\x1B[2J\x1B[H";
    // 输出统计信息
    std::cout << "Tick: " << tick
              << " | Plants: " << plants.size()
              << " | Newborns: " << newborn_count << '\n';
    std::cout << "Legend: . empty, P plant, N newborn, * overlap\n";

    // 逐行输出地图
    for (const std::string &row : map)
    {
        std::cout << row << '\n';
    }
}

/**
 * @brief 主函数：解析命令行参数，执行模拟循环。
 * @param argc 参数个数
 * @param argv 参数数组，支持 --stats-only 或 --counts-only 开启仅统计模式
 */
int main(int argc, char *argv[])
{
    // 是否仅输出统计数据（不画地图，不加延迟）
    bool stats_only = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--stats-only" || arg == "--counts-only")
        {
            stats_only = true;
        }
    }

    // 固定随机种子，保证每次运行结果可重现
    std::srand(1);

    // 散布初始植物（第一次调用有效）
    scatterPlant();
    scatterPlant(); // 第二次调用无效果（内部 static 防止重复）

    // 累计新生总数和死亡总数
    int cumulative_births = static_cast<int>(collectPlants().size());
    int cumulative_deaths = 0;

    // 输出第 0 tick 的状态
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

    // 主循环：从 tick = 1 到 kTicks
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