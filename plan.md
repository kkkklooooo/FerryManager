# EcoSim 开发计划 & 学习路径

## 一、项目概况

当前状态：一个能跑起来的生态模拟器，有 ImGui 可视化界面，但所有配置都是 C++ 代码硬编码的。

目标：做成**上帝模式游戏**，玩家可以：
- 拖拽编辑食物链（谁吃谁）
- 编辑每个物种的参数（能耗、繁殖、速度、颜色）
- 编辑地形参数
- 新建存档 / 读档 / 继续游戏
- 配置通过 JSON 文件持久化

---

## 二、分步实现计划

### Phase 0：准备工作（新手热身）

| 步骤 | 内容 | 学习要点 |
|------|------|----------|
| 0.1 | 搞懂现有代码结构，每个文件干什么的 | C++ 项目结构、头文件/源文件分离 |
| 0.2 | 在 Visual Studio 中成功编译运行当前版本 | CMake 配置、MSVC 编译、调试器使用 |
| 0.3 | 熟悉 ImGui 基础：窗口、按钮、表格、Slider | ImGui 入门、立即模式 GUI 概念 |
| 0.4 | 理解当前 World::Update() 每一行在做什么 | 游戏主循环、帧的概念 |

**检查点：** 能在 VS 里跑起来，知道代码怎么流转的。

### Phase 1：引入 JSON + 配置重构

| 步骤 | 内容 | 学习要点 |
|------|------|----------|
| 1.1 | 下载 nlohmann/json 单头文件，放到项目中 | 单头文件库的集成方式 |
| 1.2 | 把 Config 从只有数值参数扩展为完整结构，包含：- 物种列表（名字、颜色、能耗、繁殖、食谱、栖息地）- 地形列表（名字、颜色、参数）| 数据结构设计、JSON Schema 思维 |
| 1.3 | 实现 Config 的 to_json / from_json 序列化 | JSON 序列化/反序列化、默认值处理 |
| 1.4 | 创建默认配置文件 `default_config.json`，启动时加载 | 文件 I/O、错误处理、资源路径管理 |
| 1.5 | 改造 Animal/Plant/Environment 构造函数：先从 Config 查表取值，查不到才用硬编码默认值 | 配置覆盖模式（override pattern） |

**检查点：** 启动时从 JSON 读配置，生物参数从配置文件获取，行为和之前一样。

### Phase 2：食物链 + 栖息地从配置读取

| 步骤 | 内容 | 学习要点 |
|------|------|----------|
| 2.1 | 删除 Wolf/Sheep 构造函数中 diet 的硬编码 | 消除魔法数字/硬编码 |
| 2.2 | 删除 GressLand::CanLiveIn 的硬编码 push_back | — |
| 2.3 | Animal::Reproduce() 中繁殖请求的 type/name 确认走配置 | — |
| 2.4 | 验证：修改 JSON 里的食物链，重启后捕食关系改变 | 配置驱动生效 |

**检查点：** 不改 C++ 代码，只改 JSON 就能改变"谁吃谁"和"谁住哪"。

### Phase 3：运行时编辑器 UI

| 步骤 | 内容 | 学习要点 |
|------|------|----------|
| 3.1 | 在 View 菜单新增 "Species Editor" 标签页开关 | ImGui 窗口管理 |
| 3.2 | 实现物种列表面板：左侧列出所有物种，点击选中 | ImGui::ListBox / Selectable |
| 3.3 | 实现参数编辑面板：右侧显示选中物种的参数（Slider/Input/Checkbox） | ImGui 各种输入控件 |
| 3.4 | 实现颜色编辑：每个物种显示一个颜色块，点击弹出颜色选择器 | ImGuiColorEditFlags |
| 3.5 | 修改参数时实时更新 Config 对象，所有活着的个体从 Config 读参数 | 引用/指针传参，运行时数据共享 |
| 3.6 | 新增/删除物种的功能 | std::vector 增删、动态注册 |

**检查点：** 运行时打开编辑器，修改狼的速度，狼立刻跑得快了。

### Phase 4：拖拽编辑食物链

