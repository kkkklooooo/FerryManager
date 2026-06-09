# 架构设计

## 总体架构

FerryManager 采用**分层架构**，自上而下分为三层：

```
┌─────────────────────────────────────────┐
│          UI 层 (test_main.cpp / myui.cpp) │
│   ImGui 渲染、用户输入、数据可视化       │
├─────────────────────────────────────────┤
│         Operator 层 (operator/)          │
│   生物工厂、捕食规则、物种注册          │
├─────────────────────────────────────────┤
│         核心层 (core/)                   │
│   World 调度、Organism 行为、Environment │
└─────────────────────────────────────────┘
```

## 核心模式

### 1. 单例模式 (Singleton)

#### World — 全局唯一世界
```cpp
// World 是静态单例，必须通过 GetWorld() 访问
World& World::GetWorld();              // 获取已存在的 World
World& World::GetWorld(TestConfig&);   // 首次创建（必须先用 InitGameConfig）
```

**注意**：`g_GameConf` 必须通过 `InitGameConfig(cfg)` 初始化后才能创建 World，否则程序崩溃。

#### Config — 全局引擎参数
```cpp
Config& Config::GetConfig();  // 返回静态单例，硬编码默认值
```

#### TestConfig — 应用层配置
```cpp
// g_GameConf 是全局指针，由 InitGameConfig 设置
TestConfig& TestConfig::GetTestConfig();  // 返回 *g_GameConf
```

### 2. 工厂模式 (Factory)

`MyOperator` 同时承担**工厂注册**和**交互调度**两个职责：

```cpp
// 注册物种工厂（在 World 构造函数中完成）
MyOperator::register_Animal_Create("Wolf", [](int id, int x, int y, int r, auto genes) {
    return new UserAnimal(id, x, y, r, ...);
});

// 创建生物
Reproducable* org = MyOperator::GetOp()(x, y, radius, "Wolf", nextID, genes);

// 处理捕食/繁殖交互
MyOperator::GetOp()(organismA, organismB);
```

**注册链**：
1. `World::World()` 遍历配置中的物种，用 lambda 注册工厂
2. 同时 `PlantRegistrator` / `AnimalRegistrator` 静态对象在程序启动时注册硬编码物种（如 "Grass"）
3. `ReprodueNewOrganism(request)` 查表创建对应物种

### 3. 延迟请求模式 (Deferred Request)

繁殖和死亡不在当前帧立即执行，而是通过请求队列延迟到帧末统一处理：

```cpp
// 繁殖请求队列
std::vector<ReproduceRequest> reproduce_requests;  // 本帧新增
std::vector<ReproduceRequest> last_requests;       // 上一帧（UI 读取）

// World::Update() 流程
//   1. RemoveDeadOrganisms() — 删除标记死亡的生物
//   2. 状态更新（生物 Step、能量交换）
//   3. 交互检测（繁殖/捕食）→ 填充 reproduce_requests
//   4. Reproduce() → 统一创建子代 + 保存到 last_requests
```

**好处**：
- 遍历容器时不会因新增/删除导致迭代器失效
- 新生物从下一帧开始参与模拟
- UI 可以同步读取 `last_requests` 显示繁殖标记

### 4. 注册表模式 (Registry)

`MyOperator` 内部维护两张注册表：

```cpp
static std::unordered_map<std::string, Creator>& registry();        // 动物工厂
static std::unordered_map<std::string, PlantCreator>& Plantregistry(); // 植物工厂
```

通过 `register_Animal_Create` / `register_Plant_Create` 注册，`operator()(ReproduceRequest)` 查表调用。

## 类层次结构

### Organism 体系

```
Organism                          // 基类
├── energy, step_energy_cost      // 能量状态
├── Pos, explicit_pos, velocity   // 位置（离散格 + 浮点精确）
├── active, type, name            // 状态标识
├── Step()                        // 每帧行为（虚函数）
└── check_active()                // 能量 ≤ 0 则死亡

Reproducable : Organism           // 可繁殖生物
├── reproduce_energy_threshold    // 繁殖所需最低能量
├── reproduce_energy_cost         // 繁殖消耗能量
├── reproduce_radius              // 子代散布半径
├── reproduce_able                // 是否可以繁殖
├── live_environment              // 可生存环境
├── diet / predator               // 食谱 / 天敌列表
└── Reproduce()                   // 繁殖行为（纯虚）

Plant : Reproducable              // 植物
├── id                            // 唯一 ID
├── Step()                        // 代谢 + 拥挤惩罚
├── calculate_overlay_cost()      // 计算拥挤因子
└── Reproduce()                   // 随机位置产子

Animal : Reproducable             // 动物
├── id, rate, max_rate            // 移动速度
├── eat_intrval, eat_intrval_max  // 进食冷却
├── max_energy, _energy_rate      // 能量上限和转化率
├── genes (boids::Genes)          // 遗传基因
├── neighbors                     // Boids 邻居缓存
├── Step()                        // 代谢 + Boids 移动
├── SetRate()                     // 速度计算
└── Reproduce(other)              // 双亲繁殖（基因融合）
```

