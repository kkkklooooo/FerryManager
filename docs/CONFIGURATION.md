# 配置系统

## 双层配置架构

FerryManager 有两层配置：

| 层 | 类 | 位置 | 修改方式 |
|----|-----|------|---------|
| **引擎参数** | `Config` | `config/Config.h` 硬编码 | 修改源码 |
| **应用配置** | `TestConfig` | `config/default_config.json` | 编辑 JSON 或通过 UI |

## 引擎参数 (Config)

### 完整参数表

```cpp
class Config {
    int   width = 50;                              // 世界宽度（备用）
    float Environment_energy_absorb_rate = 0.01f;   // 环境吸收基础比率
    float Environment_plant_absorb_rate = 0.2f;     // 植物吸收环境比率
    float Environment_step_max_absorb = 1.5f;       // 每帧最大吸收量
    float Environment_single_chunk_max_energy = 40; // 单格能量上限
    float Organism_animal_absorb_rate = 0.30f;      // 动物捕食吸收率
    float Organism_loss_rate = 0.85f;               // 能量转化损失率
    float Organism_reproduce_energy_threshold = 22;  // 全局繁殖阈值
    float Organism_reproduce_energy_cost = 10;       // 全局繁殖消耗
    float Organism_step_energy_cost = 0.2f;          // 每帧基础消耗
    float Organism_overlay_param = 1.0f;             // 拥挤理想密度
    int   Plant_init_radius = 3;                     // 植物初始半径
    int   Organism_interact_radius = 2;              // 交互检测半径
    int   max_organisms_per_cell = 4;               // 每格最大同种
};
```

### 参数关系图

```
Environment_plant_absorb_rate  ──→  植物吸收量
Environment_step_max_absorb    ──→  吸收上限
Organism_loss_rate             ──→  能量转化效率 (所有转化)
Organism_animal_absorb_rate    ──→  动物捕食效率
Organism_step_energy_cost      ──→  基础代谢率
Organism_overlay_param         ──→  拥挤惩罚触发点
Organism_reproduce_energy_threshold → 繁殖条件
Organism_reproduce_energy_cost      → 繁殖代价
max_organisms_per_cell         ──→  空间承载上限
Organism_interact_radius       ──→  捕食/交配范围
```

## 应用配置 (TestConfig / JSON)

### JSON 结构

```json
{
  "The_Word": {
    "length": 50,      // 世界长度 (Y轴，格子数)
    "width": 50        // 世界宽度 (X轴，格子数)
  },
  "Default_Animal_Config": { ... },  // 动物默认值
  "Default_Plant_Config":  { ... },  // 植物默认值
  "The_Environments": [ ... ],       // 环境类型列表
  "The_Animals": [ ... ],            // 动物物种列表
  "The_Plants": [ ... ]              // 植物物种列表
}
```

### World 配置 (The_Word)

| 字段 | 类型 | 说明 | 最小限制 |
|------|------|------|----------|
| `length` | int | Y 轴格子数 | 50 (代码中有下限检查) |
| `width` | int | X 轴格子数 | 50 |

**注意**：更大世界 = 更多环境格 = 更多能量总量 → 可承载更大种群。

### AnimalConfig — 动物物种配置

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `name` | string | "Animal" | 物种名（需与 Environment.CanLive 匹配） |
| `diet` | string[] | [] | 食谱列表（可吃哪些物种） |
| `predator` | string[] | [] | 天敌列表（会被哪些物种吃） |
| `reproduce_original_rate` | int | 2 | 初始速度 |
| `reproduce_original_energy` | int | 18 | 初始能量 |
| `max_rate` | float | 3 | 最大速度上限 |
| `step_energy_cost` | float | 0.3 | 每帧能量消耗 |
| `energy_rate` | float | 0.2 | 能量→速度转化率 (speed = energy × rate) |
| `eat_intrval_max` | int | 10 | 进食冷却帧数 |
| `max_energy` | int | 50 | 饱食能量上限 |
| `reproduce_energy_threshold` | float | 50 | 繁殖能量阈值 |
| `reproduce_energy_cost` | float | 20 | 繁殖能量消耗 |

**默认值机制**：字段值为 `-1` 时，`Check_Animal()` 自动用 `Default_Animal_Config` 的值替代。