| 步骤 | 内容 | 学习要点 |
|------|------|----------|
| 4.1 | 理解 ImGui 拖拽机制：SetDragDropPayload() + AcceptDragDropPayload() | ImGuiPayload、drag-drop 生命周期 |
| 4.2 | 实现食谱编辑 UI：左边 "捕食者" 列表，右边 "猎物" 列表，拖拽建立关系 | 跨窗口拖拽 |
| 4.3 | 实现栖息地编辑 UI：物种 ↔ 地形 的拖拽关联 | — |
| 4.4 | 可视化食物链：用 ImDrawList 画箭头/连线 | ImDrawList 基本绘图 |
| 4.5 | 编辑后实时生效，存档保存 | — |

**检查点：** 把羊拖到狼的食谱里，狼开始吃羊；把狼从草地拖到不可居住，狼在草地上死亡。

### Phase 5：存档系统

| 步骤 | 内容 | 学习要点 |
|------|------|----------|
| 5.1 | 设计存档 JSON 结构：包含配置快照 + 世界状态（地形列表、生物列表 + 位置/能量/ID） | 完整状态序列化 |
| 5.2 | 实现 World::Serialize() — 将整个世界导出为 JSON | 递归序列化、指针处理 |
| 5.3 | 实现 World::Deserialize() — 从 JSON 恢复整个世界 | 反序列化、对象重建 |
| 5.4 | UI 增加 "New Game" / "Save" / "Load" 按钮和文件对话框 | ImGui::Dialog、原生文件对话框 |
| 5.5 | 启动时检查是否有默认存档，提供继续/新建选项 | 用户引导流程 |

**检查点：** 玩了一会儿，存档，关掉程序，重新打开，读档，世界完全恢复。

### Phase 6：收尾与打磨

| 步骤 | 内容 |
|------|------|
| 6.1 | 各种异常处理：JSON 格式错误、字段缺失、版本兼容 |
| 6.2 | 性能优化：生物数量多时是否卡顿 |
| 6.3 | 代码清理：删除不再需要的硬编码和调试 printf |
| 6.4 | 更多地形类型（FOREST、MOUTAN）的配置和实现 |

---

## 三、学习路径（按顺序）

如果你是 C++ 新手，按照这个顺序学习，每个阶段学到刚好够用就行，不用全部精通再动手：

### 第一阶段：C++ 基础（约 1-2 周）

只需要学到能看懂现有代码的程度：

1. **变量、函数、类、继承、虚函数** — 现有代码大量使用
2. **指针和引用** — 理解 `Reproducable*`、`World::GetWorld()` 这种写法
3. **std::vector、std::pair** — 代码里大量使用
4. **范围 for 循环** — `for (auto* org : orgs)`
5. **Lambda 表达式** — RemoveDeadOrganisms 里用到了
6. **static 关键字** — 单例模式、全局 ID

不需要学的（暂时）：
- 模板高级用法
- 智能指针（unique_ptr/shared_ptr）
- 多线程
- C++ 20/23 新特性

**推荐资源：**
- learncpp.com（免费，按章节读前 10 章就够了）
- 对照着看现有代码，在 VS 里打断点一步步跟

### 第二阶段：ImGui 入门（3-5 天）

1. 理解"立即模式 GUI"的概念
2. 学习常用控件：Button、Text、Slider、InputInt、Checkbox、Combo
3. 学习窗口管理：Begin/End、SetNextWindowPos/Size
4. 学习表格：BeginTable/TableSetupColumn/TableNextRow
5. 学习拖拽：SetDragDropPayload/AcceptDragDropPayload

**推荐资源：**
- ImGui 官方 `imgui_demo.cpp`（最好的学习资源，编译出来就有 Demo 窗口）
- 直接在项目里打开 View > ImGui Demo 看

### 第三阶段：JSON 与文件 I/O（3-5 天）

1. nlohmann/json 的基本用法：`json j; j["key"] = value;`
2. 序列化自定义类型：`to_json` / `from_json`
3. 文件读写：`std::ifstream` / `std::ofstream`
4. 错误处理：try-catch、文件不存在的情况

### 第四阶段：综合

做 Phase 1-5 的过程中自然会学会：
- 数据驱动设计（用数据而不是代码定义行为）
- 游戏存档设计（快照 vs 增量）
- 调试技巧（VS 断点、内存窗口、调用堆栈）

---

## 四、关键技术决策记录

