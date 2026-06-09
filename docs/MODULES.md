# 模块说明

## 目录总览

| 目录 | 职责 | 依赖 |
|------|------|------|
| `core/` | 核心模拟引擎 | config, boids, operator |
| `config/` | 配置定义、加载、校验 | vendor (json.hpp) |
| `boids/` | 集群算法、遗传基因 | core (World, Organism) |
| `operator/` | 工厂注册、捕食交互 | core, config |
| `ui/` | 启动设置界面 | config, World, ImGui |
| `imguMe/` | 增强版 UI (ecoim2) | ui, core, imgui, implot |
| `utils/` | 图片纹理加载 | stb_image, OpenGL |
| `data/` | 游戏数据结构与存档 | vendor |
| `vendor/` | 第三方头文件 (json) | — |
| `imgui/` | Dear ImGui 库 | OpenGL, Win32 |
| `implot/` | ImPlot 图表库 | ImGui |
| `assets/` | PNG 纹理资源 | — |
| `stb_image/` | 图片加载库 | — |
| `MAKE_IMAGE/` | 图像管理器 | ImGui, utils |

---

## core/ — 核心模拟引擎

### Registry.h — 全局枚举定义

```cpp
enum OrganismType { PLANT, ANIMAL, ENVIRONMENT };
enum EnvironmentType { WATER, FOREST, GRESSLEND, MOUTAN };
enum Weather { SUN, RAIN };
```

### Organism.h / OrganismDefine.cpp — 生物类体系

**文件职责**：定义所有生物类型的类结构、行为实现、繁殖逻辑、工厂函数。

**核心类**：

| 类 | 说明 |
|----|------|
| `Organism` | 所有生物的基类，能量、位置、活跃状态、Step() 虚函数 |
| `Reproducable` | 可繁殖生物，增加繁殖阈值/消耗/半径、食谱/天敌 |
| `Plant` | 植物，固定位置，从环境吸收能量，无性繁殖 |
| `Animal` | 动物，Boids 移动，捕食/交配，携带遗传基因 |

**关键结构体**：

| 结构体 | 字段 | 用途 |
|--------|------|------|
| `LeftEnergyRequest` | pos, energy | 死亡生物能量归还请求 |
| `ReproduceRequest` | type, name, pos, radius, new_genes | 子代创建请求（延迟处理） |

**核心函数**：

| 函数 | 说明 |
|------|------|
| `ReprodueNewOrganism(request)` | 工厂函数，根据 ReproduceRequest 创建对应类型的生物 |
| `isNaber(a, b)` | 判断两个生物是否相邻（8-邻域） |
| `PredationOrFuck(a, b)` | 捕食或繁殖判断的分发函数 |

**Plant 关键方法**：
- `Step()` — 代谢消耗 × 拥挤因子 `calculate_overlay_cost()`
- `Reproduce()` — 能量>=阈值时，随机位置产子，提交 ReproduceRequest

**Animal 关键方法**：
- `Step()` — 代谢 + Boids 移动（`FillNeighbors` → `ComputeFinalForce` → 更新速度和位置）
- `Reproduce(other)` — 双亲交配，`GA::Fusion` 融合基因，提交 ReproduceRequest
- `calculate_overlay_cost()` — 同 Plant，受拥挤压力影响

**全局变量**：
- `Plant_id`, `Animal_id` — 全局递增 ID 计数器

### Environment.h / EnvironmentDefine.cpp — 环境系统

**核心类**：

| 类 | 说明 |
|----|------|
| `Environment` | 基类，环境格元数据（位置、能量、容纳列表） |
| `GressLand` | 草地，晴天 +0.8 能量/帧 |
| `Water` | 水域，蒸发机制，能量转换 |

**Environment 关键方法**：
- `canPlant(request)` — 检查物种是否在 CanLiveIn 列表中，且未超过每格上限
- `EnergyExchange(organism)` — 植物吸收环境能量（吸收率 × 损失系数，不超过当前能量）
- `Update(weather)` — 处理尸体能量回收 + 自然增益

**GressLand::Update()**：
```cpp
case SUN:
    if (energy < max) energy += 0.8f;  // 每帧太阳能增益
```

### World.h / WorldDefine.cpp — 世界控制器

**单例类 `World`**：

| 成员 | 说明 |
|------|------|
| `Reproducas` | `vector<Reproducable*>` 所有活着的可繁殖生物 |
| `Environments` | `vector<Environment*>` 所有环境格（w×h 个） |
| `reproduce_requests` | `vector<ReproduceRequest>` 本帧繁殖请求 |
| `last_requests` | `vector<ReproduceRequest>` 上一帧请求（UI 读取） |
| `conf` | `Config&` 引擎参数引用 |
| `game_conf` | `TestConfig&` 应用层配置引用 |
| `CurrentWeather` | 当前天气（SUN/RAIN） |

