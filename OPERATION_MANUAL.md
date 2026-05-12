# 操作手册

## 架构核心

### 双 Config 体系

| Config | 用途 | 谁改 |
|--------|------|------|
| `Config`（旧） | 引擎参数：能量吸收率、损耗率、繁殖阈值等 | 开发者，用户不动 |
| `TestConfig`（新） | 游戏内容：物种列表、食物链、地形类型、世界长宽 | 用户编辑，JSON 持久化 |

### 全局指针设计

`TestConfig` 通过全局指针 `g_GameConf` 在 main 流程开始时赋值，World 懒加载时消费。所有 `FindXXXConfig` 函数从 `g_GameConf` 读取，不依赖 World 单例。

```
main:
  1. TestConfig cfg ← 用户编辑物种/长宽
  2. World::GetWorld(cfg) ← 第一次调：存 g_GameConf，创建 World
  3. World::GetWorld()      ← 之后零参取已有实例
```

### World 单例

`WorldDefine.cpp` 中函数内 static + 懒加载，只构造一次。构造函数接收 `Config&`（引擎参数）和 `TestConfig*`（游戏配置）。

---

## 配置文件

### default_config.json 结构

```json
{
  "The_Word": { "length": 50, "width": 50 },
  "The_Environments": [
    { "name": "GressLand", "CanLive": ["Plant", "Sheep", "Wolf"] },
    { "name": "Water", "CanLive": [] }
  ],
  "The_Animals": [
    { "name": "Wolf", "diet": ["Sheep"], ... },
    { "name": "Sheep", "diet": ["Plant"], ... }
  ],
  "The_Plants": [
    { "name": "Plant", ... }
  ]
}
```

### JSON 序列化

使用 `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` 宏（Config.h），`to_json` / `from_json` 自动生成。

---

## 关键改动记录

1. ~~`Animal::_energy_rate = World::GetWorld()...`~~ → 静态初始化顺序 fiasco，改用 Config 构造函数传值
2. **reproduce_original_rate/energy** 从 `int` 改为 `float`
3. **CanLive** → **habitat_types**（EnvironmentConfig），与 CanLiveIn（Environment）区分
4. **ReproduceRequest.name** 改为 `std::string`，从 Config 读取
5. **ReprodueNewOrganism** 通过 `FindAnimalConfig` / `FindPlantConfig` 查配置创建子代
6. **Environment::CanLiveIn** 需从 EnvironmentConfig::CanLive 填充（World 构造后统一填）

---

---

## 文件结构

### 核心引擎

| 文件 | 内容 |
|------|------|
| `Registry.h` | 枚举定义（OrganismType, EnvironmentType, Weather） |
| `Organism.h` | 基类 Organism / Reproducable / Plant / Animal 声明，ReproduceRequest 结构体 |
| `OrganismDefine.cpp` | Plant/Animal/Reproducable 实现，ReprodueNewOrganism 工厂，PredationOrFuck |
| `Environment.h` | Environment / Water / GressLand 声明 |
| `EnvironmentDefine.cpp` | Environment 构造函数，EnergyExchange，Update，FindEnvironmentConfig |
| `World.h` | World 类声明，持有 Reproducas / Environments 容器 |
| `WorldDefine.cpp` | World 构造函数 / Update / GetWorld / Reset |

### 配置系统

| 文件 | 内容 |
|------|------|
| `Config.h` | AnimalConfig / PlantConfig / EnvironmentConfig / TestConfig 结构体，NLOHMANN 宏，旧 Config 类 |
| `ConflgDefine.cpp` | Check_Animal / Check_Plant / User_AddNew_Animal 等验证函数 |
| `default_config.json` | 默认游戏配置 |

### 工厂与注册

| 文件 | 内容 |
|------|------|
| `MyOperator.h` | MyOperator 类（注册表 + 捕食逻辑），AnimalRegistrator |
| `MyOperatorDefine.cpp` | register_Animal_Create，捕食实现，registry() 单例 |
| `Animals.h` | UserAnimal 类 + Wolf/Sheep 的 static AnimalRegistrator |
| `Plants.h` | UserPlant 类 |

### UI