| 决策 | 选择 | 原因 |
|------|------|------|
| JSON 库 | nlohmann/json（单头文件） | 集成最简单，C++17 兼容 |
| 配置模式 | 配置覆盖（JSON 覆盖硬编码默认值） | 渐进式改造，每一步都能运行 |
| 存档设计 | 完整快照（配置 + 世界状态在一个文件里） | 简单可靠，不会出现配置和世界不同步 |
| UI 范式 | ImGui 标签页 + 浮动窗口 | 和现有代码风格一致 |
| 食物链编辑 | ImGuiPayload 拖拽 | 符合"拖拽编辑"的需求 |
| 运行时生效 | 修改后立即影响所有个体 | 玩家反馈即时，体验好 |

---

## 五、文件依赖关系图

```
Config.json
    │
    ▼
Config.h / Config.cpp  ←  ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
    │                                              │
    ├──▶ World (初始化时读取配置)                     │
    │       │                                       │
    │       ├──▶ Environment (从配置读 maxPlant 等)  │
    │       ├──▶ Plant (从配置读 energy_cost 等)     │
    │       └──▶ Animal → Wolf / Sheep (从配置读)   │
    │                                               │
    └──▶ UI::SpeciesEditor (运行时修改 Config)  ─ ─ ┘
              │
              ├──▶ 拖拽修改 diet / can_live_in
              ├──▶ Slider 修改数值参数
              └──▶ 颜色选择器修改颜色

存档系统：
World  ──serialize()──▶  save.json  (含完整配置快照 + 世界状态)
save.json  ──deserialize()──▶  World
```

---

## 六、给新手的建议

1. **一次只改一个东西。** 改了编译、运行、确认没问题，再改下一个。
2. **频繁提交 git。** 每完成一个小步骤就 commit，搞砸了可以回退。
3. **善用 VS 调试器。** `F9` 断点、`F10` 单步、`F11` 进入函数、`Shift+F5` 停止。看不懂代码怎么走的就打断点跟一遍。
4. **不用担心删错代码。** 有 git 历史，随时可以 `git checkout` 恢复。
5. **先跑通再优化。** 先让功能可用，再考虑代码整洁和性能。
6. **读代码比写代码更重要。** 先花时间读懂已有代码，再动手改。

---

## 七、Phase 1 的详细第一步

Phase 1 的第一步（1.1）具体操作：

1. 打开 https://github.com/nlohmann/json/releases
2. 下载 `json.hpp`（单头文件版本）
3. 放到 `E:\free\FerryManager\json.hpp`
4. 在 `Config.h` 中 `#include "json.hpp"`
5. 尝试写一个测试：创建一个 JSON 对象，序列化到字符串，再解析回来

**不需要改任何现有代码就能验证这一步。**

---

## 八、Boids 遗传算法 + 空间网格繁殖 实施计划

### 概述

**目标：**
1. 用 boids 算法替代动物随机移动，让同种羊/狼自然聚拢，解决低密度时繁殖靠运气的问题
2. boids 的三个权重 + 感知半径作为可突变基因，产生自然选择压力
3. 空间网格同时服务于 boids 邻居查询和繁殖/捕食交互，O(n) 复杂度

**当前问题：** 动物随机走位，同种相遇纯靠运气，低密度时几乎无法繁殖。

**解决思路：** Boids 的 cohesion 规则倾向聚群 → 同种必然靠近 → 繁殖自然发生。三个权重可遗传 → 进化选择。

---

### 文件改动清单

| 文件 | 改什么 |
|------|--------|
| `Organism.h` | 加 `BoidGenes` 结构体, `Animal` 加 `vx/vy`/`genes`/`BoidsMove()`; `ReproduceRequest` 加 `genes` |
| `OrganismDefine.cpp` | 实现 `BoidsMove()`, `Animal::Step()` 调用 boids, `Animal::Reproduce()` 加突变 |
| `Animals.h` | Wolf/Sheep 构造函数设初始基因默认值 |
| `MyOperatorDefine.cpp` | 工厂创建后注入基因 (`a->genes = x.genes`) |
| `WordDefine.cpp` | `Update()` 调整步骤顺序（植物solo繁殖拆分、动物交互循环） |
| `test_main.cpp` | Animal tooltip 显示基因值（可选） |

---

### Step 1: 数据结构 (`Organism.h`)