**World 关键方法**：
- `Update()` — 主循环（环境更新 → 生物步进 → 交互检测 → 繁殖创建）
- `Reproduce()` — 保存 last_requests，遍历繁殖请求创建子代
- `AddReproduceRequest(request)` — 验证位置合法性后加入队列
- `RemoveDeadOrganisms()` — 删除 active==false 的生物，能量归环境
- `calculate_overlay(pos)` — 计算某位置的拥挤密度
- `Reset()` — 重置世界到初始状态
- `GetWidth()` / `GetHeight()` — 世界尺寸

**World 初始化**（构造函数）：
1. 注册所有动物/植物的 lambda 工厂到 `MyOperator`
2. 散布 7 个种子点，每个生成 8 株 Grass（共 56 株）
3. 散布 6 群 Sheep，每群 5 只（共 30 只）
4. 在中心附近生成 8 只 Wolf
5. 创建 w×h 个 GressLand 环境格，能量按高斯分布（中心高、边缘低）

### Animals.h / Plants.h — 工厂子类

| 类 | 父类 | 用途 |
|----|------|------|
| `UserAnimal` | `Animal` | 带 `FindAnimalConfig()` 静态方法，从 TestConfig 查找物种配置 |
| `UserPlant` | `Plant` | 带 `FindPlantConfig()` 静态方法，从 TestConfig 查找物种配置 |

---

## config/ — 配置系统

### Config.h — 引擎参数与结构体

**`Config` 类**：硬编码的引擎常量（单例）。

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `Environment_energy_absorb_rate` | 0.01 | 环境能量吸收基础比率 |
| `Environment_plant_absorb_rate` | 0.2 | 植物吸收环境能量的比率 |
| `Environment_step_max_absorb` | 1.5 | 每帧植物最大吸收量 |
| `Environment_single_chunk_max_energy` | 40 | 单格环境能量上限 |
| `Organism_animal_absorb_rate` | 0.30 | 动物捕食吸收比率 |
| `Organism_loss_rate` | 0.85 | 能量转化损失率 |
| `Organism_reproduce_energy_threshold` | 22 | 全局繁殖能量阈值 |
| `Organism_reproduce_energy_cost` | 10 | 全局繁殖能量消耗 |
| `Organism_step_energy_cost` | 0.2 | 每帧基础能量消耗 |
| `Organism_overlay_param` | 1.0 | 拥挤理想密度 |
| `Plant_init_radius` | 3 | 植物初始繁殖半径 |
| `Organism_interact_radius` | 2 | 生物交互检测半径 |
| `max_organisms_per_cell` | 4 | 每格最大同种生物数 |

**配置结构体**：

| 结构体 | 对应 JSON | 用途 |
|--------|-----------|------|
| `WordConfig` | `The_Word` | 世界尺寸 (length, width) |
| `EnvironmentConfig` | `The_Environments[]` | 环境名 + 可生存物种列表 |
| `AnimalConfig` | `The_Animals[]` | 动物所有参数（速度/能量/食谱/繁殖/基因） |
| `PlantConfig` | `The_Plants[]` | 植物参数（能量/消耗/繁殖阈值） |
| `TestConfig` | 根对象 | 聚合上述所有配置，使用 nlohmann/json 序列化 |

**配置字段默认值检查**：`Check_Animal` / `Check_Plant` 方法对 `-1` 的字段用 `Default_*_Config` 的值填充，实现"未设置则用默认"的语义。

### ConflgDefine.cpp — 配置实现

```cpp
void InitGameConfig(const TestConfig& cfg);   // 初始化全局配置指针
Config& Config::GetConfig();                  // 引擎参数单例
TestConfig& TestConfig::GetTestConfig();      // 应用配置（通过 g_GameConf）
```

### default_config.json — 默认物种配置

定义 6 种动物（Rabbit, Wolf, Fox, Sheep, Mouse, Bear）、5 种植物（Grass, Berry, Seed, Leaves, Tree）、2 种环境（GressLand, Water）。

---

## boids/ — Boids 集群算法

### Genes.h — 基因与粒子

```cpp
struct Genes {
    float cohesion = 1.0f;    // 聚集：向邻居中心移动
    float alignment = 0.5f;   // 对齐：匹配邻居速度方向
    float separation = 1.0f;  // 分离：远离附近邻居
    float vision = 6.0f;      // 视野半径（格子数）
    float escape = 1.0f;      // 逃避：远离天敌
    float food_addict = 1.0f; // 觅食：靠近猎物
};

struct Particle {
    float x, y, vx, vy, speed;
    int species;              // 0=Wolf, 1=非Wolf
    Genes genes;
    vector<int> diet;         // 编码后的食谱
    vector<int> predator;     // 编码后的天敌
};
```

### boids.cpp — Boids 算法实现

**核心函数**：

