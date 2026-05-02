# OOP 最佳实践记录

这个文档用于记录 FerryManager 的面向对象设计约定。后续有新的架构讨论或决策时，优先在这里增量更新，而不是只停留在聊天记录里。

## 总原则

- `World` / `Simulation` 负责统一调度主循环，控制系统更新顺序。
- 生物对象保存自己的状态，例如位置、能量、年龄、是否存活。
- System / Manager 负责某一类规则，例如繁殖、重叠压力、死亡清理、资源更新。
- 对象不要在自己的 `Step()` 里主动调用一堆全局 Manager。
- Manager 不应该拥有生物对象，也尽量不要长期保存裸指针。
- 遍历对象时不要直接修改正在遍历的容器，出生和死亡应延迟到 tick 末尾统一处理。

推荐主循环方向：

```cpp
World::Update()
{
    overlayPressureSystem.Update(*this);

    for (auto& plant : plants)
    {
        PlantStepContext ctx;
        ctx.overlayPressure = overlayPressureSystem.GetPressure(plant->Pos);
        plant->Step(ctx);
    }

    reproduceSystem.Update(*this);
    AddBirths();
    deathSystem.RemoveDead(*this);
}
```

## World 作为对象生命周期拥有者

长期建议让 `World` 成为唯一拥有生物对象的地方：

```cpp
class World
{
public:
    std::vector<std::unique_ptr<Plant>> plants;
    std::vector<BirthRequest> birthRequests;
    int nextPlantId = 0;

    void Update();
    void AddBirths();
    void RemoveDead();
};
```

这样可以避免：

- `new Plant(...)` 后没有统一释放，导致内存泄漏。
- 多个 Manager 同时保存裸指针，死亡后产生悬空指针。
- 遍历过程中新增对象导致容器迭代器失效。
- 新生物是否立刻参与当前 tick 的行为变得不清晰。

## Plant 的职责

`Plant` 应该主要负责自身状态变化，不应该负责全局规则调度。

推荐方向：

```cpp
struct PlantStepContext
{
    float overlayPressure = 0.0f;
    float soilNutrition = 0.0f;
    float water = 0.0f;
    float sunlight = 0.0f;
};

class Plant : public Reproducable
{
public:
    void Step(const PlantStepContext& ctx)
    {
        energy += step_energy_gain;
        energy -= step_energy_cost;
        energy -= ctx.overlayPressure;

        if (energy <= 0.0f)
        {
            alive = false;
        }
    }
};
```

`Plant` 不需要知道重叠压力来自哪个系统，只根据输入上下文更新自己。

## Overlay Pressure 的设计

不建议让 `Plant` 继承 `OverlayPressure`。因为语义上不是“Plant 是一种重叠压力”，而是“Plant 会受到重叠压力影响”。

推荐使用系统：

```cpp
class OverlayPressureSystem
{
public:
    void Update(World& world);
    float GetPressureAt(std::pair<int, int> pos) const;
};
```

简单实现思路：

1. 第一遍扫描 `world.plants`，统计每个格子上的植物数量。
2. 第二步把 `count - 1` 转成压力值。
3. `World::Update()` 创建 `PlantStepContext`，把压力传给 `Plant::Step(ctx)`。

同格压力可以先这样算：

```cpp
float pressure = 0.0f;
if (count > 1)
{
    pressure = (count - 1) * 0.05f;
}
```

以后如果需要“附近也有压力”，可以从同格统计升级成半径扫描或空间网格索引。

## ReproduceManager / ReproduceSystem 的最佳实践

当前思路中的全局注册式 `ReproduceManager` 不适合作为长期架构：

```cpp
ReproduceManager::GetInstance().RegisterReproducable(this);
```

问题：

- 构造函数产生隐藏副作用。
- 全局单例让依赖关系不清晰。
- Manager 长期保存裸指针，死亡后容易悬空。
- `Update()` 遍历时如果 `Reproduce()` 新增对象，会修改同一个容器，存在迭代失效风险。
- `Plant::Reproduce()` 里直接 `new Plant(...)`，生命周期没有统一管理。

推荐改成：

> `World` 拥有对象，`ReproduceSystem` 扫描 `World`，生成 `BirthRequest`，`World` 在 tick 末尾统一创建新对象。

示意：

```cpp
struct BirthRequest
{
    int x = 0;
    int y = 0;
    int radius = 0;
};

class ReproduceSystem
{
public:
    void Update(World& world)
    {
        for (auto& plant : world.plants)
        {
            if (!plant->reproduce_able)
            {
                continue;
            }

            if (plant->energy < plant->reproduce_energy_threshold)
            {
                continue;
            }

            plant->energy -= plant->reproduce_energy_cost;
            world.birthRequests.push_back(CreatePlantBirth(*plant));
        }
    }
};
```

然后由 `World` 统一处理出生：

```cpp
void World::AddBirths()
{
    for (const BirthRequest& birth : birthRequests)
    {
        plants.push_back(std::make_unique<Plant>(
            nextPlantId++,
            birth.x,
            birth.y,
            birth.radius));
    }

    birthRequests.clear();
}
```

## Reproducable 的定位

`Reproducable` 可以保留为“可繁殖能力”的基类，但它只应该保存繁殖相关状态和简单行为，不应该自动注册到 Manager。

推荐：

```cpp
class Reproducable : public Organism
{
public:
    float reproduce_energy_threshold = 0.0f;
    float reproduce_energy_cost = 0.0f;
    int reproduce_radius = 0;
    bool reproduce_able = false;

    bool CanReproduce() const
    {
        return reproduce_able && energy >= reproduce_energy_threshold;
    }

    void PayReproduceCost()
    {
        energy -= reproduce_energy_cost;
    }
};
```

避免：

```cpp
ReproduceManager::GetInstance().RegisterReproducable(this);
```

## 出生与死亡的延迟处理

推荐每个 tick 分阶段处理：

1. 计算环境/压力。
2. 更新现有生物状态。
3. 收集出生请求。
4. 收集死亡标记。
5. tick 末尾统一添加 newborn。
6. tick 末尾统一移除 dead。

这样可以保证：

- 本 tick 的遍历对象集合稳定。
- 新生物默认从下一个 tick 开始参与更新。
- 死亡对象不会在其他系统中途消失。
- 可视化时能明确标出 newborn 和 dead。

## 后续增量更新规则

以后每次出现新的 OOP 架构决策，按这个格式追加：

```md
## 主题名称

背景：为什么需要这个设计。

结论：采用什么做法。

理由：为什么这样比其他方案好。

代码方向：简短示意，不要求一次实现完整。
```

## 更新记录

- 2026-05-02：新增 World/System 调度原则、OverlayPressure 设计、ReproduceSystem 替代全局注册式 ReproduceManager 的建议。