```cpp
// 新增基因结构体
struct BoidGenes {
    float cohesion   = 1.0f;   // 范围 0.2 ~ 3.0
    float alignment  = 0.5f;   // 范围 0.2 ~ 3.0
    float separation = 1.0f;   // 范围 0.2 ~ 3.0
    int   vision     = 3;      // 感知半径(格子), 范围 2 ~ 8
};

// Animal 类新增成员
class Animal : public Reproducable {
    // ... 现有成员保持 ...
    float vx = 0, vy = 0;   // 速度向量
    BoidGenes genes;         // 遗传性状

    void BoidsMove();        // 新增方法
};

// ReproduceRequest 加基因字段
struct ReproduceRequest {
    OrganismType type;
    std::string  name;
    std::pair<int,int> pos;
    int radius;
    BoidGenes genes;  // 新增
};
```

---

### Step 2: Boids 移动逻辑 (`OrganismDefine.cpp`)

`BoidsMove()` 复用 `Environment::Organisms` 空间网格查邻居：

```cpp
void Animal::BoidsMove() {
    World& w = World::GetWorld();
    int ww = w.GetWidth(), hh = w.GetHeight();
    int vr = genes.vision;

    float cx = 0, cy = 0;   // cohesion
    float ax = 0, ay = 0;   // alignment
    float sx = 0, sy = 0;   // separation
    int count = 0;

    int gx0 = std::max(0, Pos.first  - vr);
    int gy0 = std::max(0, Pos.second - vr);
    int gx1 = std::min(ww-1, Pos.first  + vr);
    int gy1 = std::min(hh-1, Pos.second + vr);

    for (int gy = gy0; gy <= gy1; gy++) {
        for (int gx = gx0; gx <= gx1; gx++) {
            auto* env = w.GetEnvironments()[gy * ww + gx];
            for (auto* other : env->Organisms) {
                if (other == this || other->name != name) continue;
                float dx = (float)(other->Pos.first  - Pos.first);
                float dy = (float)(other->Pos.second - Pos.second);
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist < 0.01f || dist > vr) continue;

                count++;
                cx += dx; cy += dy;                          // cohesion
                auto* a = static_cast<Animal*>(other);
                ax += a->vx; ay += a->vy;                    // alignment
                sx -= dx / (dist*dist); sy -= dy / (dist*dist); // separation
            }
        }
    }

    if (count > 0) {
        cx = (cx / count) * genes.cohesion;
        cy = (cy / count) * genes.cohesion;
        ax = (ax / count) * genes.alignment;
        ay = (ay / count) * genes.alignment;
        sx *= genes.separation;
        sy *= genes.separation;
    }

    // 噪声防止死锁
    float nx = ((float)rand()/RAND_MAX - 0.5f) * 0.4f;
    float ny = ((float)rand()/RAND_MAX - 0.5f) * 0.4f;

    vx = vx * 0.7f + cx + ax + sx + nx;  // 惯性衰减
    vy = vy * 0.7f + cy + ay + sy + ny;

    float len = std::sqrt(vx*vx + vy*vy);
    if (len > 0.001f) { vx = vx / len * rate; vy = vy / len * rate; }
    else              { vx = rate; vy = 0; }

    Pos.first  = std::clamp(Pos.first  + (int)std::round(vx), 0, ww-1);
    Pos.second = std::clamp(Pos.second + (int)std::round(vy), 0, hh-1);
}
```

`Animal::Step()` 改为：

```cpp
void Animal::Step() {
    float ori = step_energy_cost;
    step_energy_cost *= calculate_overlay_cost();
    Organism::Step();
    step_energy_cost = ori;

    if (eat_intrval-- <= 0)
        BoidsMove();  // 替代原来的随机移动
}
```

---

### Step 3: 初始基因值 (`Animals.h`)

```cpp
// Wolf: 独猎型 — 低 cohesion, 中 alignment, 中 separation, 大 vision
Wolf::Wolf(...) : Animal(...) {
    genes.cohesion   = 0.3f;
    genes.alignment  = 0.6f;
    genes.separation = 0.8f;
    genes.vision     = 5;
}

// Sheep: 聚群型 — 高 cohesion, 中 alignment, 高 separation, 中 vision
Sheep::Sheep(...) : Animal(...) {
    genes.cohesion   = 1.5f;
    genes.alignment  = 0.5f;
    genes.separation = 1.5f;
    genes.vision     = 3;
}
```

---

### Step 4: 基因突变 (`OrganismDefine.cpp`)

`Animal::Reproduce()` 中拷贝父基因并随机扰动：