| 文件 | 内容 |
|------|------|
| `test_main.cpp` | Win32 + OpenGL + ImGui 可视化界面 |

---

## 数据流

### 启动流程

```
default_config.json
       │
       ▼  (from_json)
   TestConfig 对象
       │
       ▼  World::GetWorld(cfg)
   Config 对象 ← cfg.The_Word.length/width
       │
       ▼
   World 构造函数
       ├─ new Plant() × 30      ← conf.Plant_init_radius 等
       ├─ MyOperator()("Sheep")  ← 查 registry 创建 UserAnimal
       ├─ MyOperator()("Wolf")   ← 查 registry 创建 UserAnimal
       └─ new GressLand() × N    ← conf.length × conf.width
```

### 每帧 Update

```
World::Update()
  ├─ 1. 清空每个 Environment::Organisms 网格
  ├─ 2. RemoveDeadOrganisms (清理能量≤0的个体)
  ├─ 3. Environment::Update (环境能量更新)
  ├─ 4. Environment::EnergyExchange (植物从环境吸收能量)
  ├─ 5. Organism::Step (生物消耗能量 + 移动)
  ├─ 6. 填网格 (每个生物挂到自己格子)
  ├─ 7. 9邻格查 PredationOrFuck (捕食/繁殖)
  └─ 8. Reproduce (执行繁殖请求队列)
```

### 繁殖链路

```
Animal::Reproduce() / Plant::Reproduce()
  │  构造 ReproduceRequest{type, name, pos, radius}
  ▼
World::AddReproduceRequest()
  │  调 Environment::canPlant() 检查 CanLiveIn
  ▼
World::Reproduce()
  │  调 ReprodueNewOrganism(request)
  ▼
ReprodueNewOrganism()
  ├─ type==PLANT → new UserPlant(..., FindPlantConfig(name))
  └─ type==ANIMAL → new UserAnimal(..., FindAnimalConfig(name))
```

---

## 注册表系统

### AnimalRegistrator

静态构造函数在 `Animals.h` 中注册 Wolf/Sheep 到 `MyOperator::registry()`：

```cpp
static AnimalRegistrator wolf("Wolf", [](...) -> Animal* {
    return new UserAnimal(...);
});
```

`MyOperator::registry()` 是函数内 static，Meyers 单例，首次调用时初始化。

### 使用场景

- **World 构造函数**：`MyOperator()(x, y, r, "Sheep", id)` 查注册表创建初始动物
- **捕食**：`MyOperator::GetOp()(a, b)` 查 diet 决定谁吃谁

---

## 常见坑

### 1. 静态初始化顺序 fiasco

`static AnimalRegistrator` 在 `Animals.h`，被 `OrganismDefine.cpp` 和 `WorldDefine.cpp` 各自包含。两个 TU 都有一份，都注册。如果 World 构造时 registry 还没初始化 → 返回 nullptr。

**解法**：Meyers 单例（函数内 static）保证了 registry 先于使用初始化。但 World 构造函数里的初始动物创建仍可能先于 AnimalRegistrator 的静态构造执行（不同 TU 顺序未定义）。

### 2. GetWorld 递归调用

World 构造期间，如果某个代码路径（如 FindAnimalConfig）内部调了 `GetWorld()` → 无限递归。

**解法**：已改为 `g_GameConf` 全局指针，FindXXXConfig 不走 GetWorld。

### 3. CanLiveIn 为空

`Environment::CanLiveIn` 从未从 `EnvironmentConfig::CanLive` 填充 → `canPlant()` 永远返回 false → 整个生态系统无法繁殖。

**解法**：World 构造完成后，遍历 Environments 调 `FindEnvironmentConfig(name).CanLive` 填入。

### 4. 两套 Config 区分

- `World::conf` (Config 类型) — 引擎参数，不改
- `g_GameConf` (TestConfig 类型) — 游戏参数，用户可改，JSON 读写

---

## 待办

- [ ] 在 main 中加载 `default_config.json` → `TestConfig`
- [ ] 提供用户编辑 TestConfig 的 UI（游戏前）
- [ ] World 构造后填充 Environment::CanLiveIn
- [ ] 补齐存档系统（World 序列化/反序列化）
