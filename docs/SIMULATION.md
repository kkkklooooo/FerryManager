# 模拟机制

## 能量模型

### 能量总量守恒

模拟世界中的总能量**近似守恒**（少量因浮点误差损失）：

```
Total = Σ(环境格能量) + Σ(生物能量) + Σ(尸体缓冲能量)
```

能量的形式转化：
- **太阳能** → 添加到环境格（仅 GressLand + SUN）
- **环境能量** → 被植物吸收
- **植物能量** → 被动物捕食
- **动物能量** → 被上级捕食者获取
- **尸体能量** → 归还环境（× loss_rate）

### 能量流动公式

#### 1. 太阳能输入（GressLand::Update）

```cpp
if (weather == SUN && energy < maxEnergy)
    energy += 0.8f;  // 每帧每格固定增益
```

#### 2. 植物吸收环境能量（Environment::EnergyExchange）

```cpp
float raw = min(
    plant_absorb_rate * cellEnergy,            // 按吸收率
    step_max_absorb / loss_rate                // 受上限约束
);
float absorbed = raw * loss_rate;              // 能量转化损失
absorbed = min(absorbed, cellEnergy);          // 不超过环境现存能量
plant.energy += absorbed;
cellEnergy -= absorbed;
```

**参数**：
- `plant_absorb_rate` = 0.2
- `step_max_absorb` = 1.5
- `loss_rate` = 0.85
- 实际上限：min(0.2 × cellE, 1.5/0.85 ≈ 1.76) × 0.85 ≈ min(0.17×cellE, 1.5)

#### 3. 动物捕食（MyOperator::operator()）

```cpp
// 咬一口的能量
hunger = animal.max_energy - animal.energy;
bite = min({
    hunger / loss_rate,         // 饥饿上限
    prey.energy * absorb_rate,   // 吸收比例上限
    prey.energy                  // 猎物总体能量（防止负数）
});
predator.energy += bite * loss_rate;
prey.energy -= bite;

// 杀死概率
killChance = prey.type == PLANT
    ? 0.03f                                         // 吃植物 3%
    : 0.05f + 0.30f * (predE / (predE + preyE));    // 吃动物 5-35%
```

**参数**：
- `animal_absorb_rate` = 0.30
- `loss_rate` = 0.85

#### 4. 尸体能量回收（Environment::Update）

```cpp
float gain = deadOrganismEnergy * loss_rate;
if (energy < maxEnergy * 2)
    energy += min(gain, maxEnergy - energy);
deadOrganismEnergy = 0;
```

## 代谢系统

### 基础代谢

每帧所有生物消耗能量：

```cpp
void Organism::Step() {
    energy -= step_energy_cost;
    check_active();  // 能量 ≤ 0 → active = false
}
```

### 拥挤惩罚 (Overlay)

生物的实际消耗 = 基础消耗 × 拥挤因子：

```cpp
float factor = 1 / abs(overlay - overlay_param) + (overlay_param - 1) / overlay_param;
real_cost = step_energy_cost * factor;
```

**计算流程**：
1. `calculate_overlay(pos)` — 统计周围 8 格 + 本格的生物总数
2. `overlay = total_organisms / (8 × max_per_cell)` — 归一化到 [0, 1]
3. `overlay_param = 1.0` — 理想密度参数

**公式行为**：
- overlay → 0（空旷）：factor ≈ 1 + 0 = 1（无惩罚）
- overlay → 1（拥挤）：factor → ∞（极端惩罚，生物快速死亡）
- overlay 远离 1：惩罚逐渐减小

**效果**：
- 密度过高时，消耗激增 → 生物快速死亡 → 种群回落
- 构成系统的**自限负反馈**机制

## 繁殖系统

### 植物繁殖（无性）

```cpp
void Plant::Reproduce() {
    if (energy < reproduce_energy_threshold) return;  // 能量不够
    
    // 在半径范围内随机选位置
    x_new = x + rand(-radius, +radius);
    y_new = y + rand(-radius, +radius);
    
    // 提交繁殖请求（立即扣能量，延迟创建子代）
    if (world.AddReproduceRequest({PLANT, name, {x_new, y_new}, new_radius}))
        energy -= reproduce_energy_cost;
}
```

- 子代半径 = 父半径 × random(0.25, 2.0)，至少为 1
- 子代获得 `reproduce_original_energy` 的初始能量
- `canPlant()` 检查：物种在环境 CanLiveIn 列表中 && 同种不超过 `max_organisms_per_cell`

### 动物繁殖（有性）

```cpp
void Animal::Reproduce(optional<Reproducable*> other) {
    // 必须传入配偶（另一个同种 Animal）
    if (energy < reproduce_energy_threshold) return;
    
    // 基因融合：取父母平均值 + 高斯噪声
    Genes childGenes = GA::Fusion(myGenes, spouseGenes);
    
    // 提交请求
    if (world.AddReproduceRequest({ANIMAL, name, {x_new, y_new}, new_radius, childGenes}))
        energy -= reproduce_energy_cost;
}
```

- 调用方式：`PredationOrFuck(a, b)` 中，同种且至少一方 `reproduce_able` 则触发繁殖
- 子代继承融合后的基因

### 繁殖请求处理

```cpp
void World::Reproduce() {
    last_requests = reproduce_requests;  // 保存供 UI 读取
    for (auto& req : reproduce_requests)
        Reproducas.push_back(ReprodueNewOrganism(req));  // 创建 + 加入世界
    reproduce_requests.clear();
}
```