```cpp
void Animal::Reproduce() {
    // ... 现有检查不变 ...

    BoidGenes childGenes = genes;
    childGenes.cohesion   = std::clamp(genes.cohesion   + rnd(-0.15f, 0.15f), 0.2f, 3.0f);
    childGenes.alignment  = std::clamp(genes.alignment  + rnd(-0.15f, 0.15f), 0.2f, 3.0f);
    childGenes.separation = std::clamp(genes.separation + rnd(-0.15f, 0.15f), 0.2f, 3.0f);
    childGenes.vision     = (int)std::clamp((float)genes.vision + rnd(-1.0f, 1.0f), 2.0f, 8.0f);

    ReproduceRequest req = {ANIMAL, name, {x_new, y_new}, r_int, childGenes};
    if (!World::GetWorld().AddReproduceRequest(req))
        energy -= reproduce_energy_cost;
}
```

---

### Step 5: 工厂注入基因 (`MyOperatorDefine.cpp`)

```cpp
Reproducable* MyOperator::operator()(ReproduceRequest& x, int id) {
    auto it = registry().find(x.name);
    if (it != registry().end()) {
        auto* a = it->second(id, x.pos.first, x.pos.second, x.radius);
        a->genes = x.genes;  // 注入突变后的基因
        return a;
    }
    return nullptr;
}
```

---

### Step 6: Update 调序 (`WordDefine.cpp`)

```cpp
void World::Update() {
    // 1. 清网格 + 清死生物
    for (auto& env : Environments) env->Organisms.clear();
    RemoveDeadOrganisms();

    // 2. 环境更新、能量交换、生物 Step(含 boids 移动)、填网格
    for (auto& org : Reproducas) {
        auto* env = Environments[org->Pos.second * GetWidth() + org->Pos.first];
        env->Organisms.push_back(org);
        env->EnergyExchange(org);
        org->Step();  // boids 移动在这里面
    }
    for (auto& env : Environments) env->Update(CurrentWeather);

    // 3. 繁殖 + 捕食
    for (int y = 0; y < GetHeight(); y++)
    for (int x = 0; x < GetWidth(); x++) {
        auto* env = Environments[y * GetWidth() + x];
        for (auto* a : env->Organisms) {
            // 植物自繁殖
            if (a->type == PLANT && a->reproduce_able)
                a->Reproduce();
            // 动物: 查 9 邻格
            for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                int ny = y + dy, nx = x + dx;
                if (ny < 0 || ny >= GetHeight() || nx < 0 || nx >= GetWidth()) continue;
                for (auto* b : Environments[ny * GetWidth() + nx]->Organisms) {
                    if (a < b) PredationOrFuck(a, b);
                }
            }
        }
    }

    // 4. 执行繁殖请求
    Reproduce();
}
```

---

### Step 7: UI 展示基因 (`test_main.cpp`)

Animal 列表的 tooltip 里显示基因值：

```cpp
if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    auto* a = static_cast<const Animal*>(org);
    ImGui::Text("%s #%d", OrganismDisplayName(org->name), a->id);
    ImGui::Text("Cohesion:   %.2f", a->genes.cohesion);
    ImGui::Text("Alignment:  %.2f", a->genes.alignment);
    ImGui::Text("Separation: %.2f", a->genes.separation);
    ImGui::Text("Vision:     %d",   a->genes.vision);
    ImGui::EndTooltip();
}
```

---

### 选择压力（自然形成）

| 性状 | 太低的问题 | 太高的问题 | 平衡点 |
|------|-----------|-----------|--------|
| cohesion | 找不到配偶, 灭绝 | 全挤一块吃光资源 | 中偏高 |
| alignment | 群体散开各自为战 | 全群冲进贫瘠区 | 中 |
| separation | 挤死(拥挤惩罚) | 繁殖邻居不够 | 中偏高 |
| vision | 找不到同伴 | 成本高(可选: 加能耗) | 中 |

---

### 实施顺序

1. `Organism.h` - 加 `BoidGenes`, `vx`/`vy`, `BoidsMove()` 声明, `ReproduceRequest` 加 genes
2. `OrganismDefine.cpp` - 实现 `BoidsMove()`, 改 `Step()`, `Reproduce()` 加突变
3. `Animals.h` - Wolf/Sheep 设初始基因
4. `MyOperatorDefine.cpp` - 工厂注入基因
5. `WordDefine.cpp` - `Update()` 调整顺序
6. `test_main.cpp` - 基因 UI