### PlantConfig — 植物物种配置

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `name` | string | "Plant" | 物种名 |
| `reproduce_original_energy` | int | 8 | 初始能量 |
| `step_energy_cost` | float | 0.2 | 每帧能量消耗 |
| `reproduce_energy_threshold` | float | 18 | 繁殖能量阈值 |
| `reproduce_energy_cost` | float | 10 | 繁殖能量消耗 |

### EnvironmentConfig — 环境类型

| 字段 | 类型 | 说明 |
|------|------|------|
| `name` | string | 环境名（代码中硬编码匹配 "GressLand", "Water"） |
| `CanLive` | string[] | 可在此环境生存和繁殖的物种名列表 |

**注意**：`CanLive` 为空表示任何物种都无法在此环境繁殖。

## 配置校验 (Check_*)

### Check_Animal

```cpp
if (reproduce_original_rate < 0)       → use Default_Animal_Config value
if (reproduce_original_energy < 0)     → use Default_Animal_Config value
if (step_energy_cost < 0)              → use Default_Animal_Config value
if (energy_rate < 0)                   → use Default_Animal_Config value
if (eat_intrval_max < 0)               → use Default_Animal_Config value
if (max_energy <= 0)                   → use Default_Animal_Config value
if (reproduce_energy_threshold < 0)    → use Default_Animal_Config value
if (reproduce_energy_cost < 0)         → use Default_Animal_Config value
```

### Check_Plant

```cpp
if (reproduce_original_energy < 0)     → use Default_Plant_Config value
if (step_energy_cost < 0)              → use Default_Plant_Config value
if (reproduce_energy_threshold < 0)    → use Default_Plant_Config value
if (reproduce_energy_cost < 0)         → use Default_Plant_Config value
```

### Check_Word

```cpp
if (length < 50) → length = 50
if (width < 50)  → width = 50
```

## 运行时配置加载

### 加载流程

```
程序启动
    │
    ▼
LoadConfigAny(s_GameConfig)
    │ 尝试多个路径
    │   "default_config.json"
    │   "../default_config.json"
    │   "../config/default_config.json"
    │   ...
    │
    ├── 成功 → 使用加载的配置
    └── 失败 → 使用 TestConfig() 空默认值 + WARNING
```

### UI 中的配置编辑

SetupUI 的标签页直接编辑 `s_GameConfig` 的成员：
- **World 标签**：width, length
- **Environments 标签**：环境名称和 CanLive 列表
- **Animals 标签**：所有动物参数
- **Plants 标签**：所有植物参数
- **CreatWorlds 标签**：保存/加载整个配置为 JSON 文件

点击 "Start Simulation" 后，`InitGameConfig(s_GameConfig)` 将最终配置固化到全局指针。

## 存档系统

### 文件结构

```
data/
├── game_data.json          # 存档索引: {"names": ["world1.json", "world2.json"]}
├── world1.json             # 具体世界配置 = TestConfig JSON
└── world2.json
```

### 创建新世界

在 SetupUI → CreatWorlds 标签中输入名称 → 保存：
1. `game_data.json` 的 `names` 数组中追加文件名
2. 在 `data/` 目录创建 `<name>.json`，内容为当前 `TestConfig` 的 JSON 序列化

### 加载世界

点击现存世界名称按钮 → 从 `data/<name>.json` 加载配置 → 初始化模拟。

## 配置设计约束

### 物种名一致性

物种名必须在以下位置保持一致：
1. `AnimalConfig.name` / `PlantConfig.name`
2. `EnvironmentConfig.CanLive[]` 中出现的名称
3. `assets/<Name>.png` 纹理文件（用于 ecosim2）
4. 代码中的硬编码匹配 (如 `name == "Wolf"` 的颜色判断)

### 食谱/天敌引用

`diet` 和 `predator` 列表中的名称必须是 `The_Animals` 或 `The_Plants` 中存在的物种名。

如果引用了不存在的物种，不会报错，只是该捕食关系永远不会被触发。

### 纹理文件命名

ecosim2 加载纹理时自动拼接：`"assets/" + name + ".png"`。

如物种名为 "Wolf"，则需要 `assets/Wolf.png`。