- `CreateParticle(a, gs)` — 从 Animal 创建 Particle（编码食谱/天敌为 int）
- `FillNeighbors(pos, vision, neighbors, world)` — 在 vision 半径内收集所有 Animal 粒子
- `ComputeFinalForce(self, neighbors)` — 计算合力：
  ```
  force = cohesion × (group_center - self)
        + alignment × (avg_velocity - self_v)
        + separation × Σ (repel_direction)
        + food_addict × (food_center - self)    // 向猎物方向
        + escape × Σ (flee_direction)           // 远离天敌
        + random_noise
  ```

**物种编码规则**：`name == "Wolf" ? 0 : 1` — Wolf 为 0，其他为 1。

### GA.h — 遗传算法

```cpp
namespace GA {
    Genes Fusion(const Genes& a, const Genes& b);
}
```

- 对每个基因位做**平均值融合** + **高斯噪声**（μ=0, σ=0.15）
- 子代基因 = (父+母)/2 + N(0, 0.15)

### test.cpp — Boids 独立测试

- 无需完整引擎，直接在 80×60 网格中模拟 Wolf 和 Sheep 的 Boids 集群
- 可调节 cohesion/alignment/separation/vision 参数，实时观察效果
- 环形世界边界（toroidal wrap）

---

## operator/ — 工厂与交互系统

### MyOperator — 三合一操作器

**职责 1：物种工厂注册**

```cpp
static void register_Animal_Create(name, creator_lambda);
static void register_Plant_Create(name, creator_lambda);
```

内部维护两套 `unordered_map<string, function>` 注册表。

**职责 2：捕食/繁殖交互**

```cpp
void operator()(Reproducable* a, Reproducable* b);
```

逻辑分支：
1. **单向捕食**（a 吃 b）：bite = min(hunger/loss, b.energy × absorb, b.energy)，有 3-35% 概率杀死猎物
2. **双向互食**：按能量比例分配伤害，双方都可进食
3. **互不吃**：能量高者胜，另一方直接死亡

**职责 3：生物创建**

```cpp
Reproducable* operator()(int x, int y, int r, string name, int id, optional<Genes>);
Reproducable* operator()(ReproduceRequest& request, int id);
```

查注册表，调用对应工厂 lambda。

### 静态注册器

```cpp
static PlantRegistrator Gress("Grass", [...] { ... });  // 程序启动时自动注册 Grass
```

利用静态对象的构造函数在 `main()` 之前完成注册。

---

## ui/ — 设置界面

### SetupUI.cpp

**文件职责**：模拟启动前的配置阶段。使用 ImGui 渲染一个多标签页的配置界面。

**标签页**：
1. **World** — 世界尺寸 (width, length)
2. **Environments** — 环境类型名称和可生存物种
3. **Animals** — 动物参数（速度/能量/食谱/繁殖等）
4. **Plants** — 植物参数
5. **CreatWorlds** — 世界存档管理（加载/创建 JSON 配置）

**配置加载路径**（按优先级）：
```cpp
"default_config.json"       // 同级目录
"../default_config.json"    // 上级目录
"../config/default_config.json"
"../../config/default_config.json"
```

**纹理加载**：点击 "Start Simulation" 时，为每个物种加载对应的 `assets/<Name>.png` 纹理。

---

## imguMe/ — 增强版 UI

### myui.cpp (ecosim2 入口)

与 `test_main.cpp` 的主要差异：
- **PNG 纹理渲染**：生物和环境使用图片而非色块
- **动态种群图表**：支持任意物种（而非硬编码三条线）
- **手动添加生物**：可在指定坐标添加生物（支持自定义基因）
- 移除了 Heatmap、独立 Gene Chart 等功能

---

## utils/ — 工具库

### img_utils.cpp — 纹理加载

```cpp
GLuint LoadTexture(filename, &width, &height);
```

- 使用 STB Image 加载 PNG → OpenGL GL_RGBA 纹理
- 参数：GL_NEAREST 过滤（像素风格）、GL_CLAMP 环绕

---

## data/ — 游戏数据结构

### game_struct.h

```cpp
struct gameData {
    vector<string> names;  // 世界存档文件名列表
};
```

### game_data.json

```json
{"names": []}  // 世界存档索引
```

用户创建的每个世界保存为一个独立的 JSON 文件在 `data/` 目录下。

---

## vendor/ — 第三方库

| 文件 | 来源 | 许可 |
|------|------|------|
| `json.hpp` | nlohmann/json v3.x | MIT |

`modules/Eigen/` — Eigen 线性代数库（已引入但未在核心逻辑中使用）。

---

## 入口点文件

| 文件 | 构建目标 | 说明 |
|------|----------|------|
| `test_main.cpp` | ecosim | 完整 GUI 模拟（纯色版），820 行 |
| `imguMe/myui.cpp` | ecosim2 | 增强版 GUI（纹理版），750 行 |
| `debug_main.cpp` | debug_sim | 无 GUI 批量模拟输出 CSV，113 行 |
| `boids/test.cpp` | boids_test | 独立 Boids 可视化测试，340 行 |