## 捕食系统

### 食谱与天敌

```cpp
struct Reproducable {
    vector<string> diet;      // 可以吃哪些物种
    vector<string> predator;  // 会被哪些物种吃
};
```

### 交互检测

在 `World::Update()` 中，对每个生物在交互半径（默认 2 格）内检测：

```cpp
for (每个格子的每个生物 i)
    for (半径 R 内每个邻居格的每个生物 j)
        if (i < j)  // 避免重复
            PredationOrFuck(i, j);
```

### PredationOrFuck 分支

```cpp
void PredationOrFuck(a, b) {
    if (a 是植物)  a.Reproduce();           // 植物自己繁殖
    else if (a 和 b 同名)  交配繁殖;          // 同种动物交配
    else MyOperator::GetOp()(a, b);         // 异种 → 捕食
}
```

### MyOperator 捕食详细规则

参见 [MODULES.md - MyOperator](MODULES.md#myoperator--三合一操作器)。

**关键机制**：
- **进食冷却**：动物每次进食后 `eat_intrval = eat_intrval_max`，每帧递减，冷却期间不能再次进食
- **能量到速度**：`rate = energy × energy_rate`，受 `max_rate` 上限约束
- **饱食上限**：能量达到 `max_energy` 后不再进食

## Boids 集群系统

### 邻居查找

```cpp
FillNeighbors(animal.Pos, animal.genes.vision, neighbors, world);
```

在 `vision` 半径内收集所有动物（含不同物种）。

### 合力计算（ComputeFinalForce）

```
合力 = cohesion × (邻居质心 - 自身位置)      // 聚集：向群体中心靠拢
     + alignment × (邻居平均速度 - 自身速度)   // 对齐：与群体一致方向
     + separation × Σ(-Δpos / distance²)     // 分离：避免碰撞
     + food_addict × (猎物质心 - 自身位置)    // 觅食：追逐猎物
     + escape × Σ(-Δpos / distance²)         // 逃避：远离天敌
     + random_noise × [-0.15, 0.15]          // 随机扰动
```

**重要**：
- `separation` 力使用 `-Δpos/distance²`：距离越近排斥力越大
- `escape` 力同样使用平方反比：离天敌越近逃得越快
- 只有 `self.diet` 中包含的物种才算作物（food）
- 只有 `self.predator` 中包含的物种才算天敌（escape）
- 速度受 `max_rate = min(rate, max_rate)` 上限约束

### 移动更新

```cpp
velocity += force × dt;              // dt = 0.25
speed = |velocity|;
if (speed > maxSpd) velocity *= maxSpd / speed;  // 限速
explicit_pos += velocity × dt;
Pos = (int(explicit_pos.x), int(explicit_pos.y));  // 离散化
```

## 遗传算法 (GA)

### 基因融合

```cpp
Genes Fusion(const Genes& a, const Genes& b) {
    return Genes{
        (a.cohesion   + b.cohesion)   / 2 + N(0, 0.15),
        (a.alignment  + b.alignment)  / 2 + N(0, 0.15),
        (a.separation + b.separation) / 2 + N(0, 0.15),
        (a.vision     + b.vision)     / 2 + N(0, 0.15),
        (a.escape     + b.escape)     / 2 + N(0, 0.15),
        (a.food_addict+ b.food_addict)/ 2 + N(0, 0.15),
    };
}
```

**特点**：
- 简单遗传：平均值继承 + 小幅度随机漂变
- 无显性/隐性：所有等位基因共显性
- 无突变：目前 Mutation() 函数已注释掉
- 无选择压力机制：自然选择通过捕食/饥饿死亡率隐式实现

### 演化驱动力

1. **高 vision** → 更早发现食物和天敌 → 生存优势
2. **高 escape** → 更快逃离天敌 → 生存优势
3. **高 food_addict** → 更强觅食倾向 → 能量获取优势
4. **适中的 cohesion/separation** → 保持群体但不拥挤
5. **自然选择**：跑得慢/看不到/不会逃的个体更容易被捕食

## 世界初始化

### 初始分布策略

```cpp
World::World(Config& conf, TestConfig& game_conf) {
    // 1. 注册所有物种工厂
    for (物种 in 配置) MyOperator::register_*_Create(...);

    // 2. 散布植物 (scatter pattern)
    //    7 个随机种子点 → 每个周围 8 株 Grass → 共 56 株

    // 3. 散布草食动物
    //    6 群 Sheep，每群 5 只 → 共 30 只（分散分布）

    // 4. 散布顶级捕食者
    //    8 只 Wolf（中心聚集，便于交配）

    // 5. 创建环境格 (w × h 个 GressLand)
    //    能量按高斯分布：中心高 (max ~18)，边缘低 (min ~3)
    float initEnergy = 3.0f + 15.0f * exp(-dist² / falloff²);
}
```

### Reset 行为

`World::Reset()` 完全销毁所有生物和环境，按同样规则重新初始化，各 ID 计数器归零。

## 模拟终止条件

```cpp
// GUI: 30,000 帧后自动停止
const int totalFrames = 30000;

// debug_sim: 1,200 帧批量运行
int totalSteps = 1200;

// 种群爆炸保护 (debug_sim)
if (totalOrgs > 10000) {
    printf("*** POPULATION EXPLOSION ***");
    break;
}
```