### Environment 体系

```
Environment
├── Pos, energy, name             // 位置、能量、名称
├── deadOrganismEnergy            // 尸体能量缓存
├── SingleEnvironmentMaxEnergy    // 能量上限
├── CanLiveIn                     // 可生存物种列表
├── Organisms                     // 当前格上的生物列表
├── canPlant(request)             // 检查是否可以在此繁殖
├── EnergyExchange(organism)      // 植物吸收环境能量
├── getDeadOrgnismEnergy()        // 接收尸体能量
└── Update(weather)               // 环境更新

GressLand : Environment           // 草地：晴天 +0.8 能量/帧
Water : Environment               // 水域：蒸发 + 能量转换
```

### Boids 体系

```
namespace boids {
    struct Genes {                 // 遗传特征
        float cohesion;            // 聚集倾向
        float alignment;           // 对齐倾向
        float separation;          // 分离倾向
        float vision;              // 视野半径
        float escape;              // 逃避倾向
        float food_addict;         // 觅食倾向
    };

    struct Particle {              // Boids 粒子
        float x, y, vx, vy, speed; // 位置和速度
        int species;               // 物种 (0=Wolf, 1=其他)
        Genes genes;               // 基因
        vector<int> diet, predator;// 食谱/天敌（int 编码）
    };
}
```

## 数据流总览

```
                       config JSON
                           │
                    TestConfig (应用层配置)
                           │
              ┌────────────┼────────────┐
              │            │            │
         World::World   Config      gameData
              │        (引擎参数)    (存档)
              │
    ┌─────────┼─────────┐
    │         │         │
  Plants   Animals   Environments
    │         │         │
    │    Boids::Genes   │
    │    (遗传基因)      │
    │         │         │
    └─────────┼─────────┘
              │
        reproduce_requests (延迟队列)
              │
         新生物创建 (下一帧生效)
```

## 关键设计决策

### 1. 离散坐标 + 浮点精确位置

- `Pos` (`std::pair<int,int>`)：所属格子，用于环境交互、邻居查找
- `explicit_pos` (`std::pair<float,float>`)：精确浮点位置，用于 Boids 平滑移动
- 每帧：Boids 更新 `explicit_pos` → 重新计算 `Pos`（格子归属）

### 2. 能量吸收系数

```cpp
// 植物吸收环境能量时的三个关键系数
abs = min(plant_absorb_rate * energy, max_absorb / loss_rate);
abs = abs * loss_rate;       // 损失率折扣
abs = min(abs, cellEnergy); // 不超过环境当前能量（防止负数）
```

这个三层夹紧设计确保：
- 吸收量不会过大（受 cap 限制）
- 能量转化有损失（loss_rate < 1）
- 环境能量不会为负（安全夹紧）

### 3. 拥挤惩罚 (Overlay)

```cpp
float factor = 1 / abs(overlay - overlay_param) + (overlay_param - 1) / overlay_param;
```

- `overlay`：周围 8 格生物密度（归一化到 [0, 1]）
- `overlay_param`：理想密度参数（默认 1.0）
- 当密度接近 `overlay_param` 时，惩罚因子急剧增大 → 生物能量消耗增加 → 死亡
- 这是系统的**自限机制**，防止种群无限增长

### 4. 两种 UI 实现

| 特性 | ecosim (test_main.cpp) | ecosim2 (imguMe/myui.cpp) |
|------|----------------------|--------------------------|
| 生物绘制 | 彩色圆点 | PNG 纹理贴图 |
| 环境绘制 | 颜色渐变 | PNG 纹理贴图 |
| 种群图表 | 固定三条线 (Grass/Sheep/Wolf) | 动态所有物种 |
| 基因图表 | 有 (Sheep 专用) | 无 |
| 手动添加生物 | 无 | 有 (AddNewOrganism) |
| 能量热力图 | 有 | 无 |
